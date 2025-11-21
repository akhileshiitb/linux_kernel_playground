// hello.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

/* 
 * This module checks out linux procfs interface
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand procfs interface");


static ssize_t	procfs_read(struct file *filep, char __user *usr_buf, size_t count, loff_t *l_off)
{
	printk("This is procfs read callback\n");

	return 0;
}


static ssize_t	procfs_write(struct file *filep, const char __user *usr_buf, size_t count, loff_t *l_off)
{
	printk("This is procfs write callback\n");

	return count;
}

static struct proc_ops proc_fops = {
	.proc_read = procfs_read,
	.proc_write = procfs_write,	
};

static int __init procfs_intf_init(void)
{
	volatile struct proc_dir_entry *dir; 

	dir = proc_mkdir("procfs_intf", NULL);

	if (!dir)
		printk("Procfs dir create issue\n");

	dir = proc_create("status", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file create issue\n");

    	printk(KERN_INFO "procfs interface  init!\n");

	printk("procfs init complete\n");

    return 0;
}

module_init(procfs_intf_init);

