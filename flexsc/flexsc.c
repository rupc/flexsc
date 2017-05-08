#include "flexsc.h"

#include <asm/syscall.h>
pid_t hooked_task[FLEXSC_MAX_HOOKED];
const sys_call_ptr_t *sys_ptr;

#define SYSTHREAD_NUM_MAX 1024
static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT];

int systhread_fn(void *args);
#define SYSTHREAD_NAME_MAX 20

static void flexsc_work_handler(struct work_struct *work);


/* Declaration of workqueue */
static struct workqueue_struct *flexsc_workqueue;
static struct work_struct *flexsc_works;

int systhread_on_cpu[2] = {3, 4};


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
    size_t nentry;
    char systhread_name[SYSTHREAD_NAME_MAX];
    struct flexsc_sysentry *sysentry = info->sysentry;
    int i;

    /* need_to_create_worker(pool); */
    printk("Here is in kernel I want to exploit info struct from user to generate worker thread....\n");
    printk("%p %d %d\n",
            &(info->sysentry[0]), info->sysentry[0].nargs,
            info->sysentry[30].rstatus);
    printk("%ld %ld %ld\n", info->npages, info->total_bytes, info->nentry);
    printk("Size of task_struct %lu\n", sizeof(struct task_struct *));

    syscall_table_size = sizeof(sys_ptr);
    sys_ptr = sys_call_table;

    nentry = info->nentry; 

    if (nentry != SYSENTRY_NUM_DEFAULT) {
        printk("# of entry should be equal to %d in development mode\n", SYSENTRY_NUM_DEFAULT);
    }

    /* Allocate pointers for sysentry  */
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        systhread_pool[i] = (struct task_struct *)kmalloc(sizeof(struct task_struct *) * nentry, GFP_KERNEL);
        if (!systhread_pool[i]) {
            printk(KERN_EMERG "Allocating systhread pool failed!\n");
            return -1;
        }
    }


    flexsc_workqueue = create_workqueue("flexsc-workqueue");
    flexsc_works = (struct work_struct *)kmalloc(sizeof(struct work_struct) * nentry, GFP_KERNEL);

    for (i = 0; i < nentry; i++) {
        /* INIT_WORK(&flexsc_works[i], flexsc_work_handler); */
        FLEXSC_INIT_WORK(&flexsc_works[i], flexsc_work_handler, &(info->sysentry[i]));
    }

    /* work_struct과 flexsc_sysentry를 어떻게 조화를 시킬 것이냐 하는 문제가 남아 있음 */

    /**
     * Spawn kernel threads which have equal number of sysentry.
     * Each corresponding kernel thread takes sysentry and put it into workqueue.
     * Workqueue has its own dedicated kernel thread. This in-kernel single thread does system call.
     * And then, sysentry's sysret has a return value.
     */
    for (i = 0; i < nentry; i++) {
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
    }



    /* It should be match to the address in System.map */
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


/**
 * @brief systhread checks given systentry's status and chooses what to do.
 *
 * @param args
 *
 * @return 
 */
int systhread_fn(void *args)
{

    struct flexsc_systhread_info *sysinfo = (struct flexsc_systhread_info *)args;
    struct flexsc_sysentry *entry = sysinfo->sysentry;
    struct work_struct *syswork = sysinfo->syswork;

    unsigned short status = entry->rstatus;

    while (1) {
        if (status == FLEXSC_STATUS_FREE || FLEXSC_STATUS_DONE) {
            // do nothing, go to sleep
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
        }

        /* When waked up by flexsc_register() or outer functions */
        if (status == FLEXSC_STATUS_SUBMITTED) {
            struct flexsc_sysenry *wentry;

            /**
             * Put a work into workqueue. 
             * At this moment, rstatus is being FLEXSC_STATUS_BUSY * .
             * workqueue handler of sysentry stores return value to sysret,
             * and fills status flag with FLEXSC_STATUS_DONE
             */
            entry->rstatus = FLEXSC_STATUS_BUSY;
            queue_work_on(systhread_on_cpu[0], flexsc_workqueue, syswork);
            /* wentry = syswork->work_entry; */

            /**
             * Wait until a system call issued is done...
             */
            while(entry->rstatus == FLEXSC_STATUS_BUSY) {
            }

            /* Fill return value to sysret field of sysentry */

            /* Change request flag to DONE */
            entry->rstatus = FLEXSC_STATUS_DONE;
        }
        
    }


    // 먼저 어떤 CPU에서 실행할지를 얻어내고,
    
    // 해당 CPU 번호를 구했으먼, queue_work_on(CPU#, flexsc_workqueue, flexsc_works[entry->엔트리에 대응하는 번호?]
    // 쿼드코어 환경에서는 3,*4*번 CPU에서 돌아간다고 가정. 유저 프로그램은 1, 2번 CPU
}
static void flexsc_work2_handler(struct work_struct *work)
{
    // 여기서 실제로 시스템 콜을 수행하면 됨.
    struct flexsc_sysentry *entry = work->work_entry;
}

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
