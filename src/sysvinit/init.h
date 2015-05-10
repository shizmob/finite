#pragma once

#define INITTAB_FILE       "/etc/inittab"
#define SYSV_FIFO          "/dev/initctl"
#define SYSV_FIFO_MODE     0660
#define SYSV_MESSAGE_MAGIC 0x03091969
#define SYSV_MESSAGE_SIZE  384
#define SYSV_DEFAULT_SLEEP 5

enum sysv_message_command
{
    SYSV_MESSAGE_RUNLEVEL  = 1
};

struct sysv_message
{
    int  magic;
    int  cmd;
    int  runlevel;
    int  sleeptime;
    char data[SYSV_MESSAGE_SIZE - 4 * sizeof(int)];
} __attribute__((packed));

enum init_action
{
    ACTION_OFF = 1,
    ACTION_INITDEFAULT,
    ACTION_SYSINIT,
    ACTION_BOOT,
    ACTION_BOOTWAIT,
    ACTION_ONCE,
    ACTION_WAIT,
    ACTION_RESPAWN
};

enum init_runlevels
{
    RUNLEVEL_SHUTDOWN = 0x1, /* 0 */
    RUNLEVEL_SINGLE   = 0x2, /* 1 */
    RUNLEVEL_MULTI    = 0x4, /* 3 */
    RUNLEVEL_REBOOT   = 0x8  /* 6 */
};

enum init_flags
{
    FLAG_RUNNING      = 0x1, /* started and running */
    FLAG_IDLE         = 0x2, /* started and stopped */
    FLAG_WAITING      = 0x4, /* waiting for process to finish */
    FLAG_SPAMMING     = 0x8  /* process is respawning too quickly */
};
