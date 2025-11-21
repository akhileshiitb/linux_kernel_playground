// hello.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

/* 
 * This module creates a class for character device and adds 10 devices of this class.
 * Also implements basic read/write file operations.
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand char driver");

#define CLASS_NAME "char_intf_class"
#define DEVICE_NAME "char_intf_device"
#define MAX_CHAR_DEVICES 10

static dev_t dev_num;
static struct class *char_intf_device_class;
static struct device *char_intf_device;
static struct cdev *char_intf_device_cdev;
static struct cdev *char_intf_device_cdev_10;



static ssize_t char_intf_read( struct file *filep, char __user *buf, size_t count, loff_t *f_pos) 
{
	/* get minor number for the device */
	dev_t dev_num = filep->f_inode->i_rdev;
	unsigned int minor_num = MINOR(dev_num);
	
	printk("char interface read is called\n");

	/* Driver takes action based on minor number */
	switch(minor_num) {
		case 0:
			printk("minor 0 read\n");	
			break;
		case 1:
			printk("minor 1 read\n");
			break;
		case 2:
			printk("minor 2 read\n");	
			break;
		case 3:
			printk("minor 3 read\n");
			break;
		default:
			printk("minor %d read\n", minor_num);
			break;
	}
	return 0;
}


static ssize_t char_intf_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos) 
{
	dev_t dev_num = filep->f_inode->i_rdev;
	unsigned int minor_num = MINOR(dev_num);

	printk("char interface write is called\n");

	/* Driver takes action based on minor number */
	switch(minor_num) {
		case 0:
			printk("minor 0 write\n");	
			break;
		case 1:
			printk("minor 1 write\n");
			break;
		case 2:
			printk("minor 2 write\n");	
			break;
		case 3:
			printk("minor 3 write\n");
			break;
		default:
			printk("minor %d write\n", minor_num);
			break;
	}

	return count;
}

static ssize_t char_intf_write_10(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos) 
{
	printk(KERN_WARNING "write on device with minor version 10\n");	

	return count;
}

static ssize_t char_intf_read_10( struct file *filep, char __user *buf, size_t count, loff_t *f_pos) 
{

	printk(KERN_WARNING "Read on device with minor version 10\n");	

	return 0;
}

static const struct file_operations char_intf_device_fops = {
	.owner = THIS_MODULE,
	.read = char_intf_read,
	.write = char_intf_write,
};
static const struct file_operations char_intf_device_fops_10 = {
	.owner = THIS_MODULE,
	.read = char_intf_read_10,
	.write = char_intf_write_10,
};

static int __init char_intf_init(void)
{
	int ret; 

    	printk(KERN_INFO "Char interface  init!\n");
	printk("Character driver loaded\n");

	/* Allocate device number: major and minor numbers for 10 devices supported */
	ret = alloc_chrdev_region(&dev_num, 0, MAX_CHAR_DEVICES + 1, DEVICE_NAME);

	/* Create a device class for this module*/
	char_intf_device_class = class_create(CLASS_NAME);
	
	/* Allocate the memory for cdev structure in kernel */
	char_intf_device_cdev = cdev_alloc();

	/* Map the file operations and init cdev strcture */
	cdev_init(char_intf_device_cdev, &char_intf_device_fops);

	ret = cdev_add(char_intf_device_cdev, dev_num, MAX_CHAR_DEVICES);

	/* Create devices */
	for (int i=0; i< MAX_CHAR_DEVICES; i += 1) {
		dev_t device_number = MKDEV(MAJOR(dev_num), i);
		char_intf_device = device_create(char_intf_device_class, NULL, device_number, NULL, DEVICE_NAME "%d", i);
	}

	// minor version 10 has different file operations
	/* Allocate cdev strcuture in kernel */
	char_intf_device_cdev_10 = cdev_alloc();
	cdev_init(char_intf_device_cdev_10, &char_intf_device_fops_10);
	/* Add the cdev in the system */
	ret = cdev_add(char_intf_device_cdev_10, MKDEV(MAJOR(dev_num), 10), 1);
	/* Create the device in this class with minor number 10 
	 * device can be created using mknode also with major number
	 * */
	char_intf_device = device_create(char_intf_device_class, NULL, MKDEV(MAJOR(dev_num), 10), NULL, DEVICE_NAME "10");

	printk("Char driver registration complete\n");

    return 0;
}

static void __exit char_intf_exit(void)
{
    cdev_del(char_intf_device_cdev);
    device_destroy(char_intf_device_class, dev_num);
    
    cdev_del(char_intf_device_cdev_10);
    device_destroy(char_intf_device_class, dev_num);

    class_destroy(char_intf_device_class);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "char interface driver exit!\n");
}

module_init(char_intf_init);
module_exit(char_intf_exit);

