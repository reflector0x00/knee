#include <tty_system.h>
#include <ports.h>
#include <assembly.h>


// TODO: variable arguments
[[noreturn]]
void panic(const char* msg) {
    assembly::clear_interrupts();
    printk(msg);
    while(true)
        assembly::halt();
}
