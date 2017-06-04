/**
 * @file syscall_hooking.c
 * @brief variation of syscall hooking via dynamic loading kernel module. It locates sys_call_table and intercept system call invoked.
 * @author Yongrae Jo
 * @version 1.0
 * @date 2017
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <linux/hugetlb.h>

#define SEARCH_BASE 0xffffffff81000000
#define SEARCH_OFFSET 0xffffffffa1000000

/* location of sys_call_table in my system */
#define SYSTABLE_LOC 0xffffffff81a001c0 

/* const sys_call_ptr_t sys_call_table; */
unsigned long **sys_call_table;

unsigned long **locate_sys_call_table(void)
{
    unsigned long temp;
    unsigned long *p;
    unsigned long **sys_table;

    for (temp = SEARCH_BASE; temp < SEARCH_OFFSET; temp+=sizeof(void *)) {
        p = (unsigned long *)temp;

        if ( p[__NR_close] == (unsigned long)sys_close) {
            sys_table = (unsigned long **)p;
            return &sys_table[0];
        }

    }
    return NULL;
}

static int hooked_cnt;

asmlinkage long (*original_call)(const char __user *, int, umode_t);

asmlinkage long sys_our_open(const char __user *filename, int flags, umode_t mode) {
    printk("sys_open hooked: %d\n", hooked_cnt++);
    return (original_call(filename, flags, mode));
}


int syscall_hooking_init(void)
{
    unsigned long cr0;

    if ((sys_call_table = locate_sys_call_table()) == NULL) {
        printk("Can't find sys_call_table\n");
        return -1;
    }
    printk("sys_call_table is at [%p]\n", sys_call_table);

    /* cr0 = read_cr0();
    write_cr0(cr0 & ~0x00010000); */
    /* set_memory_rw(PAGE_ALIGN((unsigned long)sys_call_table) - PAGE_SIZE, 3); */

    original_call = (void *)sys_call_table[__NR_open];
    sys_call_table[__NR_open] = (void *)sys_our_open;

    /* write_cr0(cr0); */
    printk("Hooking done!\n");
    return 0;
}

void syscall_hooking_cleanup(void)
{
    /* unsigned long cr0 = read_cr0(); */
    /* write_cr0(cr0 & ~0x00010000); */
    sys_call_table[__NR_open] = original_call;
    /* write_cr0(cr0); */
    printk("Hooking moudle cleanup\n");
    return;
}
    
module_init(syscall_hooking_init);
module_exit(syscall_hooking_cleanup);
MODULE_LICENSE("GPL");
    
