#define  _BSD_SOURCE
#define  _DEFAULT_SOURCE
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "init.h"
#include "inittab.h"


/* parse inittab file */
int parse_tab(const char *filename, struct init_task *tasks, unsigned ntasks)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        goto err;

    char buf[8192];
    int ret, off = 0;
    unsigned task = 0;
    while ((ret = read(fd, buf + off, sizeof(buf) - off - 1)) > 0) {
        buf[off + ret] = 0;
        off = 0;

        /* convert lines to separate strings */
        for (char *p = buf; *p; p++)
            if (*p == '\n')
                *p = 0;

        /* parse line by line */
        int parsed = 0;
        char *p = buf;
        while (parsed < ret) {
            char *line = p;
            int len = strlen(line);

            /* mark line as parsed */
            parsed += len + 1;
            p += len + 1;

            /* skip empty and comment lines */
            for (; *line; line++)
                if (*line == '#') {
                    while (*line)
                        line++;
                    break;
                } else if (*line != '\t' && *line != ' ')
                    break;

            if (!*line)
                continue;

            /* get fields */
            char *eid      = strsep(&line, ":");
            char *erunlvls = strsep(&line, ":");
            char *eaction  = strsep(&line, ":");
            char *ecmd     = line;

            if (!eid || !erunlvls || !eaction || !ecmd) {
                /* required fields not found, try to restart parsing if at end of buffer */
                char *linestart = p - len - 1;
                if (parsed >= ret && buf != linestart) {
                    memmove(buf, linestart, len + 1);
                    off = len;
                    continue;
                }
                goto err;
            }

            /* semantic checks */
            if (!*eid || !*eaction)
                goto err;

            int action = parse_action(eaction);
            if (!action)
                continue;

            if (!*erunlvls && action != ACTION_BOOT && action != ACTION_BOOTWAIT && action != ACTION_SYSINIT)
                goto err;

            int runlevels = parse_runlevels(erunlvls);
            if (!runlevels && action != ACTION_BOOT && action != ACTION_BOOTWAIT && action != ACTION_SYSINIT)
                continue;

            if (!*ecmd && action != ACTION_INITDEFAULT)
                goto err;

            /* add task if we have room for it */
            if (task < ntasks) {
                strncpy(tasks[task].id,  eid,  sizeof(tasks[task].id) - 1);
                strncpy(tasks[task].cmd, ecmd, sizeof(tasks[task].cmd) - 1);
                tasks[task].id[sizeof(tasks[task].id) - 1] = 0;
                tasks[task].runlevels = runlevels;
                tasks[task].action = action;
                tasks[task].cmd[sizeof(tasks[task].cmd) - 1] = 0;
                tasks[task].pid = -1;
                tasks[task].flags = 0;
            }
            task++;
        }
    }
    if (off)
        goto err;
    if (ret < 0)
        goto err;

    close(fd);
    return task;

err:
    if (fd >= 0)
        close(fd);
    return -1;
}

int parse_runlevels(const char *runlevels)
{
    int res = 0;

    for (; *runlevels; runlevels++)
        if (*runlevels == '0')
            res |= RUNLEVEL_SHUTDOWN;
        else if (*runlevels == '1')
            res |= RUNLEVEL_SINGLE;
        else if (*runlevels == '3')
            res |= RUNLEVEL_MULTI;
        else if (*runlevels == '6')
            res |= RUNLEVEL_REBOOT;

    return res;
}

int parse_action(const char *action)
{
    int res = 0;

    if (!strcmp(action, "off"))
        res = ACTION_OFF;
    else if (!strcmp(action, "initdefault"))
        res = ACTION_INITDEFAULT;
    else if (!strcmp(action, "sysinit"))
        res = ACTION_SYSINIT;
    else if (!strcmp(action, "boot"))
        res = ACTION_BOOT;
    else if (!strcmp(action, "bootwait"))
        res = ACTION_BOOTWAIT;
    else if (!strcmp(action, "once"))
        res = ACTION_ONCE;
    else if (!strcmp(action, "wait"))
        res = ACTION_WAIT;
    else if (!strcmp(action, "respawn"))
        res = ACTION_RESPAWN;

    return res;
}
