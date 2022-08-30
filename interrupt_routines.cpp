#include <sys/stat.h>
#include <sys/unistd.h>
#include <cstdlib>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <interrupt_routines.h>
#include <panic.h>
#include <tty_system.h>
#include <allocator.h>
#include <pic.h>
#include <process.h>
#include <ports.h>
#include <syscalls.h>
#include "include/elf.h"
#include "include/registers.h"
#include "include/vm86.h"


void page_fault(interrupt_frame *frame, uint32_t error_code) {
    ptr_t address = get_cr2();
    printk("Page fault at address %p with error code %d\n", address, error_code);
    printk("ss=%04X, esp=%08X, eflags=%08X, cs=%04X, eip=%08X\n", frame->ss, frame->esp, frame->eflags, frame->cs, frame->eip);
    panic("");
}

vm86_struct* saved_vm_ptr;

void general_protection(interrupt_frame *frame, uint32_t error_code) {
    // TODO: refactor
    if (frame->eflags & (1 << 17)) { // VM86
        if (((error_code >> 1) & 0b1)) { // Interrupt
            // TODO: pass as argument
            ptr_t frame_origin = frame;
            register_frame* rframe = frame_origin - sizeof(register_frame) - 4;
            if (!saved_vm_ptr)
                panic("saved_vm_ptr is null\n");

            saved_vm_ptr->regs.eax = rframe->eax;
            saved_vm_ptr->regs.ebx = rframe->ebx;
            saved_vm_ptr->regs.ecx = rframe->ecx;
            saved_vm_ptr->regs.edx = rframe->edx;
            saved_vm_ptr->regs.edi = rframe->edi;
            saved_vm_ptr->regs.esi = rframe->esi;
            saved_vm_ptr->regs.ebp = rframe->ebp;
            saved_vm_ptr->regs.esp = frame->esp;
            saved_vm_ptr->regs.eip = frame->eip;
            saved_vm_ptr->regs.eflags = frame->eflags;
            saved_vm_ptr->regs.cs = frame->cs;
            saved_vm_ptr->regs.ss = frame->ss;

            current_process->eax = ((error_code >> 3) << 8) | 2; // Int
            exit_to_current_process();
        }
    }

    printk("General protection with error code %d\n", error_code);
    printk("ss=%04X, esp=%08X, eflags=%08X, cs=%04X, eip=%08X\n", frame->ss, frame->esp, frame->eflags, frame->cs, frame->eip);
    panic("");
}

[[gnu::naked]]
void system_call() {
    asm volatile(R"(
            cld
            pusha
            sti
            call system_call_body
            cli
            add $(8*4), %esp
            iret
    )");
}

//#define printd(...) printk(__VA_ARGS__)
#define printd(...)

typedef unsigned int socklen_t;
typedef unsigned long int nfds_t;
extern "C" int poll(struct pollfd *fds, nfds_t nfds, int timeout);
extern "C" int lstat(const char *pathname, struct stat *statbuf);
extern "C" ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
extern "C" void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern "C" int socket(int domain, int type, int protocol);
extern "C" int bind(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
extern "C" int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern "C" int listen(int sockfd, int backlog);
extern "C" int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern "C" int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

#define _UTSNAME_LENGTH 65
#ifndef _UTSNAME_SYSNAME_LENGTH
# define _UTSNAME_SYSNAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_NODENAME_LENGTH
# define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_RELEASE_LENGTH
# define _UTSNAME_RELEASE_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_VERSION_LENGTH
# define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_MACHINE_LENGTH
# define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH
#endif

struct utsname
{
    /* Name of the implementation of the operating system.  */
    char sysname[_UTSNAME_SYSNAME_LENGTH];

    /* Name of this node on the network.  */
    char nodename[_UTSNAME_NODENAME_LENGTH];

    /* Current release level of this implementation.  */
    char release[_UTSNAME_RELEASE_LENGTH];
    /* Current version level of this release.  */
    char version[_UTSNAME_VERSION_LENGTH];

    /* Name of the hardware type the system is running on.  */
    char machine[_UTSNAME_MACHINE_LENGTH];

};


extern "C" uint32_t system_call_body(register_frame reg_frame, interrupt_frame int_frame) {
    uint32_t code;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    asm("mov %%eax, %0\n"
        "mov %%ebx, %1\n"
        "mov %%ecx, %2\n"
        "mov %%edx, %3\n" : "=m"(code), "=m"(a0), "=m"(a1), "=m"(a2));
    switch(code) {
        case SYSCALL_OPEN:
            printd("open(\"%s\", %d) = ", a0, a1);
            return open(reinterpret_cast<const char*>(a0), a1);

        case SYSCALL_CLOSE:
            printd("close(%d)\n", a0);
            return close(a0);

        case SYSCALL_FSTAT:
            printd("fstat(%d, %p)\n", a0, a1);
            return fstat(static_cast<int>(a0), reinterpret_cast<struct stat*>(a1));

        case SYSCALL_ISATTY:
            printd("isatty(%d)\n", a0);
            return isatty(a0);

        case SYSCALL_READ:
            printd("read(%d, %p, %d)\n", a0, a1, a2);
            return read(static_cast<int>(a0), reinterpret_cast<void*>(a1), a2);

        case SYSCALL_WRITE:
            printd("write(%d, \"%s\", %d) = ", a0, a1, a2);
            return write(static_cast<int>(a0), reinterpret_cast<const void *>(a1), a2);

        case SYSCALL_EXIT:
            printd("exit(%d)\n", a0);
            _exit(static_cast<int>(a0));

        case SYSCALL_ALLOC_PAGES: {
            printd("alloc(%d)\n", a0);
            auto result = allocate_pages(current_process->heap_end, a0, true);
            current_process->heap_end += a0 * 4096;
            return result;
        }

        case SYSCALL_DEALLOC_PAGES:
            printd("dealloc(%08X, %d)\n", a0, a1); // TODO: real deallocation
            return 0;

        case SYSCALL_GETCWD: {
            printd("getcwd()\n");
            return reinterpret_cast<uint32_t>(getcwd(reinterpret_cast<char*>(a0), reinterpret_cast<size_t>(a1)));
        }

        case SYSCALL_DIRENT_COUNT:
            printd("directory_entries_count()\n");
            return directory_entries_count(reinterpret_cast<const char*>(a0), reinterpret_cast<std::size_t*>(a1));

        case SYSCALL_GET_DIRENT: {
            printd("get_directory_entry()\n");
            struct packed {
                const char* p;
                std::size_t i;
                char* n;
                std::size_t s;
            };
            auto pack = reinterpret_cast<packed*>(a0);
            return get_directory_entry(pack->p, pack->i, pack->n, pack->s);
        }

        case SYSCALL_DUP2:
            printd("dup2()\n");
            return dup2(a0, a1);

        case SYSCALL_POLL:
            printd("poll()\n");
            return poll(reinterpret_cast<struct pollfd*>(a0), reinterpret_cast<nfds_t>(a1), a2);

        case SYSCALL_STAT:
            printd("stat()\n");
            return stat(reinterpret_cast<const char*>(a0), reinterpret_cast<struct stat*>(a1));

        case SYSCALL_LSTAT:
            printd("lstat()\n");
            return lstat(reinterpret_cast<const char*>(a0), reinterpret_cast<struct stat*>(a1));

        case SYSCALL_PID:
            printd("getpid()\n");
            return getpid();

        case SYSCALL_FORK: {
            printd("fork()\n");
            register_frame new_frame(reg_frame);
            new_frame.eax = 0;
            auto& new_process = copy_current_process(int_frame, new_frame);
            return new_process.process_id;
        }

        case SYSCALL_EXEC: {
            printd("exec()\n");
            auto pathname = reinterpret_cast<const char *>(a0);
            auto argv = reinterpret_cast<char* const*>(a1);
            auto envp = reinterpret_cast<char* const*>(a2);
            std::vector<std::string> args;
            if (argv) {
                for (std::size_t i = 0; argv[i]; ++i)
                    args.emplace_back(argv[i]);
            }
            std::vector<std::string> envs;
            if (envp) {
                for (std::size_t i = 0; envp[i]; ++i)
                    envs.emplace_back(envp[i]);
            }
            execute_elf(pathname, args, envs, false);
            // no returns
        }
        case SYSCALL_WAITPID:
            printd("waitpid()\n");
            return waitpid(a0, reinterpret_cast<int*>(a1), a2);

        case SYSCALL_CHANGE_DIR:
            printd("chdir()\n");
            return chdir(reinterpret_cast<const char*>(a0));

        case SYSCALL_SET_ERRNO_LOCATION: {
            printd("set_errno_location()\n");
            auto errno_location = reinterpret_cast<int *>(a0);
            if (current_process->errno_location)
                current_process->previous_errno = *current_process->errno_location;

            current_process->errno_location = errno_location;
            *current_process->errno_location = current_process->previous_errno;
            return 0;
        }

        case SYSCALL_MAKE_DIR:
            printd("mkdir()\n");
            return mkdir(reinterpret_cast<const char*>(a0), a1);

        case SYSCALL_DUP:
            printd("dup()\n");
            return dup(a0);

        case SYSCALL_READLINK:
            printd("readlink()\n");
            return readlink(reinterpret_cast<const char*>(a0), reinterpret_cast<char*>(a1), a2);

        case SYSCALL_MAKE_LINK:
            printd("link(\"%s\",\"%s\")\n", a0, a1);
            return link(reinterpret_cast<const char*>(a0), reinterpret_cast<const char*>(a1));

        case SYSCALL_REMOVE_FILE:
            printd("unlink(\"%s\")\n", a0);
            return unlink(reinterpret_cast<const char*>(a0));

        case SYSCALL_MMAP: {
            struct packed {
                void *addr;
                size_t length;
                int prot;
                int flags;
                int fd;
                off_t offset;
            };
            auto pack = reinterpret_cast<packed *>(a0);
            return reinterpret_cast<uint32_t>(mmap(pack->addr, pack->length, pack->prot, pack->flags, pack->fd, pack->offset));
        }

        case SYSCALL_IOPL:
            int_frame.eflags |= ((a0 & 0b11) << 12);
            return 0;

        case SYSCALL_SELECT: {
            struct packed {
                int nfds;
                fd_set *readfds;
                fd_set *writefds;
                fd_set *exceptfds;
                struct timeval *timeout;
            };
            auto pack = reinterpret_cast<packed *>(a0);
            return select(pack->nfds, pack->readfds, pack->writefds, pack->exceptfds, pack->timeout);
        }

        case SYSCALL_SOCKET:
            return socket(a0, a1, a2);

        case SYSCALL_BIND:
            return bind(a0, reinterpret_cast<const struct sockaddr_in*>(a1), a2);

        case SYSCALL_GETPEERNAME:
            return getpeername(a0, reinterpret_cast<struct sockaddr*>(a1), reinterpret_cast<socklen_t *>(a2));

        case SYSCALL_LISTEN:
            return listen(a0, a1);

        case SYSCALL_CONNECT:
            return connect(a0, reinterpret_cast<const struct sockaddr *>(a1), a2);

        case SYSCALL_ACCEPT:
            return accept(a0, reinterpret_cast<struct sockaddr *>(a1), reinterpret_cast<socklen_t *>(a2));

        case SYSCALL_SETFLAGS:
            if (set_fd_flags(a0, a1))
                return 0;
            return -1;

        case SYSCALL_UNAME: {
            auto buf = reinterpret_cast<utsname*>(a0);
            strncpy(buf->sysname, "KneeOS", sizeof(buf->sysname));
            strncpy(buf->nodename, "localhost", sizeof(buf->nodename));
            strncpy(buf->release, "0.0-demo", sizeof(buf->release));
            strncpy(buf->version, __DATE__ " " __TIME__, sizeof(buf->version));
            strncpy(buf->machine, "i686", sizeof(buf->machine));
            return 0;
        }

        case 113: {
            //; extern void enter_v86(uint32_t ss, uint32_t esp, uint32_t cs, uint32_t eip);
            //enter_v86:
            //   mov ebp, esp               ; save stack pointer
            //
            //   push dword  [ebp+4]        ; ss
            //   push dword  [ebp+8]        ; esp
            //   pushfd                     ; eflags
            //   or dword [esp], (1 << 17)  ; set VM flags
            //   push dword [ebp+12]        ; cs
            //   push dword  [ebp+16]       ; eip
            //   iret
            auto vm = reinterpret_cast<vm86_struct*>(a0);
            saved_vm_ptr = vm;
            current_process->instruction_pointer = int_frame.eip;
            current_process->flags = int_frame.eflags;
            current_process->cs = int_frame.cs;
            current_process->stack_pointer = int_frame.esp;
            current_process->ss = int_frame.ss;

            current_process->eax = reg_frame.eax;
            current_process->ecx = reg_frame.ecx;
            current_process->edx = reg_frame.edx;
            current_process->ebx = reg_frame.ebx;
            current_process->ebp = reg_frame.ebp;
            current_process->esi = reg_frame.esi;
            current_process->edi = reg_frame.edi;

            struct [[gnu::packed]] return_context {
                register_frame rframe;
                interrupt_frame iframe;
                uint32_t ss;
                uint32_t es;
                uint32_t ds;
                uint32_t fs;
                uint32_t gs;
            };

            return_context frames {
                    {
                            uint32_t(vm->regs.edi),
                            uint32_t(vm->regs.esi),
                            uint32_t(vm->regs.ebp),
                            0,
                            uint32_t(vm->regs.ebx),
                            uint32_t(vm->regs.edx),
                            uint32_t(vm->regs.ecx),
                            uint32_t(vm->regs.eax),
                    },
                    {
                            uint32_t(vm->regs.eip),
                            uint32_t(vm->regs.cs),
                            uint32_t(vm->regs.eflags | (1 << 17)),
                            uint32_t(vm->regs.esp),
                            uint32_t(vm->regs.ss)
                    },
                    vm->regs.ss,
                    vm->regs.es,
                    vm->regs.ds,
                    vm->regs.fs,
                    vm->regs.gs,
            };

            asm volatile ("lea %0, %%esp\n"
                          "popa\n"
                          "iret"::"m"(frames));
        }
        default:
            printk("Unknown interrupt: %08X\n", code);
            panic("");
    }

    return -1;
}
