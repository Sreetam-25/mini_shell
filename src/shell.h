#ifndef SHELL_H
#define SHELL_H

// The shared Data Structure
typedef struct {
    char *args[64];
    char *redir_in;
    char *redir_out;
    int append_mode;
} Command;

int tokenize(char *line, char *argv[], int max_args);
int parser(char *tokens[], int n_tokens, Command *cmd);
void execute_command(Command *cmd);
int handle_builtin(Command *cmd);

#endif