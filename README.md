````md
# Mini Shell (msh)

A minimal Unix-like shell written in **C** for **Linux/WSL**.  
This project is built to understand how real shells work internally using system calls like `fork()`, `execvp()`, `pipe()`, and `dup2()`.

---

## Features Implemented

### External Command Execution
- Executes commands using:
  - `fork()`
  - `execvp()`
  - `waitpid()`

Examples:
```sh
ls
pwd
cat file.txt
````

### Built-in Commands

* `cd` (change directory)
* `exit` (terminate the shell)

### Tokenization

* Converts raw user input into tokens
* Correctly separates operators like:

  * `<`
  * `>`
  * `>>`
  * `|`

### Input/Output Redirection

Supported:

* Input redirection: `<`
* Output redirection (overwrite): `>`
* Output redirection (append): `>>`

Examples:

```sh
cat < input.txt
echo hello > out.txt
echo again >> out.txt
```

### Pipelining

Supports piping between multiple commands using `|`

Examples:

```sh
ls | wc -l
cat file.txt | grep main | wc -l
seq 1 50 | tail -n 5
```

---

##  Planned Features

* Background execution using `&`
* Job control (`jobs`, `fg`, `bg`)
* Better signal handling (`Ctrl+C`, `Ctrl+Z`)
* Environment variables / expansions

---

## Project Structure

```txt
MINI_SHELL/
  src/
    main.c
    shell.h

    tokenizer.c
    tokenizer.h

    parser.c
    parser.h

    executor.c
    executor.h

  docs/
  .gitignore
  README.md
```

### File Responsibilities

* **main.c**
  Shell loop: prompt → read input → tokenize → parse → execute.

* **tokenizer.c / tokenizer.h**
  Splits input into tokens (commands, args, redirection symbols, pipe operators).

* **parser.c / parser.h**
  Builds command structure from tokens (including pipe segments and redirection info).

* **executor.c / executor.h**
  Executes commands, manages:

  * piping via `pipe()`
  * redirection via `dup2()`
  * child process creation using `fork()`

* **shell.h**
  Shared definitions (structs, constants, function prototypes).

---

## ⚙️ Build & Run

### Build

From the project root:

```sh
gcc -Wall -Wextra -Werror -g src/main.c src/tokenizer.c src/parser.c src/executor.c -o msh
```

### Run

```sh
./msh
```

---

## Quick Test Commands

### Redirection

```sh
echo hi > out.txt
cat < out.txt
echo again >> out.txt
```

### Pipes

```sh
echo hello | wc -c
seq 1 10 | tail -n 3
cat file.txt | sort | uniq
```

---

## Notes

This shell is a learning-focused project to understand:

* process creation & execution
* file descriptor manipulation
* pipeline chaining
* redirection handling

---
