CFLAGS =-std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
LDFLAGS=-pie -Wl,-z,relro -Wl,-z,now
PREFIX=

.PHONY: all clean install uninstall
all:

clean:
	@echo [CLN]
	@rm -rf bin/* src/*.o src/sysvinit/*.o

install:

uninstall:


.PHONY: sysvinit install-sysvinit uninstall-sysvinit
sysvinit: bin/sysvinit bin/sysvinit-halt bin/sysvinit-killall bin/sysvinit-shutdown

install-sysvinit: sysvinit
	@install -d -m 0755 $(PREFIX)/sbin
	@echo [INS] init
	@install -m 0700 bin/sysvinit $(PREFIX)/sbin/init
	@echo [INS] halt
	@install -m 0755 bin/sysvinit-halt $(PREFIX)/sbin/halt
	@echo [ LN] poweroff
	@ln -s halt $(PREFIX)/sbin/poweroff
	@echo [ LN] reboot
	@ln -s halt $(PREFIX)/sbin/reboot
	@echo [INS] shutdown
	@install -m 0755 bin/sysvinit-shutdown $(PREFIX)/sbin/shutdown
	@echo [INS] killall5
	@install -m 0700 bin/sysvinit-killall $(PREFIX)/sbin/killall5

uninstall-sysvinit:
	@echo [UNS] sysvinit
	@rm -f $(PREFIX)/sbin/{init,halt,poweroff,reboot,shutdown,killall5}

bin/sysvinit: bin src/init.o src/common.o src/sysvinit/init.o src/sysvinit/inittab.o
	@echo [ LD] sysvinit
	@$(CC) $(LDFLAGS) src/init.o src/common.o src/sysvinit/init.o src/sysvinit/inittab.o -o bin/sysvinit

bin/sysvinit-halt: bin src/sysvinit/halt.o src/sysvinit/inittab.o src/sysvinit/wall.o
	@echo [ LD] sysvinit-halt
	@$(CC) $(LDFLAGS) src/sysvinit/halt.o src/sysvinit/inittab.o src/sysvinit/wall.o -o bin/sysvinit-halt

bin/sysvinit-killall: bin src/sysvinit/killall5.o
	@echo [ LD] sysvinit-killall
	@$(CC) $(LDFLAGS) src/sysvinit/killall5.o -o bin/sysvinit-killall

bin/sysvinit-shutdown: bin src/sysvinit/shutdown.o src/sysvinit/wall.o
	@echo [ LD] sysvinit-shutdown
	@$(CC) $(LDFLAGS) src/sysvinit/shutdown.o src/sysvinit/wall.o -o bin/sysvinit-shutdown


bin:
	@echo [ MK] bin/
	@mkdir bin

%.o: %.c
	@echo [ CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@
