#include <unistd.h>
#include <sys/types.h>

#define SYSCALL_FLEXSC_REGISTER 400
#define SYSCALL_FLEXSC_WAIT 401
#define SYSCALL_FLEXSC_HOOK 402

struct flexsc_reg_info {
    unsigned long max_threads;
    unsigned long stack_base;
    unsigned long stack_size;
};

struct flexsc_sysentry {
    long sysargs[6];
    unsigned rstatus;
    unsigned sysnum;
    unsigned sysret;
};

void flexsc_register(void);
void flexsc_wait(void);


void flexsc_hook(void);

pid_t gettid(void);
