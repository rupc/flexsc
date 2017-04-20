#include "flexsc.h"
#include <stdio.h>
#include <stdlib.h>

#define SYSPAGE_PER_TASK 2


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
    printf("ucpus:%x\n", ucpus);

    while (ucpus) {
        if (ucpus & 0x1) {
            CPU_SET(cpu_num, &user_set);
        }

        ucpus = ucpus >> 1;
        ++cpu_num;
    }
    printf("ucpus:%x\n", ucpus);

    /* CPU_SET(0, &user_set);
    CPU_SET(1, &user_set);
    CPU_SET(2, &user_set);
    CPU_SET(3, &user_set); */
    sched_setaffinity(0, sizeof(cpu_set_t), &user_set);
}

int init_info_default(struct flexsc_init_info *info) 
{
    info->npages = SYSPAGE_PER_TASK;
    info->sysentry = (struct flexsc_sysentry *)aligned_alloc(getpagesize(), info->npages);

    /* info->sysentry[0].nargs = 3; */
    /* printf("%p\n", info->sysentry); */
    init_cpuinfo_default(&(info->cpuinfo));

    if (info->sysentry == NULL) {
        printf("ALLOCATION ERROR, %s %d\n", __FUNCTION__,  __LINE__);
        return FLEXSC_ERR_ALLOC;
    }

    /* Set CPU Affinity */
    init_user_affinity(&(info->cpuinfo));
    return 0;
}

int init_info(struct flexsc_init_info *info)
{
    init_info_default(info);

    return 0;
}


struct flexsc_sysentry *
flexsc_register(struct flexsc_init_info *info)
{
    init_info(info);

    return info->sysentry;

    /* __flexsc_register(&info); */
}

void flexsc_wait(void) 
{
    syscall(401);
}

void flexsc_hook(void) 
{
    syscall(402, gettid());
}

/* glibc doesn't provide wrapper of gettid */
pid_t gettid(void) 
{
    return syscall(186);
}
