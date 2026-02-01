# Mistakes & Lessons

## 1) Bug: Incorrect tokenization of quoted strings

### Symptom
Input like:
"hi"there
was incorrectly split into two arguments:
["hi", "there"]
But it should be treated as a single argument:
["hithere"]

## 2) ROOT CAUSE

My tokenizer was ending the token immediately when it encountered a closing quote (").
So it treated the quote as a hard boundary and started a new token after it, which is incorrect.

## 3) Fix

Introduced an in_quote boolean flag.
Rule used:
When in_quote == 1, whitespace does not terminate the token.
Only terminate a token when whitespace occurs outside quotes.
Quote characters toggle in_quote and are not included in the final token.

## 4) Lesson Learned

Token boundaries must be decided by context, not by individual characters.
Correct rule:
Split on whitespace only when not inside quotes.

## Changed Tokenization Logic (Operators Getting Eaten)

### **Problem**

Initially, tokenization was implemented by splitting the input string **in-place**:

* Breaking tokens only at **whitespaces**
* Inserting `'\0'` to terminate tokens directly inside the original input buffer

This approach failed when operators like `>`, `<`, `|` were used **without spaces**.

✅ Example:

```sh
cat>out.txt
```

### **Root Cause**

To support operators, I modified the break condition to split at:

* **whitespace**
* **operators** (`>`, `<`, `|`, etc.)

However, the parsing logic still relied on manipulating the same buffer and pointers.

What went wrong:

* When the loop encountered `>`, it treated it as a token boundary
* The pointer was advanced and the output pointer `out` was set to point at `>`
* Later, at the end of the loop, the code inserted `'\0'` at `out`
* Since `out` was pointing at the operator, the operator token got overwritten / terminated early

✅ Result: the operator was effectively **lost ("eaten")**, and tokenization became incorrect.

---

### **Fix**

Instead of modifying the original input string in-place, I changed the tokenizer to **extract tokens by copying** them.

New approach:

1. Compute the **exact length** of the next token first
2. Allocate a new buffer of required size using `malloc`
3. Copy characters from the input into the new token buffer
4. Store the allocated token in the token list

✅ This completely avoids pointer-side effects and ensures operators remain intact even when no spaces exist.

---

