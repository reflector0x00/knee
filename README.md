# Knee kernel

Knee is a kernel for i686-based PCs written in C++. Released under BSD-3 license.

## Requirements

Any Linux-based distributive (Ubuntu is recommended). FreeBSD (and other unix-like OS), Cygwin and MingW (MSYS) haven't been tested.

## Build

The simplest way to build is to use bootstrap scripts:
* bootstrap_compiler.sh - Downloads and builds crosstool-ng. Then configures and builds cross-compiler for kernel and software building.
* bootstrap_busybox.sh - Downloads and configures Busybox (shortened setup), but not builds (root Makefile does).

sh bootstrap_compiler.sh
sh bootstrap_busybox.sh
Make sure that both commands were executed without errors (rerun in case of errors).

Then run make:
make
It will start the following sequence of building stages:
* Building libsys - library required for building software like busybox
* Building Busybox
* Installing Busybox in initrd directory
* Making initrd.cpio from contents of initrd directory
* Building kernel ELF knee.bin (links with initrd.cpio)

## Run

You can run knee.bin with Qemu:
qemu-system-i386.exe -kernel knee.bin -m 1024 -M type=pc-i440fx-3.1 -vga std