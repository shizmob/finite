#define  _XOPEN_SOURCE     700
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static void  reap(char *argv[]);
static pid_t spawn(char *argv[]);

extern void  init(char *argv[]);


int main(int argc, char *argv[])
{
    if (getpid() == 1)
        /* parent process */
        reap(argv);
    else
        /* child process */
        init(argv + 1);
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
