# flexsc(osdi 2010)
trying to implement flexsc on ubuntu-linux 4.4.0

**You should take different approach to FlexSC because at the time that the paper is written(linux 2.6), it is possible to simply call kernel_thread() to create kernel_thread with sharing calling user space. But on recent kernel, kernel_thread() with (CLONE_FILES, CLONE_FD | CLONE_VM) flag causes segmentation fault.
**

## Modified files list
- Makefile -> Add flexsc 
- flexsc/
- include/linux/sched.h - struct task_struct -> Add struct syspage
- arch/x86/entry/syscalls/syscall_64.tbl -> Add system call 400. flexsc_register, 401 flexsc_wait, 402 flexsc_start_hook
- arch/x86/entry/entry_64.S -> Add system call hooking function


