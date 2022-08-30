#pragma once
#include <cstdint>
#include "ptr.h"

namespace assembly {
    namespace cr0 {
        constexpr uint32_t write_protect = 1 << 16;
        constexpr uint32_t paging = 1 << 31;
    }
    template <typename T>
    [[gnu::always_inline]]
    static inline void const_set_esp(T value) {
        asm volatile("mov %0, %%esp" :: "i"(value));
    }

    template <typename T = uint32_t>
    [[gnu::always_inline]]
    static inline T get_esp() {
        T result;
        asm volatile("mov %%esp, %0" : "=r"(result));
        return result;
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void set_cr3(T value) {
        asm volatile("mov %0, %%cr3" :: "r"(value));
    }

    template <typename T = uint32_t>
    [[gnu::always_inline]]
    static inline T get_cr3() {
        T result;
        asm volatile("mov %%cr3, %0" : "=r"(result));
        return result;
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void set_cr0(T value) {
        asm volatile("mov %0, %%cr0" :: "r"(value));
    }

    template <typename T = uint32_t>
    [[gnu::always_inline]]
    static inline T get_cr0() {
        T result;
        asm volatile("mov %%cr0, %0" : "=r"(result));
        return result;
    }

    template <typename T = uint32_t>
    [[gnu::always_inline]]
    static inline T get_ebx() {
        T result;
        asm volatile("mov %%ebx, %0" : "=r"(result));
        return result;
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void memory_set_ebx(T value) {
        asm volatile("mov %0, %%ebx" :: "m"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void const_jump(T address) {
        asm volatile("jmp %P0" :: "i"(address));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_jump(T address) {
        asm volatile("jmp *%0" :: "r"(address));
    }

    [[gnu::always_inline]]
    static inline void clear_interrupts() {
        asm volatile("cli");
    }

    [[gnu::always_inline]]
    static inline void set_interrupts() {
        asm volatile("sti");
    }


    [[gnu::always_inline]]
    static inline void halt() {
        asm volatile("hlt");
    }


    template <typename T>
    [[gnu::always_inline]]
    static inline void set_cs(T value) {
        asm volatile(R"(
            ljmp %0, $reload_cs
            reload_cs:
        )" :: "i"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_set_ds(T value) {
        asm volatile("mov %0, %%ds" :: "r"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_set_es(T value) {
        asm volatile("mov %0, %%es" :: "r"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_set_fs(T value) {
        asm volatile("mov %0, %%fs" :: "r"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_set_gs(T value) {
        asm volatile("mov %0, %%gs" :: "r"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void register_set_ss(T value) {
        asm volatile("mov %0, %%ss" :: "r"(value));
    }

    template <typename T>
    [[gnu::always_inline]]
    static inline void load_task_register(T value) {
        asm volatile("ltr %w0" :: "r"(value));
    }

    [[gnu::always_inline]]
    static inline void tlb_flush() {
        asm volatile(R"(
                        mov %%cr3, %%eax
                        mov %%eax, %%cr3
                    )" ::: "eax");
    }

    [[gnu::always_inline]]
    static inline void set_gdtr(uint16_t size, ptr_t base) {
        static struct [[gnu::packed]] {
            uint16_t size;
            uint32_t base;
        } gdtr {
            size,
            base
        };
        asm("lgdt %0" :: "m" (gdtr));
    }

    [[gnu::always_inline]]
    static inline void set_idtr(uint16_t size, ptr_t base) {
        static struct [[gnu::packed]] {
            uint16_t size;
            uint32_t base;
        } idtr {
                size,
                base
        };
        asm("lidt %0" :: "m" (idtr));
    }

    [[gnu::always_inline]]
    static inline void port_out8(uint16_t port, uint8_t val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }

    [[gnu::always_inline]]
    static inline void port_out16(uint16_t port, uint16_t val) {
        asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
    }

    [[gnu::always_inline]]
    static inline void port_out32(uint16_t port, uint32_t val) {
        asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
    }

    [[gnu::always_inline]]
    static inline uint8_t port_in8(uint16_t port) {
        uint8_t result;
        asm volatile( "inb %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }

    [[gnu::always_inline]]
    static inline uint16_t port_in16(uint16_t port) {
        uint16_t result;
        asm volatile( "inw %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }

    [[gnu::always_inline]]
    static inline uint32_t port_in32(uint16_t port) {
        uint32_t result;
        asm volatile( "inl %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }

    [[gnu::always_inline]]
    static inline void io_wait() {
        port_out8(0x80, 0);
    }

}
