#!/sbin/finite-run
program="/usr/sbin/sshd"
args="-D"
type="normal"                 # normal, fork, oneshot
name="root"
group="root"
groups="mail"                 # extra groups
workdir="/"
pidfile="/run/sshd.pid"       # (fork)
log="syslog:sshd@local3.info" # can stdout+stderr log to syslog...
errlog="/var/log/sshd.log"    # or stderr separately, also to files

extra="checkconfig"           # extra rc commands
extra_running="reload"        # extra rc commands when running (non-oneshot)
extra_idle="regen_keys"       # extra rc commands when not running (non-oneshot)


depend() {
    # openrc-style dependency functions
    use logger net
}

checkconfig() {
    # ...
}

regen_keys() {
    # ...
}

reload() {
    # $PID is defined for extra_running commands
    kill -HUP $PID
}


start() {
    checkconfig || exit 1
    default
}

# stop, restart, help and status automatically defined, start falls back to default impl after checking config
