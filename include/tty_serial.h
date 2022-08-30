#pragma once
#include <cstdint>
#include "tty_system.h"
#include "ptr.h"
#include "ports.h"

enum data_bits_t : uint8_t {
    data_bits_5 = 0,
    data_bits_6 = 1,
    data_bits_7 = 2,
    data_bits_8 = 3,
};

enum parity_t : uint8_t {
    parity_none = 0,
    parity_odd = 1,
    parity_even = 3,
    parity_mark = 5,
    parity_space = 7,
};

union line_control_t {
    struct  {
        data_bits_t data_bits : 2;
        bool stop_bits : 1;
        parity_t parity : 3;
        bool _reserved : 1 {};
        bool dlab : 1;
    };
    uint8_t data;

    line_control_t(uint8_t _data);
    line_control_t(data_bits_t _data_bits = data_bits_5, bool _stop_bits = false, parity_t _parity = parity_none, bool _dlab = false);
    line_control_t(const line_control_t&) = default;
    line_control_t(line_control_t&&) = default;
    line_control_t& operator=(const line_control_t&) = default;
    line_control_t& operator=(line_control_t&&) = default;

    static line_control_t dlab_bit(bool enabled);
    operator uint8_t();
};

static_assert(sizeof(line_control_t) == 1);


union interrupt_enable_t {
    struct  {
        bool data_available : 1;
        bool transmitter_empty : 1;
        bool break_error : 1;
        bool status_change : 1;
    };
    uint8_t data;

    interrupt_enable_t(uint8_t _data);
    interrupt_enable_t(bool _data_available = false, bool _transmitter_empty = false, bool _break_error = false, bool _status_change = false);

    interrupt_enable_t(const interrupt_enable_t&) = default;
    interrupt_enable_t(interrupt_enable_t&&) = default;
    interrupt_enable_t& operator=(const interrupt_enable_t&) = default;
    interrupt_enable_t& operator=(interrupt_enable_t&&) = default;

    static interrupt_enable_t disable_all();
    operator uint8_t();
};

static_assert(sizeof(interrupt_enable_t) == 1);

// receiver fifo trigger level
enum rftl_t : uint8_t {
    rftl_1_characters = 0,
    rftl_4_characters = 1,
    rftl_8_characters = 2,
    rftl_14_characters = 3,
};


union fifo_control_t {
    struct  {
        bool fifo_enable : 1;
        bool rx_fifo_reset : 1;
        bool tx_fifo_reset : 1;
        bool dma_mode : 1;
        bool enable_dma_end : 1;
        bool _reserved : 1 {};
        rftl_t rftl : 2;
    };
    uint8_t data;

    fifo_control_t(uint8_t _data);
    fifo_control_t(bool _fifo_enable = false, bool _rx_fifo_reset = false, bool _tx_fifo_reset = false, bool _dma_mode = false,
                   bool _enable_dma_end = false, rftl_t _rftl = rftl_1_characters);
    fifo_control_t(const fifo_control_t&) = default;
    fifo_control_t(fifo_control_t&&) = default;
    fifo_control_t& operator=(const fifo_control_t&) = default;
    fifo_control_t& operator=(fifo_control_t&&) = default;

    operator uint8_t();
};

static_assert(sizeof(fifo_control_t) == 1);



union modem_control_t {
    struct  {
        bool data_terminal_ready : 1;
        bool request_to_send : 1;
        bool out_1 : 1;
        bool out_2 : 1;
        bool loop : 1;
    };
    uint8_t data;

    modem_control_t(uint8_t _data);
    modem_control_t(bool _data_terminal_ready = false, bool _request_to_send = false, bool _out_1 = false,
                    bool _out_2 = false, bool _loop = false);
    modem_control_t(const modem_control_t&) = default;
    modem_control_t(modem_control_t&&) = default;
    modem_control_t& operator=(const modem_control_t&) = default;
    modem_control_t& operator=(modem_control_t&&) = default;

    operator uint8_t();
};

static_assert(sizeof(modem_control_t) == 1);

union line_status_t {
    struct  {
        bool data_ready : 1;
        bool overrun_error : 1;
        bool parity_error : 1;
        bool framing_error : 1;
        bool break_indicator : 1;
        bool transmitter_holding_empty : 1;
        bool transmitter_empty : 1;
        bool impending_error : 1;
    };
    uint8_t data;

    line_status_t(uint8_t _data);
    line_status_t(bool _data_ready = false, bool _overrun_error = false, bool _parity_error = false,
                  bool _framing_error = false, bool _break_indicator = false, bool _transmitter_holding_empty = false,
                  bool _transmitter_empty = false, bool _impending_error = false);

    line_status_t(const line_status_t&) = default;
    line_status_t(line_status_t&&) = default;
    line_status_t& operator=(const line_status_t&) = default;
    line_status_t& operator=(line_status_t&&) = default;

    operator uint8_t();
};

static_assert(sizeof(line_status_t) == 1);


struct [[gnu::packed]] serial_port {
    union {
        io_port8_t data;
        io_port8_t divisor_low;
    };
    union {
        io_port8_tmpl_t<interrupt_enable_t> interrupt_enable;
        io_port8_t divisor_high;
    };
    union {
        io_port8_t interrupt_status; // on read // TODO: bits structure
        io_port8_tmpl_t<fifo_control_t> fifo_control; // on write
    };
    io_port8_tmpl_t<line_control_t> line_control;
    io_port8_tmpl_t<modem_control_t> modem_control;
    union {
        io_port8_tmpl_t<line_status_t> line_status; // TODO: bits structure
        io_port8_t prescaler_division;
    };
    io_port8_t modem_status; // TODO: bits structure
    io_port8_t scratch;
};

static_assert(sizeof(serial_port) == 8);


class tty_serial : public tty_descriptor {
    serial_port* _port;
    static constexpr uint8_t test_serial_chip = 0xAE;
public:
    explicit tty_serial(uint16_t address);
    bool init() override;
    bool is_transmit_empty();
    bool backspace() override;
    void write_char_raw(char c);
    bool write_char(char c) override;
    bool clear_till_end() override;
};


constexpr uint16_t serial_port_1 = 0x3f8;

void tty_serial_init();
