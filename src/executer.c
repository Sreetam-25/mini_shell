#include "executer.h"
int handle_builtin(Command *cmd)
{
    if (cmd->args[0] == NULL)
        return 0; // Empty command

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
        // --PARENT--
        if (prev_fd != STDIN_FILENO)
            close(prev_fd);
        if (!is_last)
        {
            prev_fd = fd[0];
            close(fd[1]);
        }
    }
    while (wait(NULL) > 0)
        ;
}