#pragma once
#include "gdt.h"
#include "pic.h"

enum gate_t {
    gate_zero = 0x0,

    gate_task = 0x5,
    gate_interrupt16 = 0x6,
    gate_trap16 = 0x7,
    gate_interrupt32 = 0xE,
    gate_trap32 = 0xF,
};

struct [[gnu::packed]] gate_descriptor {
    uint16_t offset_low;
    gdt_segment_t segment_selector : 16;
    uint8_t reserved0 {};
    gate_t gate_type : 4;
    uint8_t reserved1 : 1 {};
    dpl_t dpl : 2;
    bool present : 1;
    uint16_t offset_high;

    constexpr gate_descriptor() :
            offset_low(0),
            segment_selector(gdt_segment_t::zero),
            gate_type(gate_zero),
            dpl(dpl_ring0),
            present(false),
            offset_high(0) {}

    gate_descriptor(ptr_t offset,
                    gdt_segment_t _segment_selector,
                    gate_t _gate_type,
                    dpl_t _dpl,
                    bool _present = true) :
            offset_low(offset),
            segment_selector(_segment_selector),
            gate_type(_gate_type),
            dpl(_dpl),
            present(_present),
            offset_high(offset >> 16) {}

    gate_descriptor(const gate_descriptor&) = default;
    gate_descriptor(gate_descriptor&&) = default;

    gate_descriptor& operator=(gate_descriptor&&) = default;
    gate_descriptor& operator=(const gate_descriptor&) = default;
};

static_assert(sizeof(gate_descriptor) == 8);


struct interrupt_frame_ring0 {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

struct interrupt_frame : public interrupt_frame_ring0 {
    uint32_t esp;
    uint32_t ss;

    [[nodiscard]] bool is_ring0() const {
        return (cs & 0b11) == dpl_ring0;
    }
    [[nodiscard]] ptr_t get_ring0_esp() const {
        // "esp" and "ss" aren't valid if interrupt occurred in ring 0
        // so previous "esp" value is the address of "esp" field of the interrupt_frame
        return &esp;
    }
};

static_assert(sizeof(interrupt_frame) == sizeof(uint32_t)*5);


typedef void (*exception_handler_t)(interrupt_frame *frame, uint32_t error_code);
typedef void (*interrupt_handler_t)(interrupt_frame *frame);


enum int_vector_t {
    iv_general_protection = 13,
    iv_page_fault = 14,

    iv_timer = pic_master_vector_offset + piciv_timer,
    iv_ps2_keyboard = pic_master_vector_offset + piciv_ps2_keyboard,

    iv_interruptions_count = 64,

    iv_system_call = 0x81
};

void set_exception_handler(std::size_t n, exception_handler_t handler);
void set_interrupt_handler(std::size_t n, interrupt_handler_t handler);

void init_interrupt_system();