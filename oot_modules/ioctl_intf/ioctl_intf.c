#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

/* Define ioctl commands fot this module */
#define IOCTL_CMD0	_IOWR('a', 1, int)
#define IOCTL_CMD1	_IOWR('a', 2, int)

/* 
 * This module checks out linux ioctl interface
 * It uses procfs interface as backend.
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand ioctl interface");

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


long procfs_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	
	printk("This is ioctl call entry with command = %d\n", cmd);

	switch(cmd) {
	case IOCTL_CMD0 : {
		printk("executing command 0 \n");	
		break;
		 }
	case IOCTL_CMD1 : {
		printk("Executing command: 1\n");
		break;
		 }
	default: {
		printk("Command not found\n");
		break;
		 }
	}

	return 0;
}

static struct proc_ops proc_fops = {
	.proc_read = procfs_read,
	.proc_write = procfs_write,	
	.proc_ioctl = procfs_ioctl,
};

static int __init ioctl_intf_init(void)
{
	volatile struct proc_dir_entry *dir; 

	dir = proc_mkdir("ioctl_intf", NULL);

	if (!dir)
		printk("iotcl intf dir create issue\n");

	dir = proc_create("status", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file create issue\n");

    	printk(KERN_INFO "ioctl interface  init!\n");

	printk("ioctl init complete\n");

    return 0;
}

module_init(ioctl_intf_init);

