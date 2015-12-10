include plate.mk

CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now


all: sysvinit
.PHONY: all install uninstall

install: install-sysvinit
uninstall: uninstall-sysvinit


# common

$(B)/init: $(O)/init/init.o $(S)/init/config.h


# sysvinit

.PHONY: sysvinit install-sysvinit uninstall-sysvinit symlink-sysvinit

sysvinit: \
    $(B)/init \
    $(B)/sysvinit-init \
    $(B)/sysvinit-halt \
    $(B)/sysvinit-killall5 \
    $(B)/sysvinit-shutdown
$(B)/sysvinit-init: $(O)/rc/sysvinit/inittab.o $(O)/rc/sysvinit/runlevel.o $(O)/rc/sysvinit/init.o
$(B)/sysvinit-halt: $(O)/rc/sysvinit/runlevel.o $(O)/rc/sysvinit/wall.o $(O)/rc/sysvinit/halt.o
$(B)/sysvinit-shutdown: $(O)/rc/sysvinit/runlevel.o $(O)/rc/sysvinit/wall.o $(O)/rc/sysvinit/shutdown.o
$(B)/sysvinit-killall5: $(O)/rc/sysvinit/killall5.o

SYSVINIT= \
    $(P)/sbin/sysvinit-init \
    $(M)/man8/sysvinit-init.8 \
    $(P)/sbin/sysvinit-halt \
    $(P)/sbin/sysvinit-poweroff \
    $(P)/sbin/sysvinit-reboot \
    $(M)/man8/sysvinit-halt.8 \
    $(P)/sbin/sysvinit-killall5 \
    $(M)/man8/sysvinit-killall5.8 \
    $(P)/sbin/sysvinit-shutdown \
    $(M)/man8/sysvinit-shutdown.8 \
    $(M)/man5/sysvinit-inittab.5
install-sysvinit: $(SYSVINIT)
uninstall-sysvinit:
	@rm -f $(SYSVINIT)
symlink-sysvinit: $(SYSVINIT)
	@for f in $(SYSVINIT) ; do nf=$$(dirname "$$f")/$$(echo $$(basename "$$f") | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -sf $$(basename "$$f") $$nf  ; done
