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
#include <linux/sched.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/hugetlb.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/types.h>

#include <asm/page.h>
#include <asm/io.h>
#include "../flexsc.h"

#define SEARCH_BASE 0xffffffff81000000
#define SEARCH_OFFSET 0xffffffffa1000000

/* location of sys_call_table in my system */
#define SYSTABLE_LOC 0xffffffff81a001c0 

/* const sys_call_ptr_t sys_call_table; */
unsigned long **sys_call_table;

extern pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);


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

int thread_main(void *arg)
{
    printk("Hello kernel_thread via system call\n");
    while (1) {
        ssleep(3);
        printk("\n");

    }
    return 0;
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
int haha=3;
struct task_struct *utask;
struct page *mypage;
#define NUM_PINNED_PAGES 1
struct page *pinned_pages[NUM_PINNED_PAGES];
void *sysentry_start_addr;

/* syscall thread main function */
int kmain(void *arg)
{
    struct flexsc_sysentry *entry = (struct flexsc_sysentry *)arg);
    /* struct flexsc_sysentry *entry = (struct flexsc_sysentry *)phys_to_virt((phys_addr_t)arg); */
    /* struct flexsc_sysentry *entry = (struct flexsc_sysentry *)arg; */
    struct flexsc_sysentry tmp;
    int is_ok;
    int cnt = 0;

    /* printk("************glbal %d\n", haha);
    printk("Got phys        %p\n", (void *)arg);
    printk("phys_to_virt    %p\n", entry); */
    /* printk("%d           \n", entry->rstatus);   */

    printk("Kthread[%d, %d], User[%d, %d] starts\n", current->pid, current->parent->pid, utask->pid, utask->parent->pid);
    print_sysentry(&entry[0]);
    printk("given sysentry %p\n", entry);

    struct mm_struct *my_mm = utask->mm;
    struct vm_area_struct *my_vma = my_mm->mmap;

    /* struct page *_syspage = alloc_page(GFP_KERNEL);
    void *virt_syspage = page_address(_syspage);
    printk("virt_syspage        %p\n", virt_syspage); */

    /* remap_pfn_range(my_vma, my_vma->start, 0, 1024, PAGE_SHARED); */
    /* is_ok = access_ok(VERIFY_WRITE, entry, 64);

    if (copy_from_user((void *)&tmp, (void *)entry, 64)) {
        printk("copy error\n");
        return -EFAULT;
    }
    if (is_ok) {
        printk("is_ok %d\n", is_ok);
        [>printk("%p->%d\n", &tmp, tmp.]);<]
        print_sysentry(&tmp);
    } */
    while (1) {
        if (cnt == 4) {
            do_exit(1);
            break;
        }
        ssleep(2);
        printk("hello! %d\n", cnt++);

        /* while(!kthread_should_stop()){
            schedule();
            break;
        } */
    }
    return 0;
}

void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n) 
{
    int i;
    for (i = 0; i < n; i++) {
        print_sysentry(&entry[i]);
    }
}
struct task_struct *kstruct;
asmlinkage long sys_hook_flexsc_register(struct flexsc_init_info __user *info)
{
    int pid;
    /* pid = kernel_thread(thread_main, (void *)NULL, CLONE_VM | CLONE_FS | CLONE_FILES); */
    /* pid = _do_fork(CLONE_VM | CLONE_FS | CLONE_FILES, (unsigned long)sthread_main, (unsigned long)NULL, NULL, NULL, 0); */

    /* if (pid < 0) {
        printk("error\n");
    } */
    struct flexsc_sysentry *entry = info->sysentry;
    int i;
    int npinned_pages;
    utask = current;
    phys_addr_t physical_address;
    physical_address = virt_to_phys(info->sysentry);

    printk("flexsc_register() hooked by %d\n", current->pid);
    printk("%d\n", PAGE_SHIFT);
    printk("sizeof entry: %ld, %ld\n", sizeof(entry), sizeof(*entry));
    print_multiple_sysentry(entry, 8);

    npinned_pages = get_user_pages(
            utask, 
            utask->mm, 
            /* PAGE_ALIGN((unsigned long)(&(info->sysentry))), [>start address<] */
            (unsigned long)(&(info->sysentry[0])), /* start address */
            NUM_PINNED_PAGES, /* number of pinned pages */
            1,               /* writable flag */
            0,               /* force flag */
            pinned_pages, /* struct page ** pointer to pinned pages */
            NULL);

    if (npinned_pages < 0) {
        printk("Error on getting pinned pages\n");
    }

    sysentry_start_addr = kmap(pinned_pages[0]);
    entry = (struct flexsc_sysentry *)sysentry_start_addr;
    mypage = virt_to_page(info->sysentry);

    printk("# of pinned pages:                   %d\n", npinned_pages);
    printk("pinned_pages[0]                      %p\n", pinned_pages[0]);
    printk("page_address(pinned_pages[0]):       %p\n", page_address(pinned_pages[0]));
    printk("sysentry_start_addr:                 %p\n", sysentry_start_addr);

    printk("physical address                     %p\n", (void *)physical_address);
    printk("info->sysentry                       %p\n", info->sysentry);
    printk("__pa(info->sysentry)                 %p\n", (void *)__pa(info->sysentry));
    printk("virt_to_page(sysentry)               %p, %p, %ld\n", mypage, virt_to_page(info->sysentry),virt_to_page(info->sysentry));

#define virt_to_pfn(kaddr)	(__pa(kaddr) >> PAGE_SHIFT)
    printk("virt_to_pfn(sysentry)                %p\n", virt_to_pfn(info->sysentry));
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

    original_call = sys_call_table[__NR_flexsc_register];
    sys_call_table[__NR_flexsc_register] = (void *)sys_hook_flexsc_register;

    write_cr0(cr0);
    printk("Hooking done!\n");
    return 0;
}

void syscall_hooking_cleanup(void)
{
    unsigned long cr0 = read_cr0();
    int i;
    write_cr0(cr0 & ~0x00010000);
    sys_call_table[__NR_flexsc_register] = original_call;
    write_cr0(cr0);
    printk("Hooking moudle cleanup\n");

    for (i = 0; i < NUM_PINNED_PAGES; i++) {
        kunmap(pinned_pages[i]);
    }

    /* kthread_stop(kstruct); */
    return;
}
    
module_init(syscall_hooking_init);
module_exit(syscall_hooking_cleanup);
MODULE_LICENSE("GPL");
    
