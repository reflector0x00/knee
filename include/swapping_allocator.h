#pragma once
#include "panic.h"
#include "allocator.h"

template <typename T>
struct swapping_allocator {
    using value_type = T;

    struct alloc_blk {
        T* origin;
        std::size_t count;
        bool allocated;

        alloc_blk() : origin(nullptr), count(0), allocated(false) {}

        T* set(std::size_t count) {
            if (allocated)
                panic("Double allocation");

            ptr_t to_free = nullptr;
            if (origin && this->count < count) {
                to_free = origin;
                origin = nullptr;
            }

            if (!origin) {
                origin = allocate_pages(count);

                // optimization: release after alloc 
                if (to_free)
                    deallocate_pages(to_free, this->count);
                this->count = count;
            }

            allocated = true;
            return origin;
        }
        void reset() {
            if (!allocated)
                panic("Double deallocation");

            this->allocated = false;
        }
    };
    alloc_blk _first;
    alloc_blk _second;

    swapping_allocator() = default;
    template <class U> constexpr swapping_allocator(const swapping_allocator<U>&) noexcept {};

    T* allocate(std::size_t n) {
        std::size_t size = sizeof(T) * n;
        std::size_t page_count = size / 4096 + (size % 4096 ? 1 : 0);

        if (_first.allocated && _second.allocated)
            panic("Not allowed third allocation");

        return !_first.allocated ? _first.set(page_count) : _second.set(page_count);
    }
    void deallocate(T* origin, std::size_t) {
        if (_first.origin == origin)
            _first.reset();
        else if (_second.origin == origin)
            _second.reset();
        else
            panic("Unknown address");
    }
};

