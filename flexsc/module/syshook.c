/**
 * @file syscall_hooking.c
 * @brief variation of syscall hooking via dynamic loading kernel module. It locates sys_call_table and intercept system call invoked.
 * @author Yongrae Jo
 * @version 1.0
 * @date 2017
 */

#include "syshook.h"

/* syscall thread main function */
int kmain(void *arg)
{
    struct flexsc_sysentry *entry = (struct flexsc_sysentry *)arg;
    int cnt = 0;

    printk("kthread[%d, %d], user[%d, %d] starts\n", current->pid, current->parent->pid, utask->pid, utask->parent->pid);
    print_sysentry(&entry[0]);

    while (1) {
        if (cnt == 4) {
            do_exit(1);
            break;
        }
        ssleep(2);
        printk("hello! %d\n", cnt++);

        while(!kthread_should_stop()){
            schedule();
        }
    }
    return 0;
}


struct task_struct *kstruct;
asmlinkage long 
sys_hook_flexsc_register(struct flexsc_init_info __user *info)
{
    int pid, i, npinned_pages;
    struct flexsc_sysentry *entry = info->sysentry;
    phys_addr_t physical_address;

    utask = current;

    printk("flexsc_register() hooked by %d\n", current->pid);
    printk("%d\n", PAGE_SHIFT);
    printk("sizeof entry: %ld, %ld\n", sizeof(entry), sizeof(*entry));
    print_multiple_sysentry(entry, 8);

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
    mypage = virt_to_page(info->sysentry);
    physical_address = virt_to_phys(info->sysentry);

    printk("# of pinned pages:                   %d\n", npinned_pages);
    printk("pinned_pages[0]                      %p\n", pinned_pages[0]);
    printk("page_address(pinned_pages[0]):       %p\n", page_address(pinned_pages[0]));
    printk("sysentry_start_addr:                 %p\n", sysentry_start_addr);

    printk("physical address                     %p\n", (void *)physical_address);
    printk("info->sysentry                       %p\n", info->sysentry);
    printk("__pa(info->sysentry)                 %p\n", (void *)__pa(info->sysentry));
    printk("virt_to_page(info->sysentry)         %p\n", virt_to_page(info->sysentry));

#define virt_to_pfn(kaddr)	(__pa(kaddr) >> PAGE_SHIFT)
    printk("virt_to_pfn(sysentry)                %ld\n", virt_to_pfn(info->sysentry));
    printk("virt_to_phys(sysentry)               %p\n", (void *)virt_to_phys(info->sysentry));

    printk("page->virt                           %p\n", page_address(virt_to_page(info->sysentry)));
    printk("%20s\n", "After kamp(pinned_pages)");

    print_multiple_sysentry(entry, 8);
    /* entry[0].sysnum = 1000; */
    info->sysentry[0].sysnum = 9999;
    print_multiple_sysentry(entry, 1);

    kstruct = kthread_run(kmain, (void *)entry, "systhread 0");
    /* print_sysentry(&entry[1]); */
    return 0;
}


int syscall_hooking_init(void)
{
    unsigned long cr0;

    if ((sys_call_table = locate_sys_call_table()) == NULL) {
        printk("Can't find sys_call_table\n");
        return -1;
    }
    printk("sys_call_table is at [%p]\n", sys_call_table);

    cr0 = read_cr0();
    write_cr0(cr0 & ~0x00010000);

    original_call = (void *)sys_call_table[__NR_flexsc_register];
    sys_call_table[__NR_flexsc_register] = (void *)sys_hook_flexsc_register;

    write_cr0(cr0);
    printk("%d %s Hooking Init done!\n", __LINE__, __func__);
    return 0;
}

void syscall_hooking_cleanup(void)
{
    unsigned long cr0 = read_cr0();
    int i;
    write_cr0(cr0 & ~0x00010000);
    sys_call_table[__NR_flexsc_register] = (void *)original_call;
    write_cr0(cr0);
    printk("Hooking moudle cleanup\n");

    for (i = 0; i < NUM_PINNED_PAGES; i++) {
        kunmap(pinned_pages[i]);
    }

    /* kthread_stop(kstruct); */
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
    
module_init(syscall_hooking_init);
module_exit(syscall_hooking_cleanup);
MODULE_LICENSE("GPL");
    
