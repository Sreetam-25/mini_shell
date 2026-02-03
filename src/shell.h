#ifndef SHELL_H
#define SHELL_H
#include <sys/types.h> 
#include <unistd.h>
#define MAX_PIPE_SEGMENTS 10
#define MAX_JOBS 20

// Job states
typedef enum
{
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;
// Job
typedef struct
{
    int id;
    pid_t pgid;
    JobState state;
    char cmd_line[1024];
} Job;
// The shared Data Structure
typedef struct
{
    char *args[64];
    char *redir_in;
    char *redir_out;
    int append_mode;
} Command;
// Pipeline structure
typedef struct
{
    // array of commands
    Command commands[MAX_PIPE_SEGMENTS];
    int cmd_counts;
    int background;
} Pipeline;
extern Job jobs[MAX_JOBS];
void init_jobs();
int add_job(pid_t pgid, JobState state, char *cmd_line);
int delete_job(pid_t pgid);
Job *find_job(pid_t pgid);
Job *find_job_by_id(int id);
void print_jobs();

int tokenize(char *line, char *argv[], int max_args);
int parse_input(char *tokens[], int n_tokens, Pipeline *p);
void execute_pipeline(Pipeline *p);
int handle_builtin(Command *cmd);

#endif