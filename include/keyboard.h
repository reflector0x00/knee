#pragma once
#include <cstdint>
#include "ports.h"

union ps2_status_byte {
    struct {
        bool output_buffer_status: 1;
        bool input_buffer_status: 1;
        bool system_flag: 1;
        bool command_data: 1;
        bool unknown_0: 1;
        bool unknown_1: 1;
        bool time_out_error: 1;
        bool parity_error: 1;
    };
    uint8_t data;

    ps2_status_byte() : data(0) {}
    ps2_status_byte(uint8_t _data) : data(_data) {}

    operator uint8_t() {
        return data;
    }
};

static_assert(sizeof(ps2_status_byte) == 1);

union ps2_configuration_byte {
    struct {
        bool first_port_interrupt: 1;
        bool second_port_interrupt: 1;
        bool system_flag: 1;
        bool zero0: 1;
        bool first_port_clock: 1;
        bool second_port_clock: 1;
        bool first_port_translation: 1;
        bool zero1: 1;
    };
    uint8_t data;

    ps2_configuration_byte() : data(0) {}
    ps2_configuration_byte(uint8_t _data) : data(_data) {
    }
    operator uint8_t() {
        return data;
    }
};


struct [[gnu::packed]] ps2_controller_port {
    io_port8_t data;
    uint8_t _padding[3];
    union {
        io_port8_tmpl_t<ps2_status_byte> status;
        io_port8_t command;
    };
};



enum scancode {
    sc_F1 = 0x07,
    sc_ESCAPE = 0x08,
    sc_TAB = 0x0D,
    sc_tilde = 0x0E,
    sc_F2 = 0x0F,
    sc_L_CTRL = 0x11,
    sc_L_SHIFT = 0x12,
    sc_CAPS = 0x14,
    sc_q = 0x15,
    sc_1 = 0x16,
    sc_F3 = 0x17,
    sc_L_ALT = 0x19,
    sc_Z = 0x1A,
    sc_S = 0x1B,
    sc_A = 0x1C,
    sc_W = 0x1D,
    sc_2 = 0x1E,
    sc_F4 = 0x1F,
    sc_C = 0x21,
    sc_X = 0x22,
    sc_D = 0x23,
    sc_E = 0x24,
    sc_4 = 0x25,
    sc_3 = 0x26,
    sc_F5 = 0x27,
    sc_SPACE = 0x29,
    sc_V = 0x2A,
    sc_F = 0x2B,
    sc_T = 0x2C,
    sc_R = 0x2D,
    sc_5 = 0x2E,
    sc_F6 = 0x2F,

    sc_N = 0x31,
    sc_B = 0x32,
    sc_H,
    sc_G,
    sc_Y,
    sc_6,
    sc_F7,

    sc_R_ALT = 0x39,
    sc_M,
    sc_J,
    sc_U,
    sc_7,
    sc_8,
    sc_F8,

    sc_comma = 0x41,
    sc_K,
    sc_I, // F0,48
    sc_O,
    sc_0,
    sc_9,
    sc_F9,

    sc_dot = 0x49,
    sc_slash,
    sc_L,
    sc_semicolon,
    sc_P,
    sc_minus,
    sc_F10,

    sc_apostrophe = 0x52,

    sc_L_square_br = 0x54,
    sc_equal,
    sc_F11,
    sc_PRNT_SCRN,
    sc_R_CTRL,
    sc_R_SHIFT,
    sc_ENTER,
    sc_R_square_br,
    sc_backslash,

    sc_F12 = 0x5E,
    sc_scroll,
    sc_D_ARROW = 0x60,
    sc_L_ARROW,
    sc_PAUSE,
    sc_U_ARROW,
    sc_DELETE,
    sc_END,
    sc_BACKSPACE = 0x66,
    sc_INSERT,

    sc_kp_1 = 0x69,
    sc_R_ARROW,
    sc_kp_4,
    sc_kp_7,
    sc_PG_DN,
    sc_HOME,
    sc_PG_UP,
    sc_kp_0 = 0x70,
    sc_kp_dot,
    sc_kp_2,
    sc_kp_5,
    sc_kp_6,
    sc_kp_8,
    sc_NUM,

    sc_kp_EN = 0x79,
    sc_kp_3,

    sc_kp_plus = 0x7C,
    sc_kp_9,
    sc_asterisk,


    sc_L_WIN = 0x8B,
    sc_R_WIN,
    sc_APPS,
};

class ps2_controller {
    enum ps2_controller_command {
        ps2cc_read_ram_0 = 0x20,
        ps2cc_write_ram_0 = 0x60,
        ps2cc_enable_first_port = 0xAE,
        ps2cc_enable_second_port = 0xA8,
        ps2cc_disable_first_port = 0xAD,
        ps2cc_disable_second_port = 0xA7,
        ps2cc_test_controller = 0xAA,
        ps2cc_test_first_port = 0xAB,
        ps2cc_test_second_port = 0xA9,
    };

    enum ps2_keyboard_command {
        ps2kc_set_scan_code_set = 0xF0,
        ps2kc_identify_keyboard = 0xF2,
        ps2kc_enable_scanning = 0xF4,
        ps2kc_reset = 0xFF
    };
    enum ps2_answers {
        ps2a_self_test = 0x55,
        ps2a_acknowledged = 0xFA,
    };
    enum scan_code_set {
        scan_code_1 = 1,
        scan_code_2 = 2,
        scan_code_3 = 3,
    };

    ps2_controller_port* _port;
public:
    explicit ps2_controller(uint16_t address = 0); // TODO: ptr_t

    void init_keyboard();

    void wait_input_available();
    void wait_output_available();
    uint8_t read_data_unchecked();
    uint8_t read_data();
    void write_data(uint8_t data);
    ps2_status_byte read_status();
    void write_command(uint8_t command);

    // TODO: inline these
    ps2_configuration_byte read_configuration_byte();
    void write_configuration_byte(ps2_configuration_byte config);
    void first_port_enable();
    void second_port_enable();
    void first_port_disable();
    void second_port_disable();
    void flush_output_buffer();
    uint8_t run_self_test();
    uint8_t run_first_port_test();
    uint8_t run_second_port_test();
    uint8_t first_port_send(uint8_t command);
    uint8_t first_port_send(uint8_t command, uint8_t data);
};

constexpr uint16_t ps2_controller_address = 0x60;
constexpr uint8_t release_default_keycode = 0xF0;
constexpr std::size_t keyboard_circular_buffer = 256;

char read_key();
bool keyboard_read_available();

void init_keyboard();
