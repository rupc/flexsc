#include "flexsc.h"

#include <asm/syscall.h>
pid_t hooked_task[FLEXSC_MAX_HOOKED];
const sys_call_ptr_t *sys_ptr;

asmlinkage long 
sys_flexsc_register(struct flexsc_init_info __user *info)
{

    struct task_struct *task;
    struct worker_pool *pool;
    int syscall_table_size;
    /* need_to_create_worker(pool); */
    printk("Here is in kernel I want to exploit info struct from user to generate worker thread....\n");
    printk("%p %d %d\n",
            &(info->sysentry[0]), info->sysentry[0].nargs,
            info->sysentry[30].rstatus);
    printk("%ld %ld %ld\n", info->npages, info->total_bytes, info->nentry);

    syscall_table_size = sizeof(sys_ptr);
    sys_ptr = sys_call_table;

    printk("sys_call_table at %p, size: %u\n", sys_ptr, syscall_table_size);

    /* init_systhread(info); */
    /* flexsc_sysentry size should be same as cache line */
    /* WARN_ON(sizeof(struct flexsc_sysentry) == FLEXSC_CACHE_LINE_SZIE); */

    if (sizeof(struct flexsc_sysentry) != FLEXSC_CACHE_LINE_SZIE) {
        /* printk(KERN_EMERG "Mismatch for cache line size
                and sysentry\n"); */
        return FLEXSC_ERR_CACHE_LINE_MISMATCH;
    }
    

    /* Address size should be same as long */
    /* WARN_ON(sizeof(void *) == sizeof(long)); */
    
    task = current;


    task->flexsc_enabled = 1;

    /* flexsc_start_hook(task->pid); */

    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_register);

asmlinkage long sys_flexsc_wait(void) 
{
    printk("flexsc wait is here!\n");
    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_wait);

asmlinkage long sys_flexsc_start_hook(pid_t hooked_pid) 
{
    printk("flexsc syscall version hook is here!\n");
    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_start_hook);
