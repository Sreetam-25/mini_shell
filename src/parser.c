#include "parser.h"
static int parse_single_segment(char *tokens[], int st, int end, Command *cmd)
{
    cmd->args[0] = NULL;
    cmd->redir_in = NULL;
    cmd->redir_out = NULL;
    cmd->append_mode = 0;
    int arg_id = 0;
    for (int i = st; i < end; i++)
    {
        if (strcmp(tokens[i], "<") == 0)
        {
            if (i + 1 >= end)
            {
                fprintf(stderr, "Syntax error: missing file for <\n");
                return -1;
            }
            cmd->redir_in = tokens[++i];
        }
        else if (strcmp(tokens[i], ">") == 0)
        {
            if (i + 1 >= end)
            {
                fprintf(stderr, "Syntax error: missing file for <\n");
                return -1;
            }
            cmd->redir_out = tokens[++i];
        }
        else if (strcmp(tokens[i], ">>") == 0)
        {
            if (i + 1 >= end)
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
    cmd->args[arg_id] = NULL; // Null terminate for execvp
    return 0;
}
int parse_input(char *tokens[], int n_tokens, Pipeline *p)
{
    p->cmd_counts = 0;
    p->background = 0;
    if (n_tokens > 0 && strcmp(tokens[n_tokens - 1], "&") == 0)
    {
        p->background = 1;
        n_tokens--; //  Reduce count so the loop ignores the "&" token
                    // We don't want to pass "&" to execvp!
    }

    int st_id = 0;
    // build single command
    for (int i = 0; i < n_tokens; i++)
    {
        if (strcmp(tokens[i], "|") == 0 || i == n_tokens - 1)
        {
            int end_idx = (strcmp(tokens[i], "|") == 0) ? i : i + 1;
            if (p->cmd_counts < MAX_PIPE_SEGMENTS)
            {
                Command *current = &p->commands[p->cmd_counts];
                if (parse_single_segment(tokens, st_id, end_idx, current) == -1)
                    return -1;
                p->cmd_counts++;
            }
            st_id = i + 1;
        }
    }
    return 0;
}