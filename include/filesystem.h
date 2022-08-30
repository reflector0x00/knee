#pragma once
#include <string>
#include "ptr.h"
#include "panic.h"


enum file_whence_t {
    whence_begin,
    whence_current,
    whence_end
};

class vfs_entry;
extern vfs_entry* vfs_root;


class vfs_entry {
    std::string _name;
    vfs_entry* _parent;
protected:
    void set_name(const std::string& name);
public:
    vfs_entry(const std::string& name, vfs_entry* parent);
    [[nodiscard]] const std::string& get_name() const;
    [[nodiscard]] vfs_entry* get_parent() const;
    [[nodiscard]] std::string get_path() const;
    bool is_root();
    [[nodiscard]] virtual std::size_t get_size() const;
    virtual bool open();
    virtual bool close();
    virtual std::size_t write(const_ptr_t buffer, std::size_t size, bool noblock);
    virtual std::size_t read(ptr_t buffer, std::size_t size, bool noblock);
    virtual vfs_entry* resolve(const std::string& name);
    virtual vfs_entry* create_file(const std::string& name);
    virtual vfs_entry* create_directory(const std::string& name);
    virtual vfs_entry* create_symbolic_link(const std::string& name, const std::string& path);
    virtual bool remove_file(const std::string& name);
    virtual bool add_entry(vfs_entry* entry);
    virtual vfs_entry* get_entry(std::size_t i);
    virtual std::size_t get_entry_count();
    virtual std::size_t seek(std::size_t offset, file_whence_t whence);
    virtual std::size_t tell();
    virtual bool is_file();
    virtual bool is_directory();
    virtual bool is_link();
    virtual const std::string& get_link();
    virtual bool available_read();
    virtual bool available_write();
};


class opened_file {
    vfs_entry* _entry;
    int _flags; // TODO: replace with internal representation
    std::size_t _position;
public:
    opened_file(vfs_entry* entry, int flags);
    vfs_entry* get_entry() const;
    std::size_t get_position() const;
    void set_flags(int flags);
    int get_flags() const;
    void set_position(std::size_t position);
};


int register_entry_as_opened(vfs_entry* entry);
opened_file& get_opened_file(int fd);
vfs_entry* resolve_parent(const std::string& pathname);
std::string get_filename(const std::string& path);
vfs_entry* path_to_entry(const std::string& path, vfs_entry* current_directory);
vfs_entry* path_to_entry(const std::string& path);

bool set_fd_flags(int fd, int flags);
bool directory_entries_count(const char* path, std::size_t* count);
bool get_directory_entry(const char* path, std::size_t index, char* name, std::size_t size);

bool create_directory(const std::string& path);
bool create_symbolic_link(const std::string& from, const std::string& to);

void init_filesystem();