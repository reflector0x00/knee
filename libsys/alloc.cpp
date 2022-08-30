#include <cstdlib>
#include <cstdint>

extern "C" void* _malloc_r(struct _reent*, std::size_t size) {
    return malloc(size);
}

extern "C" void* _calloc_r(struct _reent*, std::size_t count, std::size_t size) {
    return calloc(count, size);
}

extern "C" void* _realloc_r(struct _reent*, void* ptr, std::size_t size) {
    return realloc(ptr, size);
}

extern "C" void _free_r(struct _reent*, void* ptr) {
    free(ptr);
}
//
//void *operator new(std::size_t size) {
//    return malloc(size);
//}

//void *operator new[](std::size_t size) {
//    return malloc(size);
//}

//void operator delete(void *p) {
//    free(p);
//}

//void operator delete[](void *p) {
//    free(p);
//}

inline void  operator delete  (void *, std::size_t) throw() { };
inline void  operator delete[](void *, std::size_t) throw() { };
