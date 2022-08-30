#include "include/pseudo_fs.h"
#include "include/keyboard.h"
#include "include/tty_system.h"
#include <sstream>

stdin_file::stdin_file(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t stdin_file::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    return 0;
}

std::size_t stdin_file::read(ptr_t buffer, std::size_t size, bool noblock) {
    if (noblock)
        panic("noblock on stdin");
    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = read_key();
        if (buffer[i] == '\n')
            return i + 1;
    }
    return size;
}

bool stdin_file::open() {
    return true;
}

bool stdin_file::close() {
    return true;
}

std::size_t stdin_file::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool stdin_file::is_file() {
    return true;
}

bool stdin_file::available_read() {
    return keyboard_read_available();
}

void stdout_file::sgr_reset() {
    tty_system().set_color(vga_color_t(vga_color_light_grey, vga_color_black));
}

void stdout_file::sgr(int n) {
    switch (n) {
        case 0:
            sgr_reset();
            break;

        default:
            printk("Unknown sgr n: %d\n", n);
            panic("");
    }
}

void stdout_file::sgr(int n, int m) {
    if (n < 0 || n > 1) {
        printk("Unknown sgr n: %d\n", n);
        panic("");
    }

    if (m < 30 || m > 37) {
        printk("Unknown sgr m: %d\n", m);
        panic("");
    }
    auto color = linux_colors[m - 30][n];
    tty_system().set_color(vga_color_t(color, vga_color_black));
}

void stdout_file::parse_cs(std::stringstream &ss) {
    char sym = ss.peek();
    if (!ss)
        return;

    switch (sym) {
        case 'm':
            ss.ignore();
            sgr(0);
            break;

        case 'J':
            ss.ignore();
            tty_system().clear_till_end();
            break;

        default: {
            if (!(sym >= '0' && sym <= '9'))
                printk("[unknown cs(3): %.1s]", &sym);

            int n;
            if (!(ss >> n))
                return;

            if (!(ss.get(sym)))
                return;

            switch (sym) {
                case 'm':
                    sgr(n);
                    break;

                case ';': {
                    int m;
                    if (!(ss >> m))
                        printk("[unknown cs(4): %.1s]", &sym);


                    if (!ss.get(sym))
                        return;

                    switch (sym) {
                        case 'm':
                            sgr(n, m);
                            break;

                        default:
                            printk("[unknown cs(2): %.1s]", &sym);
                    }
                    break;
                }

                default:
                    printk("[unknown cs: %.1s]", &sym);
            }
        }
    }
}

void stdout_file::parse_escape(std::stringstream &ss) {
    char sym;
    if (!ss.get(sym))
        return;

    switch (sym) {
        case '[':
            parse_cs(ss);
            break;

        default:
            printk("[unknown escape: %.1s (%02X)]", &sym, sym);
    }
}

stdout_file::stdout_file(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t stdout_file::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    if (noblock)
        panic("noblock on stdin");
    std::stringstream ss((std::string(buffer, size)));

    char sym;
    while (ss.get(sym)) {
        switch (sym) {
            case 0x1b:
                parse_escape(ss);
                break;

            case 0x08:
                tty_system().backspace();
                break;

            default:
                if ((sym >= ' ' && sym <= '~') || sym == '\n' || sym == '\r' || sym == '\a')
                    tty_system().write_char(sym);
                else
                    printk("\\x(%02X)", sym);
        }
    }
    return size;
}

std::size_t stdout_file::read(ptr_t buffer, std::size_t size, bool noblock) {
    return 0;
}

bool stdout_file::open() {
    return true;
}

bool stdout_file::close() {
    return true;
}

std::size_t stdout_file::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool stdout_file::is_file() {
    return true;
}

dev_mem::dev_mem(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t dev_mem::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    panic("write to mem");
}

std::size_t dev_mem::read(ptr_t buffer, std::size_t size, bool noblock) {
    panic("read from mem");
}

bool dev_mem::open() {
    return true;
}

bool dev_mem::close() {
    return true;
}

std::size_t dev_mem::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool dev_mem::is_file() {
    return true;
}

bool dev_mem::available_read() {
    panic("available_read in mem");
}

dev_zero::dev_zero(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t dev_zero::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    panic("write to zero");
}

std::size_t dev_zero::read(ptr_t buffer, std::size_t size, bool noblock) {
    memset(buffer, 0, size);
    return size;
}

bool dev_zero::open() {
    return true;
}

bool dev_zero::close() {
    return true;
}

std::size_t dev_zero::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool dev_zero::is_file() {
    return true;
}

bool dev_zero::available_read() {
    panic("available_read in zero");
}

dev_stub::dev_stub(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t dev_stub::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    printk("write to %s", get_name().c_str());
    panic("");
}

std::size_t dev_stub::read(ptr_t buffer, std::size_t size, bool noblock) {
    printk("read from %s", get_name().c_str());
    panic("");
}

bool dev_stub::open() {
    return true;
}

bool dev_stub::close() {
    return true;
}

std::size_t dev_stub::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool dev_stub::is_file() {
    return true;
}

bool dev_stub::available_read() {
    panic("available_read in zero");
}
