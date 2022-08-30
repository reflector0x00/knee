#define __machine_dev_t_defined
#define __machine_ino_t_defined
typedef unsigned long long __dev_t;
typedef unsigned long __ino_t;
static_assert(sizeof(__dev_t) == 8);
static_assert(sizeof(__ino_t) == 4);
#include <panic.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <tty_system.h>
#include <process.h>
#include <allocator.h>
#include <net.h>


/* Type used for the number of file descriptors.  */
typedef unsigned long int nfds_t;

/* Data structure describing a polling request.  */
struct pollfd {
    int fd;			/* File descriptor to poll.  */
    short int events;		/* Types of events poller cares about.  */
    short int revents;		/* Types of events that actually occurred.  */
};

#define POLLIN		0x001		/* There is data to read.  */
#define POLLPRI		0x002		/* There is urgent data to read.  */
#define POLLOUT		0x004		/* Writing now will not block.  */


// TODO: solve this bug with stat structures
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

#define MAP_ANONYMOUS 0x20


#define AF_UNIX 1
#define AF_INET 2

#define SOCK_STREAM 1
#define SOCK_CLOEXEC 02000000

/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* This macro is used to declare the initial common members
   of the data types used for socket addresses, `struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */

typedef uint32_t in_addr_t;
typedef uint32_t in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};


#define	__SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

#define __SOCKADDR_COMMON_SIZE	(sizeof (unsigned short int))

typedef uint16_t in_port_t;

struct sockaddr
{
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
};


struct sockaddr_in
{
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr)
                           - __SOCKADDR_COMMON_SIZE
                           - sizeof (in_port_t)
                           - sizeof (struct in_addr)];
};

#define UNIX_PATH_MAX	108
struct sockaddr_un {
    __SOCKADDR_COMMON (sun_); /* AF_UNIX */
    char sun_path[UNIX_PATH_MAX];	/* pathname */
};

typedef unsigned int socklen_t;


static int set_errno_int(int _errno) {
    current_process->set_errno(_errno);
    return -1;
}

static nullptr_t set_errno_ptr(int _errno) {
    current_process->set_errno(_errno);
    return nullptr;
}


extern "C" {

int kill(pid_t pid, int sig) {
    panic("ERROR: kill isn't implemented\n");
}

int isatty(int fd) {
    static bool warn_once = false; // TODO: make as class
    if (!warn_once) {
        warn_once = true;
        printk("WARNING: isatty isn't implemented\n");
    }
    if (fd >= 0 && fd <= 2)
        return 1;
    return 0;
}

int getentropy(void *buffer, size_t length) {
    panic("ERROR: getentropy isn't implemented\n");
}

pid_t getpid(void) {
    return current_process->process_id;
}


pid_t waitpid(pid_t pid, int *wstatus, int options) {
    if (pid != -1) {
        for (auto iter = current_process->childs.begin(); iter != current_process->childs.end(); ++iter) {
            auto proc = *iter;
            if (proc->process_id == pid) {
                if (options & WNOHANG) {
                    if (proc->status == status_exited)
                        return pid;

                    return set_errno_int(ECHILD);
                }

                while (proc->status != status_exited);

                *wstatus = (proc->exit_code & 0xFF) << 8;
                current_process->childs.erase(iter);
                return proc->process_id;
            }
        }
        panic("No such child\n");
    }

    while (true) {
        bool is_any = false;
        for (auto iter = current_process->childs.begin(); iter != current_process->childs.end(); ++iter) {
            auto proc = *iter;
            if (proc->status == status_exited) {
                *wstatus = (proc->exit_code & 0xFF) << 8;
                current_process->childs.erase(iter);
                return proc->process_id;
            }
            is_any = true;
        }

        if (options & WNOHANG) {
            if (is_any)
                return 0;

            return set_errno_int(ECHILD);
        }
    }
}

int open(const char *pathname, int flags, ...) {
    vfs_entry* parent_entry = resolve_parent(pathname);
    if (parent_entry == nullptr) {
        printk("Unknown parent directory: %s\n", pathname);
        return set_errno_int(ENOENT);
    }

    std::string filename = get_filename(pathname);
    vfs_entry* target = parent_entry->resolve(filename);
    if (!target) {
        if (!(flags & O_CREAT)) {
            printk("Not existing file without O_CREAT: %s\n", pathname);
            return set_errno_int(ENOENT);
        }

        target = parent_entry->create_file(filename);
        if (!target)
            panic("File creation failed");
    }

    // TODO: loop risk, stack overflow
    if (target->is_link())
        return open(path_to_entry(target->get_link(), target->get_parent())->get_path().c_str(), flags);

    if (!target->open())
        panic("Can't open file for unknown reason");

    return register_entry_as_opened(target);
}


int fstat(int fd, struct stat *statbuf) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    // TODO: other fields
    statbuf->st_size = entry->get_size();
    return 0;
}

ssize_t write(int fd, const void *buf, size_t count) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    bool noblock = iter->second.get_flags() & O_NONBLOCK;

    entry->seek(iter->second.get_position(), file_whence_t::whence_begin);
    auto result = static_cast<ssize_t>(entry->write(buf, count, noblock));
    iter->second.set_position(entry->tell());
    return result;
}


ssize_t read(int fd, void *buf, size_t count) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    bool noblock = iter->second.get_flags() & O_NONBLOCK;

    entry->seek(iter->second.get_position(), file_whence_t::whence_begin);
    auto result = static_cast<ssize_t>(entry->read(buf, count, noblock));
    iter->second.set_position(entry->tell());
    return result;
}


int close(int fd) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    if (!entry->close())
        panic("Failed to close");

    current_process->descriptor_to_open.erase(fd);
    return 0;
}


off_t lseek(int fd, off_t offset, int whence) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    file_whence_t fw;
    switch (whence) {
        case SEEK_SET:
            fw = whence_begin;
            break;
        case SEEK_CUR:
            fw = whence_current;
            break;
        case SEEK_END:
            fw = whence_end;
            break;
        default:
            panic("Unknown whence");
    }
    auto result = entry->seek(offset, fw);
    iter->second.set_position(result);
    return result;
}


char* getcwd(char* buf, size_t size) {
    auto cwd = current_process->current_directory->get_path();
    if (size < cwd.size())
        return set_errno_ptr(ERANGE);
    strncpy(buf, cwd.c_str(), size);
    return buf;
}

int dup2(int oldfd, int newfd) {
    auto iter = current_process->descriptor_to_open.find(oldfd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    auto iter2 = current_process->descriptor_to_open.find(newfd);
    if (iter2 != current_process->descriptor_to_open.end())
        close(newfd);

    current_process->descriptor_to_open.emplace(newfd, iter->second);
    return newfd;
}

int dup(int fd) {
    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    return register_entry_as_opened(entry); // TODO: copy attributes
}


int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    if (nfds != 1) {
        printk("nfds > 1 isn't implemented in poll\n");
        panic("");
    }

    auto iter = current_process->descriptor_to_open.find(fds->fd);
    if (iter == current_process->descriptor_to_open.end())
        return set_errno_int(EBADF);
    auto entry = iter->second.get_entry();

    if ((fds->events & ~(POLLIN | POLLOUT)) != 0) {
        printk("New mask in poll: %08X\n", fds->events);
        panic("");
    }
    int result = 0;
    // TODO: not real timeout
    while (true) {
        if (entry->available_read()) {
            fds->revents |= POLLIN;
            result = 1;
            break;
        }

        if (fds->events & POLLOUT) {
            fds->revents |= POLLOUT;
            result = 1;
            break;
        }

        if (timeout != -1) {
            if (!--timeout)
                break;
        }
    }

    return result; // number of fds that triggered
}


int stat(const char *pathname, struct stat *statbuf) {
    vfs_entry *entry = path_to_entry(pathname);
    if (entry == nullptr)
        return set_errno_int(ENOENT);
    else if (entry->is_link()) // TODO: may cause loop, stack overflow
        return stat(path_to_entry(entry->get_link(), entry->get_parent())->get_path().c_str(), statbuf);

    auto _statbuf = reinterpret_cast<_stat*>(statbuf);


    _statbuf->st_dev = 0;
    _statbuf->st_ino = 0;
    _statbuf->st_mode = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO); // ALLPERMS
    if (entry->is_directory())
        _statbuf->st_mode |= _IFDIR;
    else if (entry->is_file())
        _statbuf->st_mode |= _IFREG;
    else
        panic("Unknown file type");

    auto size = entry->get_size();
    auto blocks = size / 512;
    if (size % 512)
        ++blocks;
    _statbuf->st_nlink = 0;
    _statbuf->st_uid = 0;
    _statbuf->st_gid = 0;
    _statbuf->st_rdev = 0;
    _statbuf->st_size = size;

//    _statbuf->st_atim.tv_sec = 0;
//    _statbuf->st_atim.tv_nsec = 0;
//    _statbuf->st_mtim.tv_sec = 0;
//    _statbuf->st_mtim.tv_nsec = 0;
//    _statbuf->st_ctim.tv_sec = 0;
//    _statbuf->st_ctim.tv_nsec = 0;

    return 0;
}

int lstat(const char *pathname, struct stat *statbuf) {
    vfs_entry* entry = path_to_entry(pathname);
    if (entry == nullptr)
        return set_errno_int(ENOENT);

    auto _statbuf = reinterpret_cast<_stat*>(statbuf);

    _statbuf->st_dev = 0;
    _statbuf->st_ino = 0;
    _statbuf->st_mode = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO); // ALLPERMS
    if (entry->is_directory())
        _statbuf->st_mode |= _IFDIR;
    else if (entry->is_file())
        _statbuf->st_mode |= _IFREG;
    else if (entry->is_link())
        _statbuf->st_mode |= _IFLNK;
    else
        panic("unknown file type\n");

    _statbuf->st_nlink = 0;
    _statbuf->st_uid = 0;
    _statbuf->st_gid = 0;
    _statbuf->st_rdev = 0;
    _statbuf->st_size = entry->get_size();
//    _statbuf->st_atim.tv_sec = 0;
//    _statbuf->st_atim.tv_nsec = 0;
//    _statbuf->st_mtim.tv_sec = 0;
//    _statbuf->st_mtim.tv_nsec = 0;
//    _statbuf->st_ctim.tv_sec = 0;
//    _statbuf->st_ctim.tv_nsec = 0;

    return 0;
}


int chdir(const char *path) {
    vfs_entry* entry = path_to_entry(path);
    if (entry == nullptr)
        return set_errno_int(ENOENT);

    current_process->current_directory = entry;
    return 0;
}

int mkdir(const char *pathname, mode_t mode) {
    if (create_directory(pathname))
        return 0;
    return -1; // TODO: errno
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    vfs_entry* entry = path_to_entry(pathname);
    if (entry == nullptr)
        return set_errno_int(ENOENT);

    if (!entry->is_link())
        return set_errno_int(EINVAL);

    auto link = path_to_entry(entry->get_link(), entry->get_parent())->get_path();
    strncpy(buf, link.c_str(), bufsiz);
    if (link.length() + 1 > bufsiz)
        return bufsiz;
    return link.length() + 1;
}

int link(const char *oldpath, const char *newpath) {
    if (!create_symbolic_link(newpath, oldpath))
        return -1; // TODO: errno
    return 0;
}

int unlink(const char *pathname) {
    auto entry = resolve_parent(pathname);
    if (!entry)
        panic("Parent isn't exists");

    auto filename = get_filename(pathname);
    auto subentry = entry->resolve(filename);
    if (!subentry)
        return set_errno_int(ENOENT);


    if (subentry->is_directory())
        panic("Pathname is a directory");

    if (!entry->remove_file(filename))
        panic("Failed for delete file");

    return 0;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (flags & MAP_ANONYMOUS)
        panic("Anonymous mapping");

    auto iter = current_process->descriptor_to_open.find(fd);
    if (iter == current_process->descriptor_to_open.end())
        panic("Unknown file descriptor");

    auto entry = iter->second.get_entry();
    auto path = entry->get_path();

    // TODO: omg, hardcode... :'(
    if (path == "/dev/mem") {
        for (std::size_t i = 0; i < length; i += 4096) {
            memory_map(ptr_t(addr) + i, offset + i, true);
        }
        return addr;
    }
    else if (path == "/dev/zero") {
        for (std::size_t i = 0; i < length; i += 4096)
            memory_map(ptr_t(addr) + i, offset + i, true);
        memset(addr, 0, length);
        return addr;
    }
    else {
        printk("Can't map file %s\n", path.c_str());
        panic("");
    }

}


int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout) {

    if (timeout == nullptr)
        panic("timout is null\n");

    // TODO: timeout
    if (writefds)
        panic("writefds isn't null\n");

    if (exceptfds)
        panic("exceptfds isn't null\n");

    bool found = false;
    int total = 0;
    for (int i = 0; i < nfds; ++i) {
        if (FD_ISSET(i, readfds)) {
            found = true;
            auto iter = current_process->descriptor_to_open.find(i);
            if (iter == current_process->descriptor_to_open.end())
                panic("Not opened fd");
            auto entry = iter->second.get_entry();
            if (entry->available_read())
                ++total;
            else
                FD_CLR(i, readfds);
        }
    }
    if (!found)
        panic("select: not found\n");
    return total;
}



int socket(int domain, int type, int protocol) {
    switch (domain) {
        case AF_UNIX:
            if (type & SOCK_CLOEXEC)
                type &= ~SOCK_CLOEXEC;

            switch (type) {
                case SOCK_STREAM:
                    switch (protocol) {
                        case 0: {
                            auto new_socket = create_unix_socket();
                            auto result = register_entry_as_opened(new_socket);
                            printk("socket() = %d\n", result);
                            return result;
                        }

                        default:
                            printk("socket(AF_INET, SOCK_STREAM, Unknown protocol(%d))\n", protocol);
                            panic("");
                    }
                    break;

                default:
                    printk("socket(AF_INET, Unknown type(%d), ...)\n", type);
                    panic("");
            }
            break;

        case AF_INET:
        default:
            printk("socket(Unknown domain(%d), ...)\n", domain);
            panic("");
    }


}


int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    auto& opened = get_opened_file(sockfd);
    auto sock = reinterpret_cast<socket_base*>(opened.get_entry()); // TODO: dyn cast
    if (!sock)
        panic("Failed to retrieve socket");
    if (!addr)
        panic("Addr is null?\n");
    switch (addr->sa_family) {
        case AF_UNIX: {
            if (!sock->is_unix_socket())
                panic("Not a unix socket\n");

            auto sock_un = reinterpret_cast<unix_socket*>(sock);
            auto addr_un = reinterpret_cast<const struct sockaddr_un *>(addr);
            auto path = addr_un->sun_path;

            vfs_entry* entry = path_to_entry(path);
            if (entry)
                panic("bind: Already exists\n");

            entry = resolve_parent(path);
            if (!entry)
                panic("bind: Parent isn't exists");

            auto filename = get_filename(path);
            sock_un->bind_name(filename);
            entry->add_entry(sock_un);
            break;
        }

        default:
            printk("bind(Unknown family(%d), ...)\n", addr->sa_family);
            panic("");
    }
    return 0;
}


int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    auto& opened = get_opened_file(sockfd);
    auto sock = reinterpret_cast<socket_base*>(opened.get_entry()); // TODO: dyn cast
    if (!sock)
        panic("Failed to retrieve socket");
    if (!addr)
        panic("Addr is null?\n");
    if (!addrlen)
        panic("Addrlen is null?\n");
    if (sock->is_unix_socket()) {
        auto sock_un = reinterpret_cast<unix_socket*>(sock);
        auto addr_un = reinterpret_cast<struct sockaddr_un *>(addr);

        auto path = sock_un->get_path();
        addr_un->sun_family = AF_UNIX;
        strncpy(addr_un->sun_path, path.c_str(), UNIX_PATH_MAX);
        *addrlen = sizeof(addr_un->sun_family) + path.length();
    }
    else
        panic("Unknown socket type\n");
    return 0;
}

int listen(int sockfd, int backlog) {
    auto& opened = get_opened_file(sockfd);
    auto sock = reinterpret_cast<socket_base*>(opened.get_entry()); // TODO: dyn cast
    if (!sock)
        panic("Failed to retrieve socket");
    if (!sock->listen(backlog))
        panic("Can't listen");
    return 0;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printk("try of connect\n");

    auto& opened = get_opened_file(sockfd);
    auto sock = reinterpret_cast<socket_base*>(opened.get_entry()); // TODO: dyn cast
    if (!sock)
        panic("connect: Failed to retrieve socket");
    switch (addr->sa_family) {
        case AF_UNIX: {
            if (!sock->is_unix_socket())
                panic("Not a unix socket\n");

            auto sock_un = reinterpret_cast<unix_socket*>(sock);
            auto addr_un = reinterpret_cast<const struct sockaddr_un *>(addr);
            auto path = addr_un->sun_path;

            auto entry = reinterpret_cast<socket_base*>(path_to_entry(path));
            if (!entry) {
                printk("connect(%d) ENOENT\n", sockfd);
                return set_errno_int(ENOENT);
            }

            if (!entry->is_unix_socket())
                panic("connect: not a unix socket\n");

            if (!sock_un->connect(entry)) {
//                    ECONNREFUSED
                panic("Failed to connect\n");
            }
            printk("connect successfull\n");
            return 0;
        }

        default:
            printk("connect(Unknown family(%d), ...)\n", addr->sa_family);
            panic("");
    }
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    auto& opened = get_opened_file(sockfd);
    auto sock = reinterpret_cast<socket_base*>(opened.get_entry()); // TODO: dyn cast
    if (!sock)
        panic("connect: Failed to retrieve socket");
    if (!addr)
        panic("Addr is null");
    if (!addrlen)
        panic("Addrlen is null");
    auto new_sock = sock->accept();
    if (!new_sock)
        panic("Failed to accept");
    if (new_sock->is_unix_socket()) {
        auto sock_un = reinterpret_cast<unix_socket*>(new_sock);
        auto addr_un = reinterpret_cast<struct sockaddr_un *>(addr);

        auto path = sock_un->get_path();
        addr_un->sun_family = AF_UNIX;
        strncpy(addr_un->sun_path, path.c_str(), UNIX_PATH_MAX);
        *addrlen = sizeof(addr_un->sun_family) + path.length();
    }
    else
        panic("Unknown socket type\n");


    auto result = register_entry_as_opened(new_sock);
    printk("accept: %d\n", result);
    return result;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    panic("gettimeofday isn't implemented");
}
}