#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
/* #include "syscall_info.h" */
#include "../libflexsc/flexsc.h"
#include <pthread.h>

#include <sys/stat.h>
/* int stat(const char *file_name, struct stat *buf); */

// This thread is responsible for checking sysentry for its status and return value
pthread_t systhread;

void *thread_main(void *arg)
{

	while (true)
	{

    }

	pthread_exit((void *) 0);
}

void create_systhread(void) 
{
    pthread_create(&systhread, NULL, &thread_main, (void *)NULL);
}


/* This program contains system calls listed in table1, flexsc paper(OSDI 10),
 * Below is the system calls that will be tested
 *****************************************************************
 * stat
 * pread
 * pwrite
 * open
 * close
 * write
 * mmap
 * munmap
 *
 ******************************************************************
*/

long flexsc_syscall(int sysnum, long args[6]) {

}

struct flexsc_sysentry* flexsc_getpid()
{
    struct flexsc_sysentry *entry;
    entry = free_syscall_entry();
    request_syscall_getpid(entry);
    return entry;
}

void flexsc_write();
void flexsc_test()
{
    pid_t pid;
    struct flexsc_sysentry *entry;
    while (1) {
        entry = flexsc_getpid();

        while (entry->rstatus != FLEXSC_STATUS_DONE) {}
        /* Consumes return value */

        long ret = entry->sysret;
        entry->rstatus = FLEXSC_STATUS_FREE;
        printf("%ld\n", pid);
        sleep(1);
    }
}

int main(int argc, const char *argv[])
{
    struct flexsc_sysentry *fentry;
    struct flexsc_init_info info;
    int i;
    int num_entry;

    fentry = flexsc_register(&info);
    num_entry = info.nentry;


    request_syscall_getpid(&fentry[0]);
    char buf[20] = "hello";
    size_t sz = 20;
    request_syscall_write(&fentry[1], 1, buf, sz);

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
