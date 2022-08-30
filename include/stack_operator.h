#pragma once
#include "ptr.h"
#include <string>

class stack_operator {
    ptr_t _ptr;
public:
    explicit stack_operator(ptr_t ptr);
    ptr_t push(uint32_t x);
    ptr_t push(ptr_t x);
    ptr_t push(const std::string& str);
    ptr_t pointer();

};