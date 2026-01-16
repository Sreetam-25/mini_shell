#include <stdio.h>
#include<stdlib.h>
int main(void) {
    char* line  = NULL;
    size_t len = 0;
    ssize_t nread;
    while(1){
        printf("$");
        fflush(stdout);
        nread = getline(&line,&len,stdin);
        // EOF
        if(nread==-1){
            printf("/n");
            break;
        }
    }
    free(line);
    return 0;
}
