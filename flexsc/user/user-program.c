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

__always_inline inline void
request_syscall_getpid(struct flexsc_sysentry *entry) 
{
    entry->sysnum = __NR_getpid;
    entry->nargs = 0;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
}

__always_inline inline void 
request_syscall_write(struct flexsc_sysentry *entry, int fd, char *buf, size_t sz)
{
    entry->sysnum = __NR_write;
    entry->nargs = __ARGS_write;
    entry->rstatus = FLEXSC_STATUS_SUBMITTED;
    entry->args[0] = (long)fd;
    entry->args[1] = (long)buf;
    entry->args[2] = (long)sz;
}

/* int flexsc_getpid(struct flexsc_cb *cb, ...)
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_getpid(entry);
} */

void flexsc_write();

int main(int argc, const char *argv[])
{
    struct flexsc_sysentry *fentry;
    struct flexsc_init_info info;
    int i;
    int num_entry;

    fentry = flexsc_register(&info);
    num_entry = info.nentry;


    request_syscall_getpid(&fentry[0]);
    printf("%d\n", fentry[0].sysnum);

    printf("num entry: %d\n", num_entry);
    for (i = 0; i < num_entry; i++) {
        print_sysentry(&fentry[i]);
    }

    /* fentry = free_syscall_entry();

    request_syscall_getpid(fentry);

    while (fentry->rstatus != FLEXSC_STATUS_DONE) {
        do_something_else();
    }

    flexsc_wait(); */


    return 0;
}
    /* printf("hello!\n"); */
    /* pid_t pid = syscall(__NR_getpid);

    printf("%d\n", pid);

    printf("sizeof sysentry: %lu\n", sizeof(first_entry)); */
