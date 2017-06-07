#include "flexsc.h"
#include <stdio.h>
#include <stdlib.h>

#define SYSPAGE_PER_TASK 1


/**
 * @brief * Initialize which CPUs are pinned to user threads or kernel threads.
 * Default setting assumes 8 cores which has 4 cores enabled with Hyper threading.
 * But for measuring correct result, Turing HT off is recommended.
 * @param cpuinfo
 */
void init_cpuinfo_default(struct flexsc_cpuinfo *cpuinfo)
{
    cpuinfo->user_cpu = FLEXSC_CPU_CLEAR;
    cpuinfo->kernel_cpu = FLEXSC_CPU_CLEAR;

    /* cpuinfo->user_cpu = FLEXSC_CPU_0 | FLEXSC_CPU_1;
    cpuinfo->kernel_cpu = FLEXSC_CPU_2 | FLEXSC_CPU_3; */

    cpuinfo->user_cpu = FLEXSC_CPU_0 | FLEXSC_CPU_1 | FLEXSC_CPU_2 | FLEXSC_CPU_3;
    cpuinfo->kernel_cpu = FLEXSC_CPU_4 | FLEXSC_CPU_5 | FLEXSC_CPU_6 | FLEXSC_CPU_7;

}

void init_user_affinity(struct flexsc_cpuinfo *ucpu)
{
    cpu_set_t user_set;
    int ucpus = ucpu->user_cpu;
    int cpu_num = 0;

    CPU_ZERO(&user_set);
    /* CPU_SET(ucpus, &user_set); */
    /* printf("ucpus:%x\n", ucpus); */

    while (ucpus) {
        if (ucpus & 0x1) {
            CPU_SET(cpu_num, &user_set);
        }

        ucpus = ucpus >> 1;
        ++cpu_num;
    }
    /* printf("ucpus:%x\n", ucpus); */

    /* CPU_SET(0, &user_set);
    CPU_SET(1, &user_set);
    CPU_SET(2, &user_set);
    CPU_SET(3, &user_set); */
    sched_setaffinity(0, sizeof(cpu_set_t), &user_set);
}

/* Prevent syspage from swapping out */
int init_lock_syspage(struct flexsc_init_info *info)
{
    int error;

    if (info->sysentry == NULL) {
        return -1;
    }
    
    error = mlock(info->sysentry, info->total_bytes);
    
    if (error == 0) {
        return FLEXSC_ERR_LOCKSYSPAGE;
    }

    return 0;
}

int init_map_syspage(struct flexsc_init_info *info)
{
    size_t pgsize = getpagesize();
    size_t total = pgsize * SYSPAGE_PER_TASK;
    struct flexsc_sysentry *entry;

    info->npages = SYSPAGE_PER_TASK;
    /* entry = (struct flexsc_sysentry *)aligned_alloc(pgsize, info->npages); */
    entry = (struct flexsc_sysentry *)aligned_alloc(pgsize, pgsize * (info->npages));
    /* entry = (struct flexsc_sysentry *)malloc(pgsize); */
    entry[0].rstatus = 0;

    if (entry == NULL) {
        return FLEXSC_ERR_MAPSYSPAGE;
    }

    info->nentry = total / sizeof(entry[0]);
    info->sysentry = entry;
    info->total_bytes = total;

    info->sysentry[0].sysnum = 1;
    info->sysentry[0].rstatus = 2;
    info->sysentry[0].nargs = 3;
    info->sysentry[0].sysret = 4;
    info->sysentry[0].args[0] = 10;
    info->sysentry[0].args[1] = 11;
    info->sysentry[0].args[2] = 12;
    info->sysentry[0].args[3] = 13;
    info->sysentry[0].args[4] = 14;
    info->sysentry[0].args[5] = 15;

    info->sysentry[1].sysnum = 11;
    info->sysentry[1].rstatus = 22;
    info->sysentry[1].nargs = 33;
    info->sysentry[1].sysret = 44;
    info->sysentry[1].args[0] = 100;
    info->sysentry[1].args[1] = 110;
    info->sysentry[1].args[2] = 120;
    info->sysentry[1].args[3] = 130;
    info->sysentry[1].args[4] = 140;
    info->sysentry[1].args[5] = 150;

    info->sysentry[7].sysnum = 7;
    info->sysentry[7].rstatus = 77;
    info->sysentry[7].sysret = 777;
    info->sysentry[7].nargs = 7777;
    info->sysentry[7].args[0] = 1;
    info->sysentry[7].args[1] = 2;
    info->sysentry[7].args[2] = 3;
    info->sysentry[7].args[3] = 4;
    info->sysentry[7].args[4] = 5;
    info->sysentry[7].args[5] = 6;

    /* print_sysentry(&(info->sysentry[0])); */


    return 0;
}

int init_info_default(struct flexsc_init_info *info) 
{
    /* Allocate syspage and map it to user space */
    init_map_syspage(info);
    /* Prevent syspage from swapping out */
    init_lock_syspage(info);

    init_cpuinfo_default(&(info->cpuinfo));

    /* Set CPU Affinity */
    init_user_affinity(&(info->cpuinfo));
    return 0;
}

int init_info(struct flexsc_init_info *info)
{
    init_info_default(info);

    return 0;
}

void print_init_info(struct flexsc_init_info *info) 
{
    printf("flexsc_init_info\n");
    printf("number of sysentry: %ld\n", sizeof(info->sysentry) / sizeof(info->sysentry[0]));
    printf("starting address of sysentry: %p\n", info->sysentry);
    printf("user cpu:%x, kernel cpu:%x\n", (info->cpuinfo).user_cpu, (info->cpuinfo).kernel_cpu);
    printf("npage: %ld\n", info->npages);
    printf("nentry: %ld\n", info->nentry);
    printf("total_bytes: %ld\n", info->total_bytes);
    printf("user pid: %d, ppid: %d\n", getpid(), getppid());
}

struct flexsc_sysentry *
flexsc_register(struct flexsc_init_info *info)
{
    /* Currently default setting is used for correctness */
    init_info(info);
    print_init_info(info);
    __flexsc_register(info);
    /* flexsc_hook(); */

    /* Set global sysentry to registered entry */
    gentry = info->sysentry;
    return info->sysentry;
}


void flexsc_exit()
{
    syscall(SYSCALL_FLEXSC_EXIT);
}

void flexsc_wait(void) 
{
    syscall(SYSCALL_FLEXSC_WAIT);
}

void flexsc_hook(void) 
{
    syscall(SYSCALL_FLEXSC_HOOK, gettid());
}

/* glibc doesn't provide wrapper of gettid */
pid_t gettid(void) 
{
    return syscall(186);
}

long flexsc_syscall(unsigned sysnum, unsigned n, long args[6], struct flexsc_cb *cb)
{

}

struct flexsc_sysentry *free_syscall_entry(void)
{
    int i;
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        if (gentry[i].rstatus == FLEXSC_STATUS_FREE) {
            return &gentry[i];
        }
    }
}
