finite
======

finite is an init implementation that strives to be robust, correct and simple. Currently it implements a somewhat sysvinit-compatible init, but a new simpler streamlined init is planned for the future.

Installation
------------
Simply run `make install-sysvinit` to compile and install the sysvinit implementation to your system. This installs /sbin/init, /sbin/halt (and its reboot and poweroff symlinks), /sbin/killall5 and their respective manpages.

Restrictions
------------
finite-sysvinit only supports runlevels `0`, `1`, `3` and `6`, mapping to shutdown, single-user, multi-user and reboot respectively. More runlevels should not be needed.

It does not support the `ctrlaltdel`, `kbrequest` or `power*` runlevel actions. Those are better left to other specialized software, not to the init system.

It does not support system halting, although this could be added moderately trivially.

It does not currently create or write to `utmp` or `wtmp` files. Support for this is under consideration.

The only `/dev/initctl` command supported is the one that changes the current runlevel (command `0x1`). This makes it incompatible with debian-sysvinit's `shutdown` and `halt` implementations, as they require more for reasons unknown.
