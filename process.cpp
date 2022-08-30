#include <sys/wait.h>
#include <process.h>
#include <gdt.h>
#include "include/panic.h"
#include "include/allocator.h"
#include "include/tty_system.h"
#include "include/ports.h"
#include "include/registers.h"
#include "include/sysdefs.h"
#include "include/assembly.h"

process* current_process;
static std::size_t last_process_id = 0;
static std::list<process> process_list;
bool need_reschedule = false;
bool preemption_enabled = false; // TODO: remove


// Should be called with disabled interrupts
process& create_process(ptr_t instruction_pointer, ptr_t stack_pointer, bool privileged) {
    auto& new_process = process_list.emplace_back();

    new_process.process_id = last_process_id++;
    new_process.status = status_normal;

    new_process.instruction_pointer = instruction_pointer;
    new_process.stack_pointer = stack_pointer;
    new_process.flags = eb_int_enable;

    uint32_t cs = user_cs;
    uint32_t ds = user_ds;
    if (privileged) {
        cs = kernel_cs;
        ds = kernel_ds;
    }
    new_process.cs = cs;
    new_process.ds = ds;
    new_process.es = ds;
    new_process.fs = ds;
    new_process.gs = ds;
    new_process.ss = ds;

    new_process.interrupt_stack = allocate_pages(1);
    new_process.esp0 = new_process.interrupt_stack + page_size;

    new_process.heap_start = nullptr;
    new_process.heap_end = nullptr;

    new_process.current_directory = vfs_root;

    return new_process;
}

process& copy_current_process(const interrupt_frame& iframe, const register_frame& regframe) {
    // TODO: save flags
    assembly::clear_interrupts();

    auto &new_process = process_list.emplace_back(*current_process);

    current_process->childs.emplace_back(&new_process);

    new_process.process_id = last_process_id++;

    new_process.instruction_pointer = iframe.eip;
    new_process.flags = iframe.eflags;
    new_process.cs = iframe.cs;
    new_process.errno_location = nullptr;
    new_process.previous_errno = 0;

    if (iframe.is_ring0())
        panic("copy_current_process in ring 0 isn't implemented");
    else {
        new_process.stack_pointer = iframe.esp;
        new_process.ss = iframe.ss;
    }

    new_process.eax = regframe.eax;
    new_process.ecx = regframe.ecx;
    new_process.edx = regframe.edx;
    new_process.ebx = regframe.ebx;
    new_process.ebp = regframe.ebp;
    new_process.esi = regframe.esi;
    new_process.edi = regframe.edi;

    new_process.interrupt_stack = allocate_pages(1);
    new_process.esp0 = new_process.interrupt_stack + page_size;

    // Duplicate all allocations
    // TODO: copy-on-write
    new_process.allocations.clear();

    for (auto iter : current_process->allocations) {
        auto ptr = iter.first;
        auto new_page = allocate_pages(1); // TODO: memory leak?
        auto new_frame = kernel_page_to_frame(new_page);
        memcpy(new_page, ptr, page_size);

        new_process.allocations.emplace(ptr, new_frame);
        memory_unmap(new_page, false); // unmap temporary mapping
    }

    assembly::set_interrupts();

    return new_process;
}


[[noreturn]]
void exit_to_current_process() {
    auto is_ring0 = (current_process->cs & 0b11) == dpl_ring0;


    ptr_t stack_pointer = current_process->stack_pointer;
    stack_pointer -= sizeof(register_frame);
    stack_pointer -= is_ring0 ? sizeof(interrupt_frame_ring0) : sizeof(interrupt_frame);

    *static_cast<register_frame *>(stack_pointer) = {
            current_process->edi,
            current_process->esi,
            current_process->ebp,
            0,
            current_process->ebx,
            current_process->edx,
            current_process->ecx,
            current_process->eax,
    };

    auto int_frame = stack_pointer + sizeof(register_frame);

    if (is_ring0)
        *static_cast<interrupt_frame_ring0 *>(int_frame) = {
                current_process->instruction_pointer,
                current_process->cs,
                current_process->flags
        };
    else
        *static_cast<interrupt_frame *>(int_frame) = {
                {
                    current_process->instruction_pointer,
                current_process->cs,
                current_process->flags
                },
                current_process->stack_pointer,
                current_process->ss
        };


    set_tss_kernel_stack(current_process->esp0);

    set_ds(current_process->ds);
    set_es(current_process->es);
    set_fs(current_process->fs);
    set_gs(current_process->gs);

    preemption_enabled = true;

    asm volatile ("mov %0, %%esp\n"
                  "popa\n"
                  "iret"::"m"(stack_pointer));
}

// TODO: reset timer quantum
[[noreturn]]
void schedule(const interrupt_frame& frame, const register_frame& regframe, bool use_esp) {
    assembly::clear_interrupts();
    preemption_enabled = false;

    if (current_process != &process_list.front())
        panic("Order fault\n");

    current_process->instruction_pointer = frame.eip;
    current_process->flags = frame.eflags;
    current_process->cs = frame.cs;

    if (frame.is_ring0() && !use_esp) {
        current_process->stack_pointer = frame.get_ring0_esp();
        current_process->ss = get_ss();
    }
    else {
        current_process->stack_pointer = frame.esp;
        current_process->ss = frame.ss;
    }

    current_process->eax = regframe.eax;
    current_process->ecx = regframe.ecx;
    current_process->edx = regframe.edx;
    current_process->ebx = regframe.ebx;
    current_process->ebp = regframe.ebp;
    current_process->esi = regframe.esi;
    current_process->edi = regframe.edi;

    // TODO: es, fs, gs, ds?

    userspace_disable();

    // Moves to the end of list
    process_list.splice(process_list.end(), process_list, process_list.begin());

    current_process = &process_list.front();

    userspace_enable();

    exit_to_current_process();
}

extern "C" void schedule_internal(ptr_t eip, ptr_t esp, register_frame reg_frame) {
    interrupt_frame int_frame{
            {
                eip,
                get_cs(),
                get_flags()
            },
            esp,
            get_ss()
    };

    schedule(int_frame, reg_frame, true);
}

[[gnu::naked]]
void schedule() {
    asm volatile(R"(
                    pusha
                    lea (4+8*4)(%esp), %eax
                    push %eax
                    push (0+8*4+4)(%esp)
                    call schedule_internal
    )");
}

void init_processes() {
    new(&process_list) std::list<process>;

    // create root task
    current_process = &create_process(nullptr, nullptr, true);
}
