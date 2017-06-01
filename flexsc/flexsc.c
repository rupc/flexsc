#include "flexsc.h"
#include <asm/syscall.h>

pid_t hooked_task[FLEXSC_MAX_HOOKED];
const sys_call_ptr_t *sys_ptr;

static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT];

int systhread_fn(void *args);
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
    char name[SYSTHREAD_NAME_MAX];
};

asmlinkage long 
sys_flexsc_exit()
{

    return 0;
}

asmlinkage long 
sys_flexsc_register(struct flexsc_init_info __user *info)
{
    struct task_struct *task;
    /* struct flexsc_sysentry *sysentry = info->sysentry; */
    size_t nentry; /* Reserved for devel mode */
    pid_t user_pid;

    nentry = info->nentry; 
    user_pid = current->pid;
    task = current;

    printk("Process(%d) on FlexSC\n", user_pid);

    if (nentry != SYSENTRY_NUM_DEFAULT) {
        printk("# of entry should be equal to %d in development mode\n", SYSENTRY_NUM_DEFAULT);
    }

    alloc_systhreads(systhread_pool, SYSENTRY_NUM_DEFAULT);
    flexsc_create_workqueue("flexsc_workqueue", flexsc_workqueue);
    alloc_workstruct(flexsc_works, info);
    spawn_systhreads(systhread_pool, info);


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
    


    task->flexsc_enabled = 1;

    /* flexsc_start_hook(task->pid); */

    return 0;
}
EXPORT_SYMBOL_GPL(sys_flexsc_register);

// 근데 이미 전역 변수로 systhread_pool이 선언될 때, 널 포인터로 초기화가 되는데,
// 그리고 여기서 루프돌면서 포인터 크기 * nentry 만큼 kmalloc하는 건 왜 하는 걸까?
// 이거 할 때 정신이 반쯤 나간 듯...
void alloc_systhreads(struct task_struct *systhread_pool[], int nentry) {
    /* Allocate struct task_struct pointers which have equal number of sysentry  */
    /* int sz = sizeof(systhread_pool) / sizeof(systhread_pool[0]); */
    int sz = SYSENTRY_NUM_DEFAULT;
    /* int i; */
    printk("INFO size of systhread_pool: %d\n", sz);
    printk("INFO Allocating task_struct pointer(#%d) for systhread...\n", sz);
    /* for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        // suspect why 
        systhread_pool[i] = (struct task_struct *)
            kmalloc(sizeof(struct task_struct *) * nentry, GFP_KERNEL);

        if (!systhread_pool[i]) {
            printk(KERN_EMERG "Allocating systhread pool failed!\n");
            return -1;
        }
    } */

}

void alloc_workstruct(struct work_struct *flexsc_works, struct flexsc_init_info *info)
{
    int nentry = info->nentry; /* Number of sysentry */
    int i;
    printk("INFO Allocating work_struct(#%d)\n", nentry);
    /* 워크 스트럭를 엔트리 갯수만큼 할당 받기 */
    flexsc_works = (struct work_struct *)kmalloc(sizeof(struct work_struct) * nentry, GFP_KERNEL);

    printk("Initializing: Binding work_struct and work_handler\n");
    for (i = 0; i < SYSENTRY_NUM_DEFAULT; i++) {
        /* INIT_WORK(&flexsc_works[i], flexsc_work_handler); */
        FLEXSC_INIT_WORK(&flexsc_works[i], flexsc_work_handler, &(info->sysentry[i]));
    }
}

void flexsc_create_workqueue(char *name, struct workqueue_struct *flexsc_workqueue) 
{
    printk("Creating FlexSC workqueue...\n");
    /* Create workqueue so that systhread can put a work */
    flexsc_workqueue = create_workqueue(name);
}

/**
 * Spawn kernel threads which have equal number of sysentry.
 * Each corresponding kernel thread takes sysentry and put it into workqueue.
 * Workqueue has its own dedicated kernel thread. This in-kernel single thread does system call.
 * And then, sysentry's sysret has a return value.
 */
void spawn_systhreads(struct task_struct *systhread_pool[], struct flexsc_init_info *info)
{
    struct flexsc_sysentry *sysentry = info->sysentry;
    int nentry = info->nentry, i;
    int sz = SYSENTRY_NUM_DEFAULT; // 8 systhreads
    char systhread_name[SYSTHREAD_NAME_MAX];

    WARN_ON(sz >= nentry);

    printk("Spawnning systhread...\n");
    for (i = 0; i < nentry; i++) {
        struct flexsc_systhread_info sysinfo = {
            .sysentry = &sysentry[i],
            .syswork = &flexsc_works[i]
        };

        snprintf(systhread_name, sizeof(systhread_name), "systhread[%d]", i);
        strcpy(sysinfo.name, systhread_name);

        /* systhread_pool[i] =
            kthread_create_on_cpu(systhread_fn, (void *)(&sysinfo),
                    systhread_on_cpu[i % 2], systhread_name); */
        systhread_pool[i] = kthread_create(systhread_fn, (void *)(&sysinfo), systhread_name);
        
        /* All systhread bound to DEFAULT_CPU(CPU 4) */
        kthread_bind(systhread_pool[i], DEFAULT_CPU); 
        wake_up_process(systhread_pool[i]);
    }

}

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
    printk("INFO: %s created\n", sysinfo->name);
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

    entry->rstatus = FLEXSC_STATUS_DONE;
}

/* Make calling thread(mostly user thread) sleep */
asmlinkage long sys_flexsc_wait(void) 
{
    /* static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT]; */
    /* int i; */
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
