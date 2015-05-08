#define  _XOPEN_SOURCE 700
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include "common.h"

/* get us into a somewhat same environment */
void prepare(void)
{
    umask(DEFAULT_UMASK);
    chdir("/");

    const char *con = getenv("console");
    if (!con)
        con = "/dev/console";

    setenv("CONSOLE", con,            0);
    setenv("HOME",    "/",            0);
    setenv("PATH",    DEFAULT_PATH,   0);
    setenv("TERM",    DEFAULT_TERM,   0);
    setenv("USER",    DEFAULT_USER,   0);
}

/* shut down the system */
void shutdown(int mode)
{
    switch (mode) {
    case MODE_POWER_OFF:
        reboot(RB_POWER_OFF);
        break;
    case MODE_REBOOT:
        reboot(RB_AUTOBOOT);
        break;
    default:
        /* ??? */
        break;
    }
}
