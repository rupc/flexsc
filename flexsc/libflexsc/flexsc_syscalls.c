#include "flexsc_syscalls.h"

long flexsc_getpid(struct flexsc_sysentry *entry)
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

/* long flexsc_mmap(struct flexsc_sysentry *entry, unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long pgoff)
{
    request_syscall_mmap(entry, addr, len, prot, flags, fd, pgoff);
} */

/* long flexsc_stat(struct flexsc_sysentry *entry); */
