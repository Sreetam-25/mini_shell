#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include"shell.h"
// parse the pipeline and build commands using pasre_single_segment
int parse_input(char*tokens[],int n_tokens,Pipeline *p);
#endif