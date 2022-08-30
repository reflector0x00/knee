#pragma once
#include <cstdint>
#include "ports.h"

struct pic_port {
    io_port8_t command;
    io_port8_t data;
};

enum pic_int_vector_t {
    piciv_timer = 0,
    piciv_ps2_keyboard = 1,
};

constexpr uint16_t pic_master_address = 0x20;
constexpr uint16_t pic_slave_address = 0xA0;

constexpr std::size_t pic_master_vector_offset = 32;
constexpr std::size_t pic_slave_vector_offset = 40;


enum pic_command_t {
    picc_icw1_icw4 = (1 << 0), // ICW4 (not) needed
    picc_icw1_single = (1 << 1), // Single (cascade) mode
    picc_icw1_interval4 = (1 << 2), // Call address interval 4 (8)
    picc_icw1_level = (1 << 3), // Level triggered (edge) mode
    picc_icw1_init = (1 << 4), // Initialization

    picc_icw4_8086 = (1 << 0), // 8086/88 (MCS-80/85) mode
    picc_icw4_auto = (1 << 1), // Auto (normal) EOI
    picc_icw4_buf_slave = (1 << 2), // Buffered mode/slave
    picc_icw4_buf_master = (1 << 3), // Buffered mode/master
    picc_icw4_sfnm = (1 << 4), // Special fully nested (not)

    picc_end_of_interrupt = (1 << 5)
};

constexpr pic_command_t operator|(pic_command_t a, pic_command_t b) {
    return static_cast<pic_command_t>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}


class pic_t {
    pic_port* _master;
    pic_port* _slave;
public:
    explicit pic_t(uint16_t master_address = 0, uint16_t slave_address = 0);

    void remap(uint8_t master_offset, uint8_t slave_offser);
    void mask_interrupt(uint8_t vector);
    void unmask_interrupt(uint8_t vector);
    void end_of_interrupt(uint8_t vector);
};



void pic_mask_interrupt(uint8_t vector);
void pic_unmask_interrupt(uint8_t vector);
void pic_end_of_interrupt(uint8_t vector);

void init_pic();