#pragma once
#include <cstdint>
#include "ports.h"

constexpr std::size_t timer_base_frequency = 3579545;
constexpr std::size_t timer_frequency = 1000; // 1 kHz = 1 ms
constexpr uint16_t timer_reload_value = timer_base_frequency / timer_frequency;
constexpr std::size_t quantum = 20; // ms
constexpr uint16_t timer_address = 0x40;


enum timer_channel_t {
    tc_channel_0 = 0,
    tc_channel_1 = 1,
    tc_channel_2 = 2,
    tc_read_back = 3,
};

enum timer_access_mode_t {
    tam_latch_count_value = 0,
    tam_low_byte = 1,
    tam_high_byte = 2,
    tam_low_high_byte = 3,
};

enum timer_operating_mode_t {
    tom_interrupt_on_terminal_count = 0,
    tom_hardware_retriggerable_oneshot = 1,
    tom_rate_generator = 2,
    tom_square_wave_generator = 3,
    tom_software_triggered_strobe = 4,
    tom_hardware_triggered_strobe = 5,
};

enum timer_mode_t {
    tm_bcd = (1 << 0),

    tm_interrupt_on_terminal_count = (tom_interrupt_on_terminal_count << 1),
    tm_hardware_retriggerable_oneshot = (tom_hardware_retriggerable_oneshot << 1),
    tm_rate_generator = (tom_rate_generator << 1),
    tm_square_wave_generator = (tom_square_wave_generator << 1),
    tm_software_triggered_strobe = (tom_software_triggered_strobe << 1),
    tm_hardware_triggered_strobe = (tom_hardware_triggered_strobe << 1),

    tm_latch_count_value = (tam_latch_count_value << 4),
    tm_low_byte = (tam_low_byte << 4),
    tm_high_byte = (tam_high_byte << 4),
    tm_low_high_byte = (tam_low_high_byte << 4),

    tm_channel_0 = (tc_channel_0 << 6),
    tm_channel_1 = (tc_channel_1 << 6),
    tm_channel_2 = (tc_channel_2 << 6),
    tm_read_back = (tc_read_back << 6),
};

constexpr timer_mode_t operator|(timer_mode_t a, timer_mode_t b) {
    return static_cast<timer_mode_t>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}


struct timer_port {
    io_port8_t channel_0;
    io_port8_t channel_1;
    io_port8_t channel_2;
    io_port8_t mode;
};

static_assert(sizeof(timer_port) == 4);


// TODO: solve types conflict
class _timer_t {
    timer_port* _port;
public:
    explicit _timer_t(uint16_t address = 0);
    void set_rate_generator(uint16_t reload_value);
};


void init_timer();