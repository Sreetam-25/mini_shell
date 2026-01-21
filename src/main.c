#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        printf("$ ");
        fflush(stdout);

        nread = getline(&line, &len, stdin);

        // EOF (Ctrl+D)
        if (nread == -1) {
            printf("\n");
            break;
        }
            // trim trailing whitespace
        while (nread > 0 &&
            (line[nread - 1] == '\n' || line[nread - 1] == ' ' || line[nread - 1] == '\t')) {
            line[nread - 1] = '\0';
            nread--;
        }

        // skip leading whitespace
        char *cmd = line;
        while (*cmd == ' ' || *cmd == '\t') {
            cmd++;
        }

        // sf → ls
        if (strcmp(cmd, "sf") == 0) {
            pid_t pid = fork();

            if (pid == 0) {
                char *argv[] = { "ls", NULL };
                execv("/bin/ls", argv);
                perror("execv");
                exit(1);
            } else {
                wait(NULL);
            }
        }
    }

    free(line);
    return 0;
}
