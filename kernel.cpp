#include <cstdint>
#include <allocator.h>
#include <cpio.h>
#include <elf.h>
#include <pic.h>
#include <process.h>
#include <timer.h>
#include <ports.h>
#include <net.h>
#include <keyboard.h>
#include <interrupts.h>
#include <filesystem.h>
#include "include/tty_vga.h"
#include "include/tty_serial.h"
#include "include/boot_info.h"

// TODO: move somewhere else
#if defined(__linux__)
#error "You are not using a cross-compiler"
#endif
 
#if !defined(__i386__)
#error "Wrong architecture. i386 is required"
#endif

// TODO: move somewhere else
extern uint8_t _initrd_begin;

void kernel_main(boot_info_t* boot_info) {
    assembly::clear_interrupts();

	allocator_init(boot_info);

    tty_vga_init();
    tty_serial_init();

	init_interrupt_system();

    init_processes();

    init_filesystem();

    init_networking();

	// init_pci();

    init_pic();

    init_timer();

    init_keyboard();

    cpio_extractor initrd(&_initrd_begin);
    initrd.extract("/");

    printf("\x1b[0;35mKnee kernel\x1b[0m successfully loaded\n");

    execute_elf("/bin/busybox", {"ash"}, true);

    while (true)
        schedule();
}