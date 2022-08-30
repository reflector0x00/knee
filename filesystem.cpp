#include <string>
#include <list>
#include <utility>
#include <vector>
#include <panic.h>
#include <unordered_map>
#include <sstream>
#include <fcntl.h>
#include <tty_system.h>
#include <ptr.h>
#include <keyboard.h>
#include <filesystem.h>
#include <process.h>
#include <vga_color.h>
#include <limits>
#include <ram_fs.h>
#include <pseudo_fs.h>


void vfs_entry::set_name(const std::string &name) {
    _name = name;
}

vfs_entry::vfs_entry(const std::string &name, vfs_entry *parent) : _name(name), _parent(parent) {}

const std::string &vfs_entry::get_name() const {
    return _name;
}

vfs_entry *vfs_entry::get_parent() const {
    return _parent;
}

std::string vfs_entry::get_path() const {
    if (_parent == nullptr)
        return _name;
    auto parent = _parent->get_path();
    if (_parent->is_root())
        return parent + _name;
    return parent + "/" + _name;
}

bool vfs_entry::is_root() {
    return this == vfs_root;
}

std::size_t vfs_entry::get_size() const {
    return 0;
}

bool vfs_entry::open() {
    return false;
}

bool vfs_entry::close() {
    return false;
}

std::size_t vfs_entry::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    return 0;
}

std::size_t vfs_entry::read(ptr_t buffer, std::size_t size, bool noblock) {
    return 0;
}

vfs_entry *vfs_entry::resolve(const std::string &name) {
    return nullptr;
}

vfs_entry *vfs_entry::create_file(const std::string &name) {
    return nullptr;
}

vfs_entry *vfs_entry::create_directory(const std::string &name) {
    return nullptr;
}

vfs_entry *vfs_entry::create_symbolic_link(const std::string &name, const std::string &path) {
    return nullptr;
}

bool vfs_entry::remove_file(const std::string &name) {
    return false;
}

bool vfs_entry::add_entry(vfs_entry *entry) {
    return false;
}

vfs_entry *vfs_entry::get_entry(std::size_t i) {
    return nullptr;
}

std::size_t vfs_entry::get_entry_count() {
    return 0;
}

std::size_t vfs_entry::seek(std::size_t offset, file_whence_t whence) {
    return -1;
}

std::size_t vfs_entry::tell() {
    return -1;
}

bool vfs_entry::is_file() {
    return false;
}

bool vfs_entry::is_directory() {
    return false;
}

bool vfs_entry::is_link() {
    return false;
}

const std::string &vfs_entry::get_link() {
    panic("not a link");
}

bool vfs_entry::available_read() {
    return false;
}

bool vfs_entry::available_write() {
    return false;
}


opened_file::opened_file(vfs_entry *entry, int flags) : _entry(entry), _flags(flags), _position(0) {}

vfs_entry *opened_file::get_entry() const {
    return _entry;
}

std::size_t opened_file::get_position() const {
    return _position;
}

void opened_file::set_flags(int flags) {
    _flags = flags;
}

int opened_file::get_flags() const {
    return _flags;
}

void opened_file::set_position(std::size_t position) {
    _position = position;
}


vfs_entry* vfs_root;

int allocate_fd() {
    auto& opened_files = current_process->descriptor_to_open;
    auto& last_fd = current_process->last_fd;
    for (std::size_t i = 0; i < std::numeric_limits<uint32_t>::max(); ++i, ++last_fd)
        if (opened_files.find(last_fd) == opened_files.end())
            return last_fd++;

    panic("Failed to allocate descriptor");
}

vfs_entry* path_to_entry(const std::string& path, vfs_entry* current_directory) {
    std::stringstream ss(path);
    vfs_entry* current;
    switch (ss.peek()) {
        case '.':
            ss.get();
            current = current_directory;
            if (ss.peek() == '.') {
                ss.get();
                auto parent = current->get_parent();
                if (parent)
                    current = parent;
            }
            if (ss.get() != '/' && ss) {
                printk("Unknown path pattern: %s", path.c_str());
                panic("");
            }
            break;


        case '/':
            ss.get(); 
            current = vfs_root;
            break;

        default:
            current = current_directory;
            break;
    }

    std::string next;
    while (std::getline(ss, next, '/')) {
        if (!current->is_directory())
            return nullptr;
        current = current->resolve(next);

        if (!current)
            return nullptr;
    }
    return current;
}

vfs_entry* path_to_entry(const std::string& path) {
    return path_to_entry(path, current_process->current_directory);
}

std::string get_filename(const std::string& path) {
    auto pos = path.rfind('/');
    if (pos != std::string::npos)
        return path.substr(pos + 1);
    return path;
}


vfs_entry* resolve_parent(const std::string& pathname) {
    std::string parent(pathname);
    auto pos = parent.rfind('/');
    if (pos != std::string::npos)
        parent.resize(pos);
    else
        parent.clear();

    return path_to_entry(parent);
}



bool set_fd_flags(int fd, int flags) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end()) {
        printk("set_fd_flags: unknown fd: %d\n", fd);
        panic("");
    }
    iter->second.set_flags(flags);
    printk("flag set for fd %d: %08X\n", fd, flags);
    return true;
}

// todo: iterative method
bool directory_entries_count(const char* path, std::size_t* count) {
    vfs_entry* entry = path_to_entry(path);
    if (!entry || !entry->is_directory()) {
        return false;
    }
    *count = entry->get_entry_count();
    return true;
}
bool get_directory_entry(const char* path, std::size_t index, char* name, std::size_t size) {
    vfs_entry* entry = path_to_entry(path);
    if (!entry->is_directory())
        return false;
    auto subentry = entry->get_entry(index);
    if (!subentry)
        return false;
    auto& subentry_name = subentry->get_name();
    if (size < subentry_name.size() + 1)
        return false;
    strncpy(name, subentry_name.c_str(), size);
    return true;
}

// TODO: error codes
bool create_directory(const std::string& path) {
    vfs_entry* entry = path_to_entry(path);
    if (entry)
        panic("Already exists\n");

    entry = resolve_parent(path);
    if (!entry)
        panic("Parent isn't exists");

    auto filename = get_filename(path);

    return entry->create_directory(filename);
}

bool create_symbolic_link(const std::string& from, const std::string& to) {
    vfs_entry* entry = path_to_entry(from);
    if (entry)
        panic("Already exists\n");

    entry = resolve_parent(from);
    if (!entry)
        panic("Parent isn't exists");

    auto filename = get_filename(from);
    return entry->create_symbolic_link(filename, to);
}


int register_entry_as_opened(vfs_entry* entry) {
    int fd = allocate_fd();
    current_process->descriptor_to_open.emplace(fd, opened_file(entry, 0));
    return fd;
}

opened_file& get_opened_file(int fd) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        panic("get_opened_file: Not opened fd");
    return iter->second;
}

void init_filesystem() {
    vfs_root = new ram_directory("/", nullptr);
    vfs_root->add_entry(new stdin_file("stdin", vfs_root));
    vfs_root->add_entry(new stdout_file("stdout", vfs_root));

    current_process->current_directory = vfs_root;

    // TODO: move to dev
    open("/stdin", 0); // TODO: read flag
    open("/stdout", 0); // TODO: write flag
    open("/stdout", 0); // TODO: write flag

    auto dev = vfs_root->create_directory("dev");
    dev->add_entry(new dev_mem("mem", dev));
    dev->add_entry(new dev_zero("zero", dev));
    dev->add_entry(new dev_stub("tty", dev));
    dev->add_entry(new dev_stub("ptmx", dev));
}
