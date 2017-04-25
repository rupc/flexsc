#include "flexsc.h"
#include <asm/syscall.h>

/* const extern sys_call_ptr_t sys_call_table[]; */

/* asmlinkage const sys_call_ptr_t sys_call_table[__NR_syscall_max+1]; */

/* const sys_call_ptr_t *sys_ptr; */

void init_systhread(struct flexsc_init_info *info)
{
    /* int syscall_table_size = sizeof(sys_ptr);

    sys_ptr = sys_call_table;

    printk("sys_call_table at %p, size: %u\n", sys_ptr, syscall_table_size); */

    return;
}
