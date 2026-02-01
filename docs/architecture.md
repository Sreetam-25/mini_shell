
## Main Idea

A loop-based Unix shell that:

1. Reads a full input line
2. Tokenizes it into **words + operators** (`<`, `>`, `>>`, `|`)
3. Parses tokens into one or more commands (pipeline segments)
4. Executes:

   * built-ins in the parent (when applicable)
   * external commands using `fork()` + `execvp()`
   * pipelines using `pipe()` + `dup2()`

---

## Execution Flow

### 1) Read Input

* Read a full line from stdin (`getline()` or equivalent)
* If empty input → just show prompt again

---

### 2) Tokenize Input (Updated Logic)

Tokenization is **NOT only whitespace-based anymore**.

Token boundaries include:

* **Whitespace**
* **Operators**

  * `<`
  * `>`
  * `>>`
  * `|`

 This fixes the bug where operators got “eaten” when no spaces were present.

Example that must work:

```sh
cat>out.txt
ls|wc -l
echo hi>>out.txt
```

---

### 3) Parse Tokens

Convert token stream into structured commands:

* Split commands by `|`
* For each command:

  * collect `argv[]`
  * detect and store redirections:

    * `redir_in`
    * `redir_out`
    * `append_mode`

Example:

```sh
cat < in.txt | grep hello | wc -l > out.txt
```

Becomes 3 commands connected by pipes, with redirection metadata.

---

### 4) Execute

####  Built-ins (`cd`, `exit`)

Run in the **parent process** (no fork), because they modify shell state.

* `cd` must change the shell’s working directory (child changing directory is useless)
* `exit` must terminate the shell itself

---

#### External Commands (Single Command)

If there is **no pipe**, then:

* `fork()`
* child:

  * apply redirections using `open()` + `dup2()`
  * `execvp(argv[0], argv)`
* parent:

  * `waitpid()`

---

#### Pipelines (Multiple Commands)

If pipeline exists (`N` commands):

* create `N-1` pipes
* fork **N children**
* for each child:

  * connect stdin/stdout using `dup2()` depending on position in pipeline
  * apply redirections if present
  * close all pipe fds
  * `execvp()`
* parent closes all pipe fds and waits for all children

---

## Important Notes

* `argv` **must be NULL-terminated** for `execvp()`
* `cd` **must not run in child process**
* Tokenization handles operators even without spaces
   `cat>out.txt` works correctly now
* Redirections are implemented using:

  * `open()`
  * `dup2()`
  * `close()`
* Pipes are implemented using:

  * `pipe()`
  * `dup2()`
  * `close()` (critical to avoid hanging)
* Quotes handling:

  * quoted strings should be treated as a single token
  * quotes should terminate only when a valid matching closing quote is found

