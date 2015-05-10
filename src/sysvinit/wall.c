#define  _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <utmpx.h>
#include <sys/types.h>
#include "init.h"
#include "wall.h"

#define  max(a, b) ((a) > (b) ? (a) : (b))


void warn(int runlevel, int seconds)
{
    const char *purpose;
    switch (runlevel) {
    case RUNLEVEL_SINGLE:
        purpose = "for maintenance";
        break;
    case RUNLEVEL_SHUTDOWN:
        purpose = "for shutdown";
        break;
    case RUNLEVEL_REBOOT:
        purpose = "for reboot";
        break;
    }
    int mins = seconds / 60;

    if (seconds)
        wall("The system is going down %s in %s%d minute%s!\n", purpose, mins ? "" : "<", max(mins, 1), mins == 1 ? "" : "s");
    else
        wall("The system is going down %s NOW!\n", purpose);
}

void wall(const char *message, ...)
{
    /* get username */
    uid_t uid = getuid();
    const char *username;
    struct passwd *pwd = getpwuid(uid);
    if (!pwd)
        username = pwd->pw_name;
    else
        username = "unknown";

    /* get TTY */
    char *tty = ttyname(1);
    if (tty) {
        if (!strncmp(tty, "/dev/", sizeof("/dev/")))
            tty += sizeof("/dev/");
    } else
        tty = "unknown";

    /* get hostname */
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)))
        strncpy(hostname, "[unknown]", sizeof(hostname) - 1);
    hostname[sizeof(hostname) - 1] = 0;

    /* get datetime string */
    time_t t = time(NULL);
    char *tm = ctime(&t);
    /* remove newline at end */
    for (char *p = tm; *p; p++)
        if (*p == '\n')
            *p = 0;

    /* finally, start writing messages */
    va_list ap;
    va_start(ap, message);

    setutxent();
    struct utmpx *u;
    while ((u = getutxent())) {
        if (u->ut_type != USER_PROCESS)
            continue;

        /* device does not always start with /dev/ */
        char device[32];
        if (strncmp(u->ut_line, "/dev/", sizeof("/dev/"))) {
            strncpy(device, "/dev/", sizeof(device) - 1);
            device[sizeof(device) - 1] = 0;
        } else
            device[0] = 0;
        strncat(device, u->ut_line, sizeof(device) - 1);
        device[sizeof(device) - 1] = 0;

        FILE *f = NULL;
        int fd = open(device, O_WRONLY | O_NOCTTY);
        if (fd < 0 || !isatty(fd))
            goto end;
        f = fdopen(fd, "w");
        if (!f)
            goto end;

        va_list nap;
        va_copy(nap, ap);
        fprintf(f, "\a\nBroadcast message from %s@%s(%s) at %s:\n", username, hostname, tty, tm);
        vfprintf(f, message, nap);
        fputc('\n', f);
        va_end(nap);

end:
        if (fd >= 0)
            close(fd);
        if (f)
            fclose(f);
    }
    endutxent();

    va_end(ap);
}
