#pragma once

#include "ptr.h"

enum gdt_indices_t {
    index_zero = 0,
    index_kernel_code = 1,
    index_kernel_data = 2,
    index_user_code = 3,
    index_user_data = 4,
    index_task = 5,
    gdt_count = 6
};


// TODO: move
// Descriptor privilege level
enum dpl_t {
    dpl_ring0 = 0,
    dpl_ring1 = 1,
    dpl_ring2 = 2,
    dpl_ring3 = 3
};

// TODO: move
enum eflags_bit_t {
    eb_carry = (1 << 0),
    eb_parity = (1 << 2),
    eb_auxiliary = (1 << 4),
    eb_zero = (1 << 6),
    eb_sign = (1 << 7),
    eb_trap = (1 << 8),
    eb_int_enable = (1 << 9),
    eb_direction = (1 << 10),
    eb_overflow = (1 << 11),
    eb_iopl_ring0 = (dpl_ring0 << 12),
    eb_iopl_ring1 = (dpl_ring1 << 12),
    eb_iopl_ring2 = (dpl_ring2 << 12),
    eb_iopl_ring3 = (dpl_ring3 << 12),
    eb_nested_task = (1 << 14),
    eb_resume = (1 << 16),
    eb_virtual_8086_mode = (1 << 17),
    eb_alignment_check = (1 << 18),
    eb_virtual_int = (1 << 19),
    eb_virtual_int_pending = (1 << 20),
    eb_cpuid_available = (1 << 21)
};


constexpr eflags_bit_t operator|(eflags_bit_t a, eflags_bit_t b) {
    return static_cast<eflags_bit_t>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}



enum access_byte_bits_t : uint8_t {
    ab_zero = 0,
    ab_access = 1 << 0,
    ab_read_write = 1 << 1,
    ab_direction = 1 << 2,
    ab_executable = 1 << 3,
    ab_descriptor_type = 1 << 4,
    ab_dpl_ring_0 = dpl_ring0 << 5,
    ab_dpl_ring_1 = dpl_ring1 << 5,
    ab_dpl_ring_2 = dpl_ring2 << 5,
    ab_dpl_ring_3 = dpl_ring3 << 5,
    ab_present = 1 << 7,
};

constexpr access_byte_bits_t operator|(access_byte_bits_t a, access_byte_bits_t b) {
    return static_cast<access_byte_bits_t>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}


enum flags_bits_t : uint8_t {
    f_zero = 0,
    f_reserved = 1 << 0,
    f_long_mode = 1 << 1,
    f_size = 1 << 2,
    f_granularity = 1 << 3,
};

constexpr flags_bits_t operator|(flags_bits_t a, flags_bits_t b) {
    return static_cast<flags_bits_t>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}


struct [[gnu::packed]] segment_descriptor {
    uint16_t limit_low;
    uint32_t base_low : 24;
    union {
        access_byte_bits_t access_byte;
        struct [[gnu::packed]] {
            bool access : 1;
            bool read_write : 1;
            bool direction : 1;
            bool executable : 1;
            bool descriptor_type : 1;
            dpl_t privileges : 2;
            bool present : 1;
        };
    };
    union {
        struct {
            uint8_t limit_high: 4;
            flags_bits_t flags: 4;
        };
        struct {
            uint8_t _: 4; // alias to limit_high
            bool reserved : 1;
            bool long_mode : 1;
            bool size : 1;
            bool granularity : 1;
        };
    };
    uint8_t base_high;

    segment_descriptor(ptr_t base = 0UL, uint32_t limit = 0, access_byte_bits_t _access_byte = ab_zero, flags_bits_t _flags = f_zero) :
            limit_low(limit & 0xFFFF),
            base_low(base & 0xFFFFFF),
            access_byte(_access_byte),
            limit_high(limit >> 16),
            flags(_flags),
            base_high(base >> 24) {}

    segment_descriptor& operator=(segment_descriptor&& other) = default;
};

static_assert(sizeof(segment_descriptor) == 8);



struct [[gnu::packed]] task_state_segment {
    uint32_t prev_tss{}; // The previous TSS - with hardware task switching these form a kind of backward linked list.
    uint32_t esp0{};     // The stack pointer to load when changing to kernel mode.
    uint32_t ss0{};      // The stack segment to load when changing to kernel mode.
    // Everything below here is unused.
    uint32_t esp1{}; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
    uint32_t ss1{};
    uint32_t esp2{};
    uint32_t ss2{};
    uint32_t cr3{};
    uint32_t eip{};
    uint32_t eflags{};
    uint32_t eax{};
    uint32_t ecx{};
    uint32_t edx{};
    uint32_t ebx{};
    uint32_t esp{};
    uint32_t ebp{};
    uint32_t esi{};
    uint32_t edi{};
    uint32_t es{};
    uint32_t cs{};
    uint32_t ss{};
    uint32_t ds{};
    uint32_t fs{};
    uint32_t gs{};
    uint32_t ldt{};
    uint16_t trap{};
    uint16_t iomap_base{};
    uint32_t ssp{};
};

static_assert(sizeof(task_state_segment) == 0x6c);

enum gdt_segment_t {
    zero = index_zero * sizeof(segment_descriptor),
    kernel_cs = (index_kernel_code * sizeof(segment_descriptor)) | dpl_ring0,
    kernel_ds = (index_kernel_data * sizeof(segment_descriptor)) | dpl_ring0,
    user_cs = (index_user_code * sizeof(segment_descriptor)) | dpl_ring3,
    user_ds = (index_user_data * sizeof(segment_descriptor)) | dpl_ring3,
    task_seg = index_task * sizeof(segment_descriptor) | dpl_ring0,
};

constexpr access_byte_bits_t access_kernel_code = ab_read_write | ab_executable | ab_dpl_ring_0 | ab_descriptor_type | ab_present;
constexpr access_byte_bits_t access_kernel_data = ab_read_write | ab_dpl_ring_0 | ab_descriptor_type | ab_present;
constexpr access_byte_bits_t access_user_code = ab_read_write | ab_executable | ab_dpl_ring_3 | ab_descriptor_type | ab_present;
constexpr access_byte_bits_t access_user_data = ab_read_write | ab_dpl_ring_3 | ab_descriptor_type | ab_present;
constexpr access_byte_bits_t access_task =
        ab_access  // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
        /*& (~ab_read_write)*/  // For a TSS, indicates busy (1) or not busy (0).
        /*& (~ab_direction)*/ // always 0 for TSS
        | ab_executable  // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
        | ab_dpl_ring_0
        /*& (~ab_descriptor_type)*/ // indicates TSS/LDT
        | ab_present;

constexpr flags_bits_t flags_general = f_size | f_granularity;
constexpr flags_bits_t flags_task = f_zero; // size should be zero; limit is in bytes, not pages

constexpr ptr_t flat_base = 0UL;
constexpr uint32_t flat_limit = 0x000FFFFF;

void set_tss_kernel_stack(uint32_t stack);
void gdt_init();