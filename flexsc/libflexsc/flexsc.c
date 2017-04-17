#include "flexsc.h"
#include <stdio.h>
#include <stdlib.h>

#define SYSPAGE_PER_TASK 2

inline static void __flexsc_register(struct flexsc_reg_info *info) 
{
    syscall(400, info);

}
void flexsc_register(void)
{
    int pg_size = getpagesize();

    struct flexsc_reg_info info = {
        .max_threads = 10,
        .stack_base = 20,
        .stack_size = 30
    };

    /* void * aligned_alloc (size_t alignment, size_t size) */
    unsigned long *addr =  (unsigned long *)aligned_alloc(getpagesize(), SYSPAGE_PER_TASK);

    printf("addr : %p, sz : %lu\n", addr, sizeof(*addr));

    if (addr == NULL) {
        printf("ALLOCATION ERROR on %d\n", __LINE__);
    }

    printf("page size : %d\n", pg_size);


    __flexsc_register(&info);
    printf("%lu %lu %lu\n", info.max_threads, info.stack_base, info.stack_size);
}
void flexsc_wait(void) 
{
    syscall(401);
}

void flexsc_hook(void) 
{
    syscall(402, gettid());
}

/* glibc doesn't provide wrapper of gettid */
pid_t gettid(void) 
{
    return syscall(186);
}
