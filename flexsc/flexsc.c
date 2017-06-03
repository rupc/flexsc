#include "flexsc.h"
#include <asm/syscall.h>
#include <linux/delay.h>

pid_t hooked_task[FLEXSC_MAX_HOOKED];
const sys_call_ptr_t *sys_ptr;

static struct task_struct *systhread_pool[SYSENTRY_NUM_DEFAULT] = {NULL,};
/* Declaration of workqueue */
static struct workqueue_struct *flexsc_workqueue;
static struct work_struct *flexsc_works = NULL;

static int systhread_fn(void *args);
static void flexsc_work_handler(struct work_struct *work);

struct flexsc_systhread_info *_sysinfo[SYSENTRY_NUM_DEFAULT] = {NULL,};
int systhread_on_cpu[2] = {5, 6};
struct task_struct *user_task;
size_t nentry; /* Reserved for devel mode */

int thread_main(void *arg)
{
    printk("Hello kernel_thread via system call\n");
    while (1) {
        ssleep(3);
        printk("제발 좀 되라\n");

    }
    return 0;
}


pid_t kernel_creation_test(void) 
{
    const unsigned long flags = CLONE_VM | CLONE_FS | CLONE_FILES;
    pid_t pid;
    int cpu = 5;
    void *arg = (void *)(long)cpu;
    printk("smp_processor_id(): %d\n", smp_processor_id());

    pid = kernel_thread(thread_main, arg, flags);

    if (pid < 0) {
        printk("Error when create kernel_thread\n");
    }

    printk("pid: %d\n", pid);
    return pid;
}


static int thread_fn(void *unused)
{
    int cnt = 0;
    while (1)
    {
        printk(KERN_INFO "Thread Running:%d\n", cnt++);
        ssleep(5);
    }
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}

static struct task_struct *thread_st;
int kthread_test(void) 
{
    /* thread_st = kthread_run(thread_fn, NULL, "mythread"); */
    /* thread_st = kthread_create(kthread_worker_fn, NULL, "mythread"); */
    thread_st = kthread_create(thread_fn, NULL, "mythread");
    kthread_bind(thread_st, DEFAULT_CPU); 
    wake_up_process(thread_st);

    if (thread_st) {
        printk(KERN_INFO "Thread created successfully\n");
    } else {
        printk(KERN_ERR "Thread creation failed\n");
    }
    return 0;
}

static void syswork_handler(struct work_struct *work)
{
    printk("flexsc: in syswork handler\n");
}

#define MAX_THREADS 8
struct task_struct *syspool[MAX_THREADS];
struct flexsc_sysentry *k_sysentry;
/* struct flexsc_sysentry *gentry[MAX_THREADS]; */

int systhread_main(void *arg)
{
    int cnt = 0;
    /* int idx = *((int *)arg); */
    /* printk("Got an index(%d)\n", idx); */
    
    // This entry pointer exactly points to an entry with having right offset

    struct work_struct flexsc_work;
    struct flexsc_sysentry *entry = (struct flexsc_sysentry *)(arg);
    FLEXSC_INIT_WORK(&flexsc_work, syswork_handler, entry);

    ssleep(1);
    /* printk("syspool[0], [1] = %p, %p\n", syspool[0], syspool[1]); */
    printk(KERN_INFO "flexsc: systhread PID[%d] PPID[%d]\n", current->pid, current->parent->pid);
    /* printk("gentry[%d]->rstatus\n", idx, gentry[idx]->rstatus); */

    while (1)
    {
        /* printk(KERN_INFO "flexsc: systhread PID[%d] is running :%d\n", current->pid, cnt++); */
        // 현재 entry 포인터를 참조하려고 하면 에러가 발생한다는 것을 발견함.
        /* printk(KERN_INFO "Thread Running:%d, entry->rstatus:%d\n", cnt++, entry->rstatus); */
        /* printk(KERN_EMERG "PID[%d]: I'm alive! %d %d %d\n", current->pid, entry->rstatus, entry->sysnum, entry->sysret); */
        printk(KERN_EMERG "PID[%d]: I'm alive!\n", current->pid);
        /* printk("%d %d\n", k_sysentry[idx].rstatus, k_sysentry[idx].sysnum); */

        /* while (entry->rstatus == FLEXSC_STATUS_FREE ||  */
               /* entry->rstatus == FLEXSC_STATUS_DONE) { */
            ssleep(5);
        /* } */

        /* if (entry->rstatus == FLEXSC_STATUS_SUBMITTED) {
            entry->rstatus = FLEXSC_STATUS_BUSY;
            barrier();
            queue_work_on(DEFAULT_CPU, flexsc_workqueue, &flexsc_work);
        } */
    }
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}


int sthread_main(void *arg)
{
    printk("Hello kernel_thread via system call\n");
    while (1) {
        ssleep(3);
        printk("제발 좀 되라\n");

    }
    return 0;
}

int kthread_multiple_test(struct flexsc_init_info *info) 
{
    int i;
    char name[32];

    flexsc_workqueue = create_workqueue("flexsc_workqueue");

    for (i = 0; i < MAX_THREADS; i++) {
        snprintf(name, sizeof(name), "systhread[%d]", i);
        /* printk("sysentry[i] at %p\nIts value: ", &(info->sysentry[i])); */
        /* print_sysentry(&(info->sysentry[i])); // OK, This statement works */

        /* gentry[i] = &(info->sysentry[i]); */
        barrier();
        /* syspool[i] = kthread_create(systhread_main, (void *)(&(info->sysentry[i])), name); */
        syspool[i] = kthread_create(systhread_main, (void *)&(info->sysentry[i]), name);


        if (!syspool[i]) {
            printk(KERN_ERR "systhread[%d] creation failed\n", i);
            break;
        }

        printk(KERN_INFO "systhread[%d] at %p created successfully\n", i, syspool[i]);

        kthread_bind(syspool[i], DEFAULT_CPU); 
        wake_up_process(syspool[i]);
    }

    return 0;
}

void flexsc_create_workqueue(char *name, struct workqueue_struct *flexsc_workqueue) 
{
    printk("Creating flexsc workqueue...\n");
    /* Create workqueue so that systhread can put a work */
    flexsc_workqueue = create_workqueue(name);
    printk("Address of flexsc_workqueue: %p\n", flexsc_workqueue);
}

int kthread_worker_fn_test(void)
{
    thread_st = kthread_create(kthread_worker_fn, NULL, "kthread_worker");

}


static pid_t pid_systhread[MAX_THREADS];
int kernel_thread_multiple_test(struct flexsc_init_info *info)
{
    int i;
    /* flexsc_workqueue = create_workqueue("flexsc_workqueue"); */
    pid_t pid;
    int num = MAX_THREADS;

    /* num = 2; */
    for (i = 0; i < 1; i++) {
        pid = kernel_thread(sthread_main, (void *)NULL, CLONE_VM | CLONE_FS | CLONE_FILES);

        if (pid < 0) {
            printk("Error when create kernel_thread\n");
        }

        printk("flexsc: systhread[%d, %d] created successfully\n", pid, i);
        pid_systhread[i] = pid;
    }

    return 0;
}

int kthread_copy(struct flexsc_init_info *info)
{

}

asmlinkage long 
sys_flexsc_register(struct flexsc_init_info __user *info)
{
    /* kthread_test(); */
    /* kernel_creation_test(); */
    struct task_struct *task;
    /* struct flexsc_sysentry *sysentry = info->sysentry; */
    pid_t user_pid;

    nentry = info->nentry; 
    user_pid = current->pid;
    task = current;

    k_sysentry = kmalloc(sizeof(struct flexsc_sysentry) * nentry, GFP_KERNEL);
    copy_from_user((void *)k_sysentry, (void *)(info->sysentry), sizeof(struct flexsc_sysentry) * nentry);

    /* k_sysentry = (struct flexsc_sysentry *)flexsc_mmap(sizeof(struct flexsc_sysentry) * nentry, 0, NULL);
    printk("k_sysentry(%p), sz:%ld\n", k_sysentry, sizeof(k_sysentry)); */

    print_sysentry(&k_sysentry[0]);
    print_sysentry(&k_sysentry[1]);
    print_sysentry(&k_sysentry[2]);
    print_sysentry(&k_sysentry[3]);


    /* gentry = info->sysentry; */

    printk("flexsc: size of a sysentry: %ld\n", sizeof(info->sysentry[0]));
    /* print_sysentry(&((info->sysentry)[0]));
    print_sysentry(&((info->sysentry)[1]));
    print_sysentry(&((info->sysentry)[2]));
    print_sysentry(&((info->sysentry)[3])); */

    printk("flexsc: smp_processor_id(): %d\n", smp_processor_id());
    printk("flexsc: process(%d) calls flexsc_register\n", user_pid);
    printk("flexsc: number of entry: %ld\n", nentry);
    printk("flexsc: size of a entry: %ld\n", sizeof(info->sysentry[0]));

    /* alloc_systhreads(systhread_pool, SYSENTRY_NUM_DEFAULT); */
    /* alloc_workstruct(flexsc_works, info); */

    
    /* kthread_multiple_test(info); */
    kernel_thread_multiple_test(info);

    /* spawn_systhreads(systhread_pool, info); */

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


asmlinkage long sys_flexsc_exit(void)
{
    printk("%s\n", __func__);
    flexsc_destroy_workqueue(flexsc_workqueue);
    flexsc_free_works(flexsc_works);
    flexsc_stop_systhreads();
    /* flexsc_free_sysinfo(_sysinfo); */
    return 0;
}


void flexsc_destroy_workqueue(struct workqueue_struct *flexsc_workqueue)
{
    if (flexsc_workqueue == NULL) {
        printk("flexsc workqueue is empty!\n");
        return;
    }

    printk("Destroying flexsc workqueue...\n");
    destroy_workqueue(flexsc_workqueue);
}

void flexsc_free_works(struct work_struct *flexsc_works)
{
    if (flexsc_works == NULL) {
        printk("flexsc works is empty!\n");
        return;
    }

    printk("Deallocating flexsc work structs...\n");
    kfree(flexsc_works);
}

void flexsc_stop_systhreads()
{
    int i;

    for (i = 0; i < MAX_THREADS; i++) {
        if (syspool[i] == NULL) {
            continue;
        }
        printk(KERN_EMERG "Terminate systhread[%d]\n", i);
        kthread_stop(syspool[i]);
    }
}

void flexsc_free_sysinfo(struct flexsc_systhread_info *_sysinfo[])
{
    if (_sysinfo == NULL) {
        printk("_sysinfo is empty!\n");
        return;
    }

    while (*_sysinfo) {
        kfree(*_sysinfo);
        (*_sysinfo)++;
    }
}

/**
 * Spawn kernel threads which have equal number of sysentry.
 * Each corresponding kernel thread takes sysentry and put it into workqueue.
 * Workqueue has its own dedicated kernel thread. This in-kernel single thread does system call.
 * And then, sysentry's sysret has a return value.
 */
static struct task_struct *__syspool[8] = {NULL, };

void spawn_systhreads(struct task_struct *systhread_pool[], struct flexsc_init_info *info)
{
    struct flexsc_sysentry *sysentry = info->sysentry;
    int nentry = info->nentry, i;
    int sz = SYSENTRY_NUM_DEFAULT; 
    char systhread_name[128];

    /* WARN_ON(sz >= nentry); */

    printk("flexsc: Spawnning systhread...\n");
    for (i = 0; i < 8; i++) {

        _sysinfo[i] = (struct flexsc_systhread_info *)
            kmalloc(sizeof(struct flexsc_systhread_info), GFP_KERNEL);

        _sysinfo[i]->sysentry = &sysentry[i];
        _sysinfo[i]->syswork = &flexsc_works[i];

        snprintf(systhread_name, sizeof(systhread_name), "systhread[%d]", i);
        strcpy(_sysinfo[i]->name, systhread_name);

        /* systhread_pool[i] =
            kthread_create_on_cpu(systhread_fn, (void *)(&sysinfo),
                    systhread_on_cpu[i % 2], systhread_name); */
        snprintf(systhread_name, sizeof(systhread_name), "systhread[%d]", i);
        __syspool[i] = kthread_create(systhread_fn, (void *)(NULL), systhread_name);

        /* All systhread bound to DEFAULT_CPU(CPU 4) */
        kthread_bind(__syspool[i], DEFAULT_CPU); 
        wake_up_process(__syspool[i]);
    }

}

/**
 * @brief systhread checks a given systentry's status and chooses what to do.
 * @param args
 * @return 
 */
static int systhread_fn(void *args)
{
    struct flexsc_systhread_info *sysinfo = (struct flexsc_systhread_info *)args;
    struct flexsc_sysentry *entry = sysinfo->sysentry;
    struct work_struct *syswork = sysinfo->syswork;
    printk("INFO: %s created\n", sysinfo->name);
    int cnt = 0;
    while (1) {
        ssleep(3);
        printk("systhread count: %d\n", cnt++);
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
    /* printk("Waking up sleeping systhread..."); */
    printk("%d is going to sleep\n", current->pid);

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

void print_sysentry(struct flexsc_sysentry *entry)
{
    printk("%p %d-%d-%d-%d with %lu,%lu,%lu,%lu,%lu,%lu\n",
            entry,
            entry->sysnum, entry->nargs,
            entry->rstatus, entry->sysret,
            entry->args[0], entry->args[1],
            entry->args[2], entry->args[3],
            entry->args[4], entry->args[5]);
}
