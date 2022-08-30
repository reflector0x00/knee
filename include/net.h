#pragma once
#include "filesystem.h"

class socket_base : public vfs_entry {
public:
    socket_base();

    virtual bool register_backlog(socket_base* sock);
    virtual bool connect(socket_base* socket);
    virtual socket_base* accept();
    virtual bool is_listening();
    virtual bool listen(int backlog);
    virtual bool is_unix_socket();
};

class unix_socket : public socket_base {
    bool _listening;
    unix_socket* _server_socket;
    std::list<unix_socket*> _backlog_sockets;
    std::size_t _backlog_count;
    unix_socket* _client_socket;
    unix_socket* _link_socket;
    std::list<std::pair<uint8_t*, std::size_t>> _received_data;
    bool _lock;

    void lock();
    void unlock();
    void receive(const_ptr_t buffer, std::size_t size);
    void shutdown(unix_socket* sock);
public:
    unix_socket();

    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    bool available_read() override;
    void bind_name(const std::string& name);
    bool connect(socket_base* socket) override;
    bool is_listening() override;
    bool listen(int backlog) override;
    bool register_backlog(socket_base* sock) override;
    bool is_file() override;
    bool is_unix_socket() override;
    socket_base* accept() override;
};

socket_base* create_unix_socket();

void init_networking();