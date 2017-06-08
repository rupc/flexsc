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
#include <stdio.h>
#include <sys/stat.h>
#include "flexsc_cpu.h"
#include "flexsc_types.h"
#include "syscall_info.h"

struct flexsc_sysentry *flexsc_register(struct flexsc_init_info *info);
void flexsc_wait(void);
int init_info(struct flexsc_init_info *);

/* Globally used sysentry; it's used for free_syscall_entry() */
static struct flexsc_sysentry *gentry; 

/* Find free sysentry and returns it */
struct flexsc_sysentry *free_syscall_entry(void);

void flexsc_hook(void);

pid_t gettid(void);

void flexsc_exit();

static void __flexsc_register(struct flexsc_init_info *info) 
{
    printf("%s\n", __func__);
    syscall(SYSCALL_FLEXSC_REGISTER, info); 
}

void print_sysentry(struct flexsc_sysentry *entry);

long flexsc_syscall(unsigned sysnum, unsigned n, long args[6], struct flexsc_cb *cb);
void init_cpuinfo_default(struct flexsc_cpuinfo *cpuinfo);
