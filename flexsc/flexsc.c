#include "flexsc.h"

#include <asm/syscall.h>
pid_t hooked_task[FLEXSC_MAX_HOOKED];
const sys_call_ptr_t *sys_ptr;

#define SYSTHREAD_NUM_MAX 1024
static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT];

int systhread_fn(void *args);
#define SYSTHREAD_NAME_MAX 20

#define DEFAULT_CPU 4
static void flexsc_work_handler(struct work_struct *work);


/* Declaration of workqueue */
static struct workqueue_struct *flexsc_workqueue;
static struct work_struct *flexsc_works;

int systhread_on_cpu[2] = {3, 4};

struct task_struct *user_task;

/**
 * @brief Argument to systhread 
 */
struct flexsc_systhread_info {
    struct flexsc_sysentry *sysentry;
    struct work_struct *syswork;
};

asmlinkage long 
sys_flexsc_register(struct flexsc_init_info __user *info)
{
    struct task_struct *task;
    /* struct worker_pool *pool; */
    int syscall_table_size;
    size_t nentry; /* Reserved for devel mode */
    char systhread_name[SYSTHREAD_NAME_MAX];
    struct flexsc_sysentry *sysentry = info->sysentry;
    int i;

    /* need_to_create_worker(pool); */
    /* printk("Here is in kernel I want to exploit info struct from user to generate worker thread....\n"); */
    /* printk("%p %d %d\n",
            &(info->sysentry[0]), info->sysentry[0].nargs,
            info->sysentry[30].rstatus);
    printk("%ld %ld %ld\n", info->npages, info->total_bytes, info->nentry);
    printk("Size of task_struct %lu\n", sizeof(struct task_struct *)); */

    /* Print sys_call_table information  */
    syscall_table_size = sizeof(sys_ptr);
    sys_ptr = sys_call_table;
    /* It should be match to the address in System.map */
    /* printk("sys_call_table at %p, size: %u\n", sys_ptr, syscall_table_size); */

    nentry = info->nentry; 
    user_pid = current->pid;
    printk("A process using FlexSC has a pid(%d)\n", user_pid);

    if (nentry != SYSENTRY_NUM_DEFAULT) {
        printk("# of entry should be equal to %d in development mode\n", SYSENTRY_NUM_DEFAULT);
    }

    /* Allocate struct task_struct pointers which have equal number of sysentry  */
    printk("Allocating task_structs(#%d) for systhread...\n", SYSENTRY_NUM_DEFAULT);
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        systhread_pool[i] = (struct task_struct *)
            kmalloc(sizeof(struct task_struct *) * nentry, GFP_KERNEL);

        if (!systhread_pool[i]) {
            printk(KERN_EMERG "Allocating systhread pool failed!\n");
            return -1;
        }
    }


    printk("Creating FlexSC workqueue...\n");
    /* Create workqueue that systhread can put a work */
    flexsc_workqueue = create_workqueue("flexsc-workqueue");

    printk("Allocating work_struct(#%d)\n", nentry);
    /* 워크 스트럭를 엔트리 갯수만큼 할당 받기 */
    flexsc_works = (struct work_struct *)kmalloc(sizeof(struct work_struct) * nentry, GFP_KERNEL);

    printk("Initializing: Binding work_struct and work_handler\n");
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        /* INIT_WORK(&flexsc_works[i], flexsc_work_handler); */
        FLEXSC_INIT_WORK(&flexsc_works[i], flexsc_work_handler, &(info->sysentry[i]));
    }

    /**
     * Spawn kernel threads which have equal number of sysentry.
     * Each corresponding kernel thread takes sysentry and put it into workqueue.
     * Workqueue has its own dedicated kernel thread. This in-kernel single thread does system call.
     * And then, sysentry's sysret has a return value.
     */
    printk("Spawnning systhread...\n");
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        struct flexsc_systhread_info sysinfo = {
            .sysentry = &sysentry[i],
            .syswork = &flexsc_works[i]
        };

        snprintf(systhread_name, sizeof(systhread_name), "systhread[%d]", i);

        /* systhread_pool[i] =
            kthread_create_on_cpu(systhread_fn, (void *)(&sysinfo),
                    systhread_on_cpu[i % 2], systhread_name); */
        systhread_pool[i] = kthread_create(systhread_fn, (void *)(&sysinfo), systhread_name);
        kthread_bind(systhread_pool[i], 4); // All systhread bound to CPU 4

        wake_up_process(systhread_pool[i]);
    }

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


/**
 * @brief systhread checks a given systentry's status and chooses what to do.
 * @param args
 * @return 
 */
int systhread_fn(void *args)
{
    struct flexsc_systhread_info *sysinfo = (struct flexsc_systhread_info *)args;
    struct flexsc_sysentry *entry = sysinfo->sysentry;
    struct work_struct *syswork = sysinfo->syswork;

    while (1) {
        /* If it has nothing to do, go to sleep */
        /* if (entry->rstatus == FLEXSC_STATUS_FREE 
             * || entry->rstatus ==FLEXSC_STATUS_DONE) {
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
        } */

        /* Polling wait until system call is submitted */
        while (entry->rstatus == FLEXSC_STATUS_FREE || 
               entry->rstatus == FLEXSC_STATUS_DONE) {}

        /* When waked up by flexsc_register() or other functions,
         * It checks whether a request comes in.  */
        if (entry->rstatus == FLEXSC_STATUS_SUBMITTED) {
            /**
             * Put a work into workqueue. 
             * At this moment, rstatus is being FLEXSC_STATUS_BUSY.
             * Workqueue handler of sysentry stores return value to sysret,
             * and fills status flag with FLEXSC_STATUS_DONE
             */
            entry->rstatus = FLEXSC_STATUS_BUSY;
            queue_work_on(DEFAULT_CPU, flexsc_workqueue, syswork);

            /**
             * Wait until a system call issued is done...
             * If it completes, rstatus have FLEXSC_STATUS_DONE.
             */
            while(entry->rstatus == FLEXSC_STATUS_BUSY) {}

            /* Change user thread to active if it is sleeping */
            if (user_task->state != TASK_RUNNING) {
                wake_up_process(user_task);
            }
        }
        
    }
    // 먼저 어떤 CPU에서 실행할지를 얻어내고,
    // 해당 CPU 번호를 구했으먼, queue_work_on(CPU#, flexsc_workqueue, flexsc_works[entry->엔트리에 대응하는 번호?]
    // 쿼드코어 환경에서는 3,*4*번 CPU에서 돌아간다고 가정. 유저 프로그램은 1, 2번 CPU
}

static void flexsc_work_handler(struct work_struct *work)
{
    /* Here is the place where system calls are actually executed */
    struct flexsc_sysentry *entry = work->work_entry;
    /* It doesn't need nargs because it always allocates 6 args. */
    /* unsigned nargs = entry->nargs; */
    /* long ret; */

    /* Get a sys_call_table */
    /* const sys_call_ptr_t *flexsc_sysptr = sys_call_table; */
    /* Locate first address of system call: sys_call_table[0], write */
    /* sys_call_table[entry->sysnum](entry->args[0], entry->args[1], entry->args[2]); */

    /* FLEXSC_DO_SYSCALL(entry) macro seems more powerful. 
     * It will be implemented soon.
     * Before that, program correctness comes first.
     * Fill return value to sysret field of sysentry */
    long arg0 = entry->args[0];
    long arg1 = entry->args[1];
    long arg2 = entry->args[2];
    long arg3 = entry->args[3];
    long arg4 = entry->args[4];
    long arg5 = entry->args[5];


    const int sysnum = entry->sysnum;
    entry->sysret = __syscall_XX(sysnum, arg0, arg1, arg2, arg3, arg4, arg5);

    /* Stupid code... */
    /* if (nargs == 0) {
        ret = syscall0(sysnum);
    } else if (nargs == 1) {
        ret = syscall1(sysnum, entry->args[0]);
    } else if (nargs == 2) {
        ret = syscall2(sysnum, entry->args[0], entry->args[1]);
    } else if (nargs == 3) {
        ret = syscall3(sysnum, entry->args[0], entry->args[1], entry->args[2]);
    } else if (nargs == 4) {
        ret = syscall4(sysnum, entry->args[0], entry->args[1], entry->args[2], entry->args[3]);
    } else if (nargs == 5) {
        ret = syscall5(sysnum, entry->args[0], entry->args[1], entry->args[2], entry->args[3], entry->args[4]);
    } else {
        ret = syscall6(sysnum, entry->args[0], entry->args[1], entry->args[2], entry->args[3], entry->args[4], entry->args[5]);
    } */

    /* Change request flag to DONE */
    entry->rstatus = FLEXSC_STATUS_DONE;
}

asmlinkage long sys_flexsc_wait(void) 
{
    /* static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT]; */
    int i;
    printk("Waking up sleeping systhread...");

    /* user thread goes to sleep */
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();

    /* for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        wake_up_process(systhread_pool[i]);
    } */
    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_wait);

asmlinkage long sys_flexsc_start_hook(pid_t hooked_pid) 
{
    printk("flexsc syscall version hook is here!\n");
    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_start_hook);
