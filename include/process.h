#pragma once

#include <list>
#include <unordered_map>
#include <map>
#include "ptr.h"
#include "filesystem.h"
#include "interrupt_routines.h"
#include "page_descriptor.h"
#include "tty_system.h"

// TODO: move
struct register_frame {
    uint32_t edi{};
    uint32_t esi{};
    uint32_t ebp{};
    uint32_t esp{};
    uint32_t ebx{};
    uint32_t edx{};
    uint32_t ecx{};
    uint32_t eax{};
};


enum process_status {
    status_normal,
    status_exited
};

struct process {
    std::size_t process_id{};
    process_status status{};
    uint32_t exit_code{};

    ptr_t instruction_pointer{};
    ptr_t stack_pointer{};
    uint32_t flags{};

    uint32_t eax{};
    uint32_t ecx{};
    uint32_t edx{};
    uint32_t ebx{};
    uint32_t ebp{};
    uint32_t esi{};
    uint32_t edi{};

    uint32_t cs{};
    uint32_t ds{};
    uint32_t es{};
    uint32_t fs{};
    uint32_t gs{};
    uint32_t ss{};

    ptr_t interrupt_stack{};
    ptr_t esp0{};

    int* errno_location{};
    int previous_errno{};

    ptr_t heap_start{};
    ptr_t heap_end{};

    std::unordered_map<int, opened_file> descriptor_to_open{};
    int last_fd{};

    vfs_entry* current_directory{};

    std::map<ptr_t, page_descriptor> allocations;

    std::list<process*> childs;

    bool set_errno(int value) {
        if (!errno_location)
            previous_errno = value;
        else
            *errno_location = value;
        return true;
    }

    process() = default;
    process(const process& other) = default;
    process(process&& other) = default;

    process& operator=(const process& other) = default;
    process& operator=(process&& other) = default;
};

extern process* current_process;
extern bool need_reschedule;
extern bool preemption_enabled; // TODO: remove

process& create_process(ptr_t instruction_pointer, ptr_t stack_pointer, bool privileged);

[[noreturn]]
void schedule(const interrupt_frame& frame, const register_frame& regframe, bool use_esp = false);
void schedule();

process& copy_current_process(const interrupt_frame& iframe, const register_frame& regframe);

[[noreturn]]
void exit_to_current_process();

void init_processes();