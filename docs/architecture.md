````md
# Architecture — Mini Shell (msh)

This document describes the internal architecture of **msh**, a minimal Unix-like shell written in **C** for Linux/WSL.  
The goal of the project is to understand how real shells work using process creation, file descriptor manipulation, pipelines, redirections, and basic job control.

---

## 1. High-Level Overview

`msh` follows a classic shell pipeline:

1. Read a full input line
2. Tokenize it into **words + operators**
3. Parse tokens into a structured pipeline representation
4. Execute the pipeline:
   - run built-ins in the parent process
   - run external commands using `fork()` + `execvp()`
   - connect pipelines using `pipe()` + `dup2()`
   - apply redirections using `open()` + `dup2()`
5. Track jobs for background execution and basic job control (`jobs`, `fg`, `bg`)

---

## 2. Core Components

### 2.1 Input Loop (`main.c`)
**Responsibility:** control flow and orchestration.

Main loop responsibilities:
- configure and install signal handlers
- print prompt and read input (`getline()`)
- call tokenizer → parser → executor
- reap background processes periodically (via `SIGCHLD` + `waitpid(WNOHANG)`)

---

### 2.2 Tokenizer (`tokenizer.c/.h`)
**Responsibility:** convert raw input into a clean token stream.

Tokenizer guarantees:
- whitespace-separated words become individual tokens
- operators are always separated into individual tokens even without spaces

Supported operator tokens:
- `<` (input redirection)
- `>` (output redirection, truncate)
- `>>` (output redirection, append)
- `|` (pipeline)
- `&` (background execution)

Examples handled correctly:
```sh
cat<in.txt
ls|wc -l
echo hi>>out.txt
sleep 5&
````

Output of tokenization:

```txt
["cat", "<", "in.txt"]
["ls", "|", "wc", "-l"]
["echo", "hi", ">>", "out.txt"]
["sleep", "5", "&"]
```

---

### 2.3 Parser (`parser.c/.h`)

**Responsibility:** transform tokens into structured commands.

The parser builds a `Pipeline` object containing one or more `Command` objects.

#### Data Model (Conceptual)

* A **Command** represents a single executable with arguments and optional redirections.
* A **Pipeline** is an ordered list of commands optionally executed in background.

Typical fields stored:

* `argv[]` (argument vector, NULL-terminated)
* `redir_in` (input file, optional)
* `redir_out` (output file, optional)
* `append_mode` (for `>>`)
* `background` (set when `&` is present)

Example:

```sh
cat < in.txt | grep hello | wc -l > out.txt &
```

Parsed structure:

* `Pipeline.background = 1`
* `Pipeline.commands[0]` has `redir_in = "in.txt"`
* `Pipeline.commands[2]` has `redir_out = "out.txt"`

---

## 3. Execution Layer

### 3.1 Built-in Commands (`executor.c`)

**Responsibility:** handle commands that must run in the parent process.

Built-ins supported:

* `cd`
* `exit`
* `jobs`
* `fg`
* `bg`

Why built-ins run in the parent:

* `cd` must modify the shell’s working directory
* `exit` must terminate the shell process
* job-control commands (`jobs`, `fg`, `bg`) manage shell-owned state

---

### 3.2 External Command Execution

External commands are executed by:

1. `fork()`
2. child process sets up redirection/pipes
3. child calls `execvp()`

Parent controls:

* process group creation
* terminal foreground control for FG jobs
* job tracking for BG jobs
* wait behavior

---

### 3.3 Pipeline Execution

For `N` commands:

* `N - 1` pipes are created
* `N` children are forked

For each pipeline stage:

* stdin is connected from:

  * input redirection `<` **or**
  * previous pipe read end
* stdout is connected to:

  * output redirection `>` / `>>` **or**
  * next pipe write end

Implementation uses:

* `pipe()`
* `dup2()`
* `close()`

Correct fd cleanup is required to avoid:

* leaked file descriptors
* hanging pipelines (when pipe ends remain open)

---

## 4. Job Control Architecture

`msh` implements basic job control using **process groups** and a lightweight job table.

### 4.1 Process Groups

Each pipeline runs as **one process group**:

* the first child becomes the group leader
* all pipeline children join the same group via `setpgid()`

This enables:

* sending signals to the entire pipeline (`kill(-pgid, SIGCONT)`)
* foreground terminal control per job (`tcsetpgrp()`)

---

### 4.2 Job Table (`jobs.c/.h`)

Jobs are tracked in a fixed-size array (e.g., `Job jobs[MAX_JOBS]`).

Each job stores:

* job id (`%1`, `%2`, …)
* process group id (`pgid`)
* job state (`Running` / `Stopped`)
* command string (for user display)

Provided operations:

* `init_jobs()`
* `add_job(pgid, state, cmdline)`
* `delete_job(pgid)`
* `find_job(pgid)`
* `find_job_by_id(id)`
* `print_jobs()`

---

### 4.3 Foreground / Background Behavior

#### Foreground execution

* shell gives terminal control to the job process group:

  * `tcsetpgrp(STDIN_FILENO, pgid)`
* shell waits for job state change using `waitpid(-pgid, ..., WUNTRACED)`
* if stopped:

  * job is added/marked as `Stopped`
* if finished:

  * job is removed from job table
* terminal control is returned to the shell process group

#### Background execution

* shell does not block waiting
* job is added as `Running`
* prompt returns immediately

---

### 4.4 `fg` / `bg` Built-ins

#### `fg`

* selects a job by id (`fg %1`) or most recent
* transfers terminal control to the job group
* resumes the group (`SIGCONT`)
* waits until the job exits or stops again
* returns terminal control back to the shell

#### `bg`

* selects a stopped job by id or most recent stopped job
* resumes it (`SIGCONT`)
* updates state to `Running`
* does not wait

---

## 5. Signals and Reaping

### 5.1 Shell Signal Policy

The interactive shell typically ignores:

* `SIGINT` (Ctrl+C)
* `SIGTSTP` (Ctrl+Z)
* `SIGTTIN`
* `SIGTTOU`

Child processes restore defaults so they behave normally.

---

### 5.2 Reaping Background Jobs

To avoid zombie processes, `msh` handles `SIGCHLD` and reaps terminated children using:

* `waitpid(-1, WNOHANG | WUNTRACED | WCONTINUED)`

The shell updates job state when jobs:

* stop
* continue
* exit

---

## 6. Known Limitations (Current Scope)

This implementation is intentionally minimal and does not aim to be a full POSIX shell.

Not implemented:

* quoting / escaping rules (`"..."`, `'...'`, `\`)
* variable expansion (`$HOME`, `$PATH`)
* globbing (`*`)
* logical operators (`&&`, `||`)
* command separators (`;`)
* subshells / parentheses
* command history and line editing

Some job-control edge cases (pipeline-wide lifecycle correctness) may require additional refinements for full robustness.

---

## 7. Summary

`msh` is structured as a clean pipeline of components:

**Input → Tokenizer → Parser → Executor → Job Manager**

This design keeps concerns separated and makes it easy to extend the shell with new features such as better parsing, more built-ins, and more robust job control.

---

```
