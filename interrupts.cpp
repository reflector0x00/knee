#include <cstdint>
#include <utility>
#include <interrupt_routines.h>
#include <interrupts.h>
#include <assembly.h>
#include <sysdefs.h>
#include <panic.h>
#include <tty_system.h>
#include <process.h>
#include <pic.h>

exception_handler_t exception_handlers[exception_vector_count];
interrupt_handler_t interrupt_handlers[interrupt_vector_count - exception_vector_count];

static gate_descriptor interrupt_descriptor_table[interrupt_vector_count];



template <std::size_t N>
void exception_routine_body(register_frame reg_frame, uint32_t error_code, interrupt_frame int_frame) {
    auto handler = exception_handlers[N];
    if (handler)
        handler(&int_frame, error_code);
    else {
        printk("Unregistered exception %d", N);
        panic("");
    }
}

template <std::size_t N>
void interrupt_routine_body(register_frame reg_frame, interrupt_frame int_frame) {
    auto handler = interrupt_handlers[N - exception_vector_count];
    if (handler)
        handler(&int_frame);
    else {
        printk("Unregistered interrupt %d", N);
        panic("");
    }

    pic_end_of_interrupt(N - exception_vector_count);

    if (need_reschedule) {
        if (!preemption_enabled) // TODO: remove
            panic("Try of preemption after interrupt flag clear");
        need_reschedule = false;
        schedule(int_frame, reg_frame);
    }
}

template <std::size_t N>
[[gnu::naked]]
void exception_routine() {
    asm volatile("pusha\n"
                 "cld\n"
                 "call %P0\n"
                 "popa\n"
                 "iret\n":: "i"(exception_routine_body<N>));
}

template <std::size_t N>
[[gnu::naked]]
void interrupt_routine() {
    asm volatile("pusha\n"
                 "cld\n"
                 "call %P0\n"
                 "popa\n"
                 "iret\n":: "i"(interrupt_routine_body<N>));
}



void set_exception_handler(std::size_t n, exception_handler_t handler) {
    if (n >= exception_vector_count)
        panic("Wrong handler type");
    if (n > interrupt_vector_count)
        panic("Unknown interrupt");
    if (exception_handlers[n])
        panic("Already registered");
    exception_handlers[n] = handler;
}


void set_interrupt_handler(std::size_t n, interrupt_handler_t handler) {
    if (n < exception_vector_count)
        panic("Wrong handler type");
    if (n > interrupt_vector_count)
        panic("Unknown interrupt");
    if (interrupt_handlers[n - exception_vector_count])
        panic("Already registered");
    interrupt_handlers[n - exception_vector_count] = handler;
}


template <int ... Is>
void do_init_exceptions(std::integer_sequence<int, Is...> const &) {
    ((
            interrupt_descriptor_table[Is] = gate_descriptor(
                    reinterpret_cast<void*>(exception_routine<Is>),
                    kernel_cs,
                    gate_interrupt32,
                    dpl_ring0)
    ), ...);
}

template <int ... Is>
void do_init_interrupts(std::integer_sequence<int, Is...> const &) {
    ((
        interrupt_descriptor_table[Is + exception_vector_count] = gate_descriptor(
                reinterpret_cast<void*>(interrupt_routine<Is + exception_vector_count>),
                kernel_cs,
                gate_interrupt32,
                dpl_ring0)
                        ), ...);
}

template <std::size_t N>
void initialize_interrupts() {
    do_init_exceptions(std::make_integer_sequence<int, exception_vector_count>{});
    do_init_interrupts(std::make_integer_sequence<int, N - exception_vector_count>{});
}

void init_interrupt_system() {

    initialize_interrupts<iv_interruptions_count>();

    set_exception_handler(iv_general_protection, general_protection);
    set_exception_handler(iv_page_fault, page_fault);

    interrupt_descriptor_table[iv_system_call] = gate_descriptor(
            reinterpret_cast<void *>(system_call),
            kernel_cs,
            gate_interrupt32,
            dpl_ring3
    );

    assembly::set_idtr(sizeof(interrupt_descriptor_table) - 1, interrupt_descriptor_table);
}
