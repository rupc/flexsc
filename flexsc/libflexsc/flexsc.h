#include <unistd.h>
#include <sys/types.h>
#include "flexsc_cpu.h"

#define SYSCALL_FLEXSC_REGISTER 400
#define SYSCALL_FLEXSC_WAIT 401
#define SYSCALL_FLEXSC_HOOK 402

#define FLEXSC_STATUS_FREE 0 
#define FLEXSC_STATUS_SUBMITTED 1
#define FLEXSC_STATUS_DONE 2
#define FLEXSC_STATUS_BUSY 3

struct flexsc_reg_info {
    unsigned long max_threads;
    unsigned long stack_base;
    unsigned long stack_size;
};

struct flexsc_cpuinfo {
    unsigned user_cpu;
    unsigned kernel_cpu;
};

struct flexsc_sysentry {
    long sysargs[6];
    unsigned nargs;
    unsigned short rstatus;
    unsigned short sysnum;
    unsigned sysret;
} ____cacheline_aligned_in_smp;

struct flexsc_init_info {
    struct flexsc_sysentry *sysentry; /* Pointer to first sysentry */
    unsigned npages; /* Number of syspages */
    struct flexsc_cpuinfo pinned_cpu; 
};

struct flexsc_sysentry* flexsc_register(void);
void flexsc_wait(void);

/* Find free sysentry and returns it */
struct flexsc_sysentry *free_syscall_entry(void);

void flexsc_hook(void);

pid_t gettid(void);
