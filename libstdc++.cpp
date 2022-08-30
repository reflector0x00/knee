#include <new>
#include <cstdlib>
#include "include/panic.h"
#include "include/ptr.h"

void *operator new(std::size_t size) {
    return malloc(size);
}

void *operator new[](std::size_t size) {
    return malloc(size);
}

void operator delete(void *p) {
    if (p == ptr_t(0xFD000040))
        panic("here");
    free(p);
}

void operator delete[](void *p) {
    if (p == ptr_t(0xFD000040))
        panic("here");
    free(p);
}

inline void  operator delete  (void *, std::size_t) throw() { };
inline void  operator delete[](void *, std::size_t) throw() { };
