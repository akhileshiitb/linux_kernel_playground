// hello.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("xband module");

// value stored in this module
static unsigned long value_store = 0x0;

static struct dentry *dir;

static void *vpage;

void page_alloc_test(void)
{
	void *vaddr = NULL;
	struct page *page;

	page = alloc_page(GFP_KERNEL);

	if (page)
		vaddr = page_address(page);

	if (vaddr) {
		memset(vaddr, 0xD, PAGE_SIZE);
		printk("page is filled at %x\n", (unsigned long)vaddr);
	}

	// checkout kmalloc()
	vaddr = kmalloc(4, GFP_KERNEL); // physically contiguous memory.
	if (vaddr) {
		memset(vaddr, 0xAE, 4);
		kfree(vaddr);
	}

	// checkout kzalloc()
	vaddr = kzalloc(4, GFP_KERNEL);	
	if (vaddr) {
		memset(vaddr, 0xAE, 4);
		kfree(vaddr);
	}

	/* Allocate virtually contiguous memory */
	vaddr = vmalloc(4);
	if (vaddr) {
		memset(vaddr, 0xAA, 4);	
		vfree(vaddr);
	}

	vaddr = vzalloc(4);
	if(vaddr) {
		memset(vaddr, 0xBB, 4);	
		vfree(vaddr);
	}

	// intentionally not de-allocating the page.
}

ssize_t xband_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) 
{
	printk("read operation on xband: %d\n", value_store);

	return 0;
}

ssize_t xband_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) 
{
	value_store += 1;
	printk("Write operations on xband: %d\n", value_store);

	return count;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = xband_read,
	.write = xband_write,
};

static int __init xband_init(void)
{

	struct page *page;

    	printk(KERN_INFO "Xband init!\n");
	printk("My new kernel driver is started\n");    

	dir = debugfs_create_dir("xband", NULL);
	debugfs_create_file("value", 0644, dir, NULL, &fops);
	debugfs_create_file("name", 0644, dir, NULL, &fops);

	// allocate a page for this module lifetime
	page = alloc_page(GFP_KERNEL);
	vpage = page_address(page);

	page_alloc_test();

    return 0;
}

static void __exit xband_exit(void)
{
    printk(KERN_INFO "xband exit!\n");
    printk("removing debugfs file");
    debugfs_remove_recursive(dir);
}

module_init(xband_init);
module_exit(xband_exit);

