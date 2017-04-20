#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/bitops.h>
#include <asm/uaccess.h>
#include <linux/mman.h>
#include <linux/slab.h>

#include <asm/cache.h>

#define FLEXSC_ERR_INIT_TGROUP_EMPTY 500
#define FLEXSC_ERR_INIT_COPY 501
#define FLEXSC_ERR_CACHE_LINE_MISMATCH 502

#define FLEXSC_ALREADY_HOOKED 400
#define FLEXSC_ALREADY_NOT_HOOKED 401

#define FLEXSC_STATUS_FREE 0 
#define FLEXSC_STATUS_SUBMITTED 1
#define FLEXSC_STATUS_DONE 2
#define FLEXSC_STATUS_BUSY 3

#define FLEXSC_PAGE_SIZE 4096

/* L1_CACHE_BYTES usually is 64 */
#define FLEXSC_ENTRY_SIZE L1_CACHE_BYTES

 /*
  * !Important 
  * FlexSC Configuration Options. We follow as the paper specify.
  * 8 System call pages per core, allowing up to 512 concurrent 
  * exception-less system calls per core.
  */
#define FLEXSC_MAX_SYSPAGE_PER_CPU 8
#define FLEXSC_MAX_ENTRY 64
#define FELXSC_MAX_CPUS 4

#define FLEXSC_MAX_HOOKED 100


/* 
 * Maximum Pid default by 32768 
 * sysctl -w kernel.pid_max can change the maximum pid
 * */

#define FLEXSC_MAX_PID 8192
#define BITMAP_ENTRY 64

/**
 * @brief cache line size can be determined using in-kernel function
 */
#define FLEXSC_CACHE_LINE_SZIE 64

struct flexsc_reg_info {
    unsigned long max_threads;
    unsigned long stack_base;
    unsigned long stack_size;
};

// asmlinkage void flexsc_syscall_hook(struct pt_regs *regs)
/**
 * @brief Define syscall entry. It should be same as cache line(64 bytes)
 */
struct flexsc_sysentry {
    unsigned request_status;
    unsigned syscall_number;
    long args[6];
    long ret_val;
} ____cacheline_aligned_in_smp;


/**
 * @brief syspage size should be 4KB(linux page size)
 */
struct flexsc_syspage {
    struct list_head flexsc_page_list; /* 8 bytes */
    struct flexsc_sysentry *entries[FLEXSC_MAX_ENTRY];
};

void init_syspage(volatile struct flexsc_syspage *);
void init_sysentry(volatile struct flexsc_sysentry *);
void alloc_syspage(volatile struct flexsc_syspage *);

void flexsc_start_hook(pid_t hooked_pid);
void flexsc_end_hook(pid_t hooked_pid);

int flexsc_enable_hook(void);
int flexsc_disable_hook(void);

void flexsc_map_syspage(void);
void flexsc_create_systhread_pool(void);
void flexsc_clone_systhread(void);  

void create_flexsc_systhread(void);


#define bitmap_nr(h_pid) (h_pid % 64)
#define bitmap_mem(h_pid) (h_pid / 64)

#define BITMAP_ENTRIES 128

static volatile long pid_bitmap[BITMAP_ENTRIES];

/* Manipulating pid_bitmap */
static inline void set_pid_bitmap(pid_t pid)
{
    set_bit(pid % BITMAP_ENTRY, &pid_bitmap[pid / BITMAP_ENTRIES]);
}

static inline void clear_pid_bitmap(pid_t pid)
{
    set_bit(pid % BITMAP_ENTRY, &pid_bitmap[pid / BITMAP_ENTRIES]);
}

static inline int test_pid_bitmap(pid_t pid) 
{
    return test_bit(pid % BITMAP_ENTRY, &pid_bitmap[pid / BITMAP_ENTRIES]);
}
