CFLAGS =-std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
LDFLAGS=-pie -Wl,-z,relro -Wl,-z,now
PREFIX=/

.PHONY: all clean sysvinit install install-sysvinit
all:

sysvinit: bin/sysvinit bin/sysvinit-halt bin/sysvinit-killall

clean:
	@echo [CLN]
	@rm -rf bin/* src/*.o src/sysvinit/*.o

install:

install-sysvinit:
	@install -d -m 0755 $(PREFIX)/sbin
	@echo [INS] init
	@install -m 0700 bin/sysvinit $(PREFIX)/sbin/init
	@echo [INS] halt
	@install -m 0755 bin/sysvinit-halt $(PREFIX)/sbin/halt
	@echo [ LN] poweroff
	@ln -s halt $(PREFIX)/sbin/poweroff
	@echo [ LN] reboot
	@ln -s halt $(PREFIX)/sbin/reboot
	@echo [INS] killall5
	@install -m 0700 bin/sysvinit-killall $(PREFIX)/sbin/killall5

uninstall:

uninstall-sysvinit:
	@echo [UNS] sysvinit
	@rm -f $(PREFIX)/sbin/{init,halt,poweroff,reboot,killall5}


bin/sysvinit: src/init.o src/common.o src/sysvinit/init.o src/sysvinit/inittab.o
	@echo [ LD] sysvinit
	@$(CC) $(LDFLAGS) src/init.o src/common.o src/sysvinit/init.o src/sysvinit/inittab.o -o bin/sysvinit

bin/sysvinit-halt: src/sysvinit/halt.o
	@echo [ LD] sysvinit-halt
	@$(CC) $(LDFLAGS) src/sysvinit/halt.o -o bin/sysvinit-halt

bin/sysvinit-killall: src/sysvinit/killall5.o
	@echo [ LD] sysvinit-killall
	@$(CC) $(LDFLAGS) src/sysvinit/killall5.o -o bin/sysvinit-killall


%.o: %.c
	@echo [ CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@
