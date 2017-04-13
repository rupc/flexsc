#include "flexsc.h"

pid_t hooked_task[FLEXSC_MAX_HOOKED];

static unsigned long test_address = 0x80000000;

static void print_reg_info(struct flexsc_reg_info *info)
{
    printk("%lu %lu %lu\n",
            info->max_threads, info->stack_base, info->stack_size);
}

static void set_reg_info(struct flexsc_reg_info *info)
{
    info->max_threads = 100;
    info->stack_base = 200;
    info->stack_size = 300;
}

asmlinkage long 
sys_flexsc_register(struct flexsc_reg_info __user *info)
{

    struct task_struct *task;
    struct flexsc_reg_info reg_info;
    print_reg_info(info);
    set_reg_info(info);
    print_reg_info(info);

    /* printk("size of flexsc_sysentry : %ld\n", sizeof(struct flexsc_sysentry)); */

    /* flexsc_sysentry size should be same as cache line */
    /* WARN_ON(sizeof(struct flexsc_sysentry) == FLEXSC_CACHE_LINE_SZIE); */

    /* Address size should be same as long */
    /* WARN_ON(sizeof(void *) == sizeof(long)); */
    
    task = current;

    /* init_syspage(task->syspage); */

    /* task->syspage = (struct flexsc_syspage *)test_address; */

    task->flexsc_enabled = 1;

    /* printk("plz:%d %p\n", task->pid, task->syspage); */
    printk("plz:%d\n", task->pid);

    if (!thread_group_empty(task)) {
        return FLEXSC_ERR_INIT_TGROUP_EMPTY;
    }

    if (copy_from_user(&reg_info, info, sizeof(reg_info)) != 0) {
        return FLEXSC_ERR_INIT_COPY;
    }

    /* Do some with user struct */

    if (copy_to_user(info, &reg_info, sizeof(reg_info) != 0)) {
        return FLEXSC_ERR_INIT_COPY;
    }


    /* printk("Start FlexSC hook here!\n"); */

    if(flexsc_enable_hook() == FLEXSC_ALREADY_HOOKED) {
        printk("Already Enabled FlexSC Hook\n");
    }

    /* printk("pid:%d\n", task->pid); */
    flexsc_start_hook(task->pid);

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
