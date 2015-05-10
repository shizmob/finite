#define  _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

static int killall(int sig);


int main(int argc, char *argv[])
{
    int sig = SIGKILL;

    if (argc > 1)
        if (*argv[1]++ == '-') {
            errno = 0;
            sig = strtol(argv[1], NULL, 10);
            if (errno)
                sig = -1;
        }
    if (sig < 1) {
        fprintf(stderr, "killall5: invalid signal: %s\n", argv[1]);
        return 1;
    }

    return killall(sig);
}

static int killall(int sig)
{
    int sid = getsid(0);

    /* ensure proc is mounted */
    struct stat st;
    if (stat("/proc/stat", &st) < 0) {
        perror("killall5: can't access /proc/stat");
        return 1;
    }
    if (stat("/proc", &st) < 0 || st.st_ino != 1) {
        fprintf(stderr, "killall5: /proc not mounted");
        return 1;
    }
    if (chdir("/proc") < 0) {
        perror("killall5: can't enter /proc");
        return 1;
    }

    DIR *dir = opendir(".");
    if (!dir) {
        perror("killall5: can't open /proc");
        return 1;
    }

    /* iterate over everything and send the signal */
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        /* skip non-processes and init */
        errno = 0;
        pid_t pid = strtol(entry->d_name, NULL, 10);
        if (errno || pid <= 1)
            continue;

        char path[32], buf[BUFSIZ];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);

        FILE *f = fopen(path, "rb");
        if (!f)
            goto next;
        if (!fgets(buf, BUFSIZ, f))
            goto next;

        /* skip over unwieldy process name */
        const char *p = strrchr(buf, ')') + 2;
        int procsid;
        unsigned long startcode, endcode;
        if (sscanf(p, "%*c %*d %*d %d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %*d %*u %lu %lu", &procsid, &startcode, &endcode) < 3)
            goto next;

        /* skip processes in our session and kernel processes */
        if (!procsid || procsid == sid || (startcode == 0 && endcode == 0))
            goto next;

        /* send the signal */
        if (kill(pid, sig) < 0 && errno == EINVAL) {
            fclose(f);
            closedir(dir);
            fprintf(stderr, "killall5: invalid signal: %d\n", sig);
            return 1;
        }
next:
        if (f)
            fclose(f);
    }
    closedir(dir);

    return 0;
}
