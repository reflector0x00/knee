#define __machine_dev_t_defined
#define __machine_ino_t_defined
typedef unsigned long long __dev_t;
typedef unsigned long __ino_t;
static_assert(sizeof(__dev_t) == 8);
static_assert(sizeof(__ino_t) == 4);

#include "syscalls.h"
#include <fcntl.h>
#include <cstdio>
#include "liballoc.h"
#include <cstring>
#include <string>
#include "../include/syscalls.h"
#include <signal.h>
#include <sys/time.h>
#include "grp.h"
#include <cstdarg>
#undef sigaddset
#undef sigdelset
#undef sigemptyset
#undef sigfillset
#undef sigismember

#define SYSCALL0(SC) asm("mov %0, %%eax\n int $0x81" : : "i"(SC) : "eax");
#define SYSCALL1(SC, A) asm("mov %1, %%ebx\n mov %0, %%eax \n int $0x81" : : "i"(SC), "m"(A) : "eax", "ebx");
#define SYSCALL2(SC, A, B) asm("mov %1, %%ebx\n mov %2, %%ecx\n mov %0, %%eax \n int $0x81" : : "i"(SC), "m"(A), "m"(B) : "eax", "ebx", "ecx");
#define SYSCALL3(SC, A, B, C) asm("mov %1, %%ebx\n mov %2, %%ecx\n mov %3, %%edx\n mov %0, %%eax \n int $0x81" : : "i"(SC), "m"(A), "m"(B), "m"(C) : "eax", "ebx", "ecx", "edx");

//#define printd(...) printf(...)
#define printd(...)

#define DO_STUB(X) \
    char data[256];\
    snprintf(data, 256, #X " called\n"); \
    write(2, data, strlen(data));                \
    while(1);

#define STUB(X) int X(...) { char data[256]; \
snprintf(data, 256, #X  " called\n");        \
write(2, data, strlen(data));                \
while(1);                                    \
}
extern "C" {
//    void* __dso_handle;

    int open(const char *pathname, int flags, ...) {
        SYSCALL2(SYSCALL_OPEN, pathname, flags);
    }

    int fstat(int fd, struct stat *statbuf) {
        SYSCALL2(SYSCALL_FSTAT, fd, statbuf);
//        while(1);
    }

    ssize_t write(int fd, const void *buf, size_t count) {
//        printk("twrite(%d, %p, %d)\n", fd, buf, count);
        SYSCALL3(SYSCALL_WRITE, fd, buf, count);
    }

    ssize_t read(int fd, void *buf, size_t count) {
        SYSCALL3(SYSCALL_READ, fd, buf, count);
    }

    int close(int fd) {
        SYSCALL1(SYSCALL_CLOSE, fd);
    }

    [[noreturn]]
    void _exit(int status) {
        SYSCALL1(SYSCALL_EXIT, status);
        while(1);
    }
//    [[noreturn]]
//    void exit(int status) {
//        _exit(status);
//    }


    int isatty(int fd) {
        SYSCALL1(SYSCALL_ISATTY, fd);
    }

    off_t lseek(int fd, off_t offset, int whence) {
        SYSCALL3(SYSCALL_LSEEK, fd, offset, whence);
    }


    int symlink(const char *target, const char *linkpath) {
        DO_STUB(symlink called);
    }
    int chdir(const char *path) {
        SYSCALL1(SYSCALL_CHANGE_DIR, path);
    }

    int truncate(const char *path, off_t length) {
        DO_STUB(truncate);
    }
    int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
        DO_STUB(fchmodat);
    }
    long pathconf(const char *path, int name) {
        DO_STUB(pathconf);
    }
    char *getcwd(char *buf, size_t size) {
        SYSCALL2(SYSCALL_GETCWD, buf, size);
    }
    int getentropy(void *buffer, size_t length) {
        DO_STUB(getentropy);
    }
    int kill(pid_t pid, int sig) {
        DO_STUB(kill);
    }

    int liballoc_lock() {
        // pthread_mutex_lock( &mutex );
        return 0;
    }
    int liballoc_unlock() {
        // pthread_mutex_unlock( &mutex );
        return 0;
    }

    void* liballoc_alloc(size_t pages) {
        SYSCALL1(SYSCALL_ALLOC_PAGES, pages);
    }

    int liballoc_free( void* ptr, size_t pages ) {
        SYSCALL2(SYSCALL_DEALLOC_PAGES, ptr, pages);
        return 0;
    }

    bool directory_entries_count(const char* path, std::size_t* count) {
        SYSCALL2(SYSCALL_DIRENT_COUNT, path, count);
    }
    bool get_directory_entry(const char* path, std::size_t index, char* name, std::size_t size) {
        struct {
            const char* p;
            std::size_t i;
            char* n;
            std::size_t s;
        } pack {
            path,
            index,
            name,
            size
        };
        auto ptr = &pack;
        SYSCALL1(SYSCALL_GET_DIRENT, ptr);
    }





STUB(setresgid)
STUB(syslog)
STUB(pipe)
STUB(fchdir)
STUB(rmdir)
//STUB(print_signames)
STUB(__libc_current_sigrtmin)
STUB(__libc_current_sigrtmax)
STUB(getpwnam)
STUB(endpwent)
STUB(getpwent)
STUB(setpwent)
STUB(chown)
STUB(mknod)
STUB(lchown)
STUB(getgroups)
STUB(regfree)
STUB(regexec)
STUB(regcomp)
STUB(fnmatch)
STUB(binop)
STUB(dirname)
STUB(getrlimit)
STUB(setrlimit)
STUB(vfork)
STUB(wait)
STUB(sigisemptyset)
STUB(setpgrp)
STUB(dup)
STUB(globfree)
STUB(glob)
STUB(times)
STUB(timegm)
STUB(clock_gettime)
STUB(ttyname_r)
STUB(sendto)
STUB(chroot)
STUB(setresuid)
//STUB(chrootsetresuid)
//STUB(lseek64)
//STUB(pow)
//STUB(err)
//STUB(__assert_fail)
//STUB(vwarn)
STUB(getsockname)
STUB(setsockopt)
STUB(inet_addr)
STUB(inet_ntoa)
STUB(getservbyname)
STUB(ntohs)
STUB(readv)
//STUB(fbdevInitialize)
STUB(munmap)
STUB(shmat)
STUB(shmctl)
STUB(shmdt)
STUB(Error)
STUB(fchown)
STUB(execl)



struct group	*getgrnam (const char *) {
    DO_STUB(getgrnam);
    }

char *	realpath (const char *__restrict path, char *__restrict resolved_path) {
    DO_STUB(realpath);
    }

int utimes (const char *__path, const struct timeval *__tvp) {
    DO_STUB(utimes);

}


int killpg (pid_t, int) {
    DO_STUB(killpg);

    }

    int fcntl(int fd, int cmd, ... /* arg */ ) {
        switch (cmd) {
            case F_DUPFD:
            case F_DUPFD_CLOEXEC:
            case 1030: // linux F_DUPFD_CLOEXEC
                SYSCALL1(SYSCALL_DUP, fd);
                break;

            case F_GETFL:
                fprintf(stderr, "F_GETFL()\n");
                return 0;

            case F_SETFL: {
                va_list args;
                va_start(args, cmd);
                int newflags = va_arg(args, int);
                va_end(args);

                if (newflags == 0x800) // ??? idk why, but there are both...
                    newflags = O_NONBLOCK;

                SYSCALL2(SYSCALL_SETFLAGS, fd, newflags);
                fprintf(stderr, "F_SETFL(%d)\n", newflags);
                if (newflags != O_NONBLOCK) {
                    fprintf(stderr, "Unknown flag\n");
                    while (1);
                }
                return 0;
            }
            case F_SETFD: {
                va_list args;
                va_start(args, cmd);
                int fdarg = va_arg(args, int);
                va_end(args);
                fprintf(stderr, "F_SETFD(%d)\n", fdarg);
                if (fdarg != FD_CLOEXEC) {
                    fprintf(stderr, "Unknown fdarg\n");
                    while (1);
                }
                return 0;
            }

            default:
                printf("fcntl(fd=%d, cmd=%d) isn't implemented\n", fd, cmd);
                while(true);
        }
    }
#undef stdin
#undef stdout
#undef stderr
#define	_stdin	(_REENT->_stdin)
#define	_stdout	(_REENT->_stdout)
#define	_stderr	(_REENT->_stderr)

    FILE* stdin = _stdin;
    FILE* stdout = _stdout;
    FILE* stderr = _stderr;
#include <errno.h>
    int* __errno_location() {
        auto result = __errno();
        SYSCALL1(SYSCALL_SET_ERRNO_LOCATION, result);
        return result;
    }


    int mallopt(int param, int value) {
        printd("WARN: mallopt isn't implemented\n");
        return 1;
    }

    uid_t getuid(void) {
        return 0;
    }
    pid_t getppid(void) {
        return 0;
    }

    int ioctl(int fd, unsigned long request, ...) {
        printd("WARN: ioctl isn't implemented (fd = %d, request = %lu)\n", fd, request);
        return 0;
    }

    int dup2(int oldfd, int newfd) {
        SYSCALL2(SYSCALL_DUP2, oldfd, newfd);
    }
//
//    #include <setjmp.h>
//    int _setjmp(jmp_buf env) {
//        return 0;
//    }
    asm(".global _setjmp\n"
        "_setjmp:\n"
        "jmp setjmp\n");
    asm(".global __sigsetjmp\n"
        "__sigsetjmp:\n"
        "jmp setjmp\n");

    pid_t getpid(void) {
        SYSCALL0(SYSCALL_PID);
    }


#define _sigaddset(what,sig) (*(what) |= (1<<(sig)), 0)
#define _sigdelset(what,sig) (*(what) &= ~(1<<(sig)), 0)
#define _sigemptyset(what)   (*(what) = 0, 0)
#define _sigfillset(what)    (*(what) = ~(0), 0)
#define _sigismember(what,sig) (((*(what)) & (1<<(sig))) != 0)

    int sigaddset (sigset_t * set, const int sig) {
        return _sigaddset(set, sig);
    }
    int sigdelset (sigset_t * set, const int sig) {
        return _sigdelset(set, sig);
    }
    int sigismember (const sigset_t* set, int sig) {
        return _sigismember(set, sig);
    }
    int sigfillset(sigset_t *set) {
        return _sigfillset(set);
    }
    int sigemptyset (sigset_t * set) {
        return _sigemptyset(set);
    }
    int sigaction (int sig, const struct sigaction * act, struct sigaction * oldact) {
        printd("sigaction called: %d\n", sig);
        return 0;
    }

    int uname(struct utsname *buf) {
        SYSCALL1(SYSCALL_UNAME, buf);
    }
    pid_t tcgetpgrp(int fd) {
        printd("tcgetpgrp(%d) isn't implemented\n", fd);
        return 0;
    }
    pid_t getpgrp(void) {
        return 0;
    }
    int setpgid(pid_t pid, pid_t pgid) {
        printd("setpgid(%d, %d) isn't implemented\n", pid, pgid);
        return 0;
    }
    int tcsetpgrp(int fd, pid_t pgrp) {
        printd("tcsetpgrp(%d, %d) isn't implemented\n", fd, pgrp);
        return 0;
    }
    uid_t geteuid(void) {
        return 0;
    }
    gid_t getgid(void) {
        return 0;
    }
    gid_t getegid(void) {
        return 0;
    }

typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;
#define NCCS 32
struct termios
{
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_line;			/* line discipline */
    cc_t c_cc[NCCS];		/* control characters */
};

    int tcgetattr(int fd, struct termios *termios_p) {
        termios_p->c_iflag = 0;
        termios_p->c_oflag = 0;
        termios_p->c_cflag = 0;
        termios_p->c_lflag = 0;
        termios_p->c_line = 0;
        memset(termios_p->c_cc, 0, sizeof(cc_t) * NCCS);
        printd("tcgetattr(%d) isn't implemented\n", fd);
        return 0;
    }
    int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
        printd("tcsetattr(%d, %d, %08X, %08X, %08X, %08X, %02X, ...) isn't implemented\n", fd, optional_actions, termios_p->c_iflag, termios_p->c_oflag, termios_p->c_cflag, termios_p->c_lflag, termios_p->c_line);
        return 0;
    }

//#include "pwd.h"
struct passwd {
	char	*pw_name;		/* user name */
	char	*pw_passwd;		/* encrypted password */
	uid_t	pw_uid;			/* user uid */
	gid_t	pw_gid;			/* user gid */
	char	*pw_comment;		/* comment */
	char	*pw_gecos;		/* Honeywell login info */
	char	*pw_dir;		/* home directory */
	char	*pw_shell;		/* default shell */
};


    struct passwd *getpwuid(uid_t uid) {
        // TODO: omg fck, rewrite this
        char* pw_name = (char*)malloc(64);
        char* pw_passwd = (char*)malloc(64);
        char* pw_dir = (char*)malloc(64);
        char* pw_shell = (char*)malloc(64);
        struct passwd* p = (struct passwd*)malloc(sizeof(struct passwd));
        p->pw_name = pw_name;
        p->pw_passwd = pw_passwd;
        p->pw_uid = 0;
        p->pw_gid = 0;
        p->pw_comment = nullptr;
        p->pw_gecos = nullptr;
        p->pw_dir = pw_dir;
        p->pw_shell = pw_shell;

        strncpy(pw_name, "user", 64);
        strncpy(pw_passwd, "xxx", 64);
        strncpy(pw_dir, "/", 64);
        strncpy(pw_shell, "ash", 64);

        printd("getpwuid(%d) isn't implemented\n", uid);
        return p;
    }
    struct passwd* getpwuid_r(struct _reent*, uid_t uid) {
        return getpwuid(uid);
    }

/* Type used for the number of file descriptors.  */
typedef unsigned long int nfds_t;

/* Data structure describing a polling request.  */
struct pollfd
{
    int fd;			/* File descriptor to poll.  */
    short int events;		/* Types of events poller cares about.  */
    short int revents;		/* Types of events that actually occurred.  */
};

#define POLLIN		0x001		/* There is data to read.  */
#define POLLPRI		0x002		/* There is urgent data to read.  */
#define POLLOUT		0x004		/* Writing now will not block.  */

    int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
        SYSCALL3(SYSCALL_POLL, fds, nfds, timeout);
    }

    int stat(const char *pathname, struct stat *statbuf) {
        SYSCALL2(SYSCALL_STAT, pathname, statbuf);
    }
    int lstat(const char *pathname, struct stat *statbuf) {
//        printf("pathname: %s\n", pathname);
//        while(true);
//        printd("WARN: lstat calls stat\n");
        SYSCALL2(SYSCALL_LSTAT, pathname, statbuf);
    }

    pid_t fork(void) {
        SYSCALL0(SYSCALL_FORK);
    }

    int execve(const char *pathname, char *const argv[], char *const envp[]) {
        SYSCALL3(SYSCALL_EXEC, pathname, argv, envp);
    }
    extern char **environ;
    int execvp(const char *file, char *const argv[]) {
        // TODO: search
//        int* envp = environ;
//        fprintf(stderr, "execvp\n");
//        while(1);
        SYSCALL3(SYSCALL_EXEC, file, argv, environ);
    }


//    int _gettimeofday_r(struct _reent*, struct timeval *tv, struct timezone *tz) {
//        if (tv) {
//            tv->tv_sec = 0;
//            tv->tv_usec = 0;
//        }
//        if (tz) {
//            tz->tz_minuteswest = 0;
//            tz->tz_dsttime = 0;
//        }
//        return 0;
//    }

    int gettimeofday (struct timeval *__restrict tv,
                      void *__restrict __tz) {
        if (tv) {
            tv->tv_sec = 0;
            tv->tv_usec = 0;
        }
        if (__tz) {
            auto tz = reinterpret_cast<struct timezone*>(__tz);
            tz->tz_minuteswest = 0;
            tz->tz_dsttime = 0;
        }
    }

    int settimeofday (const struct timeval *, const struct timezone *) {
        DO_STUB(settimeofday);
    }
    unsigned int gnu_dev_major(dev_t dev) {
        return 0;
    }
    unsigned int gnu_dev_minor(dev_t dev) {
        return 0;
    }

    bool directory_entries_count(const char* path, std::size_t* count);
    bool get_directory_entry(const char* path, std::size_t index, char* name, std::size_t size);
struct dirent {
    ino_t          d_ino;       /* Inode number */
    off_t          d_off;       /* Not an offset; see below */
    unsigned short d_reclen;    /* Length of this record */
    unsigned char  d_type;      /* Type of file; not supported
                                                  by all filesystem types */
    char           d_name[256]; /* Null-terminated filename */
};

    struct DIR {
        char path[512];
        std::size_t ind;
        std::size_t count;
        dirent dir;
    };

    DIR *opendir(const char *name) {
        std::size_t n;
        if (!directory_entries_count(name, &n)) {
            errno = -ENOENT;
            return nullptr;
        }
        auto dir = reinterpret_cast<DIR*>(malloc(sizeof(DIR)));
        strncpy(dir->path, name, sizeof(dir->path));
        dir->ind = 0;
        dir->count = n;
        return dir;
    }


    struct dirent *readdir(DIR *dirp) {
        if (dirp->ind == dirp->count)
            return nullptr;
        char filename[256];
        if (!get_directory_entry(dirp->path, dirp->ind++, filename, sizeof(filename))) {
            DO_STUB(Undefined behaviour);
        }
        auto dir = &dirp->dir;
        dir->d_ino = 0;
        dir->d_off = 0;
        dir->d_reclen = 0;
        dir->d_type = 0;
        strncpy(dir->d_name, filename, sizeof(dir->d_name));
        return dir;
    }
    int closedir(DIR *dirp) {
        free(reinterpret_cast<void*>(dirp));
        return 0;
    }

    pid_t waitpid(pid_t pid, int *wstatus, int options) {
        SYSCALL3(SYSCALL_WAITPID, pid, wstatus, options);
    }

    ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
        char* data = reinterpret_cast<char*>(malloc(count));
        if (offset)
            lseek(in_fd, *offset, SEEK_SET);

        auto result = read(in_fd, data, count);
        write(out_fd, data, result);

        if (offset)
            *offset += result;

        free(data);
        return result;
    }
    int mkdir(const char *pathname, mode_t mode) {
        SYSCALL2(SYSCALL_MAKE_DIR, pathname, mode);
    }

    struct group *getgrgid(gid_t gid) {
        static char* gr_name = nullptr;
        static char* gr_passwd = nullptr;
        static char** gr_mem = nullptr;
        static struct group* gp = nullptr;

        if (!gp) {
            gp = reinterpret_cast<struct group*>(malloc(sizeof(struct group)));
            gr_name = reinterpret_cast<char*>(malloc(32));
            gr_passwd = reinterpret_cast<char*>(malloc(32));
            gr_mem = reinterpret_cast<char**>(malloc(2));

            strcpy(gr_name, "root");
            strcpy(gr_passwd, "test");
            gr_mem[0] = gr_name;
            gr_mem[1] = nullptr;

            gp->gr_name = gr_name;
            gp->gr_passwd = gr_passwd;
            gp->gr_gid = 0;
            gp->gr_mem = gr_mem;
        }
//        static std::size_t last_group = 0;

//        auto& result = *gp;
//        last_group %= sizeof(groups);

        return gp;
    }

    ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
        SYSCALL3(SYSCALL_READLINK, pathname, buf, bufsiz);
    }


#define _SC_OPEN_MAX 4
    // from here - Xkdrive
    // idk, why, let's make it 1024
    int getdtablesize(void) {
        return 1024;
    }

    long sysconf(int name) {
        switch (name) {
            case _SC_OPEN_MAX:
                return getdtablesize();

            default:
                fprintf(stderr, "sysconf(%d) failed\n", name);
                errno = EINVAL;
                return -1;
        }
    }

    const char *__ctype_ptr__;
    const void* __ctype_b_loc (void) {
        __ctype_ptr__ = reinterpret_cast<const char*>(_ctype_);
        return &__ctype_ptr__;
    }

    int fchmod(int fd, mode_t mode) {
        return 0;
    }

    int link(const char *oldpath, const char *newpath) {
        SYSCALL2(SYSCALL_MAKE_LINK, oldpath, newpath);
    }
    int unlink(const char *pathname) {
        SYSCALL1(SYSCALL_REMOVE_FILE, pathname);
    }


typedef struct _KdOsFuncs {
    int (*Init) (void);
    void (*Enable) (void);
    /*Bool*/ int (*SpecialKey) (void*/*KeySym*/);
    void (*Disable) (void);
    void (*Fini) (void);
    void (*pollEvents) (void);
} KdOsFuncs;

static const KdOsFuncs LinuxFuncs = {
//        LinuxInit,
//        LinuxEnable,
//        LinuxSpecialKey,
//        LinuxDisable,
//        LinuxFini,
//        0
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
};
    __attribute__((weak))
    void KdOsInit(const KdOsFuncs * const pOsFuncs) {
        fprintf(stderr, "internal KdOsInit should be never used\n");
        while (1);
    }

    void OsVendorInit() {
        KdOsInit(&LinuxFuncs);
    }

    int setitimer (int __which, const struct itimerval *__restrict __value,
               struct itimerval *__restrict __ovalue) {
        fprintf(stderr, "setitimer(%d, %p, %p)\n", __which, __value, __ovalue);
        if (__ovalue) {
            fprintf(stderr, "requested oldvalue\n");
            while (1);
        }
        return 0;
    }

    // Not real sockets yet
    int socket(int domain, int type, int protocol) {
        SYSCALL3(SYSCALL_SOCKET, domain, type, protocol);
//        static int last_id = 0;
//        char filename[256];
//        snprintf(filename, sizeof(filename), "socket%d", last_id++);
//        int result = open(filename, O_CREAT | O_RDWR);
//        return result;
    }

    typedef unsigned int socklen_t;

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        SYSCALL3(SYSCALL_CONNECT, sockfd, addr, addrlen);
//        char filename[256];
//        snprintf(filename, sizeof(filename), "socket%d", 0);
//        auto result = open(filename, O_RDWR);
//        auto result2 = dup2(result, sockfd);
//        fprintf(stderr, "connect(%d) = %d, %d\n", sockfd, result, result2);
//        return result2 != -1;
    }


#define SOL_SOCKET 1
#define SO_ACCEPTCONN 30
#define SO_BROADCAST 6
#define SO_DONTROUTE 5
#define SO_ERROR 4
#define SO_KEEPALIVE 9
#define SO_LINGER 13
#define SO_OOBINLINE 10
#define SO_RCVBUF 8
#define SO_RCVLOWAT 18
#define SO_REUSEADDR 2
#define SO_SNDBUF 7
#define SO_SNDLOWAT 19
#define SO_TYPE 3
    int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen) {

        switch (level) {
//            case IPPROTO_TCP:
//                switch (optname) {
//
//                    case TCP_NODELAY:
//                        return 0;
//
//                    default:
//                        fprintf(stderr, "getsockopt(%d, IPPROTO_TCP) optname: %d\n", sockfd, optname);
//                        while(1);
//                }
//                break;

            default:
                fprintf(stderr, "getsockopt(%d) Unknown level/optname: %d/%d\n", sockfd, level, optname);
                errno = ENOPROTOOPT;
                return -1;
        }
    }

    mode_t	umask (mode_t __mask ) {
        return __mask;
    }

    int	chmod (const char *__path, mode_t __mode ) {
        return 0;
    }

    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
//        fprintf(stderr, "bind(%d) (not implemented)\n", addrlen);
        SYSCALL3(SYSCALL_BIND, sockfd, addr, addrlen);
        return 0;
    }

    int listen(int sockfd, int backlog) {
        SYSCALL2(SYSCALL_LISTEN, sockfd, backlog)
    }
    void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset) {

        struct {
            void *addr;
            size_t length;
            int prot;
            int flags;
            int fd;
            off_t offset;
        } pack {
                addr,
                length,
                prot,
                flags,
                fd,
                offset
        };
        auto ptr = &pack;
        SYSCALL1(SYSCALL_MMAP, ptr);

    }

    int iopl(int level) {
        SYSCALL1(SYSCALL_IOPL, level);
    }

    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
        errno = EINVAL;
        return -1;
    }
    int __getpagesize() {
        return 4096;
    }
    int getpagesize() {
        return __getpagesize();
    }


typedef struct _KdKeyboardFuncs {
    void (*Load) (void);
    int (*Init) (void);
    void (*Leds) (int);
    void (*Bell) (int, int, int);
    void (*Fini) (void);
    int LockLed;
} KdKeyboardFuncs;

void LinuxKeyboardLoad() {}
int LinuxKeyboardInit() {
    return 1;
}
void LinuxKeyboardLeds(int) {}
void LinuxKeyboardBell(int, int, int) {}
void LinuxKeyboardFini() {}


extern const KdKeyboardFuncs LinuxKeyboardFuncs = {
        LinuxKeyboardLoad,
        LinuxKeyboardInit,
        LinuxKeyboardLeds,
        LinuxKeyboardBell,
        LinuxKeyboardFini,
        3,
};

typedef struct _KdMouseFuncs {
    int /*Bool*/(*Init) (void);
    void (*Fini) (void);
} KdMouseFuncs;

int MouseInit() {
    return 1;
}
void MouseFini() {}

extern const KdMouseFuncs LinuxMouseFuncs = {
        MouseInit,
        MouseFini
};

int __isoc99_sscanf(const char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsscanf(buffer, format, args);
    va_end(args);
    return result;
}

int __isoc99_fscanf(FILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfscanf(stream, format, args);
    va_end(args);
    return result;
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout) {
    struct {
        int nfds;
        fd_set *readfds;
        fd_set *writefds;
        fd_set *exceptfds;
        struct timeval *timeout;
    } pack {
            nfds,
            readfds,
            writefds,
            exceptfds,
            timeout
    };
    auto ptr = &pack;
    SYSCALL1(SYSCALL_SELECT, ptr);

}

STUB(check_internal_connections)
STUB(__assert_fail)
STUB(append_pending_request)
STUB(fpathconf)
STUB(siglongjmp)
STUB(grantpt)
STUB(setsid)
STUB(unlockpt)
STUB(ptsname)
STUB(ttyname)
STUB(ttyslot)
STUB(execlp)
STUB(tcflush)
STUB(usleep)
STUB(tgetstr)
STUB(tgetent)
STUB(getpwnam_r)
STUB(gethostbyaddr)
STUB(dlsym)

void		 endgrent (void) {
    DO_STUB(endgrent);
}
int	pclose (FILE *) {
    DO_STUB(pclose);
}

FILE *  popen (const char *, const char *) {
    DO_STUB(popen);
}

int creat (const char *, mode_t) {
    DO_STUB(popen);
}

int setpriority(int which, id_t who, int prio) {
    return 0;
}

unsigned int alarm(unsigned int seconds) {
    fprintf(stderr, "alarm(%d)\n", seconds);
    return 0;
}


int sigsuspend(const sigset_t *mask) {
    errno = EINTR;
    return -1;
}

static __inline uint16_t __bswap_16(uint16_t __x)
{
    return __x<<8 | __x>>8;
}

static __inline uint32_t __bswap_32(uint32_t __x)
{
    return __x>>24 | __x>>8&0xff00 | __x<<8&0xff0000 | __x<<24;
}

static __inline uint64_t __bswap_64(uint64_t __x)
{
    return __bswap_32(__x)+0ULL<<32 | __bswap_32(__x>>32);
}

#define bswap_16(x) __bswap_16(x)
#define bswap_32(x) __bswap_32(x)
#define bswap_64(x) __bswap_64(x)

uint16_t htons(uint16_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_16(n) : n;
}

uint32_t htonl(uint32_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_32(n) : n;
}

/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* This macro is used to declare the initial common members
   of the data types used for socket addresses, `struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */

#define        __SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

#define __SOCKADDR_COMMON_SIZE        (sizeof (unsigned short int))
typedef uint32_t in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};


struct sockaddr {
    sa_family_t sa_family;
    char sa_data[];
};
//
//struct sockaddr_in
//{
//    __SOCKADDR_COMMON (sin_);
//    in_port_t sin_port;                 /* Port number.  */
//    struct in_addr sin_addr;            /* Internet address.  */
//
//    /* Pad to size of `struct sockaddr'.  */
//    unsigned char sin_zero[sizeof (struct sockaddr)
//                           - __SOCKADDR_COMMON_SIZE
//                           - sizeof (in_port_t)
//                           - sizeof (struct in_addr)];
//};
struct sockaddr_in {
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
    unsigned char  sin_zero[8];
};

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    SYSCALL3(SYSCALL_GETPEERNAME, sockfd, addr, addrlen);
//    auto ptr = reinterpret_cast<sockaddr_in*>(addr);
//    ptr->sin_family = /*AF_INET*/ 2;
//    ptr->sin_addr.s_addr = htonl(127 << 24);
//    ptr->sin_port = htons(5000);
//    fprintf(stderr, "getpeername called\n");
//    while(1);
    return 0;
}
struct hostent
{
    char *h_name;			/* Official name of host.  */
    char **h_aliases;		/* Alias list.  */
    int h_addrtype;		/* Host address type.  */
    int h_length;			/* Length of address.  */
    char **h_addr_list;		/* List of addresses from name server.  */
#ifdef __USE_MISC
# define	h_addr	h_addr_list[0] /* Address, for backward compatibility.*/
#endif
};
#define AF_UNIX 1
#define AF_INET 2
struct hostent *gethostbyname(const char *name) {
    static hostent h;
    if (!h.h_name)
        h.h_name = reinterpret_cast<char*>(malloc(64));

    strcpy(h.h_name, name);
    if (!h.h_aliases) {
        h.h_aliases = reinterpret_cast<char**>(malloc(sizeof(char**)));
        *h.h_aliases = nullptr;
    }
    h.h_addrtype = AF_INET;
    h.h_length = 4;
    if (!h.h_addr_list) {
        h.h_addr_list = reinterpret_cast<char**>(malloc(sizeof(char**)));
        *h.h_addr_list = nullptr;
    }
    return &h;
//    fprintf(stderr, "gethostbyname(%s)\n", name);
//    while(1);
}
int gethostname(char *name, size_t len) {
    strncpy(name, "localhost", len);
    return 0;
}


unsigned int sleep(unsigned int seconds) {
    for (std::size_t i = 0; i < seconds; ++i) {
        for (std::size_t j = 0; j < 0x10000; ++j)
            asm volatile ("nop");
    }
    return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    SYSCALL3(SYSCALL_ACCEPT, sockfd, addr, addrlen);
}

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
    ssize_t result = 0;
    for (int i = 0; i < iovcnt; ++i)
        result += write(fd, iov[i].iov_base, iov[i].iov_len);
    return result;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    if (flags != 0)
        fprintf(stderr, "recv flags isn't zero: %d\n", flags);
    return read(sockfd, buf, len);
}
int shutdown(int sockfd, int how) {
    return close(sockfd);
}

int setuid(uid_t uid) {
    return 0;
}
int cfsetispeed(struct termios *termios_p, speed_t speed) {
    return 0;
}
int cfsetospeed(struct termios *termios_p, speed_t speed) {
    return 0;
}
int setgid(gid_t gid) {
    return 0;
}
int setegid(gid_t egid) {
    return 0;
}
int seteuid(uid_t euid) {
    return 0;
}

struct _stat {
    __dev_t st_dev;
    uint16_t __pad1;
    __ino_t st_ino;
    __mode_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    __dev_t st_rdev;
    uint16_t __pad2;
    uint32_t st_size;
    uint32_t st_blksize;
    uint32_t st_blocks;
    timespec st_atim;
    timespec st_mtim;
    timespec st_ctim;
    unsigned int __unused4;
    unsigned int __unused5;
};

int access(const char *pathname, int mode) {
    _stat statbuf;
    int ret = stat(pathname, reinterpret_cast<struct stat*>(&statbuf));
    if (ret < 0) {
        fprintf(stderr, "access(%s, %o) failed\n", pathname, mode);
        errno = ENOENT;
        return -1;
    }
    return 0;
}

size_t __ctype_get_mb_cur_max() {
    return 2;
}

void *dlopen(const char *filename, int flags) {
    fprintf(stderr, "try of dlopen(%s, %d)\n", filename, flags);
    return 0;
}

int shmget(key_t key, size_t size, int shmflg) {
    return -1;
//    fprintf(stderr, "shmget(%d, %d, %d)\n", key, size, shmflg);
//    while(1);
}

}