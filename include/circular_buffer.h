#pragma once
#include <cstdint>
#include "assembly.h"


template <typename T, std::size_t N>
class circular_buffer {
    T buffer[N];
    std::size_t in_index;
    std::size_t out_index;
public:
    circular_buffer() : in_index(0), out_index(0) {}

    bool available() {
        return in_index != out_index;
    }

    T read() {
        while (in_index == out_index)
            assembly::halt();

        auto result = buffer[out_index];
        out_index = (out_index + 1) % N;
        return result;
    }

    // Warning: rewrites symbols
    void write(T data) {
        buffer[in_index] = data;
        in_index = (in_index + 1) % N;
    }

    // Warning: rewrites symbols
    void write(const T* data, std::size_t size) {
        for (std::size_t i = 0; i < size; ++i)
            buffer[(in_index + i) % N] = data[i];
        in_index = (in_index + size) % N;
    }

};
