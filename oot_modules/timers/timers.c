// hello.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/delay.h>

/* 
 * This module checks out linux procfs interface
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand timers in linux OS");

static struct timer_list timer1;

void timer_callback(struct timer_list * timer) {
	printk("Timer callback\n");

	printk("Inside interrupt context\n");

	printk("jiffies = %lu\n", jiffies);
}

static ssize_t	procfs_read(struct file *filep, char __user *usr_buf, size_t count, loff_t *l_off)
{
	printk("Timer 1 is disabled\n");

	/* Try out delay routines */
	mdelay(5000);
	
	printk("timer procfs read complete\n");

	del_timer_sync(&timer1);

	return 0;
}


static ssize_t procfs_write(struct file *filep, const char __user *usr_buf, size_t count, loff_t *l_off)
{
	char * kernel_buffer;
	unsigned long seconds;

	kernel_buffer = kzalloc(count, GFP_KERNEL);

	copy_from_user(kernel_buffer, usr_buf, count);

	kstrtoul(kernel_buffer, 10, &seconds);

	printk("Timer 1 is enabled with %u seconds\n", seconds);

	timer1.expires = jiffies + seconds*HZ;

	timer_setup(&timer1, timer_callback, 0);

	add_timer(&timer1);

	return count;
}


static struct proc_ops proc_fops = {
	.proc_read = procfs_read,
	.proc_write = procfs_write,	
};

static int __init mumbai_timer_init(void)
{
	volatile struct proc_dir_entry *dir; 

	dir = proc_mkdir("timers", NULL);

	if (!dir)
		printk("Procfs dir create issue\n");

	dir = proc_create("timer1_control", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file create issue\n");


    	printk(KERN_INFO "timer procfs entry  init!\n");

    return 0;
}

module_init(mumbai_timer_init);

