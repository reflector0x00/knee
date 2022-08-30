#include <pic.h>
#include <new>
#include <ports.h>


pic_t::pic_t(uint16_t master_address, uint16_t slave_address) :
        _master(reinterpret_cast<pic_port*>(master_address)),
        _slave(reinterpret_cast<pic_port*>(slave_address)) {}

void pic_t::remap(uint8_t master_offset, uint8_t slave_offser) {

    _master->command = picc_icw1_init | picc_icw1_icw4;  // starts the initialization sequence (in cascade mode)
    assembly::io_wait();
    _slave->command = picc_icw1_init | picc_icw1_icw4;
    assembly::io_wait();
    _master->data = master_offset; // ICW2: Master PIC vector offset
    assembly::io_wait();
    _slave->data = slave_offser;  // ICW2: Slave PIC vector offset
    assembly::io_wait();
    _master->data = 4; // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    assembly::io_wait();
    _slave->data = 2; // ICW3: tell Slave PIC its cascade identity (0000 0010)
    assembly::io_wait();


    _master->data = picc_icw4_8086;
    assembly::io_wait();
    _slave->data = picc_icw4_8086;
    assembly::io_wait();

    _master->data = 0xF;  // restore saved masks.
    _slave->data = 0xF;
}

void pic_t::mask_interrupt(uint8_t vector) {
    auto port = vector < 8 ? _master : _slave;
    vector %= 8;

    uint8_t mask = port->data;
    port->data = mask | (1 << vector);
}

void pic_t::unmask_interrupt(uint8_t vector) {
    auto port = vector < 8 ? _master : _slave;
    vector %= 8;

    uint8_t mask = port->data;
    port->data = mask & ~(1 << vector);
}

void pic_t::end_of_interrupt(uint8_t vector) {
    if(vector >= 8)
        _slave->command = picc_end_of_interrupt;

    _master->command = picc_end_of_interrupt;
}

static pic_t pic;

void pic_mask_interrupt(uint8_t vector) {
    pic.mask_interrupt(vector);
}

void pic_unmask_interrupt(uint8_t vector) {
    pic.unmask_interrupt(vector);
}

void pic_end_of_interrupt(uint8_t vector) {
    pic.end_of_interrupt(vector);
}


void init_pic() {
    new(&pic) pic_t(pic_master_address, pic_slave_address);

    pic.remap(pic_master_vector_offset, pic_slave_vector_offset);
}