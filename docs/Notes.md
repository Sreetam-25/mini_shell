````md
# Background & Foreground Processes + Signal Handling (Mini Shell Notes)

## Background Processes (`&`)

### ✅ Expected Behavior
When a command is started in the **background**, the shell **must not block** waiting for it to finish.

Example:
```sh
sleep 5 &
# shell should immediately return to prompt
````

### ✅ Why Reaping is Required

Even after a background process finishes execution, it does **not disappear immediately**.

Instead, it becomes a **zombie process** until the shell collects its exit status using `wait()` / `waitpid()`.

So the shell must **periodically reap finished background children**.

---

## ✅ Reaping Background Processes (Non-blocking)

### Approach

At the start of every shell loop iteration, run a non-blocking reap:

* Use `waitpid(-1, &status, WNOHANG)`
* Put it inside a **while loop**

### Why a `while` Loop?

Because:

* Background processes can exit **at any time**
* Multiple background processes may exit before the next prompt
* `waitpid()` returns **one child at a time**, so we must keep reaping until none are left

### Typical Pattern

```c
while (waitpid(-1, &status, WNOHANG) > 0) {
    // reap all finished background children
}
```

---

## Foreground Processes (Normal Execution)

### ✅ Expected Behavior

For a foreground process, the shell must:

* **wait until it finishes**
* return to prompt only after completion

### ⚠️ Why `wait(NULL)` is Incorrect

Using `wait(NULL)` in a shell is risky because it waits for **any child**, not the one you just launched.

So if a background process finishes first, `wait(NULL)` might reap that background process instead, leaving the foreground process still running.

That breaks the behavior of foreground execution.

### ✅ Correct Fix

Use `waitpid(fg_pid, &status, 0)` to wait for the **specific foreground PID**:

```c
waitpid(fg_pid, &status, 0);
```

---

# Signal Handling

## Ctrl+C (`SIGINT`) Handling in a Shell

### ✅ Required Behavior

* The shell itself should **not die** when the user presses `Ctrl+C`
* But foreground commands **should terminate normally** when interrupted

So the shell must:

1. **Ignore SIGINT in the parent (shell)**
2. **Restore default SIGINT handling in the child**

### Parent (Shell)

```c
signal(SIGINT, SIG_IGN);
```

### Child (Before `execvp`)

```c
signal(SIGINT, SIG_DFL);
```

---

## What Happens After `execvp()`?

### Key Idea

`execvp()` replaces the current process image (code + data), but **does NOT change**:

✅ PID
✅ file descriptor table (unless modified)
✅ signal dispositions (unless explicitly reset)

So if the child restores `SIGINT` handling before `execvp()`, the executed program will also receive `SIGINT` normally.

This is why:

* Shell ignores Ctrl+C → shell stays alive
* Child restores default → command is interruptible
* After exec, the new program still has the correct signal behavior

---

## Summary

### Background (`&`)

* Do not block on background jobs
* Reap completed children using:

  * `waitpid(-1, &status, WNOHANG)` in a loop

### Foreground

* Wait only for the launched process:

  * `waitpid(fg_pid, &status, 0)`
* Avoid `wait(NULL)` because it may reap a background process first

### Signals

* Shell ignores `SIGINT`
* Child restores default `SIGINT` before `execvp()`
* `execvp()` keeps PID, FDs, and signal behavior intact

```
```
