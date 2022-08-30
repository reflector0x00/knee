#pragma once
#include <cstdint>
#include "gdt.h"
#include "interrupts.h"


void page_fault(interrupt_frame *frame, uint32_t error_code);
void general_protection(interrupt_frame *frame, uint32_t error_code);
void system_call();
