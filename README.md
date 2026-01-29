# Mini Shell

A minimal Unix-like shell written in C (WSL/Linux).  
Supports running external commands using `fork()` + `execvp()` and basic built-ins.

---

## Features
- Execute external commands (e.g., `ls`, `pwd`, `cat`)
- Built-in commands:
  - `cd`
  - `exit`
- Tokenization of input
- (Planned) Redirection `<`, `>`, `>>`
- (Planned) Pipelining `|`
- (Planned) Background execution `&`

---

## Folder Structure
```txt
src/
  main.c
