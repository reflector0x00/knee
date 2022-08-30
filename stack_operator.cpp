#include "include/stack_operator.h"
#include <cstring>

stack_operator::stack_operator(ptr_t ptr) : _ptr(ptr) {}

ptr_t stack_operator::push(uint32_t x) {
    _ptr -= 4;
    *static_cast<uint32_t*>(_ptr) = x;
    return _ptr;
}

ptr_t stack_operator::push(ptr_t x) {
    _ptr -= 4;
    *static_cast<uint32_t*>(_ptr) = x;
    return _ptr;
}

ptr_t stack_operator::push(const std::string &str) {
    auto size = ((str.length() + 1) / 4) * 4;
    if ((str.length() + 1) % 4 != 0)
        size += 4;
    _ptr -= size;
    strcpy(_ptr, str.c_str());
    return _ptr;
}

ptr_t stack_operator::pointer() {
    return _ptr;
}
