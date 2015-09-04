#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "common.h"

static sigset_t    prepsigs(void);
static void        reap(char *argv[]);
static pid_t       spawn(char *argv[]);
extern void        init(char *argv[]);

static const char *path;


int main(int argc, char *argv[])
{
    path = strdup(argv[0]);
    const char *name = getenv("PROCNAME");
    if (name && *name)
        setprocname(name, argv[0]);

    if (getpid() == 1) {
        /* parent process */
        reap(argv);
    } else {
        /* child process */
        prepenv();
        init(argv + 1);
    }
    return 1;
}

static sigset_t prepsigs(void)
{
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGHUP);
    sigaddset(&sigs, SIGCHLD);

    struct sigaction sa;
    sa.sa_handler = exit;
    sa.sa_mask = sigs;
    sa.sa_flags = 0;

    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGHUP,  &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    return sigs;
}

static void reap(char *argv[])
{
    sigset_t sigs = prepsigs();

    pid_t child = 0;
    for (;;) {
        if (!child)
            child = spawn(argv);

        siginfo_t info;
        switch (sigwaitinfo(&sigs, &info)) {
        case SIGINT:
            /* shutdown request */
            exit(0);
            break;
        case SIGHUP:
            /* reboot request: just terminate everything including the child, to be respawned */
            kill(-1, SIGKILL);
            break;
        case SIGCHLD:
            /* child died */
            waitpid(info.si_pid, NULL, 0);
            if (info.si_pid == child)
                child = 0;
            break;
        }
    }
}

static pid_t spawn(char *argv[])
{
    pid_t pid = fork();
    if (!pid) {
        sigset_t sigs;
        sigfillset(&sigs);
        sigprocmask(SIG_UNBLOCK, &sigs, 0);
        execv(path, argv);
    }

    return pid;
}
