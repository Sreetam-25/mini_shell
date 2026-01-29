#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
int tokenize(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;
    while (*p != '\0')
    {
        // skip leading whitespaces
        while (isspace((unsigned char)*p))
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }
        if (argc >= max_args - 1)
        {
            break;
        }
        argv[argc++] = p;
        char *out = p;    // we build the final argument here (in-place)
        int in_quote = 0; // 0 means we can terminate at white spaces 1 means skip
        char quote_char = '\0';
        while (*p)
        {
            if (!in_quote && isspace((unsigned char)*p))
            {
                break;
            }
            if (!in_quote && (*p == '"' || *p == '\''))
            {
                in_quote = 1;
                quote_char = *p;
                p++; // skip opening quote
                continue;
            }
            if (in_quote && *p == quote_char)
            {
                in_quote = 0;
                quote_char = '\0';
                p++; // skip closing quote
                continue;
            }
            *out++ = *p++;
        }

        if (in_quote)
        {
            fprintf(stderr, "syntax error: missing closing quote\n");
            argv[0] = NULL;
            return -1;
        }
        if (*p != '\0')
        {
            p++;
        }
        *out = '\0';

        // move p past whitespace (if any)
        while (isspace((unsigned char)*p))
            p++;
    }
    argv[argc] = NULL;
    return argc;
}
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1)
    {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0)
        {
        }
        printf("$ ");
        fflush(stdout);

        nread = getline(&line, &len, stdin);

        // EOF (Ctrl+D)
        if (nread == -1)
        {
            printf("\n");
            break;
        }
        line[strcspn(line, "\r\n")] = '\0';
        int bg = 0;
        char *argv[64];
        int argc = tokenize(line, argv, 64);
        if (argc <= 0)
            continue;

        if (strcmp(argv[0], "cd") == 0)
        {
            char *path = argv[1] ? argv[1] : getenv("HOME");

            if (path == NULL)
            {
                fprintf(stderr, "cd: HOME not set\n");
            }
            else if (chdir(path) != 0)
            {
                perror("cd");
            }
            continue;
        }
        if (strcmp(argv[0], "exit") == 0)
        {
            break;
        }
        if (strcmp(argv[argc - 1], "&") == 0)
        {
            argv[argc - 1] = NULL;
            bg = 1;
        }
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            continue;
        }
        // checking bg process
        if (pid == 0)
        {
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        }
        else if (!bg)
        {
            waitpid(pid, NULL, 0);
        }
    }

    free(line);
    return 0;
}
