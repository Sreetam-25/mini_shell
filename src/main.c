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

        if (n_tokens < 0)
            continue; // Syntax error in tokenizer
        if (n_tokens == 0)
            continue; // Empty line
        // 2. Parse (Organize tokens into Command struct)
        Command cmd;
        if (parser(tokens, n_tokens, &cmd) ==0)
        {
            // 3. Check Builtins
            int status = handle_builtin(&cmd);

            if (status == -1)
            { // User typed "exit"
                for (int i = 0; i < n_tokens; i++)
                    free(tokens[i]);
                break;
            }

            // 4. Execute External Command (if not builtin)
            if (status == 0)
            {
                execute_command(&cmd);
            }
        }

        // 5. Cleanup (Free tokens)
        for (int i = 0; i < n_tokens; i++)
        {
            free(tokens[i]);
        }
    }

    free(line);
    return 0;
}