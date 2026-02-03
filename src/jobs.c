#include "shell.h"
#include <stdio.h>
#include <string.h>

Job jobs[MAX_JOBS];

void init_jobs()
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        jobs[i].id = 0;
    }
}
int add_job(pid_t pgid, JobState state, char *cmd_line)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].id == 0)
        {
            jobs[i].id = i + 1;
            jobs[i].pgid = pgid;
            jobs[i].state = state;
            strncpy(jobs[i].cmd_line, cmd_line, 1023);
            return jobs[i].id;
        }
    }
    fprintf(stderr, "Error: Too many jobs\n");
    return -1;
}
int delete_job(pid_t pgid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pgid == pgid)
        {
            jobs[i].id = 0;
            return 0;
        }
    }
    return -1;
}
Job *find_job(pid_t pgid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].id != 0 && jobs[i].pgid == pgid)
            return &jobs[i];
    }
    return NULL;
}
Job *find_job_by_id(int id)
{
    if (id < 1 || id > MAX_JOBS)
        return NULL;
    if (jobs[id - 1].id != 0)
        return &jobs[id - 1];
    return NULL;
}

void print_jobs()
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].id != 0)
        {
            printf("[%d] %d %s %s\n",
                   jobs[i].id,
                   jobs[i].pgid,
                   (jobs[i].state == JOB_STOPPED) ? "Stopped" : "Running",
                   jobs[i].cmd_line);
        }
    }
}
