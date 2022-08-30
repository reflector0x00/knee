#pragma once
#include "panic.h"
#include "allocator.h"
#include "sysdefs.h"

template <typename T>
class linked_paged_allocator {

    static constexpr size_t entries_count() {
        auto free = 4096 - sizeof(ptr_t*);
        auto count = free / sizeof(T);
        free -= count * sizeof(T);
        while (free < count * 8 + sizeof(std::bitset<0>)) {
            free += sizeof(T);
            --count;
        }
        return count;
    }

    // TODO: node_t
    struct _node {
        _node* next;
        std::bitset<entries_count()> used;
        std::array<T, entries_count()> entries;
        
        _node() : next(nullptr) {}
    }; 
    static_assert(sizeof(_node) <= page_size);

    _node* data; // TODO: underscore
    bool _lock;
public:
    [[nodiscard]] bool get_lock() const {
        return _lock;
    }

    using value_type = T;
    linked_paged_allocator() : data(nullptr), _lock(false) {}
    template <class U>
    constexpr linked_paged_allocator(const linked_paged_allocator<U>& other) noexcept : data(nullptr), _lock(other.get_lock()) {}

    T* allocate (std::size_t count) {
        if (count != 1)
            panic("Can't be used for multiple allocations");
        
        if (data == nullptr) {
            _lock = true;
            data = allocate_pages(1);
            _lock = false;
            new(data) _node();
        }

        std::size_t index;
        for (_node* ptr = data; ptr; ptr = ptr->next) {
            _node& node = *ptr;
            if (node.used.all())
                continue;

            for (index = 0; index < node.used.size(); ++index)
                if (!node.used[index]) { 
                    node.used[index] = true;
                    return ptr->entries.data() + index;       
                }
            panic("unreachable code");
        }

        _node* ptr;
        for (ptr = data; ptr->next; ptr = ptr->next);

        _lock = true;
        ptr->next = allocate_pages(1);
        _lock = false;

        ptr = ptr->next;
        new(ptr) _node();

        ptr->used[0] = true;
        return ptr->entries.data();
    }

    void deallocate (T* item, std::size_t count) {
        if (count != 1)
            panic("Can't be used for multiple allocations");

        if (data == nullptr)
            panic("Unknown behaviour");

        for (_node* ptr = data; ptr; ptr = ptr->next) {
            _node& node = *ptr;
            T* begin = node.entries.data();
            if (begin > item)
                continue;
            
            std::size_t index = item - begin;
            if (index >= node.entries.size())
                continue;
            
            if (!node.used[index])
                panic("Try of deallocation unallocated");
            
            node.used[index] = false;
            return;
        }
        panic("Try of deallocation unallocated");
    }

};