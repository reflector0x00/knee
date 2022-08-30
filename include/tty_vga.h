#pragma once
#include <cstdint>
#include "tty_system.h"
#include "vga_color.h"


static constexpr std::size_t vga_width = 80;
static constexpr std::size_t vga_height = 25;

struct [[gnu::packed]] vga_entry {
    uint8_t symbol{};
    vga_color_t color{};
    vga_entry() = default;
    vga_entry(const vga_entry&) = default;
    vga_entry(vga_entry&&) = default;
    vga_entry(uint8_t _symbol, vga_color_t _color) : symbol(_symbol), color(_color) {}
    vga_entry& operator=(const vga_entry&) = default;
    vga_entry& operator=(vga_entry&&) = default;
};

static_assert(sizeof(vga_entry) == 2);


class tty_vga : public tty_descriptor {
    std::size_t _row;
    std::size_t _column;
    vga_color_t _color;
    vga_entry* _buffer;

    void update_cursor(std::size_t x, std::size_t y);

public:
    explicit tty_vga(vga_entry* buffer) :
            _row(0),
            _column(0),
            _color(vga_color_light_grey, vga_color_black),
            _buffer(buffer) {}

    void write_at(char c, vga_color_t color, size_t x, size_t y);
    void scroll_line_up();
    bool init() override;
    bool set_color(vga_color_t color) override;
    bool backspace() override;
    bool clear_till_end() override;
    bool write_char(char c) override;

};


void tty_vga_init();
