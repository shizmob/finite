#define  _XOPEN_SOURCE
#define  _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>

#include "../common.h"
#include "init.h"
#include "inittab.h"

static void   prepare(void);
static void   prepare_system(void);
static int    determine_runlevel(char *argv[]);
static int    enter_runlevel(int runlevel, int sleeptime);
static int    run_task(struct init_task *task, int wait);
static int    kill_tasks(int newlevel, int sleeptime);
static void   reap_task(int sig);
static void   handle_communication(void);
static int    reopen_fifo(int fd);

static int    runlevel;
static struct init_task tasks[256];
static int    ntasks;
static int    reappipe[2];


void init(char *argv[])
{
    int level;
    prepare();
    prepare_env();

    ntasks = parse_tab(INITTAB_FILE, tasks, sizeof(tasks) / sizeof(*tasks));
    prepare_system();

    level = determine_runlevel(argv);
    enter_runlevel(level, SYSV_DEFAULT_SLEEP);

    handle_communication();
}

/* set up reap pipe and signal handlers */
static void prepare(void)
{
    pipe2(reappipe, O_CLOEXEC);

    sigset_t sigs;
    sigfillset(&sigs);

    struct sigaction act;
    act.sa_handler = reap_task;
    act.sa_mask    = sigs;
    act.sa_flags   = 0;
    sigaction(SIGCHLD, &act, NULL);
}

/* prepare system by running the important commands */
static void prepare_system(void)
{
    /* run preparation commands */
    for (unsigned i = 0; i < ntasks; i++)
        if (tasks[i].action == ACTION_SYSINIT)
            run_task(&tasks[i], 0);
    for (unsigned i = 0; i < ntasks; i++)
        if (tasks[i].action == ACTION_BOOT || tasks[i].action == ACTION_BOOTWAIT)
            run_task(&tasks[i], 0);
}

/* determine runlevel to start in */
static int determine_runlevel(char *argv[])
{
    for (unsigned i = 0; i < ntasks; i++)
        if (tasks[i].action == ACTION_INITDEFAULT)
            return tasks[i].runlevels;
    for (; *argv; argv++)
        if (!strcmp(*argv, "single") || !strcmp(*argv, "emergency"))
            return RUNLEVEL_SINGLE;
        else if (*argv[0] && !*argv[1] && parse_runlevels(*argv))
            return parse_runlevels(*argv);
    return RUNLEVEL_MULTI;
}

/* enter runlevel */
static int enter_runlevel(int newlevel, int sleeptime)
{
    /* if we're shutting down, wait for everything to finish */
    int wait = (newlevel == RUNLEVEL_SHUTDOWN || newlevel == RUNLEVEL_REBOOT);
    int oldlevel = runlevel;
    runlevel = newlevel;

    if (oldlevel)
        kill_tasks(oldlevel, sleeptime);
    for (unsigned i = 0; i < ntasks; i++)
        if ((tasks[i].runlevels & runlevel) && (tasks[i].action == ACTION_ONCE || tasks[i].action == ACTION_WAIT || tasks[i].action == ACTION_RESPAWN))
            run_task(&tasks[i], wait);

    /* all commands finished, power down the system */
    if (newlevel == RUNLEVEL_SHUTDOWN)
        reboot(RB_POWER_OFF);
    else if (newlevel == RUNLEVEL_REBOOT)
        reboot(RB_AUTOBOOT);
    return 1;
}

/* run task */
static int run_task(struct init_task *task, int wait)
{
    if (task->action == ACTION_OFF || (task->flags & FLAG_RUNNING))
        return 1;
    task->flags |= FLAG_RUNNING;

    wait |= (task->action == ACTION_SYSINIT || task->action == ACTION_BOOTWAIT || task->action == ACTION_WAIT);

    pid_t pid = fork();
    switch (pid) {
    case -1:
        task->flags &= ~FLAG_RUNNING;
        return 0;
    case 0:
        /* child process */
        setsid();
        ioctl(1, TIOCSCTTY, 1);

        char *args[4] = { "/bin/sh", "-c", task->cmd, NULL };
        execv(args[0], args);
    default:
        /* parent process */
        task->pid = pid;
        if (wait) {
            while (waitpid(task->pid, NULL, 0) < 0 && errno == EINTR);
            task->pid = -1;
            task->flags &= ~FLAG_RUNNING;
        }
        break;
    }

    return 1;
}

/* kill tasks in old runlevel */
static int kill_tasks(int oldlevel, int sleeptime)
{
    for (int r = 0; r <= sleeptime; r++) {
        int done = 1;
        for (unsigned i = 0; i < ntasks; i++) {
            if (!(tasks[i].flags & FLAG_RUNNING))
                continue;
            if (oldlevel && tasks[i].runlevels && !(tasks[i].runlevels & runlevel)) {
                done = 0;
                if (r == 0)
                    kill(tasks[i].pid, SIGTERM);
                else if (r == sleeptime)
                    kill(tasks[i].pid, SIGKILL);
            }
        }
        if (done)
            break;
        sleep(1);
    }
    return 1;
}

/* reap died tasks */
static void reap_task(int sig)
{
    pid_t pid = wait(NULL);
    if (pid < 0)
        return;
    for (int i = 0; i < ntasks; i++)
        if (tasks[i].pid == pid) {
            tasks[i].pid = -1;
            tasks[i].flags &= ~FLAG_RUNNING;
            /* respawn task if needed */
            if (tasks[i].action == ACTION_RESPAWN && (tasks[i].runlevels & runlevel))
                write(reappipe[1], &i, sizeof(i));
        }
}

/* handle task respawn and /dev/initctl input */
static void handle_communication(void)
{
    int fifo = reopen_fifo(-1);
    struct pollfd polls[2] = {
        { reappipe[0], POLLIN, 0 },
        { fifo,        POLLIN | POLLHUP, 0 }
    };

    for (;;) {
        int ret = poll(polls, 2, -1);
        if (ret <= 0)
            continue;

        if (polls[0].revents & POLLIN) {
            /* reap pipe: respawn task */
            int i;
            read(reappipe[0], &i, sizeof(i));
            run_task(&tasks[i], 0);
        }
        if (polls[1].revents & POLLIN) {
            /* initctl pipe */
            struct sysv_message msg;
            if (read(fifo, &msg, sizeof(msg)) == sizeof(msg) && msg.magic == SYSV_MESSAGE_MAGIC)
                switch (msg.cmd) {
                case SYSV_MESSAGE_RUNLEVEL:
                    if (msg.runlevel != runlevel)
                        enter_runlevel(msg.runlevel, msg.sleeptime);
                    break;
                default:
                    /* ??? */
                    break;
                }
        }
        if (polls[0].revents & POLLIN || polls[1].revents & POLLHUP)
             /* reopen FIFO after writer is done with it, or after a child exec, just to be sure */
             polls[1].fd = fifo = reopen_fifo(fifo);
    }
}

/* get FIFO pipe */
static int reopen_fifo(int fd)
{
    if (fd >= 0)
        close(fd);
    struct stat st;
    if (stat(SYSV_FIFO, &st) < 0 || !S_ISFIFO(st.st_mode)) {
        unlink(SYSV_FIFO);
        mkfifo(SYSV_FIFO, SYSV_FIFO_MODE);
    }
    return open(SYSV_FIFO, O_RDONLY | O_CLOEXEC);
}
