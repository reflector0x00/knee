#pragma once

// from Linux sources

struct vm86_regs {
/*
 * normal regs, with special meaning for the segment descriptors..
 */
    long ebx;
    long ecx;
    long edx;
    long esi;
    long edi;
    long ebp;
    long eax;
    long __null_ds;
    long __null_es;
    long __null_fs;
    long __null_gs;
    long orig_eax;
    long eip;
    unsigned short cs, __csh;
    long eflags;
    long esp;
    unsigned short ss, __ssh;
/*
 * these are specific to v86 mode:
 */
    unsigned short es, __esh;
    unsigned short ds, __dsh;
    unsigned short fs, __fsh;
    unsigned short gs, __gsh;
};

struct revectored_struct {
    unsigned long __map[8];			/* 256 bits */
};

struct vm86_struct {
    struct vm86_regs regs;
    unsigned long flags;
    unsigned long screen_bitmap;		/* unused, preserved by vm86() */
    unsigned long cpu_type;
    struct revectored_struct int_revectored;
    struct revectored_struct int21_revectored;
};
