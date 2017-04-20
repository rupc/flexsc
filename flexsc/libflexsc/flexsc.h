/* Copyright (C) 
 * 2017 - Yongrae Jo
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sched.h>
#include "flexsc_cpu.h"

#define SYSCALL_FLEXSC_REGISTER 400
#define SYSCALL_FLEXSC_WAIT 401
#define SYSCALL_FLEXSC_HOOK 402

/* When initialize sysentry, each field is filld with zero which means *Free*  */
#define FLEXSC_STATUS_FREE 0 
#define FLEXSC_STATUS_SUBMITTED 1
#define FLEXSC_STATUS_DONE 2
#define FLEXSC_STATUS_BUSY 3

#define FLEXSC_ERR_ALLOC 600
#define FLEXSC_ERR_LOCKSYSPAGE 601
#define FLEXSC_ERR_MAPSYSPAGE 602


struct flexsc_cpuinfo {
    int user_cpu;
    int kernel_cpu;
};

void init_cpuinfo_default(struct flexsc_cpuinfo *cpuinfo);

struct flexsc_sysentry {
    long args[6];
    unsigned nargs;
    unsigned short rstatus;
    unsigned short sysnum;
    unsigned sysret;
} ____cacheline_aligned_in_smp;

struct flexsc_init_info {
    struct flexsc_sysentry *sysentry; /* Pointer to first sysentry */
    struct flexsc_cpuinfo cpuinfo; 
    size_t npages; /* Number of Syspages */
    size_t nentry; /* # of workers should be equal to # of sysentries */
    size_t total_bytes;
};

struct flexsc_cb {
    void (*callback) (struct flexsc_cb *);
    void *args;
    int64_t ret;
};

struct flexsc_sysentry *flexsc_register(struct flexsc_init_info *info);
void flexsc_wait(void);
int init_info(struct flexsc_init_info *);

/* Find free sysentry and returns it */
struct flexsc_sysentry *free_syscall_entry(void);

void flexsc_hook(void);

pid_t gettid(void);

static void __flexsc_register(struct flexsc_init_info *info) 
{
    syscall(400, info);
}
void print_sysentry(struct flexsc_sysentry *entry);
