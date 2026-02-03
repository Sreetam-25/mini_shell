#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include "tokenizer.h"
#include "parser.h"
#include "executor.h"

// --- Async Signal Handler ---
void sigchld_handler(int sig)
{
    (void)sig;
    int status;
    pid_t pid;

    // WNOHANG: Don't wait if nothing is dead. Just check.
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            // A background job finished. Remove it.
            delete_job(pid);
        }
        else if (WIFSTOPPED(status))
        {
            Job *j = find_job(pid);
            if (j)
                j->state = JOB_STOPPED;
        }
        else if (WIFCONTINUED(status))
        {
            Job *j = find_job(pid);
            if (j)
                j->state = JOB_RUNNING;
        }
    }
}

int main(void)
{
    signal(SIGINT, SIG_IGN);
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    // outside of while loop
    init_jobs(); // Initialize array

    while (1)
    {
        // 1. Setup Signals
        signal(SIGINT, SIG_IGN);  // Ignore Ctrl+C (only send to child)
        signal(SIGTSTP, SIG_IGN); // Ignore Ctrl+Z
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, sigchld_handler); // Catch dead children

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
        char *tokens[64];
        int n_tokens = tokenize(line, tokens, 64);
        if (n_tokens <= 0)
            continue;
        Pipeline p;
        if (parse_input(tokens, n_tokens, &p) == 0)
        {
            if (p.cmd_counts > 0)
            {
                if (p.cmd_counts == 1 && handle_builtin(&p.commands[0]))
                {
                    if (strcmp(p.commands[0].args[0], "exit") == 0)
                    {
                        for (int i = 0; i < n_tokens; i++)
                            free(tokens[i]);
                        break;
                    }
                }
                else
                {
                    execute_pipeline(&p);
                }
            }
        }

        for (int i = 0; i < n_tokens; i++)
            free(tokens[i]);
    }
    free(line);
    return 0;
}