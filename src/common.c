#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"

/* get us into a somewhat sane environment */
void prepare_env(void)
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

/* set process name */
void setprocname(const char *name, const char *argv0)
{
    prctl(PR_SET_NAME, name);
    while (*argv0 && *name) *argv0++ = *name++;
    while (*argv0) *argv0++ = 0;
}
