#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "syscall_info.h"
#include "../libflexsc/flexsc.h"

/* This program contains system calls listed in table1, flexsc paper(OSDI 10),
 * Below is the system calls which I want to test
 *****************************************************************
 * stat
 * pread
 * pwrite
 * open
 * close
 * write
 * mmap
 * munmap
 * pread
 * pwrite
 *
 ******************************************************************
*/

inline void request_syscall_pid(struct flexsc_sysentry *entry)
{
    entry->sysnum = __NR_getpid;
    entry->nargs = 0;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
}

inline void request_syscall_write(struct flexsc_sysetnry *entry, int fd, char *buf, size_t sz)
{
    entry->sysnum = __NR_write;
    entry->nargs = __ARGS_write;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = fd;
    entry->args[1] = buf;
    entry->args[2] = sz;
        
}

int main(int argc, const char *argv[])
{
    struct flexsc_sysetnry *fentry;

    flexsc_register();

    fentry = free_syscall_entry();

    request_syscall_pid(fentry);

    while (fentry->rstatus != FLEXSC_STATUS_DONE) {
        do_something_else();
    }

    flexsc_wait();

    return 0;
}
    /* printf("hello!\n"); */
    pid_t pid = syscall(__NR_getpid);

    printf("%d\n", pid);

    printf("sizeof sysentry: %lu\n", sizeof(first_entry));
