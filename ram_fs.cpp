#include <cstring>
#include "include/ram_fs.h"
#include "include/sysdefs.h"
#include "include/allocator.h"

ram_file::ram_file(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent) {}

std::size_t ram_file::get_size() const {
    return _size;
}

std::size_t ram_file::write(const_ptr_t buffer, std::size_t size, bool noblock) {
    auto _page_index = page_index(_position);

    if (_page_index >= _pages.size())
        _pages.emplace_back(allocate_pages(1));

    auto first_page = _pages[_page_index];
    auto first_offset = _position % page_size;
    auto first_size = page_size - first_offset;
    if (size < first_size)
        first_size = size;

    memcpy(first_page + first_offset, buffer, first_size);
    _position += first_size;
    buffer += first_size;
    if (first_size == size) {
        if (_position > _size)
            _size = _position;
        return size;
    }

    ++_page_index;
    for (auto left_size = size - first_size; left_size; ++_page_index) {
        if (_page_index >= _pages.size())
            _pages.emplace_back(allocate_pages(1));

        auto next_page = _pages[_page_index];
        auto next_size = left_size > page_size ? page_size : left_size;

        memcpy(next_page, buffer, next_size);

        _position += next_size;
        buffer += next_size;
        left_size -= next_size;
    }

    if (_position > _size)
        _size = _position;

    return size;
}

std::size_t ram_file::read(ptr_t buffer, std::size_t size, bool noblock) {
    auto _page_index = page_index(_position);

    if (_page_index >= _pages.size())
        _pages.emplace_back(allocate_pages(1));

    auto first_page = _pages[_page_index];
    auto first_offset = _position % page_size;
    auto first_size = page_size - first_offset;
    if (size < first_size)
        first_size = size;
    if (_position + first_size > _size)
        first_size = _size - _position;

    memcpy(buffer, first_page + first_offset, first_size);
    _position += first_size;
    buffer += first_size;
    if (first_size == size || _position == _size)
        return first_size;

    ++_page_index;
    for (auto last_size = size - first_size; last_size; ++_page_index) {
        if (_page_index >= _pages.size())
            return size - last_size;

        auto next_page = _pages[_page_index];
        auto next_size = last_size > page_size ? page_size : last_size;
        if (_position + next_size > _size)
            next_size = _size - _position;

        memcpy(buffer, next_page, next_size);
        _position += next_size;
        buffer += next_size;
        last_size -= next_size;
        if (_position == _size)
            return size - last_size;
    }

    return size;
}

bool ram_file::open() {
    _position = 0;
    return true;
}

bool ram_file::close() {
    return true;
}

std::size_t ram_file::seek(std::size_t offset, file_whence_t whence) {
    switch (whence) {
        case whence_begin:
            _position = offset;
            break;

        case whence_current:
            _position += offset;
            break;

        case whence_end:
            _position = _size - offset;
            break;
    }
    return _position;
}

std::size_t ram_file::tell() {
    return _position;
}

bool ram_file::is_file() {
    return true;
}

ram_symbolic_link::ram_symbolic_link(const std::string &name, vfs_entry *parent, std::string path) : vfs_entry(name, parent), _path(std::move(path)) {}

bool ram_symbolic_link::is_link() {
    return true;
}

const std::string &ram_symbolic_link::get_link() {
    return _path;
}

ram_directory::ram_directory(const std::string &name, vfs_entry *parent) : vfs_entry(name, parent), _entries() {}

vfs_entry *ram_directory::resolve(const std::string &name) {
    if (name == ".")
        return this;
    if (name == "..")
        return get_parent();

    for (auto& entry : _entries)
        if (entry->get_name() == name)
            return entry;

    return nullptr;
}

vfs_entry *ram_directory::create_file(const std::string &name) {
    // TODO: Double creation check
    auto result = new ram_file(name, this);
    _entries.push_back(result);
    return result;
}

vfs_entry *ram_directory::create_directory(const std::string &name) {
    // TODO: Double creation check
    auto result = new ram_directory(name, this);
    _entries.push_back(result);
    return result;
}

vfs_entry *ram_directory::create_symbolic_link(const std::string &name, const std::string &path) {
    auto result = new ram_symbolic_link(name, this, path);
    _entries.push_back(result);
    return result;
}

bool ram_directory::remove_file(const std::string &name) {
    auto entry = resolve(name);
    if (!entry)
        return false;
    if (entry->is_directory())
        return false;
    _entries.remove(entry);
    return true;
}

bool ram_directory::add_entry(vfs_entry *entry) {
    _entries.push_back(entry);
    return true;
}

vfs_entry *ram_directory::get_entry(std::size_t i) {
    if (i >= _entries.size())
        return nullptr;
    return *std::next(_entries.begin(), i);
}

std::size_t ram_directory::get_entry_count() {
    return _entries.size();
}

bool ram_directory::is_directory() {
    return true;
}
