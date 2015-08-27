CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now

DESTDIR   =
PREFIX    =
MANPREFIX = $(if $(subst /,,$(PREFIX)),$(PREFIX),/usr)
MAN5DIR   = $(MANPREFIX)/share/man/man5
MAN8DIR   = $(MANPREFIX)/share/man/man8

.PHONY: all clean install uninstall
all: sysvinit

clean:
	@echo [CLN]
	@rm -rf bin/* src/*.o src/sysvinit/*.o

install: install-sysvinit
uninstall: uninstall-sysvinit


.PHONY: sysvinit install-sysvinit uninstall-sysvinit
sysvinit: \
    bin/sysvinit-init \
    bin/sysvinit-halt \
    bin/sysvinit-killall5 \
    bin/sysvinit-shutdown
install-sysvinit: \
    $(DESTDIR)$(PREFIX)/sbin/init \
    $(DESTDIR)$(MAN8DIR)/init.8 \
    $(DESTDIR)$(PREFIX)/sbin/halt \
    $(DESTDIR)$(PREFIX)/sbin/poweroff \
    $(DESTDIR)$(PREFIX)/sbin/reboot \
    $(DESTDIR)$(MAN8DIR)/halt.8 \
    $(DESTDIR)$(PREFIX)/sbin/killall5 \
    $(DESTDIR)$(MAN8DIR)/killall5.8 \
    $(DESTDIR)$(PREFIX)/sbin/shutdown \
    $(DESTDIR)$(MAN8DIR)/shutdown.8 \
    $(DESTDIR)$(MAN5DIR)/inittab.5

uninstall-sysvinit: .manifest-sysvinit
	@cat .manifest-sysvinit | xargs -t rm -f

.manifest-sysvinit:
	@touch $@


bin/sysvinit-init: src/init.o src/common.o src/sysvinit/inittab.o src/sysvinit/runlevel.o
bin/sysvinit-halt: src/sysvinit/runlevel.o src/sysvinit/wall.o
bin/sysvinit-shutdown: src/sysvinit/runlevel.o src/sysvinit/wall.o
bin/sysvinit-%: bin src/sysvinit/%.o
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $(filter-out bin,$^) -o $@

$(DESTDIR)$(PREFIX)/sbin $(DESTDIR)$(MAN5DIR) $(DESTDIR)$(MAN8DIR):
	@install -d -m 0755 $@

$(DESTDIR)$(PREFIX)/sbin/%: bin/sysvinit-% $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(PREFIX)/sbin/poweroff $(DESTDIR)$(PREFIX)/sbin/reboot: $(DESTDIR)$(PREFIX)/sbin/halt
	@echo [ LN] $(notdir $@)
	@ln -s $(notdir $<) $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN5DIR)/%.5: src/sysvinit/%.5 $(DESTDIR)$(MAN5DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN8DIR)/%.8: src/sysvinit/%.8 $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit


bin:
	@mkdir bin

%.o: %.c
	@echo [ CC] $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

