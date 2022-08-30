#include <liballoc.h>
#include <allocator.h>
#include <panic.h>

extern "C" {

    bool lock = false; // TODO: mutexes

    int liballoc_lock() {
        while (lock)
            asm volatile ("nop");
        lock = true;
        return 0;
    }
    int liballoc_unlock() {
        if (!lock)
            panic("Unlock fault\n");
        lock = false;
        return 0;
    }

    void* liballoc_alloc(size_t pages) {
        return allocate_pages(pages);
    }

    int liballoc_free(void* ptr, size_t pages) {
        deallocate_pages(ptr, pages);
        return 0;
    }
}