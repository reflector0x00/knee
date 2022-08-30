#pragma once
#include <cstdint>
#include <cstddef>
#include "ptr.h"

// TODO: copy/move semantics
struct [[gnu::packed]] page_entry {
    bool present: 1;
    bool writable: 1;
    bool user: 1;
    bool write_through: 1;
    bool cache_disable: 1;
    bool accessed: 1;
    bool dirty: 1;
    bool page_size: 1;
    bool global: 1;
    uint8_t _available: 3;
    uint32_t address_part: 20;

    constexpr void set_address(ptr_t address) {
        address_part = address >> 12;
    }
};

static_assert(sizeof(page_entry) == 4);

// TODO: uint32_t or std::size_t?
constexpr uint32_t page_directory_index(ptr_t address) {
    return address >> 22;
}

template <typename T>
constexpr uint32_t page_directory_index(T address) {
    return page_directory_index(ptr_t(address));
}

constexpr uint32_t page_table_index(ptr_t address) {
    return (address >> 12) & 0x3FF;
}

template <typename T>
constexpr uint32_t page_table_index(T address) {
    return page_table_index(ptr_t(address));
}
