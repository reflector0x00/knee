#pragma once
#include <cstdint>


struct [[gnu::packed]] boot_info_t {
    uint32_t flags;
    uint32_t mem_lower; // in kib
    uint32_t mem_upper; // in kib
    // TODO: other fields
};
