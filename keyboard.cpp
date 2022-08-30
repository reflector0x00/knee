#include <keyboard.h>
#include <cstdint>
#include <allocator.h>
#include <tty_system.h>
#include <panic.h>
#include <ports.h>
#include <pic.h>
#include <interrupt_routines.h>
#include <assembly.h>
#include <circular_buffer.h>


static const char atkbd_set3_keycode_normal[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, '\t', '`', 0,
        0, 0, 0, 0, 0, 'q', '1', 0,
        0, 0, 'z', 's', 'a', 'w', '2', 0,
        0,'c', 'x', 'd', 'e', '4', '3', 0,
        0, ' ', 'v', 'f', 't', 'r', '5', 0,
        0, 'n', 'b', 'h', 'g', 'y', '6', 0,
        0, 0, 'm', 'j','u', '7', '8', 0,
        0, ',', 'k', 'i', 'o', '0', '9', 0,
        0, '.', '/', 'l', ';', 'p', '-', 0,
        0, 0, '\'', 0, '[', '=', 0, 0,
        0, 0, '\n', ']', '\\',0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
};

static const char atkbd_set3_keycode_shifted[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, '\t', '~', 0,
        0, 0, 0, 0, 0, 'Q', '!', 0,
        0, 0, 'Z', 'S', 'A', 'W', '@', 0,
        0,'C', 'X', 'D', 'E', '$', '#', 0,
        0, ' ', 'V', 'F', 'T', 'R', '%', 0,
        0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
        0, 0, 'M', 'J','U', '&', '*', 0,
        0, '<', 'K', 'I', 'O', ')', '(', 0,
        0, '>', '?', 'L', ':', 'P', '_', 0,
        0, 0, '"', 0, '{', '+', 0, 0,
        0, 0, '\n', '}', '|',0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
};


static ps2_controller ps2;


// TODO: event-based (not char-) system
circular_buffer<char, keyboard_circular_buffer> keyboard_buffer;

bool keyboard_read_available() {
    return keyboard_buffer.available();
}

char read_key() {
    return keyboard_buffer.read();
}

void write_key(char key) {
    keyboard_buffer.write(key);
}

void write_keystring(const char* keys) {
    keyboard_buffer.write(keys, strlen(keys));
}

// TODO: rewrite this
static bool release = false;
static bool shifted = false;
static bool controled = false;

static void keyboard_interrupt(interrupt_frame *frame) {
    auto sym = ps2.read_data_unchecked();
    if (sym == release_default_keycode)
        release = true;
    else if (!release) {
        switch (sym) {
            case sc_L_ARROW:
                write_keystring("\x1b[D");
                break;
            case sc_R_ARROW:
                write_keystring("\x1b[C");
                break;
            case sc_U_ARROW:
                write_keystring("\x1b[A");
                break;
            case sc_D_ARROW:
                write_keystring("\x1b[B");
                break;

            case sc_BACKSPACE:
                write_key('\b');
                break;

            case sc_L_SHIFT:
            case sc_R_SHIFT:
                shifted = true;
                break;


            default:
                char key;
                if (shifted)
                    key = atkbd_set3_keycode_shifted[sym];
                else
                    key = atkbd_set3_keycode_normal[sym];
                write_key(key);
        }
    }
    else {
        switch (sym) {
            case sc_L_SHIFT:
            case sc_R_SHIFT:
                shifted = false;
                break;
        }

        release = false;
    }
}

ps2_controller::ps2_controller(uint16_t address) : _port(reinterpret_cast<ps2_controller_port*>(address)) {}

void ps2_controller::init_keyboard() {
    first_port_disable();
    second_port_disable();

    flush_output_buffer();
    auto config = read_configuration_byte();
    config.first_port_interrupt = false;
    config.second_port_interrupt = false;
    config.first_port_translation = false;
    write_configuration_byte(config);

    auto self_test = run_self_test();
    if (self_test != ps2a_self_test)
        panic("PS/2 self-test failed");

    auto first_port_test = run_first_port_test();
    if (first_port_test)
        panic("PS/2 first port test failed");
    first_port_enable();

    // reset keyboard
    if (first_port_send(ps2kc_reset) != ps2a_acknowledged)
        panic("PS/2 failed to reset device\n");
    read_data();

    if (first_port_send(ps2kc_identify_keyboard) != ps2a_acknowledged)
        panic("PS/2 keyboard not found\n");
    read_data();
    read_data();

    first_port_send(ps2kc_set_scan_code_set, scan_code_3);
    if (first_port_send(ps2kc_enable_scanning) != ps2a_acknowledged)
        panic("PS/2 failed to enable scanning\n");

    config = read_configuration_byte();
    config.first_port_interrupt = true;
    write_configuration_byte(config);
}

void ps2_controller::wait_input_available() {
    while (read_status().input_buffer_status);
}

void ps2_controller::wait_output_available() {
    while (!read_status().output_buffer_status);
}

uint8_t ps2_controller::read_data_unchecked() {
    return _port->data;
}

uint8_t ps2_controller::read_data() {
    wait_output_available();
    return _port->data;
}

void ps2_controller::write_data(uint8_t data) {
    wait_input_available();
    _port->data = data;
}

ps2_status_byte ps2_controller::read_status() {
    return _port->status;
}

void ps2_controller::write_command(uint8_t command) {
    wait_input_available();
    _port->command = command;
}

ps2_configuration_byte ps2_controller::read_configuration_byte() {
    write_command(ps2cc_read_ram_0);
    return {read_data()};
}

void ps2_controller::write_configuration_byte(ps2_configuration_byte config) {
    write_command(ps2cc_write_ram_0);
    write_data(config);
}

void ps2_controller::first_port_enable() {
    write_command(ps2cc_enable_first_port);
}

void ps2_controller::second_port_enable() {
    write_command(ps2cc_enable_second_port);
}

void ps2_controller::first_port_disable() {
    write_command(ps2cc_disable_first_port);
}

void ps2_controller::second_port_disable() {
    write_command(ps2cc_disable_second_port);
}

void ps2_controller::flush_output_buffer() {
    while (read_status().output_buffer_status)
        read_data();
}

uint8_t ps2_controller::run_self_test() {
    write_command(ps2cc_test_controller);
    return read_data();
}

uint8_t ps2_controller::run_first_port_test() {
    write_command(ps2cc_test_first_port);
    return read_data();
}

uint8_t ps2_controller::run_second_port_test() {
    write_command(ps2cc_test_second_port);
    return read_data();
}

uint8_t ps2_controller::first_port_send(uint8_t command) {
    write_data(command);
    return read_data();
}

uint8_t ps2_controller::first_port_send(uint8_t command, uint8_t data) {
    write_data(command);
    write_data(data);
    return read_data();
}


void init_keyboard() {
    new(&ps2) ps2_controller(ps2_controller_address);
    new(&keyboard_buffer) circular_buffer<char, keyboard_circular_buffer>();

    ps2.init_keyboard();
    set_interrupt_handler(iv_ps2_keyboard, keyboard_interrupt);
    pic_unmask_interrupt(piciv_ps2_keyboard);
}
