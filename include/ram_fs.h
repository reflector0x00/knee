#pragma once
#include <cstdint>
#include <vector>
#include <list>
#include "filesystem.h"


class ram_file : public vfs_entry {
    std::vector<ptr_t> _pages;
    std::size_t _position {}; // TODO: get rid of position as member
    std::size_t _size {};

public:
    ram_file(const std::string& name, vfs_entry* parent);

    [[nodiscard]] std::size_t get_size() const override;
    std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock) override;
    std::size_t read(ptr_t buffer, std::size_t size, bool noblock) override;
    bool open() override;
    bool close() override;
    std::size_t seek(std::size_t offset, file_whence_t whence) override;
    std::size_t tell() override;
    bool is_file() override;
};


class ram_symbolic_link : public vfs_entry {
    std::string _path;
public:
    ram_symbolic_link(const std::string& name, vfs_entry* parent, std::string path);

    bool is_link() override;
    const std::string& get_link() override;
};



class ram_directory : public vfs_entry {
    std::list<vfs_entry*> _entries;
public:
    ram_directory(const std::string& name, vfs_entry* parent);

    vfs_entry* resolve(const std::string& name) override;
    vfs_entry* create_file(const std::string& name) override;
    vfs_entry* create_directory(const std::string& name) override;
    vfs_entry* create_symbolic_link(const std::string& name, const std::string& path) override;
    bool remove_file(const std::string& name) override;
    bool add_entry(vfs_entry* entry) override;
    vfs_entry* get_entry(std::size_t i) override;
    std::size_t get_entry_count() override;
    bool is_directory() override;
};

