#ifndef EXECUTOR_H
#define EXECUTOR_H
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include "shell.h"
// handle commands like cd,exit 
// works only in parent
int handle_builtin(Command *cmd);
void execute_pipeline(Pipeline *p);
void build_cmd_string(Pipeline *p, char *buffer);
#endif