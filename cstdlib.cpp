#include <panic.h>
#include <process.h>

extern "C" {

[[noreturn]]
void _exit(int code) {
    // TODO: rewrite with local API
    current_process->exit_code = code;
    current_process->status = status_exited;
    while (true)
        schedule();
}


void abort(void) {
    panic("ERROR: abort isn't implemented\n");
}


void* _malloc_r(struct _reent*, std::size_t size) {
    return malloc(size);
}
void* _calloc_r(struct _reent*, std::size_t count, std::size_t size) {
    return calloc(count, size);
}

void* _realloc_r(struct _reent*, void* ptr, std::size_t size) {
    return realloc(ptr, size);
}

void _free_r(struct _reent*, void* ptr) {
    free(ptr);
}
}
