#include "executor.h"
void build_cmd_string(Pipeline *p, char *buffer)
{
    buffer[0] = '\0';
    for (int i = 0; i < p->cmd_counts; i++)
    {
        // copy the command to the buffer
        strcat(buffer, p->commands[i].args[0]);
        // if multiple commands then add |
        if (i < p->cmd_counts - 1)
            strcat(buffer, " | ");
    }
    if (p->background)
        strcat(buffer, " &");
}

int handle_builtin(Command *cmd)
{
    if (cmd->args[0] == NULL)
        return 0; // Empty command
    // jobs is a builtin cmd
    if (strcmp(cmd->args[0], "jobs") == 0)
    {
        print_jobs();
        return 1;
    }

    // eg:- fg%1
    if (strcmp(cmd->args[0], "fg") == 0)
    {
        Job *j = NULL;

        if (cmd->args[1] == NULL)
        {
            // Case 1: User typed just "fg"
            // Find the most recent job (Stopped OR Running)
            // We iterate backwards from MAX_JOBS to 1 to find the highest ID
            for (int i = MAX_JOBS; i > 0; i--)
            {
                j = find_job_by_id(i);
                if (j)
                    break; // Found the most recent job
            }
            if (!j)
            {
                fprintf(stderr, "fg: no current job\n");
                return 1;
            }
        }
        else
        {
            // Case 2: User typed "fg %2" or "fg 2"
            char *arg = cmd->args[1];
            if (arg[0] == '%')
                arg++; // Skip '%'

            int job_id = atoi(arg);
            j = find_job_by_id(job_id);

            if (!j)
            {
                fprintf(stderr, "fg: %s: no such job\n", cmd->args[1]);
                return 1;
            }
        }
        // Standard shells print the command name when you bring it to FG
        printf("%s\n", j->cmd_line);

        // --- GIVE TERMINAL ---
        tcsetpgrp(STDIN_FILENO, j->pgid);

        // ---  WAKE UP ---
        // Even if it's already running, SIGCONT is safe.
        if (kill(-j->pgid, SIGCONT) < 0)
            perror("kill (SIGCONT)");

        j->state = JOB_RUNNING;

        // --- WAIT ---
        int status;
        waitpid(-j->pgid, &status, WUNTRACED);

        // ---  HANDLE RESULT ---
        if (WIFSTOPPED(status))
        {
            // User hit Ctrl+Z again
            j->state = JOB_STOPPED;
            printf("\n[%d]+ Stopped %s\n", j->id, j->cmd_line);
        }
        else
        {
            // Job finished (Exited or Killed)
            delete_job(j->pgid);
        }

        // --- TAKE TERMINAL BACK ---
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpid());
        signal(SIGTTOU, SIG_DFL);

        return 1;
    }
    if (strcmp(cmd->args[0], "bg") == 0)
    {
        Job *j = NULL;
        if (cmd->args[1] == NULL)
        {
            // Case 1: User typed just "bg"
            // Default behavior: Find the most recent STOPPED job
            // We iterate backwards from MAX_JOBS to 1
            for (int i = MAX_JOBS; i > 0; i--)
            {
                j = find_job_by_id(i);
                if (j && j->state == JOB_STOPPED)
                    break;
            }
            if (!j)
            {
                fprintf(stderr, "bg: no current job\n");
                return 1;
            }
        }
        else
        {
            // Case 2: User typed "bg %2" or "bg 2"
            char *arg = cmd->args[1];

            // Skip the '%' if user typed it
            if (arg[0] == '%')
                arg++;

            int job_id = atoi(arg); // Convert string "2" to int 2
            j = find_job_by_id(job_id);

            if (!j)
            {
                fprintf(stderr, "bg: %s: no such job\n", cmd->args[1]);
                return 1;
            }
        }

        // Logic to Unfreeze the Job
        if (j->state == JOB_STOPPED)
        {
            kill(-j->pgid, SIGCONT);
            j->state = JOB_RUNNING;
            printf("[%d]+ %s &\n", j->id, j->cmd_line);
        }
        else
        {
            fprintf(stderr, "bg: job %d is already running\n", j->id);
        }
        return 1;
    }

    // handle cmd like exit and cd

    if (strcmp(cmd->args[0], "exit") == 0)
    {
        return -1; // Signal to main to break loop
    }
    if (strcmp(cmd->args[0], "cd") == 0)
    {
        char *path = cmd->args[1] ? cmd->args[1] : getenv("HOME");
        if (path == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
        }
        else if (chdir(path) != 0)
        {
            perror("cd");
        }
        return 1;
    }
    return 0; // Not a builtin
}

void execute_pipeline(Pipeline *p)
{
    int prev_fd = STDIN_FILENO;
    int fd[2];
    pid_t group_id = 0;
    char cmd_string[1024];
    build_cmd_string(p, cmd_string);
    for (int i = 0; i < p->cmd_counts; i++)
    {
        Command *cmd = &p->commands[i];
        int is_last = (i == p->cmd_counts - 1);
        if (!is_last)
        {
            if (pipe(fd) == -1)
            {
                perror("pipe");
                exit(1);
            }
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            // JOB CONTROL
            // grp will have first command's pid as grp id
            if (group_id == 0)
                group_id = getpid();
            // For each child set group_id
            // 0 means current process
            setpgid(0, group_id);

            if (!p->background)
                tcsetpgrp(STDIN_FILENO, group_id);

            // Restore signals
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);

            if (cmd->redir_in)
            {
                int f = open(cmd->redir_in, O_RDONLY);
                if (f < 0)
                {
                    perror("input");
                    exit(1);
                }
                dup2(f, STDIN_FILENO);
                close(f);
                // redirin has more priority than prev_fd
                if (prev_fd != STDIN_FILENO)
                    close(prev_fd);
            }
            else if (prev_fd != STDIN_FILENO)
            {
                // read from pipe buffer
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (cmd->redir_out)
            {
                int flags = O_WRONLY | O_CREAT | (cmd->append_mode ? O_APPEND : O_TRUNC);
                int f = open(cmd->redir_out, flags, 0644);
                if (f < 0)
                {
                    perror("output");
                    exit(1);
                }
                dup2(f, STDOUT_FILENO);
                close(f);
                if (!is_last)
                {
                    close(fd[1]);
                    close(fd[0]);
                }
            }
            else if (!is_last)
            {
                // write into the pipe buffer
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
            }
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
            exit(1);
        }
        // PARENT
        if (group_id == 0)
            group_id = pid;
        setpgid(pid, group_id);

        if (prev_fd != STDIN_FILENO)
            close(prev_fd);
        if (!is_last)
        {
            prev_fd = fd[0];
            close(fd[1]);
        }
    }
    // ---PARENT WAIT LOGIC---
    if (!p->background)
    {
        // Foreground
        tcsetpgrp(STDIN_FILENO, group_id);

        int status;
        waitpid(-group_id, &status, WUNTRACED); // Wait for ANY in group

        if (WIFSTOPPED(status))
        {
            // User pressed Ctrl+Z
            printf("\n"); // Newline after ^Z
            int jid = add_job(group_id, JOB_STOPPED, cmd_string);
            printf("[%d]+ Stopped %s\n", jid, cmd_string);
        }

        // Take Control Back
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpid());
        signal(SIGTTOU, SIG_DFL);
    }
    else
    {
        // Background
        int jid = add_job(group_id, JOB_RUNNING, cmd_string);
        printf("[%d] %d\n", jid, group_id);
    }
}