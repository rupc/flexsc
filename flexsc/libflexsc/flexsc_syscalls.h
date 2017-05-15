#include "flexsc.h"
#include "syscall_info.h"
#include <sys/types.h>

struct call_info {
    struct flexsc_sysentry *entry;
    struct flexsc_cb *cb;
};

/* long flexsc_getpid(struct call_info *info);
long flexsc_read(struct call_info *info);
long flexsc_write(struct call_info *info);
long flexsc_mmap(struct call_info *info);
long flexsc_stat(struct call_info *info);
 */

long flexsc_getpid(struct flexsc_sysentry *entry);
long flexsc_read(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count);
long flexsc_write(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count);
// long flexsc_mmap(struct flexsc_sysentry *entry, unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long pgoff);

void request_syscall_read(struct flexsc_sysentry *entry, unsigned int fd, char  *buf, size_t count)
{
    entry->sysnum = __NR_read;
    entry->nargs = __ARGS_read;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)fd;
    entry->args[1] = (long)buf;
    entry->args[2] = (long)count;
}

void request_syscall_write(struct flexsc_sysentry *entry, unsigned int fd, const char  *buf, size_t count)
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
