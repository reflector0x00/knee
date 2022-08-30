CROSS_COMPILE=$(PWD)/toolchain/i686-elf/bin/i686-elf-
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
AS=$(CROSS_COMPILE)as
CFLAGS= -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude -g
CXXFLAGS= --std=c++20 -O2 -Wall -Wextra -fno-exceptions -fno-rtti -fno-unwind-tables -Iinclude -g


all: knee.bin

knee.bin: entry.o kernel.o panic.o tty_serial.o tty_vga.o tty_system.o gdt.o allocator.o liballoc.o liballoc_hooks.o keyboard.o filesystem.o interrupts.o interrupt_routines.o cpio.o initrd.o elf.o pic.o process.o timer.o net.o init.o posix.o cstdlib.o misc.o stack_operator.o libstdc++.o ram_fs.o pseudo_fs.o
	$(CXX) -T link.ld -o $@ -O2 $^ -nostartfiles

entry.o: entry.s
	$(AS) $^ -o $@

initrd.o: initrd.cpio
	objcopy -I binary -O elf32-i386 $^ $@

initrd.cpio: libsys/libsys.a busybox/busybox
	cd initrd && find . | cpio -ov > ../initrd.cpio

libsys/libsys.a:
	make -C libsys/ libsys.a

busybox/busybox: libsys/libsys.a
	cp libsys/crt0.o busybox/
	rm -rf busybox/busybox
	rm -rf busybox/busybox_unstripped
	PROJECT_ROOT="$(PWD)" make -C busybox -j8
	make -C busybox install CONFIG_PREFIX=../initrd

.PHONY: initrd.cpio busybox/busybox

clean:
	rm -rf smos.bin
	rm -rf *.o
	rm -rf initrd.cpio
	rm -rf libsys/libsys.a
	rm -rf libsys/*.o
