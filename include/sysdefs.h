#pragma once
#include <cstdint>
#include "ptr.h"


constexpr std::size_t kib(std::size_t count) {
    return count << 10;
}
constexpr std::size_t mib(std::size_t count) {
    return count << 20;
}
constexpr std::size_t gib(std::size_t count) {
    return count << 30;
}

constexpr std::size_t page_size = kib(4);

constexpr std::size_t pages(std::size_t count) {
    return page_size * count;
}

constexpr std::size_t page_count(std::size_t bytes) {
    return bytes / page_size;
}

constexpr ptr_t page_address(std::size_t index) {
    return index << 12; // TODO: shifts may cause bug with different page size
}
constexpr std::size_t page_index(ptr_t address) {
    return address >> 12; // TODO: shifts may cause bug with different page size
}

constexpr ptr_t kernel_base = 0xC0000000;
constexpr ptr_t address_space_end = 0xFFFFFFFF; // not inclusive
constexpr ptr_t page_catalog_base = 0xFC000000;
constexpr ptr_t page_catalog_size = 0x01000000;
constexpr ptr_t vga_console_physical_address = 0xB8000;
constexpr std::size_t page_table_size = 1024;

// end - not inclusive
constexpr std::size_t size_between(ptr_t start, ptr_t end) {
    return end - start + 1;
}


constexpr ptr_t bootstrap_address(ptr_t address) {
    return address - kernel_base;
}

template <typename T>
constexpr T* bootstrap_address(T* address) {
    return bootstrap_address(ptr_t(address));
}

constexpr ptr_t relocated_address(ptr_t address) {
    return address + kernel_base;
}

template <typename T>
constexpr T* relocated_address(T* address) {
    return relocated_address(ptr_t(address));
}

constexpr std::size_t exception_vector_count = 32;
constexpr std::size_t interrupt_vector_count = 256;


[[gnu::always_inline]]
inline uint8_t low_byte(uint16_t data) {
    return data & 0xFF;
}

[[gnu::always_inline]]
inline uint8_t high_byte(uint16_t data) {
    return data >> 8;
}