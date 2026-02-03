````md
# Mini Shell (msh)

A minimal Unix-like shell written in **C** for **Linux / WSL**.  
Built to understand how real shells work internally using system calls like `fork()`, `execvp()`, `pipe()`, `dup2()`, `waitpid()`, and basic **job control** with process groups.

---

## Features

### 1) External Command Execution
Runs normal Linux commands using:
- `fork()`
- `execvp()`
- `waitpid()`

Examples:
```sh
ls
pwd
cat file.txt
````

---

### 2) Built-in Commands

Supported built-ins:

* `cd` — change directory
* `exit` — quit the shell
* `jobs` — list background/stopped jobs
* `fg` — bring a job to foreground
* `bg` — resume a stopped job in background

Examples:

```sh
cd ..
jobs
fg %1
bg %1
exit
```

---

### 3) Tokenization

Converts raw input into tokens and correctly recognizes operators:

* `<` input redirection
* `>` output redirection (overwrite)
* `>>` output redirection (append)
* `|` pipe
* `&` background execution

Examples:

```sh
cat<in.txt
echo hello>out.txt
echo again>>out.txt
cat file.txt|grep main|wc -l
sleep 5&
```

---

### 4) Input / Output Redirection

Supported:

* `<` input redirection
* `>` output redirection (truncate/overwrite)
* `>>` output redirection (append)

Examples:

```sh
cat < input.txt
echo hello > out.txt
echo again >> out.txt
```

---

### 5) Pipelining

Supports multi-stage pipelines using `|`.

Examples:

```sh
ls | wc -l
cat file.txt | grep main | wc -l
seq 1 50 | tail -n 5
```

---

### 6) Background Execution (`&`)

Commands can be run in the background using `&`.

Examples:

```sh
sleep 10 &
cat file.txt | grep hello &
```

---

### 7) Job Control (Basic)

Implemented job control features:

* process groups (`setpgid`)
* foreground terminal control (`tcsetpgrp`)
* job tracking with a jobs table (`jobs.c`)
* job commands: `jobs`, `fg`, `bg`
* resume jobs using `SIGCONT`

Example workflow:

```sh
sleep 100
# press Ctrl+Z
jobs
bg %1
fg %1
```

---

## Project Structure

```txt
mini_shell/
  src/
    main.c
    shell.h

    tokenizer.c
    tokenizer.h

    parser.c
    parser.h

    executor.c
    executor.h

    jobs.c

  docs/
  .gitignore
  README.md
```

---

## ⚙️ Build & Run

### Build

From the project root:

```sh
gcc -Wall -Wextra -Werror -g \
  src/main.c src/tokenizer.c src/parser.c src/executor.c src/jobs.c \
  -o msh
```

### Run

```sh
./msh
```

---

## 🧪 Quick Tests

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

### Background + Jobs

```sh
sleep 5 &
jobs
fg %1
bg %1
```

---

## Notes

This project is meant for learning and understanding:

* process creation and execution
* file descriptor manipulation using `dup2()`
* pipes and multi-process pipelines
* redirection behavior (`<`, `>`, `>>`)
* job control basics (process groups + `jobs/fg/bg`)

---

```

## Known Limitations / Notes

This project is meant for learning purposes and intentionally keeps the implementation minimal.
Because of that, there are some limitations compared to a full POSIX-compliant shell:

- **Foreground pipeline waiting is simplified**
  - The shell currently uses `waitpid(-pgid, ...)` to wait for a job.
  - In complex multi-process pipelines, a complete implementation should wait until **all**
    processes in the process group have exited or the job is stopped.

- **Job cleanup depends on correct reaping logic**
  - Background processes are reaped using `SIGCHLD` + `waitpid(WNOHANG, ...)`.
  - A fully robust shell must carefully map child `pid -> pgid` and only delete a job
    after the entire process group has finished.

- **Terminal foreground control is basic**
  - The shell uses process groups + `tcsetpgrp()` to give terminal control to foreground jobs.
  - Race conditions can exist in minimal implementations if terminal control is not handled strictly by the parent.

- **Command string stored for jobs is simplified**
  - Job command display is a simplified representation of the pipeline.
  - A real shell stores the full raw command line (including arguments and redirections) exactly as typed.

- **No advanced parsing**
  - Not implemented: quotes (`" "` / `' '`), escaping (`\`), parentheses/subshells, `&&`, `||`, `;`,
    wildcard expansion (`*`), variable expansion (`$PATH`, `$HOME`), command substitution, etc.

- **No interactive features**
  - Not implemented: command history, arrow-key navigation, autocomplete, prompt customization.

- **Limited error handling**
  - The goal is correctness for common cases.
  - A production-grade shell would include more complete validation and error messages.

```
