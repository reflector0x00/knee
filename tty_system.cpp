#include <tty_system.h>
#include <cstdio>
#include <cstdarg>
#include <panic.h>


bool tty_descriptor::set_color(vga_color_t color) {
    return false;
}

_tty_system &_tty_system::instance() {
    static _tty_system singleton;
    return singleton;
}

void _tty_system::register_output(tty_descriptor *desc) {
    if (!desc->init())
        panic("Failed to init");
    _outputs.push_back(desc);
}

void _tty_system::write_char(char c) {
    for (auto& output : _outputs)
        output->write_char(c);
}

void _tty_system::write_data(const char *data, std::size_t size) {
    for (auto& output : _outputs)
        for (std::size_t i = 0; i < size; i++)
            output->write_char(data[i]);
}

void _tty_system::write_string(const char *string) {
    for (auto& output : _outputs) {
        char c;
        for (auto ptr = string; (c = *ptr); ++ptr)
            output->write_char(c);
    }
}

void _tty_system::set_color(vga_color_t color) {
    for (auto& output : _outputs)
        output->set_color(color);
}

void _tty_system::backspace() {
    for (auto& output : _outputs)
        output->backspace();
}

void _tty_system::clear_till_end() {
    for (auto& output : _outputs)
        output->clear_till_end();
}


void printk(const char* fmt, ...) {
    static char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    tty_system().write_string(buffer);
    va_end(args);
}