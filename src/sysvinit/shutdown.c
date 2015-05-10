#define  _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <spawn.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>
#include "init.h"
#include "wall.h"

#define  PID_FILE       "/var/run/shutdown.pid"
#define  NOLOGIN_FILE   "/etc/nologin"
#define  NOLOGIN_CUTOFF 300

static void  setup_shutdown(void);
static int   cancel_shutdown(void);
static void  cleanup(int signal);
static int   parse_when(const char *when);
static int   create_nologin(const char *message);
static char *sysv_runlevel(int runlevel);

static int   nologin;


int main(int argc, char *argv[])
{
    int cancel = 0, doit = 1;
    int runlevel = RUNLEVEL_SINGLE;

    char o;
    while ((o = getopt(argc, argv, "akrhPHfFnc:")) >= 0) {
        switch (o) {
        case 'a':
        case 'f':
        case 'F':
        case 'n':
            /* legacy, unused */
            break;
        case 'h':
        case 'H':
        case 'P':
            runlevel = RUNLEVEL_SHUTDOWN;
            break;
        case 'r':
            runlevel = RUNLEVEL_REBOOT;
            break;
        case 'k':
            doit = 0;
            break;
        case 'c':
            cancel = 1;
            break;
        default:
            fprintf(stderr, "shutdown: unknown option: -%c\n", optopt);
            return 1;
        }
    }
    if (cancel)
        return cancel_shutdown();

    if (!argv[optind]) {
        fprintf(stderr, "shutdown: need time argument\n");
        return 1;
    }
    const char *when = argv[optind++];
    const char *message = argv[optind];


    struct stat st;
    if (!stat(PID_FILE, &st)) {
        fprintf(stderr, "shutdown: an instance is already running\n");
        return 1;
    }


    setup_shutdown();

    /* parse time formats: "now", +m, hh:mm */
    int seconds = parse_when(when);
    if (seconds < 0) {
        fprintf(stderr, "shutdown: invalid time format: %s\n", when);
        goto err;
    }

    if (seconds) {
        /* warn user and sleep until we have to create /etc/nologin */
        warn(runlevel, seconds);
        if (seconds > NOLOGIN_CUTOFF) {
            int tosleep = seconds - NOLOGIN_CUTOFF;
            while ((tosleep = sleep(tosleep)));
            warn(runlevel, NOLOGIN_CUTOFF);
        }

        /* create nologin and sleep until it's time */
        nologin = create_nologin(message);
        while ((seconds = sleep(seconds)));
    }
    warn(runlevel, 0);

    /* finally, shutdown */
    cleanup(0);
    if (doit) {
        char *run = sysv_runlevel(runlevel);
        pid_t pid;
        if (posix_spawnp(&pid, "halt", NULL, NULL, (char *[]) { "halt", run, NULL }, NULL)) {
            perror("shutdown: could not spawn halt");
            goto err;
        }
        int status = 1;
        while (waitpid(pid, &status, 0) < 0 && errno == EINTR);
        if (status) {
            fprintf(stderr, "shutdown: halt reported failure\n");
            goto err;
        }
    }

    /* we're done here, clean up */
    return 0;
err:
    cleanup(0);
    return 1;
}

static void setup_shutdown(void)
{
    /* write PID file */
    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        perror("shutdown: could not open PID file");
        return;
    }
    pid_t pid = getpid();
    if (write(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        perror("shutdown: could not write PID file");
        close(fd);
        return;
    }
    close(fd);

    /* setup cleanup handlers */
    sigset_t sigs;
    sigemptyset(&sigs);

    struct sigaction act;
    act.sa_handler = cleanup;
    act.sa_mask = sigs;
    act.sa_flags = 0;
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

static int cancel_shutdown(void)
{
    int fd = open(PID_FILE, O_RDONLY);
    if (fd < 0) {
        perror("shutdown: could not open PID file");
        return 1;
    }
    pid_t pid;
    if (read(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        perror("shutdown: could not read PID file");
        close(fd);
        return 1;
    }
    close(fd);

    if (kill(pid, SIGINT) < 0) {
        perror("shutdown: could not signal other instance");
        return 1;
    }
    return 0;
}

static void cleanup(int signal)
{
    unlink(PID_FILE);
    if (nologin)
        unlink(NOLOGIN_FILE);
}

static int parse_when(const char *when)
{
    unsigned hour, min;
    if (!strcmp(when, "now"))
        return 0;
    else if (when[0] == '+') {
        errno = 0;
        int s = strtol(++when, NULL, 10) * 60;
        if (!errno)
            return s;
    } else if (sscanf(when, "%u:%u", &hour, &min) == 2) {
        if (hour < 24 && min < 60) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            int then = hour * 3600 + min * 60;
            int now = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
            return then - now + 60 * 60 * 24 * (now > then);
        }
    }
    return -1;
}

static int create_nologin(const char *message)
{
    struct stat st;
    if (stat(NOLOGIN_FILE, &st) < 0) {
        FILE *f = fopen(NOLOGIN_FILE, "w");
        if (f) {
            time_t t = time(NULL) + NOLOGIN_CUTOFF;
            fprintf(f, "The system is going down on %s\n", ctime(&t));
            if (message) {
                fputs(message, f);
                fputc('\n', f);
            }
            fclose(f);
            return 1;
        }
    }
    return 0;
}

static char *sysv_runlevel(int runlevel)
{
    switch (runlevel) {
    case RUNLEVEL_SINGLE:
        return "1";
    case RUNLEVEL_SHUTDOWN:
        return "0";
    case RUNLEVEL_REBOOT:
        return "6";
    }
    return NULL;
}
