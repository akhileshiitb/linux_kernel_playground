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
MODULE_DESCRIPTION("Module to understand high resolution timer in linux OS");

struct hrtimer mumbai_hrtimer;

enum  hrtimer_restart hrtimer_callback(struct hrtimer * hrtimer) {

	printk("High Resolution Timer callback\n");

	printk("jiffies = %lu\n", jiffies);

	return  HRTIMER_NORESTART;
}

static ssize_t hrtimer_read(struct file *filep, char __user *usr_buf, size_t count, loff_t *l_off)
{

	hrtimer_cancel(&mumbai_hrtimer);
	printk("Hr timer is disabled\n");

	return 0;
}


ssize_t hrtimer_write(struct file *filep, const char __user *usr_buf, size_t count, loff_t *l_off)
{
	char * kernel_buffer;
	unsigned long timer_unit;
	ktime_t ktime;

	kernel_buffer = kzalloc(count, GFP_KERNEL);

	copy_from_user(kernel_buffer, usr_buf, count);

	kstrtoul(kernel_buffer, 10, &timer_unit);

	ktime = ktime_set(timer_unit, 200000000); // 200ms timerout

	hrtimer_init(&mumbai_hrtimer, CLOCK_MONOTONIC , HRTIMER_MODE_REL_SOFT);
	/* Register the timer expiry callback */
	mumbai_hrtimer.function = hrtimer_callback;

	hrtimer_start(&mumbai_hrtimer, ktime, HRTIMER_MODE_REL_SOFT); // start in softirq

	printk("Timer 1 is enabled with %u units and few ms\n", timer_unit);

	return count;
}


static struct proc_ops proc_fops = {
	.proc_read = hrtimer_read,
	.proc_write = hrtimer_write,	
};

static int __init mumbai_hrtimer_init(void)
{
	struct proc_dir_entry *dir; 

	dir = proc_mkdir("hrtimers", NULL);

	if (!dir)
		printk("Procfs dir create issue\n");

	dir = proc_create("hrtimer_control", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file creation issue\n");


    	printk(KERN_INFO "High resolution timer procfs entry  init!\n");

    return 0;
}

module_init(mumbai_hrtimer_init);

