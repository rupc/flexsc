#include "flexsc_syscalls.h"


struct flexsc_sysentry* flexsc_getppid()
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    entry->sysnum = __NR_getppid;
    entry->nargs = __ARGS_getppid;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    return entry;
}

struct flexsc_sysentry* flexsc_getpid()
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_getpid(entry);
    return entry;
}

struct flexsc_sysentry* flexsc_read(unsigned int fd, char  *buf, size_t count)
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_read(entry, fd, buf, count);
    return entry;
}

struct flexsc_sysentry* flexsc_write(unsigned int fd, char  *buf, size_t count)
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_write(entry, fd, buf, count);
    return entry;
}


struct flexsc_sysentry* flexsc_stat(const char *pathname, struct stat *statbuf)
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_stat(entry, pathname, statbuf);
    return entry;
}


void request_syscall_stat(struct flexsc_sysentry *entry, const char *pathname, struct stat *statbuf)
{
    entry->sysnum = __NR_stat;
    entry->nargs = __ARGS_stat;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)pathname;
    entry->args[1] = (long)statbuf;
}

void request_syscall_read(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count)
{
    entry->sysnum = __NR_read;
    entry->nargs = __ARGS_read;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)fd;
    entry->args[1] = (long)buf;
    entry->args[2] = (long)count;
}

void request_syscall_write(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count)
{
    entry->sysnum = __NR_write;
    entry->nargs = __ARGS_write;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)fd;
    entry->args[1] = (long)buf;
    entry->args[2] = (long)count;
}

void request_syscall_open(struct flexsc_sysentry *entry, const char  *filename, int flags, mode_t mode)
{
    entry->sysnum = __NR_open;
    entry->nargs = __ARGS_open;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)filename;
    entry->args[1] = (long)flags;
    entry->args[2] = (long)mode;
}

void request_syscall_close(struct flexsc_sysentry *entry, unsigned int fd)
{
    entry->sysnum = __NR_close;
    entry->nargs = __ARGS_close;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)fd;
}

void request_syscall_getpid(struct flexsc_sysentry *entry)
{
    entry->sysnum = __NR_getpid;
    entry->nargs = __ARGS_getpid;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
}

/* long flexsc_getpid(struct flexsc_sysentry *entry)
{
    request_syscall_getpid(entry);
}

long flexsc_read(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count)
{
    request_syscall_read(entry, fd, buf, count);
}

long flexsc_write(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count) 
{
    request_syscall_write(entry, fd, buf, count);
}
 */
/* long flexsc_mmap(struct flexsc_sysentry *entry, unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long pgoff)
{
    request_syscall_mmap(entry, addr, len, prot, flags, fd, pgoff);
} */

/* long flexsc_stat(struct flexsc_sysentry *entry); */
