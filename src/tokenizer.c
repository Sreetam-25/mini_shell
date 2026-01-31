#include "tokenizer.h"
int is_op_char(char c)
{
    return (c == '>' || c == '<' || c == '|');
}
int tokenize(char *line, char *tokens[], int max_args)
{
    int ntok = 0;
    char *p = line;
    while (*p != '\0')
    {
        // skip leading whitespaces
        while (isspace((unsigned char)*p))
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }
        if (ntok >= max_args - 1)
        {
            break;
        }
        // handle operators
        if (is_op_char(*p))
        {
            if (*p == '>' && *(p + 1) == '>') // Check for ">>"
            {
                // strndup uses malloc and copies the 2 ch and add '\0' to it
                tokens[ntok] = strndup(p, 2); // Copy ">>"
                p += 2;
            }
            else // Single char operators: <, >, |
            {
                tokens[ntok] = strndup(p, 1); // Copy single char
                p++;
            }
            ntok++;
            continue;
        }
        // handle words(quoataion included)
        char *start = p;  // we build the final argument here (in-place)
        int in_quote = 0; // 0 means we can terminate at white spaces 1 means skip
        char quote_char = '\0';
        int len = 0;
        while (*p)
        {
            if (!in_quote && (isspace((unsigned char)*p) || is_op_char(*p)))
            {
                break;
            }
            if (!in_quote && (*p == '"' || *p == '\''))
            {
                in_quote = 1;
                quote_char = *p;
            }
            if (in_quote && *p == quote_char)
            {
                in_quote = 0;
                quote_char = '\0';
            }
            p++;
            len++;
        }

        if (in_quote)
        {
            fprintf(stderr, "syntax error: missing closing quote\n");
            return -1;
        }
        tokens[ntok] = malloc(len + 1);
        char *src = start;
        char *dst = tokens[ntok];
        char q = '\0';
        for (int i = 0; i < len; i++)
        {
            char c = src[i];
            // skipping quotes
            if (q == '\0' && (c == '"' || c == '\'')) {
                q = c; // Enter quote
            }
            else if (q != '\0' && c == q) {
                q = '\0'; // Exit quote
            }
            else{
                *dst++ =c;
            }
        }
        *dst='\0';
        ntok++;
    }
    tokens[ntok] = NULL;
    return ntok;
}