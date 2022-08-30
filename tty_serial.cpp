#include <tty_serial.h>
#include <tty_system.h>
#include <ports.h>


line_control_t line_control_t::dlab_bit(bool enabled) {
    return line_control_t(data_bits_5, false, parity_none, enabled);
}

line_control_t::operator uint8_t() {
    return data;
}

line_control_t::line_control_t(data_bits_t _data_bits, bool _stop_bits, parity_t _parity, bool _dlab) :
        data_bits(_data_bits),
        stop_bits(_stop_bits),
        parity(_parity),
        dlab(_dlab) {}

line_control_t::line_control_t(uint8_t _data) : data(_data) {}

interrupt_enable_t interrupt_enable_t::disable_all() {
    return interrupt_enable_t();
}

interrupt_enable_t::operator uint8_t() {
    return data;
}

interrupt_enable_t::interrupt_enable_t(bool _data_available, bool _transmitter_empty, bool _break_error,
                                       bool _status_change) :
        data_available(_data_available),
        transmitter_empty(_transmitter_empty),
        break_error(_break_error),
        status_change(_status_change)
{}

interrupt_enable_t::interrupt_enable_t(uint8_t _data) : data(_data) {}

fifo_control_t::operator uint8_t() {
    return data;
}

fifo_control_t::fifo_control_t(bool _fifo_enable, bool _rx_fifo_reset, bool _tx_fifo_reset, bool _dma_mode,
                               bool _enable_dma_end, rftl_t _rftl) :
        fifo_enable(_fifo_enable),
        rx_fifo_reset(_rx_fifo_reset),
        tx_fifo_reset(_tx_fifo_reset),
        dma_mode(_dma_mode),
        enable_dma_end(_enable_dma_end),
        rftl(rftl_1_characters) {}

fifo_control_t::fifo_control_t(uint8_t _data) : data(_data) {}

modem_control_t::operator uint8_t() {
    return data;
}

modem_control_t::modem_control_t(bool _data_terminal_ready, bool _request_to_send, bool _out_1, bool _out_2, bool _loop)
        :
        data_terminal_ready(_data_terminal_ready),
        request_to_send(_request_to_send),
        out_1(_out_1),
        out_2(_out_2),
        loop(_loop) {}

modem_control_t::modem_control_t(uint8_t _data) : data(_data) {}

line_status_t::operator uint8_t() {
    return data;
}

line_status_t::line_status_t(bool _data_ready, bool _overrun_error, bool _parity_error, bool _framing_error,
                             bool _break_indicator, bool _transmitter_holding_empty, bool _transmitter_empty,
                             bool _impending_error) :
        data_ready(_data_ready),
        overrun_error(_overrun_error),
        parity_error(_parity_error),
        framing_error(_framing_error),
        break_indicator(_break_indicator),
        transmitter_holding_empty(_transmitter_holding_empty),
        transmitter_empty(_transmitter_empty),
        impending_error(_impending_error) {}

line_status_t::line_status_t(uint8_t _data) : data(_data) {}

bool tty_serial::init() {
    _port->interrupt_enable = interrupt_enable_t::disable_all();  // Disable all interrupts
    _port->line_control = line_control_t::dlab_bit(true); // Enable DLAB (set baud rate divisor)
    _port->divisor_low = 0x03;  // Set divisor to 3 (lo byte) 38400 baud
    _port->divisor_high = 0x00; //                  (hi byte)
    _port->line_control = line_control_t(data_bits_8, false, parity_none); // 8 bits, no parity, one stop bit
    // Enable FIFO, clear them, with 14-byte threshold
    _port->fifo_control = fifo_control_t(true, true, true, false, false, rftl_14_characters);
    _port->modem_control = modem_control_t(true, true, false, true, false); // IRQs enabled, RTS/DSR set
    _port->modem_control = modem_control_t(false, true, true, true, true); // Set in loopback mode, test the serial chip


    _port->data = test_serial_chip; // Test serial chip (send byte 0xAE and check if serial returns same byte)
    // Check if serial is faulty (i.e: not same byte as sent)
    if (_port->data != test_serial_chip)
        return false;

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    _port->modem_control = modem_control_t(true, true, true, true, false);
    return true;
}

bool tty_serial::is_transmit_empty() {
    line_status_t line_status = _port->line_status;
    return line_status.transmitter_holding_empty;
}

bool tty_serial::backspace() {
    write_char('\b');
    return true;
}

void tty_serial::write_char_raw(char c) {
    while (is_transmit_empty() == 0);
    _port->data = c;
}

bool tty_serial::write_char(char c) {
    switch (c) {
        case '\n':
            write_char_raw('\r');
            write_char_raw('\n');
            break;

        case '\a': // bell
            return true;

        case '\r':
            write_char_raw('\r');
            break;


        default:
            write_char_raw(c);
    }
    return true;
}

bool tty_serial::clear_till_end() {
    return false; // TODO: implement
}

tty_serial::tty_serial(uint16_t address) : _port(reinterpret_cast<serial_port*>(address)) {} // TODO: ptr_t


void tty_serial_init() {
    static tty_serial uart1(serial_port_1);
    tty_system().register_output(&uart1);
}