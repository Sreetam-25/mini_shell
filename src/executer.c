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
        return 1; // Handled
    }
    return 0; // Not a builtin
}
void execute_command(Command *cmd)
{
    if (cmd->args[0] == NULL)
        return;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        // --- CHILD PROCESS ---

        // 1. Handle Input Redirection
        if (cmd->redir_in)
        {
            int fd = open(cmd->redir_in, O_RDONLY);
            if (fd < 0)
            {
                perror("Error opening input file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // 2. Handle Output Redirection
        if (cmd->redir_out)
        {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->append_mode)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;

            int fd = open(cmd->redir_out, flags, 0644);
            if (fd < 0)
            {
                perror("Error opening output file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // 3. Execute
        execvp(cmd->args[0], cmd->args);
        perror("execvp"); // Error if we return
        exit(1);
    }
    else
    {
        // --- PARENT PROCESS ---
        waitpid(pid, NULL, 0);
    }
}