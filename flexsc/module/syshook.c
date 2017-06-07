/**
 * @file syscall_hooking.c
 * @brief variation of syscall hooking via dynamic loading kernel module. It locates sys_call_table and intercept system call invoked.
 * @author Yongrae Jo
 * @version 1.0
 * @date 2017
 */

#include "syshook.h"

/* syscall thread main function */
int queue_worker(void *arg)
{
    struct flexsc_sysentry *entry = (struct flexsc_sysentry *)arg;
    int cnt = 0;
    int i;

    printk("kthread[%d, %d], user[%d, %d] starts\n", current->pid, current->parent->pid, utask->pid, utask->parent->pid);
    printk("*****************  entry[3] before main loop  *****************\n");
    print_sysentry(&entry[3]);

    while (1) {
        /* for (i = 0; i < NUM_SYSENTRY; i++) {
            if (entry[i].rstatus == FLEXSC_STATUS_SUBMITTED) {
                printk("entry[%d].rstatus == SUBMITTED %d\n", i);
            }
        } */
        printk("*****************  entry[3]  *****************\n");
        print_sysentry(&entry[3]);

        ssleep(3);
        /* printk("hello! %d\n", cnt++); */

        if (kthread_should_stop()) {
            printk("kernel thread dying...\n");
            do_exit(0);
        }

    }
    return 0;
}


struct task_struct *kstruct;
asmlinkage long 
sys_hook_flexsc_register(struct flexsc_init_info __user *info)
{
    int i, npinned_pages;
    struct flexsc_sysentry *entry;
    /* phys_addr_t physical_address; */

    utask = current;

    /* Print first 8 sysentries */
    print_multiple_sysentry(info->sysentry, 8);

    /* Get syspage from user space 
     * and map it to kernel virtual address space */
    npinned_pages = get_user_pages(
            utask, 
            utask->mm, /* mm_strcut of user task */
            /* PAGE_ALIGN((unsigned long)(&(info->sysentry))), [>start address<] */
            (unsigned long)(&(info->sysentry[0])), /* Start address to map */
            NUM_PINNED_PAGES, /* Number of pinned pages */
            1,               /* Writable flag */
            0,               /* Force flag */
            pinned_pages, /* struct page ** pointer to pinned pages */
            NULL);

    if (npinned_pages < 0) {
        printk("Error on getting pinned pages\n");
    }

    sysentry_start_addr = kmap(pinned_pages[0]);

    entry = (struct flexsc_sysentry *)sysentry_start_addr;
    print_multiple_sysentry(entry, 8);

    kstruct = kthread_run(queue_worker, (void *)entry, "flexsc-systhread");
    return 0;
}

asmlinkage long 
sys_hook_flexsc_exit(void)
{
    int i, ret;
    printk("flexsc_exit hooked start\n");
    for (i = 0; i < NUM_PINNED_PAGES; i++) {
        kunmap(pinned_pages[i]);
    }

    ret = kthread_stop(kstruct);
    if (!ret) {
        printk("kthread stopped\n");
    }
    printk("flexsc_exit hooked end\n");
    return 0;
}

int syscall_hooking_init(void)
{
    unsigned long cr0;

    if ((sys_call_table = locate_sys_call_table()) == NULL) {
        printk("Can't find sys_call_table\n");
        return -1;
    }
    printk("-----------------------syscall hooking module-----------------------\n");
    printk("[%p] sys_call_table\n", sys_call_table);

    cr0 = read_cr0();
    write_cr0(cr0 & ~0x00010000);

    flexsc_register_orig = (void *)sys_call_table[__NR_flexsc_register];
    flexsc_exit_orig = (void *)sys_call_table[__NR_flexsc_exit];
    sys_call_table[__NR_flexsc_register] = (void *)sys_hook_flexsc_register;
    sys_call_table[__NR_flexsc_exit] = (void *)sys_hook_flexsc_exit;

    write_cr0(cr0);
    printk("%d %s syscall hooking module init\n", __LINE__, __func__);
    return 0;
}

void syscall_hooking_cleanup(void)
{
    unsigned long cr0 = read_cr0();
    write_cr0(cr0 & ~0x00010000);
    sys_call_table[__NR_flexsc_register] = (void *)flexsc_register_orig;
    sys_call_table[__NR_flexsc_exit] = (void *)flexsc_exit_orig;
    write_cr0(cr0);

    printk("Hooking moudle cleanup\n");
    return;
}

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


void print_sysentry(struct flexsc_sysentry *entry)
{
    printk("[%p] %d-%d-%d-%d with %lu,%lu,%lu,%lu,%lu,%lu\n",
            entry,
            entry->sysnum, entry->nargs,
            entry->rstatus, entry->sysret,
            entry->args[0], entry->args[1],
            entry->args[2], entry->args[3],
            entry->args[4], entry->args[5]);
}

void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n) 
{
    int i;
    for (i = 0; i < n; i++) {
        print_sysentry(&entry[i]);
    }
}

void address_stuff(void *addr)
{
    /* printk("flexsc_register() hooked by %d\n", current->pid);
    printk("%d\n", PAGE_SHIFT);
    printk("sizeof entry: %ld, %ld\n", sizeof(entry), sizeof(*entry)); */
/* 
    physical_address = virt_to_phys(info->sysentry);

    printk("# of pinned pages:                   %d\n", npinned_pages);
    printk("pinned_pages[0]                      %p\n", pinned_pages[0]);
    printk("page_address(pinned_pages[0]):       %p\n", page_address(pinned_pages[0]));
    printk("sysentry_start_addr:                 %p\n", sysentry_start_addr);

    printk("physical address                     %p\n", (void *)physical_address);
    printk("info->sysentry                       %p\n", info->sysentry);
    printk("__pa(info->sysentry)                 %p\n", (void *)__pa(info->sysentry));
    printk("virt_to_page(info->sysentry)         %p\n", virt_to_page(info->sysentry));

    printk("virt_to_pfn(sysentry)                %ld\n", virt_to_pfn(info->sysentry));
    printk("virt_to_phys(sysentry)               %p\n", (void *)virt_to_phys(info->sysentry));

    printk("page->virt                           %p\n", page_address(virt_to_page(info->sysentry)));
    printk("%20s\n", "After kamp(pinned_pages)"); */
}
    
module_init(syscall_hooking_init);
module_exit(syscall_hooking_cleanup);
MODULE_LICENSE("GPL");
    
