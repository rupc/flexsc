#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "syscall_info.h"

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

struct flexsc_sysentry {
    unsigned short request_status;
    unsigned short syscall_number;
    unsigned nargs;
    long sysargs[6];
    long ret_val;
} ____cacheline_aligned_in_smp;

int main(int argc, const char *argv[])
{
    /* printf("hello!\n"); */
    pid_t pid = syscall(__NR_getpid);

    printf("%d\n", pid);
    struct flexsc_sysentry first_entry = {
        .request_status = 0,
        .syscall_number = __NR_getpid,
        .nargs = 0,
        .ret_val = 0
    };

    printf("sizeof sysentry: %lu\n", sizeof(first_entry));
    return 0;
}
