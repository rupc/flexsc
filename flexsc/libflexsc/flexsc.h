#include <unistd.h>
#include <sys/types.h>

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

struct flexsc_sysentry {
    long sysargs[6];
    unsigned nargs;
    unsigned short rstatus;
    unsigned short sysnum;
    unsigned sysret;
} ____cacheline_aligned_in_smp;

void flexsc_register(void);
void flexsc_wait(void);

/* Find free sysentry and returns it */
struct flexsc_sysentry *free_syscall_entry(void);

void flexsc_hook(void);

pid_t gettid(void);
