#pragma once
#include "ptr.h"
#include <string>
#include <cstdint>
#include <filesystem>


struct [[gnu::packed]] cpio_header {
    uint16_t c_magic;
    uint16_t c_dev;
    uint16_t c_ino;
    uint16_t c_mode;
    uint16_t c_uid;
    uint16_t c_gid;
    uint16_t c_nlink;
    uint16_t c_rdev;
    uint16_t c_mtime[2];
    uint16_t c_namesize;
    uint16_t c_filesize[2];

    enum cpio_type {
        named_pipe = 001,
        symbol_dev = 002,
        catalog = 004,
        block_dev = 006,
        file = 010,
        symlink = 012,
        socket = 014,
    };

    [[nodiscard]] uint16_t get_name_size() const;
    [[nodiscard]] uint32_t get_file_size() const;
    [[nodiscard]] cpio_type get_type() const;
};

class cpio_extractor {
    ptr_t _address;

    static ptr_t extract_file(ptr_t address, const std::filesystem::path& directory);

public:
    explicit cpio_extractor(ptr_t address);
    void extract(const std::string& path);
};
