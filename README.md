finite
======

finite is an init implementation that strives to be robust, correct and simple. Currently it implements a somewhat sysvinit-compatible init, but a new simpler streamlined init is planned for the future.

Installation
------------
Simply run `make install-sysvinit` to compile and install the sysvinit implementation to your system. This installs /sbin/init, /sbin/halt (and its reboot and poweroff symlinks), /sbin/killall5 and their respective manpages.
