#include <timer.h>
#include <ports.h>
#include <pic.h>
#include <interrupt_routines.h>
#include <process.h>
#include "include/sysdefs.h"

_timer_t::_timer_t(uint16_t address) : _port(reinterpret_cast<timer_port*>(address)) {}

// TODO: channel select
void _timer_t::set_rate_generator(uint16_t reload_value) {
    _port->mode = tm_channel_0 | tm_low_high_byte | tm_rate_generator;

    _port->channel_0 = low_byte(reload_value);
    _port->channel_0 = high_byte(reload_value);
}


static _timer_t timer;

static std::size_t timer_value = 0;

static void timer_interrupt(interrupt_frame *frame) {
    if (timer_value++ == quantum) {
        timer_value = 0;
        need_reschedule = true;
    }
}


void init_timer() {
    new(&timer) _timer_t(timer_address);

    timer.set_rate_generator(timer_reload_value);

    set_interrupt_handler(iv_timer, timer_interrupt);
    pic_unmask_interrupt(piciv_timer);
}