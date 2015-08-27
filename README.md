finite
======

finite is an init implementation that strives to be robust, correct and simple.  
Currently it implements a somewhat sysvinit-compatible init (`src/sysvinit`) and a simpler respawning init (`src/simple`).

Installation
------------
Simply run `make install-sysvinit` to compile and install the sysvinit implementation to your system.
This installs `/sbin/sysvinit-init`, `/sbin/sysvinit-halt` (and its `reboot` and `poweroff` symlinks), `/sbin/sysvinit-killall5` and their respective manpages.

For the simple init implementation, run `make install-simple`. This installs `/sbin/simple-init` and its manpage.

To symlink any implementation to the classic non-prefixed binaries, run `make symlink-<implementation>`, for instance `make symlink-sysvinit`.
To remove these symlinks again, run `make uninstall-symlinks`.

To uninstall an implementation, simply run `make uninstall-<implementation>`, for instance `make uninstall-sysvinit`.

Restrictions
------------
finite-sysvinit only supports runlevels `0`, `1`, `3` and `6`, mapping to shutdown, single-user, multi-user and reboot respectively. More runlevels should not be needed.

It does not support the `ctrlaltdel`, `kbrequest` or `power*` runlevel actions. Those are better left to other specialized software, not to the init system.

It does not support system halting, although this could be added moderately trivially.

It does not currently create or write to `utmp` or `wtmp` files. Support for this is under consideration.

The only `/dev/initctl` command supported is the one that changes the current runlevel (command `0x1`). This makes it incompatible with debian-sysvinit's `shutdown` and `halt` implementations, as they require more for reasons unknown.
