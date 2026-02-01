#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include "tokenizer.h"
#include "parser.h"
#include "executer.h"
int main(void)
{
    signal(SIGINT, SIG_IGN);
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
        char *tokens[64];
        int n_tokens = tokenize(line, tokens, 64);

        if (n_tokens <= 0)
            continue;
        Pipeline p;
        if (parse_input(tokens, n_tokens, &p) == 0)
        {
            if (p.cmd_counts > 0)
            {
                // 3. Check for Builtin (Only valid for single commands)
                // Note: "cd" inside a pipe (cd | ls) runs in a child, so we skip this check
                if (p.cmd_counts == 1 && handle_builtin(&p.commands[0]))
                {
                    // Builtin handled (cd, exit)
                    // If exit returned -1, we break
                    if (strcmp(p.commands[0].args[0], "exit") == 0)
                    {
                        for (int i = 0; i < n_tokens; i++)
                            free(tokens[i]);
                        break;
                    }
                }
                else
                {
                    // 4. Execute (Handles Single Commands AND Pipelines)
                    execute_pipeline(&p);
                }
            }
        }

        // 5. Cleanup
        for (int i = 0; i < n_tokens; i++)
            free(tokens[i]);
    }
    free(line);
    return 0;
}