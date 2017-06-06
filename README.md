# flexsc(osdi 2010)
trying to implement flexsc on ubuntu-linux 4.4.0

You should take *different approach* to FlexSC because at the time when the paper is written(linux 2.6), it is possible to simply call kernel_thread() to create kernel_thread with sharing memory of calling user space process. But on recent kernel, kernel_thread() with (CLONE_FILES, CLONE_FD | CLONE_VM) flag causes segmentation fault. (See [kernel_thread() causes segfault](http://www.spinics.net/lists/newbies/msg57445.html))

## Modified files list
- Makefile -> Add flexsc 
- flexsc/
- include/linux/sched.h - struct task_struct -> Add struct syspage
- arch/x86/entry/syscalls/syscall_64.tbl -> Add system call 400. flexsc_register, 401 flexsc_wait, 402 flexsc_start_hook
- arch/x86/entry/entry_64.S -> Add system call hooking function

## Memory mapping related API
- [remap_pfn_range - remap kernel memory to userspace](http://elixir.free-electrons.com/linux/latest/source/mm/memory.c#L1876)
- [kmap, kunmap](http://elixir.free-electrons.com/linux/latest/source/include/linux/highmem.h#L56)
- [High memory & Low Memory](http://egloos.zum.com/slgi97/v/10973585): mapping with HIGH_MEM of user process

## Shared memory between user thread and kernel thread
- [Performing Direct I/O: Detailed description of get_user_pages](http://www.makelinux.net/ldd3/chp-15-sect-3)
- [how-to-use-mmapproc-shared-memory-between-kernel-and-userspace](https://stackoverflow.com/questions/36762974/how-to-use-mmapproc-shared-memory-between-kernel-and-userspace)
- [how-to-use-mmap-to-share-user-space-and-kernel-threads](https://stackoverflow.com/questions/7943993/how-to-use-mmap-to-share-user-space-and-kernel-threads)
- [User space memory access from the Linux kernel](https://www.ibm.com/developerworks/library/l-kernel-memory-access/index.html)
- [SHARING MEMORY BETWEEN KERNEL AND USER SPACE IN LINUX](ftp://164.41.45.4/pub/os/rtlinux/papers/rtos-ws/p-c01_motylewski.pdf)
- [How to mmap a file in linux kernel space?](https://stackoverflow.com/questions/13465095/how-to-mmap-a-file-in-linux-kernel-space)
- [Implementing mmap for transferring data from user space to kernel space](https://coherentmusings.wordpress.com/2014/06/10/implementing-mmap-for-transferring-data-from-user-space-to-kernel-space/)
- [how-can-i-access-memory-at-known-physical-address-inside-a-kernel-module](https://stackoverflow.com/questions/4219504/how-can-i-access-memory-at-known-physical-address-inside-a-kernel-module)


## Reference
- [spinlock/flexsc](https://github.com/spinlock/flexsc)
- [kernel_thread() causes segfault](http://www.spinics.net/lists/newbies/msg57467.html)



