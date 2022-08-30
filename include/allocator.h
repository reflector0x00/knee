#pragma once
#include <cstdint>
#include "ptr.h"
#include "boot_info.h"

ptr_t allocate_pages(std::size_t n, bool userspace = false);
ptr_t allocate_pages(ptr_t address, std::size_t n, bool userspace = false);
void deallocate_pages(ptr_t origin, std::size_t n);

void memory_map(ptr_t page, ptr_t frame, bool userspace = false);
void memory_unmap(ptr_t page, bool mark); // TODO: do not allow to use "mark" outside

void userspace_disable();
void userspace_enable();

ptr_t kernel_page_to_frame(ptr_t page);

void allocator_init(boot_info_t* boot_info);