#pragma  once
#include <sys/types.h>

#define  SYSV_ID_LENGTH     4

struct init_task
{
    char  id[SYSV_ID_LENGTH + 1];
    int   runlevels;
    int   action;
    char  cmd[4096];
    pid_t pid;
    int   flags;
};

int parse_tab(const char *file, struct init_task *tasks, unsigned ntasks);
