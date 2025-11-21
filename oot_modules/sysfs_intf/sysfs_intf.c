// hello.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

/* 
 * This module checks out linux sysfs interface
 * This creates two attribute files value 1 and value 2 in sysfs i.e at /sys/-
 *  from user-space:
 *  cat < /sys/sysfs_intf/value1  --> will invoke show1 function in kernel
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand sysfs interface");

/* Define kernel object for sysfs entry */
static struct kobject *k_object;
ssize_t show1(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk("Show 1 \n");

	return 0;
}

ssize_t store1(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk("store 1\n");

	return count;
}

static ssize_t show2(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk("Show 2 \n");

	return 0;
}

static ssize_t store2(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk("store 2\n");

	return count;
}

static struct kobj_attribute my_attr1 = __ATTR(value1, 0664, show1, store1) ;
static struct kobj_attribute my_attr2 = __ATTR(value2, 0664, show2, store2) ;

static int __init sysfs_intf_init(void)
{
	int ret; 
	k_object = kobject_create_and_add("sysfs_intf", NULL);

	/* Create a file in sysfs */
	ret = sysfs_create_file(k_object, &my_attr1.attr);
	ret = sysfs_create_file(k_object, &my_attr2.attr);

    	printk(KERN_INFO "sysfs interface  init!\n");

	printk("sysfs registration complete\n");

    return 0;
}

static void __exit sysfs_intf_exit(void)
{
    printk(KERN_INFO "sysfs interface exit!\n");
    kobject_put(k_object);
}

module_init(sysfs_intf_init);
module_exit(sysfs_intf_exit);

