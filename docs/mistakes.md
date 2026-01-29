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
