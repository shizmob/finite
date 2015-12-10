#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "config.h"

static sigset_t    prepsigs(void);
static void        prepenv(void);
static void        reap(void);
static void        init(void);


int main(int argc, char *argv[])
{
    init();
    if (getpid() == 1)
        reap();

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

static void prepenv(void)
{
    umask(DEFAULT_UMASK);
    chdir("/");

    const char *con = getenv("console");
    if (!con)
        con = "/dev/console";

    setenv("CONSOLE", con,            0);
    setenv("HOME",    "/",            0);
    setenv("PATH",    DEFAULT_PATH,   0);
    setenv("PWD",     "/",            0);
    setenv("TERM",    DEFAULT_TERM,   0);
    setenv("USER",    DEFAULT_USER,   0);
}

static void reap(void)
{
    sigset_t sigs = prepsigs();

    for (;;) {
        siginfo_t info;
        switch (sigwaitinfo(&sigs, &info)) {
        case SIGINT:
            /* shutdown request */
            exit(0);
            break;
        case SIGHUP:
            /* reboot request: just terminate everything. */
            kill(-1, SIGKILL);
            break;
        case SIGCHLD:
            /* child died */
            waitpid(info.si_pid, NULL, 0);
            break;
        }
    }
}

static void init(void)
{
    char *cmd = getenv("rc");
    if (!cmd)
        cmd = RC_COMMAND;

    pid_t pid = fork();
    if (!pid) {
        sigset_t sigs;
        sigfillset(&sigs);
        sigprocmask(SIG_UNBLOCK, &sigs, 0);

        prepenv();
        execv(cmd, (char *[]){ cmd, NULL, });
        fprintf(stderr, "init: exec %s: ", cmd);
        perror(NULL);
    }
}
