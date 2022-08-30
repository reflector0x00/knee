#pragma once
#include <cstdint>
#include "ptr.h"
#include "assembly.h"

struct _stub_type;

template <typename T>
class [[gnu::packed]] io_port8_tmpl_t {
    uint8_t _padding; // do not init
public:
    io_port8_tmpl_t() = delete;
    operator uint8_t() {
        return assembly::port_in8(ptr_t(this));
    }
    operator T() {
        return static_cast<T>(this->operator uint8_t());
    }

    uint8_t operator=(uint8_t value) {
        assembly::port_out8(ptr_t(this), value);
        return value;
    }

    T operator=(T value) {
        this->operator=(static_cast<uint8_t>(value));
        return value;
    }
};
typedef io_port8_tmpl_t<_stub_type> io_port8_t;
