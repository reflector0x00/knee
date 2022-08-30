CROSS_COMPILE=$(PWD)/toolchain/i686-elf/bin/i686-elf-
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
AS=$(CROSS_COMPILE)as
AR=$(CROSS_COMPILE)ar
CFLAGS=-I.
CXXFLAGS=-I.
all: libsys.a


libsys.a: crt0.o liballoc.o liballoc_hooks.o alloc.o syscalls.o
	$(AR) rcs $@ $^ 

