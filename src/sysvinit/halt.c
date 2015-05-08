#define  _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "init.h"

#define  FIFO_TIMEOUT 5

static struct command {
    const char *program;
    int runlevel;
} commands[] = {
    { "poweroff", RUNLEVEL_SHUTDOWN },
    { "reboot",   RUNLEVEL_REBOOT   },
    { 0 }
};

static void timeout(int sig);
static int  timedout;


int main(int argc, char *argv[])
{
    const char *name = strrchr(argv[0], '/');
    if (name)
        name++;
    else
        name = argv[0];

    const struct command *cmd;
    for (cmd = commands; cmd->program; cmd++)
        if (!strcmp(name, cmd->program))
            break;
    if (!cmd->program) {
        fprintf(stderr, "halt: unknown mode: %s\n", name);
        return 1;
    }


    /* enter runlevel */
    signal(SIGALRM, timeout);
    alarm(FIFO_TIMEOUT);
    int fd = open(SYSV_FIFO, O_WRONLY);
    alarm(0);
    if (fd < 0) {
        if (timedout)
            fprintf(stderr, "%s: can't open %s: timed out (%d seconds)\n", cmd->program, SYSV_FIFO, FIFO_TIMEOUT);
        else
            fprintf(stderr, "%s: can't open %s: %s\n", cmd->program, SYSV_FIFO, strerror(errno));
        goto err;
    }

    struct sysv_message msg = { 0 };
    msg.magic     = SYSV_MESSAGE_MAGIC;
    msg.cmd       = SYSV_MESSAGE_RUNLEVEL;
    msg.runlevel  = cmd->runlevel;
    msg.sleeptime = SYSV_DEFAULT_SLEEP;
    if (write(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        fprintf(stderr, "%s: can't write to %s\n", cmd->program, SYSV_FIFO);
        goto err;
    }

    return 0;
err:
    if (fd >= 0)
        close(fd);
    return 1;
}

static void timeout(int signal)
{
    timedout = 1;
}
