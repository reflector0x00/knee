#pragma once
#include <cstdint>
#include <cstring>
#include <list>
#include "vga_color.h"

struct tty_descriptor {
    virtual bool init() = 0;
    virtual bool write_char(char c) = 0;
    virtual bool set_color(vga_color_t color); // TODO: find better way
    virtual bool backspace() = 0;
    virtual bool clear_till_end() = 0;
};

class _tty_system {
    std::list<tty_descriptor*> _outputs;
    _tty_system() = default;
public:
    _tty_system(const _tty_system&) = delete;
    void operator=(const _tty_system&) = delete;

    static _tty_system& instance();

    void set_color(vga_color_t color); // TODO: find better way
    void backspace();
    void clear_till_end();
    void register_output(tty_descriptor* desc);
    void write_char(char c);
    void write_data(const char* data, std::size_t size);
    void write_string(const char* string);
};


// short pseudonym
[[gnu::always_inline]]
static inline _tty_system& tty_system() {
    return _tty_system::instance();
}

void printk(const char* fmt, ...);
