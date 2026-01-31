#include "parser.h"
int parser(char *tokens[], int n_tokens, Command *cmd)
{
    cmd->args[0] = NULL;
    cmd->redir_in = NULL;
    cmd->redir_out = NULL;
    cmd->append_mode = 0;
    int arg_id = 0;
    for (int i = 0; i < n_tokens; i++)
    {
        if (strcmp(tokens[i], "<") == 0)
        {
            if (i + 1 >= n_tokens)
            {
                fprintf(stderr, "Syntax error: missing file for <\n");
                return -1;
            }
            cmd->redir_in = tokens[++i];
        }
        else if (strcmp(tokens[i], ">") == 0)
        {
            if (i + 1 >= n_tokens)
            {
                fprintf(stderr, "Syntax error: missing file for <\n");
                return -1;
            }
            cmd->redir_out = tokens[++i];
        }
        else if (strcmp(tokens[i], ">>") == 0)
        {
            if (i + 1 >= n_tokens)
            {
                fprintf(stderr, "Syntax error: missing file for <\n");
                return -1;
            }
            cmd->redir_out = tokens[++i];
            cmd->append_mode = 1;
        }
        else
        {
            if (arg_id < 63)
            {
                cmd->args[arg_id++] = tokens[i];
            }
        }
    }
    cmd->args[arg_id] = NULL; // Null-terminate for execvp
    return 0;
}