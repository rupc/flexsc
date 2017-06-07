#include "../flexsc.h"

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/hugetlb.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <asm/page.h>
#include <asm/io.h>

#define SEARCH_BASE 0xffffffff81000000
#define SEARCH_OFFSET 0xffffffffa1000000

/* location of sys_call_table in my system */
#define SYSTABLE_LOC 0xffffffff81a001c0 

#define NUM_PINNED_PAGES 1


unsigned long **locate_sys_call_table(void);

void print_sysentry(struct flexsc_sysentry *entry);
void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n);

struct task_struct *utask;
struct page *mypage;
struct page *pinned_pages[NUM_PINNED_PAGES];
void *sysentry_start_addr;
unsigned long **sys_call_table;

asmlinkage long (*original_call)(const char __user *, int, umode_t);
