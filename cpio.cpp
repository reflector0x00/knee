#include <cpio.h>
#include <panic.h>
#include <tty_system.h>
#include <filesystem.h>


uint16_t cpio_header::get_name_size() const {
    auto size = c_namesize;
    if (size % 2 == 1)
        ++size;
    return size;
}

uint32_t cpio_header::get_file_size() const {
    return (static_cast<uint32_t>(c_filesize[0]) << 16) | c_filesize[1];
}

cpio_header::cpio_type cpio_header::get_type() const {
    return static_cast<cpio_type>((c_mode >> 12) & 017);
}

ptr_t cpio_extractor::extract_file(ptr_t address, const std::filesystem::path &directory) {
    cpio_header* header = address;

    if (header->c_magic != 0x71c7) {
        if (header->c_magic == 0xc771)
            panic("Big endian");
        printk("%04X\n", header->c_magic);
        panic("Bad magic");
    }

    auto name_size = header->get_name_size();
    auto file_size = header->get_file_size();
    std::string name_string(address + sizeof(cpio_header)); // TODO: check name size
    auto file_begin = address + sizeof(cpio_header) + name_size;
    if (name_string == "TRAILER!!!")
        return nullptr;

    // TODO: C++ - style
    auto result_dir = (directory / name_string);
    auto file_path = result_dir.c_str();
    printk("%s\n", file_path);

    switch (header->get_type()) {
        case cpio_header::cpio_type::file: {
            FILE *file = fopen(file_path, "w");
            if (!file) {
                printk("Can't open file %s\n", file_path);
                panic("");
            }
            fwrite(file_begin, 1, file_size, file);
            fclose(file);
            break;
        }

        case cpio_header::cpio_type::catalog: {
            // TODO: temporary stub
            if (name_string == ".")
                break;

            create_directory(name_string);
            break;
        }

        case cpio_header::cpio_type::symlink: {
            create_symbolic_link(name_string, std::string(file_begin, file_size));
            break;
        }

        default:
            printk("Unknown type: %o\n", header->get_type());
            panic("");
    }

    auto result = file_begin + file_size;;
    if (result % 2)
        result += 1; //TODO: increment
    return result;
}

cpio_extractor::cpio_extractor(ptr_t address) : _address(address) {}

void cpio_extractor::extract(const std::string &path) {
    auto directory = std::filesystem::path(path);
    for (ptr_t entry = _address; entry; entry = extract_file(entry, directory));
}
