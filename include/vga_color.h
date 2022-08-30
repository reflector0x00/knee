#pragma once

/* Hardware text mode color constants. */
// TODO: naming
enum vga_color {
    vga_color_black = 0,
    vga_color_blue = 1,
    vga_color_green = 2,
    vga_color_cyan = 3,
    vga_color_red = 4,
    vga_color_magenta = 5,
    vga_color_brown = 6,
    vga_color_light_grey = 7,
    vga_color_dark_grey = 8,
    vga_color_light_blue = 9,
    vga_color_light_green = 10,
    vga_color_light_cyan = 11,
    vga_color_light_red = 12,
    vga_color_light_magenta = 13,
    vga_color_light_brown = 14,
    vga_color_white = 15,
};

struct [[gnu::packed]] vga_color_t {
    vga_color foreground : 4 {};
    vga_color background : 4 {};

    vga_color_t() = default;
    vga_color_t(const vga_color_t&) = default;
    vga_color_t(vga_color_t&&) = default;
    vga_color_t(vga_color _foreground, vga_color _background) :
            foreground(_foreground),
            background(_background) {}
    vga_color_t& operator=(const vga_color_t&) = default;
    vga_color_t& operator=(vga_color_t&&) = default;
};

static_assert(sizeof(vga_color_t) == 1);