#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
/* #include "syscall_info.h" */
#include "../libflexsc/flexsc_syscalls.h"
#include <pthread.h>

#include <sys/stat.h>
/* int stat(const char *file_name, struct stat *buf); */
#include <signal.h>

void sig_handler(int signo)
{
    printf("flexsc user program is going die SIGINT(%d)\n", SIGINT);
    /**
     * flexsc_exit terminates systhreads,
     * deallocates sysentry array */
    flexsc_exit();
    exit(EXIT_SUCCESS);
}
// This thread is responsible for checking sysentry for its status and return value
pthread_t systhread;

void *thread_main(void *arg)
{
	while (1)
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

int main(int argc, const char *argv[])
{
    struct flexsc_sysentry *entry;
    struct flexsc_sysentry *receiver;
    struct flexsc_init_info info;
    int i, num_entry, cnt = 0;
    pid_t mypid;

    signal(SIGINT, (void *)sig_handler);
    
    /*
     * You may ask "where the info struct is initialized?"
     * For ease of testing, it has default setting 
     */
    entry = flexsc_register(&info);
    printf("After registering flexsc\n");
    sleep(3);
    /* entry[3].rstatus = 5;
    entry[3].nargs = 6;
    entry[3].sysnum = 3;
    entry[3].sysret = 4;
    entry[3].args[0] = 99;
    entry[3].args[1] = 101;
    entry[3].args[2] = 103;
    entry[3].args[3] = 105;
    entry[3].args[4] = 107;
    entry[3].args[5] = 109; */

/*
 *     num_entry = info.nentry;
 *     printf("Number of entry: %d\n", num_entry);
 * 
 *     [>Print global entry whether it is set as expected<]
 *     print_sysentry(gentry);
 * 
 *     [>Print info of created sysentries<]
 *     for (i = 0; i < num_entry; i++) {
 *         print_sysentry(&entry[i]);
 *     }
 */

    /* Call getpid() - flexsc version*/
    receiver = flexsc_getpid();

    /* Do something until issued system call is done */
    while (receiver->rstatus != FLEXSC_STATUS_DONE) {
        sleep(1);
        printf("wait count: %d\n", cnt++);
    }

    /*Consumes return value*/
    mypid = receiver->sysret;

    /*Change a entry to FREE*/
    asm volatile ("" : : : "memory");
    receiver->rstatus = FLEXSC_STATUS_FREE;

    /*Usage of sysret*/
    printf("PID: %d\n", mypid);

    /* Wait until it gets SIGTERM */
    while (1) {
        sleep(100);
    }

    return 0;
}

void test_register() 
{

}
