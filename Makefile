CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now

DESTDIR   =
PREFIX    =
MANPREFIX = $(if $(subst /,,$(PREFIX)),$(PREFIX),/usr)
MAN5DIR   = $(MANPREFIX)/share/man/man5
MAN8DIR   = $(MANPREFIX)/share/man/man8

.PHONY: all clean install uninstall
all: sysvinit simple

clean:
	@echo [CLN]
	@rm -rf bin/* src/*.o src/sysvinit/*.o

install: install-sysvinit
uninstall: uninstall-sysvinit
uninstall-%: .manifest-%
	@cat $< | xargs -t rm -f
symlink-%: .manifest-% install-%
	@while read -r f ; do nf=$$(echo "$$f" | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -s $$(basename "$$f") $$nf ; done < $<
.manifest-%:
	@touch $@

bin $(DESTDIR)$(PREFIX)/sbin $(DESTDIR)$(MAN5DIR) $(DESTDIR)$(MAN8DIR):
	@install -d -m 0755 $@


.PHONY: sysvinit install-sysvinit uninstall-sysvinit symlink-sysvinit
sysvinit: \
    bin/sysvinit-init \
    bin/sysvinit-halt \
    bin/sysvinit-killall5 \
    bin/sysvinit-shutdown
install-sysvinit: \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-init \
    $(DESTDIR)$(MAN8DIR)/sysvinit-init.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-halt \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-poweroff \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-reboot \
    $(DESTDIR)$(MAN8DIR)/sysvinit-halt.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-killall5 \
    $(DESTDIR)$(MAN8DIR)/sysvinit-killall5.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-shutdown \
    $(DESTDIR)$(MAN8DIR)/sysvinit-shutdown.8 \
    $(DESTDIR)$(MAN5DIR)/sysvinit-inittab.5

bin/sysvinit-init: src/init.o src/common.o src/sysvinit/inittab.o src/sysvinit/runlevel.o
bin/sysvinit-halt: src/sysvinit/runlevel.o src/sysvinit/wall.o
bin/sysvinit-shutdown: src/sysvinit/runlevel.o src/sysvinit/wall.o
bin/sysvinit-%: bin src/sysvinit/%.o
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $(filter-out bin,$^) -o $@

$(DESTDIR)$(PREFIX)/sbin/sysvinit-%: bin/sysvinit-% $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(PREFIX)/sbin/sysvinit-poweroff $(DESTDIR)$(PREFIX)/sbin/sysvinit-reboot: $(DESTDIR)$(PREFIX)/sbin/sysvinit-halt
	@echo [ LN] $(notdir $@)
	@ln -s $(notdir $<) $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN5DIR)/sysvinit-%.5: src/sysvinit/%.5 $(DESTDIR)$(MAN5DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN8DIR)/sysvinit-%.8: src/sysvinit/%.8 $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit


.PHONY: simple install-simple uninstall-simple symlink-simple
simple: bin/simple-init
install-simple: \
    $(DESTDIR)$(PREFIX)/sbin/simple-init \
    $(DESTDIR)$(MAN8DIR)/simple-init.8

bin/simple-init: src/init.o src/common.o
bin/simple-%: bin src/simple/%.o
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $(filter-out bin,$^) -o $@

$(DESTDIR)$(PREFIX)/sbin/simple-%: bin/simple-% $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@
	@echo $@ >> .manifest-simple

$(DESTDIR)$(MAN8DIR)/simple-%.8: src/simple/%.8 $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-simple


%.o: %.c
	@echo [ CC] $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

