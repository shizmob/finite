CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now

DESTDIR   =
PREFIX    =
MANPREFIX = $(if $(subst /,,$(PREFIX)),$(PREFIX),/usr)
MAN5DIR   = $(MANPREFIX)/share/man/man5
MAN8DIR   = $(MANPREFIX)/share/man/man8


all: sysvinit simple
.PHONY: all clean distclean install uninstall

clean:
	@echo [CLN]
	@rm -rf bin obj
distclean: clean
	@rm -f .manifest-*

install: install-sysvinit install-simple
uninstall: uninstall-sysvinit uninstall-simple
uninstall-%: .manifest-%
	@while read -r f ; do echo [ RM] $$(basename "$$f") ; rm -f "$$f" ; done < $<

symlink-%: .manifest-%
	@while read -r f ; do nf=$$(dirname "$$f")/$$(echo $$(basename "$$f") | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -sf $$(basename "$$f") $$nf ; echo "$$nf" >> .manifest-symlinks ; done < $<

bin obj:
	@mkdir $@
$(DESTDIR)$(PREFIX)/sbin $(DESTDIR)$(MAN5DIR) $(DESTDIR)$(MAN8DIR):
	@install -d -m 0755 $@


.SECONDARY:

.PHONY: sysvinit install-sysvinit
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

bin/sysvinit-init: obj/init.o obj/common.o obj/sysvinit/inittab.o obj/sysvinit/runlevel.o
bin/sysvinit-halt: obj/sysvinit/runlevel.o obj/sysvinit/wall.o
bin/sysvinit-shutdown: obj/sysvinit/runlevel.o obj/sysvinit/wall.o
bin/sysvinit-%: obj/sysvinit/%.o | bin
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $^ -o $@

$(DESTDIR)$(PREFIX)/sbin/sysvinit-%: bin/sysvinit-% | $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(PREFIX)/sbin/sysvinit-poweroff $(DESTDIR)$(PREFIX)/sbin/sysvinit-reboot: $(DESTDIR)$(PREFIX)/sbin/sysvinit-halt
	@echo [ LN] $(notdir $@)
	@ln -s $(notdir $<) $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN5DIR)/sysvinit-%.5: src/sysvinit/%.5 | $(DESTDIR)$(MAN5DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit

$(DESTDIR)$(MAN8DIR)/sysvinit-%.8: src/sysvinit/%.8 | $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-sysvinit


.PHONY: simple install-simple
simple: bin/simple-init
install-simple: \
    $(DESTDIR)$(PREFIX)/sbin/simple-init \
    $(DESTDIR)$(MAN8DIR)/simple-init.8

bin/simple-init: obj/init.o obj/common.o
bin/simple-%: obj/simple/%.o | bin
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $^ -o $@

$(DESTDIR)$(PREFIX)/sbin/simple-%: bin/simple-% | $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@
	@echo $@ >> .manifest-simple

$(DESTDIR)$(MAN8DIR)/simple-%.8: src/simple/%.8 | $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@
	@echo $@ >> .manifest-simple


obj/%.o: src/%.c | obj
	@mkdir -p $(@D)
	@echo [ CC] $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@
