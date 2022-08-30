#pragma once
#include <cstdint>

static inline void set_ds(uint32_t cs) {
    asm volatile(
            "mov %0, %%ax\n"
            "mov %%ax, %%ds" : : "m"(cs) : "ax");
}
static inline void set_es(uint32_t es) {
    asm volatile(
            "mov %0, %%ax\n"
            "mov %%ax, %%es" : : "m"(es) : "ax");
}

static inline void set_fs(uint32_t fs) {
    asm volatile(
            "mov %0, %%ax\n"
            "mov %%ax, %%fs" : : "m"(fs) : "ax");
}

static inline void set_gs(uint32_t gs) {
    asm volatile(
            "mov %0, %%ax\n"
            "mov %%ax, %%gs" : : "m"(gs) : "ax");
}

static inline uint32_t get_flags() {
    uint32_t flags;
    asm volatile("pushf\n"
                 "pop %0" : "=r"(flags));
    return flags;
}

static inline uint32_t get_cs() {
    uint32_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return cs;
}


static inline uint32_t get_ss() {
    uint32_t ss;
    asm volatile("mov %%ss, %0" : "=r"(ss));
    return ss;
}


static inline uint32_t get_cr2() {
    uint32_t ss;
    asm volatile("mov %%cr2, %0" : "=r"(ss));
    return ss;
}