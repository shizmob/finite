#pragma  once
#define  DEFAULT_PATH  "/usr/sbin:/usr/bin:/sbin:/bin"
#define  DEFAULT_TERM  "linux"
#define  DEFAULT_UMASK 022
#define  DEFAULT_USER  "root"

enum shutdown_mode
{
    MODE_POWER_OFF = 1,
    MODE_REBOOT
};

void prepare(void);
void shutdown(int mode);
