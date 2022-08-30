#include <tty_vga.h>
#include <tty_system.h>
#include <ports.h>


void tty_vga::update_cursor(std::size_t x, std::size_t y) {
    uint16_t pos = y * vga_width + x;

    // TODO: rewrite it as struct-port-access
    assembly::port_out8(0x3D4, 0x0F);
    assembly::port_out8(0x3D5, pos & 0xFF);
    assembly::port_out8(0x3D4, 0x0E);
    assembly::port_out8(0x3D5, pos >> 8);
}

bool tty_vga::init() {
    for (std::size_t y = 0; y < vga_height; y++)
        for (std::size_t x = 0; x < vga_width; x++)
            _buffer[y * vga_width + x] = vga_entry(' ', _color);

    update_cursor(_column, _row);
    return true;
}

bool tty_vga::set_color(vga_color_t color) {
    _color = color;
    return true;
}

void tty_vga::write_at(char c, vga_color_t color, size_t x, size_t y) {
    _buffer[y * vga_width + x] = vga_entry(c, color);
}

void tty_vga::scroll_line_up() {
    for (std::size_t y = 1; y < vga_height; ++y)
        for(std::size_t x = 0; x < vga_width; ++x)
            _buffer[(y - 1) * vga_width + x] = _buffer[y * vga_width + x];

    for(std::size_t x = 0; x < vga_width; ++x)
        _buffer[(vga_height - 1) * vga_width + x] = vga_entry(' ', _color);
}

bool tty_vga::backspace() {
    if (_column == 0)
        return true;
    --_column;
    update_cursor(_column, _row);
    return true;
}

bool tty_vga::clear_till_end() {
    for (size_t x = _column; x < vga_width; ++x)
        write_at(' ', _color, x, _row);
    return true;
}

bool tty_vga::write_char(char c) {
    switch (c) {
        case '\n':
            _column = -1;
            ++_row;
            break;

        case '\a': // bell
            return true;

        case '\r':
            _column = -1;
            break;


        default:
            write_at(c, _color, _column, _row);
    }

    if (++_column == vga_width) {
        _column = 0;
        ++_row;
    }
    if (_row == vga_height) {
        --_row;
        scroll_line_up();
    }
    update_cursor(_column, _row);
    return true;
}

extern vga_entry vga_console;

void tty_vga_init() {
    static tty_vga vga(&vga_console);
    tty_system().register_output(&vga);
}