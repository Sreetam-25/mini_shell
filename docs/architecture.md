# Mini Shell Architecture

## Main Idea
A loop-based shell:
1) Read input
2) Tokenize input
3) Execute built-in OR external command

## Execution Flow
- If command is built-in (cd/exit): run in parent process
- Else:
  - fork()
  - child: execvp()
  - parent: waitpid()

## Important Notes
- argv must be NULL terminated for execvp
- cd must NOT run in child process
- Tokenization is done on the basis of whitespaces
- If whitespaces appear then null terminate the argv
- Quotes can be terminate if ending quote is there
