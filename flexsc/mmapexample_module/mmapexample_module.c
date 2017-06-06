/* https://coherentmusings.wordpress.com/2014/06/10/implementing-mmap-for-transferring-data-from-user-space-to-kernel-space/ */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/file.h>

#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif
 
struct dentry  *file;
 
struct mmap_info
{
    char *data;            
    int reference;      
};
 
void mmap_open(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference++;
}
 
void mmap_close(struct vm_area_struct *vma)
{
    struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
    info->reference--;
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    struct page *page;
    struct mmap_info *info;    
     
    info = (struct mmap_info *)vma->vm_private_data;
    if (!info->data)
    {
        printk("No data\n");
        return 0;    
    }
     
    page = virt_to_page(info->data);    
     
    get_page(page);
    vmf->page = page;            
     
    return 0;
}
 
struct vm_operations_struct mmap_vm_ops =
{
    .open =     mmap_open,
    .close =    mmap_close,
    .fault =    mmap_fault,    
};
 
int op_mmap(struct file *filp, struct vm_area_struct *vma)
{
    vma->vm_ops = &mmap_vm_ops;
    vma->vm_flags |= VM_RESERVED;    
    vma->vm_private_data = filp->private_data;
    mmap_open(vma);
    return 0;
}
 
int mmapfop_close(struct inode *inode, struct file *filp)
{
    struct mmap_info *info = filp->private_data;
     
    free_page((unsigned long)info->data);
    kfree(info);
    filp->private_data = NULL;
    return 0;
}
 
int mmapfop_open(struct inode *inode, struct file *filp)
{
    struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
    info->data = (char *)get_zeroed_page(GFP_KERNEL);
    printk("%d joined\n", current->pid);
    /* memcpy(info->data, "hello from kernel this is file: ", 32); */
    /* memcpy(info->data + 32, filp->f_dentry->d_name.name, strlen(filp->f_dentry->d_name.name)); */
    /* assign this info struct to the file */
    filp->private_data = info;
    return 0;
}
 
static const struct file_operations mmap_fops = {
    .open = mmapfop_open,
    .release = mmapfop_close,
    .mmap = op_mmap,
};

struct task_struct *systhread;
extern asmlinkage long sys_mmap_pgoff(unsigned long addr, unsigned long len,
                                      unsigned long prot, unsigned long flags,
                                      unsigned long fd, unsigned long pgoff);

loff_t pos = 0;
struct file *flip;
mm_segment_t old_fs;

struct file* open_in_module(char *name, int flags, int perms)
{
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    if ( (flip = filp_open(name, flags, perms)) < 0) {
        return NULL;
    }
    return flip;
}

int close_in_module(struct file *flip)
{
    if (flip) {
        filp_close(flip, NULL);
        pos = 0;
        set_fs(old_fs);
        return 0;
    }
    return -1;
}

int syshandler(void *arg)
{
    int cnt = 0;
    open_in_module("/sys/kernel/debug/mmap_example");
    /* unsigned long prots = PROT_READ | PROT_WRITE;
    unsigned long flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_LOCKED;
    long addr;

    addr = sys_mmap_pgoff(0, roundup(size, PAGE_SIZE), prots, flags, -1, 0); */
    while (1) {
        ssleep(2);
        printk("Hello systhread! %d\n", cnt++);
    }

    return 0;
}
static void thread_init(void)
{
    systhread = kthread_run(syshandler, NULL, "haha");
}
 
static int __init mmapexample_module_init(void)
{
    /* thread_init(); */
    file = debugfs_create_file("mmap_example", 0644, NULL, NULL, &mmap_fops);
    return 0;
}
 
static void __exit mmapexample_module_exit(void)
{
    kthread_stop(systhread);
    debugfs_remove(file);
}
 
module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");
