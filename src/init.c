#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "common.h"

static void  reap(char *argv[]);
static pid_t spawn(char *argv[]);
extern void  init(char *argv[]);


int main(int argc, char *argv[])
{
    const char *name = getenv("PROCNAME");
    if (name && *name)
        setprocname(name, argv[0]);

    if (getpid() == 1) {
        /* parent process */
        reap(argv);
    } else {
        /* child process */
        prepare_env();
        init(argv + 1);
    }
    return 1;
}

static void reap(char *argv[])
{
    pid_t child = 0;
    for (;;) {
        if (!child)
            child = spawn(argv);

        pid_t died = wait(NULL);
        if (died > 0 && died == child)
            child = 0;
    }
}

static pid_t spawn(char *argv[])
{
    /* block signals so child process can't wreak havoc for parent */
    sigset_t sigs;
    sigfillset(&sigs);
    sigprocmask(SIG_BLOCK, &sigs, 0);

    pid_t pid = fork();
    if (!pid) {
        sigprocmask(SIG_UNBLOCK, &sigs, 0);
        execv(argv[0], argv);
    }

    return pid;
}
