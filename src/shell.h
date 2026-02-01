#ifndef SHELL_H
#define SHELL_H
#define MAX_PIPE_SEGMENTS 10
// The shared Data Structure
typedef struct
{
    char *args[64];
    char *redir_in;
    char *redir_out;
    int append_mode;
} Command;
// Pipeline structure
typedef struct
{
    // array of commands
    Command commands[MAX_PIPE_SEGMENTS];
    int cmd_counts;
} Pipeline;

int tokenize(char *line, char *argv[], int max_args);
int parse_input(char*tokens[],int n_tokens,Pipeline *p);
void execute_pipeline(Pipeline *p);
int handle_builtin(Command *cmd);

#endif