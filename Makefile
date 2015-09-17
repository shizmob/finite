include plate.mk

CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now


all: sysvinit simple
.PHONY: all install uninstall

install: install-sysvinit install-simple
uninstall: uninstall-sysvinit uninstall-simple


# common

$(O)/init.o: $(S)/common.h


# sysvinit

.PHONY: sysvinit install-sysvinit uninstall-sysvinit symlink-sysvinit

sysvinit: \
    $(B)/sysvinit-init \
    $(B)/sysvinit-halt \
    $(B)/sysvinit-killall5 \
    $(B)/sysvinit-shutdown
$(B)/sysvinit-init: $(O)/init.o $(O)/common.o $(O)/sysvinit/inittab.o $(O)/sysvinit/runlevel.o $(O)/sysvinit/init.o
$(B)/sysvinit-halt: $(O)/sysvinit/runlevel.o $(O)/sysvinit/wall.o $(O)/sysvinit/halt.o
$(B)/sysvinit-shutdown: $(O)/sysvinit/runlevel.o $(O)/sysvinit/wall.o $(O)/sysvinit/shutdown.o
$(B)/sysvinit-killall5: $(O)/sysvinit/killall5.o

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


# simple

.PHONY: simple install-simple uninstall-simple simlink-simple
simple: \
    $(B)/simple-init
$(B)/simple-init: $(O)/init.o $(O)/common.o $(O)/simple/init.o

SIMPLE= \
    $(P)/sbin/simple-init \
    $(M)/man8/simple-init.8
install-simple: $(SIMPLE)
uninstall-simple:
	@rm -f $(SIMPLE)
symlink-simple: $(SIMPLE)
	@for f in $(SIMPLE) ; do nf=$$(dirname "$$f")/$$(echo $$(basename "$$f") | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -sf $$(basename "$$f") $$nf ; done
