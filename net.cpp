#include <list>
#include <cstring>
#include <net.h>
#include <filesystem.h>
#include <tty_system.h>
#include "include/process.h"

socket_base::socket_base() : vfs_entry("", nullptr) {}

bool socket_base::register_backlog(socket_base *sock) {
    return false;
}

bool socket_base::connect(socket_base *socket) {
    return false;
}

socket_base *socket_base::accept() {
    return nullptr;
}

bool socket_base::is_listening() {
    return false;
}

bool socket_base::listen(int backlog) {
    return false;
}

bool socket_base::is_unix_socket() {
    return false;
}

void unix_socket::lock() {
    while (_lock)
        asm volatile ("nop");
    _lock = true;
}

void unix_socket::unlock() {
    if (!_lock)
        panic("unlocked");
    _lock = false;
}

void unix_socket::receive(const_ptr_t buffer, std::size_t size) {
    lock();
    auto& block = _received_data.emplace_back(new uint8_t[size], size);
    memcpy(block.first, buffer, size);
    unlock();
}

void unix_socket::shutdown(unix_socket *sock) {
    if (!_listening)
        panic("Not a server");
    if (_client_socket && _client_socket != sock) {
        _client_socket->close();
        _client_socket = nullptr;
    }
    if (_link_socket && _link_socket != sock) {
        _link_socket->close();
        _link_socket = nullptr;
    }
    lock();
    for (auto& bsock : _backlog_sockets)
        if (bsock == sock) {
            _backlog_sockets.remove(sock);
            break;
        }
    _received_data.clear();
    unlock();
}

unix_socket::unix_socket() :
        socket_base(),
        _listening(false),
        _server_socket(nullptr),
        _backlog_sockets(),
        _backlog_count(0),
        _client_socket(nullptr),
        _link_socket(nullptr),
        _received_data(),
        _lock(false) {}

std::size_t unix_socket::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    if (_listening)
        panic("Write on sever socket");
    if (_link_socket)
        _link_socket->receive(buffer, size);
    else if (_server_socket)
        _server_socket->receive(buffer, size);
    else
        panic("Not connected");
    return size;
}

std::size_t unix_socket::read(ptr_t buffer, std::size_t size, bool noblock) {
    lock();
    if (noblock && _received_data.empty()) {
        unlock();
        current_process->set_errno(EAGAIN);
        return -1;
    }
    unlock();

    while (_received_data.empty())
        asm volatile ("nop");

    lock();
    std::size_t total = 0;
    while(!_received_data.empty() && total != size) {
        auto block = _received_data.front();
        _received_data.pop_front();

        if (total + block.second <= size) {
            memcpy(buffer + total, block.first, block.second);
            total += block.second;
            delete[] block.first;
        }
        else {
            auto avail = size - total;
            auto last = block.second - avail;
            memcpy(buffer + total, block.first, avail);
            auto new_block = _received_data.emplace_front(new uint8_t[last], last);
            memcpy(new_block.first, block.first + avail, last);
        }
    }
    unlock();
    return total;
}

bool unix_socket::open() {
    return true;
}

bool unix_socket::close() {
    if (_listening)
        panic("close server");
    if (_server_socket)
        _server_socket->shutdown(this);
    return true;
}

std::size_t unix_socket::seek(std::size_t offset, file_whence_t whence) {
    return 0;
}

bool unix_socket::available_read() {
    if (_listening)
        return !_backlog_sockets.empty();
    return !_received_data.empty();
}

void unix_socket::bind_name(const std::string &name) {
    set_name(name);
}

bool unix_socket::connect(socket_base *socket) {
    if (_listening)
        panic("Not a client");
    if (!socket->is_unix_socket())
        panic("Not a unix socket\n");
    if (!socket->is_listening())
        return false;
    if (!socket->register_backlog(this))
        panic("failed to register\n");
    _server_socket = reinterpret_cast<unix_socket*>(socket);
    return true;
}

bool unix_socket::is_listening() {
    return _listening;
}

bool unix_socket::listen(int backlog) {
    _backlog_count = backlog;
    _listening = true;
    return true;
}

bool unix_socket::register_backlog(socket_base *sock) {
    if (!sock->is_unix_socket())
        panic("Not a unix socket\n");
    if (_backlog_sockets.size() == _backlog_count) {
        printk("Waiting for backlog decrement\n");
        while (_backlog_sockets.size() == _backlog_count)
            asm volatile ("nop");
    }
    lock();
    _backlog_sockets.emplace_back(reinterpret_cast<unix_socket*>(sock));
    unlock();
    return true;
}

bool unix_socket::is_file() {
    return true;
}

bool unix_socket::is_unix_socket() {
    return true;
}

socket_base *unix_socket::accept() {
    if (!_listening)
        panic("Not a server");
    while (_backlog_sockets.empty())
        assembly::halt();

    lock();
    _client_socket = _backlog_sockets.front();
    _backlog_sockets.pop_front();

    auto recv_sock = new unix_socket();
    _link_socket = recv_sock;
    recv_sock->set_name(get_name());

    recv_sock->_link_socket = _client_socket;
    _client_socket->_link_socket = recv_sock;

    for (auto& block : _received_data) {
        recv_sock->receive(block.first, block.second); // TODO: faster transfer
        delete[] block.first;
    }
    _received_data.clear();

    unlock();

    printk("%p -> %p: accepted socket\n", _client_socket, this);
    return recv_sock;
}


std::list<socket_base*> sockets;


socket_base* create_unix_socket() {
    return sockets.emplace_back(new unix_socket());
}


void init_networking() {
    new(&sockets) std::list<socket_base*>();
}
