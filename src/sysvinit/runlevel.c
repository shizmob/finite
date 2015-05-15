#include "init.h"
#include "runlevel.h"

int parse_runlevels(const char *runlevels)
{
    int res = 0;

    for (; *runlevels; runlevels++)
        res |= parse_runlevel(*runlevels);

    return res;
}

int parse_runlevel(char runlevel)
{
    if (runlevel == '0')
        return RUNLEVEL_SHUTDOWN;
    else if (runlevel == '1')
        return RUNLEVEL_SINGLE;
    else if (runlevel == '3')
        return RUNLEVEL_MULTI;
    else if (runlevel == '6')
        return RUNLEVEL_REBOOT;
    return 0;
}

char emit_runlevel(int runlevels)
{
    if (runlevels & RUNLEVEL_SHUTDOWN)
        return '0';
    if (runlevels & RUNLEVEL_SINGLE)
        return '1';
    if (runlevels & RUNLEVEL_MULTI)
        return '3';
    if (runlevels & RUNLEVEL_REBOOT)
        return '6';
    return 0;
}
