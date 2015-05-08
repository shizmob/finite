program=""
args=""
type="normal"
name="root"
group="root"
groups=""
workdir="/"
pidfile=""
log=""
errlog=""

extra=""
extra_running=""
extra_idle=""


# predefined vars: $service, $status [running, idle, oneshot], $runlevel, $command

# default implementations

default_start() {
    return 0
}

default_stop() {
    return 0
}

default_restart() {
    stop || return 1
    start || return 1
}

default_help() {
    return 0
}

default_status() {
    return 0
}

default() {
    return eval "default_${currcommand}"
}

# stop, restart, help and status automatically defined, start falls back to default impl after checking config

