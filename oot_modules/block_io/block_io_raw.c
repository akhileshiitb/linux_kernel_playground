#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/bio.h>

/* 
 * modile to check bio request submission to block device
 * This module is to understand how to submid block io request to block device using block layer.
 * We use ram block device driver from kernel to understand this.
 * drivers/block/brd.c --> needs to be enabled in kernel configuration.
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand blockdevices and block layer");

static struct  block_device *block_dev;
static struct page *page;
static char *buffer;
static struct bio *bio;

#define BLOCK_DEVICE_PATH	"/dev/ram0"

static void bio_done_callback(struct bio *bio)
{
	/* BIO request completion callback */
	printk("Requested raw bio is complete\n");
	complete(bio->bi_private);
}

static ssize_t	procfs_read_scatter_gather(struct file *filep, char __user *usr_buf, size_t count, loff_t *l_off)
{
	struct completion done;
	struct page *page1, *page2;
	volatile char *buf1, *buf2; // corresponding to each page

	printk("This is procfs read scatter/gather callback\n");

	/* Get the block device to play with */
	block_dev = blkdev_get_by_path(BLOCK_DEVICE_PATH, FMODE_READ | FMODE_WRITE, NULL, NULL);

	/* Allocate some page on RAM to read data on */
	page1 = alloc_page(GFP_KERNEL);
	page2 = alloc_page(GFP_KERNEL);

	if (!page1 || !page2)
		printk("page allocation error");

	/* Get virtual address of page */
	buf1 = page_address(page1);
	buf2 = page_address(page2);

	/* Initilize the bio strcture with 2 vectors: block IO strcture */
	bio = bio_alloc(block_dev, 2, REQ_OP_READ | REQ_OP_WRITE, GFP_KERNEL);
	if (!bio)
		printk("bio init failed");

	/* Set up the bio: this bio needs to submit to this device */
	bio->bi_iter.bi_sector = 0;
	bio->bi_iter.bi_size = 16; // only need to read 16 bytes
	bio->bi_iter.bi_idx = 0; // point to vector 0
	bio->bi_opf = REQ_OP_READ;

	/* Add page1 to the IO vector*/
	bio->bi_io_vec[0].bv_page = page1;
	bio->bi_io_vec[0].bv_len = 8; // read only 8 bytes from block device to page1
	bio->bi_io_vec[0].bv_offset = 0;

	/* Add page2 to the IO vector*/
	bio->bi_io_vec[1].bv_page = page2;
	bio->bi_io_vec[1].bv_len = 8; // read only 8 bytes from block device to page2 
	bio->bi_io_vec[1].bv_offset = 8; // att offset 8

	bio->bi_vcnt = 2; // there are 2 vectors

	// setup completion 
	init_completion(&done);
	bio->bi_private = &done;
	bio->bi_end_io = bio_done_callback;
	
	// submit bio
	submit_bio(bio);

	/* Wait for completion */
	wait_for_completion(&done);

	// (gdb) dump and see buf1 and buf2 here
	for (int i = 0; i < 8; i++) {
		printk("val buf1: %d\n", *buf1++);	
	}
	for (int i = 0; i < 8; i++) {
		printk("val buf2: %d\n", *buf2++);	
	}

	printk("Block IO read request operation scatter/gather complete\n");

	return 0;

}

static ssize_t	procfs_read(struct file *filep, char __user *usr_buf, size_t count, loff_t *l_off)
{
	struct completion done;
	printk("This is procfs read callback\n");

	/* Get the block device to play with */
	block_dev = blkdev_get_by_path(BLOCK_DEVICE_PATH, FMODE_READ | FMODE_WRITE, NULL, NULL);

	/* Allocate some page on RAM to read data on */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		printk("page allocation error");
	/* Get virtual address of page */
	buffer = page_address(page);

	/* Initilize the bio strcture: block IO strcture */
	bio = bio_alloc(block_dev, 1, REQ_OP_READ | REQ_OP_WRITE, GFP_KERNEL);
	if (!bio)
		printk("bio init failed");

	/* Set up the bio: this bio needs to submit to this device */
	bio->bi_iter.bi_sector = 0;
	bio->bi_iter.bi_size = PAGE_SIZE;
	bio->bi_iter.bi_idx = 0;
	bio->bi_opf = REQ_OP_READ;

	/* Add pages to the IO */
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_page = page;
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_len = PAGE_SIZE;
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_offset = 0;

	// setup completion 
	init_completion(&done);
	bio->bi_private = &done;
	bio->bi_end_io = bio_done_callback;
	
	// submit bio
	submit_bio(bio);

	/* Wait for completion */
	wait_for_completion(&done);

	/* Print */
	print_hex_dump(KERN_INFO, "Data: ", DUMP_PREFIX_OFFSET, 16, 1, buffer, 32, true);

	printk("Block IO read request operation complete\n");

	return 0;
}


static ssize_t	procfs_write(struct file *filep, const char __user *usr_buf, size_t count, loff_t *l_off)
{
	struct completion done;

	static struct page *pageWrite;
	volatile char * bufferWrite;
	struct bio *bio;

	printk("This is procfs write callback\n");

	/* Get the block device to play with */
	block_dev = blkdev_get_by_path("/dev/ram0", FMODE_READ | FMODE_WRITE, NULL, NULL);

	/* Allocate some page on RAM to read data on */
	pageWrite = alloc_page(GFP_KERNEL);
	if (!pageWrite)
		printk("page allocation error");
	/* Get virtual address of page */
	bufferWrite = page_address(pageWrite);

	/* Fill in buffer data 16 bytes (8 + 8) */
	for (int i = 0; i < 16; i++) {
		bufferWrite[i] = i;
	}

	/* Initilize the bio strcture: block IO strcture with only 1 vector*/
	bio = bio_alloc(block_dev, 1, REQ_OP_READ | REQ_OP_WRITE, GFP_KERNEL);
	if (!bio)
		printk("bio init failed");

	/* Set up the bio: this bio needs to submit to this device */
	bio->bi_iter.bi_sector = 0;
	bio->bi_iter.bi_size = PAGE_SIZE;
	bio->bi_iter.bi_idx = 0;

	bio->bi_opf = REQ_OP_WRITE;  // set bio operation flag.
	/* Add pages to the IO */
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_page = pageWrite;
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_len = PAGE_SIZE;
	bio->bi_io_vec[bio->bi_iter.bi_idx].bv_offset = 0;

	// setup completion 
	init_completion(&done);
	bio->bi_private = &done;
	bio->bi_end_io = bio_done_callback;
	
	// submit bio
	submit_bio(bio);

	/* Wait for completion */
	wait_for_completion(&done);

	if (bio->bi_status) {
		printk("Block write operation failes\n");	
	}

	printk("Block IO request write operation complete\n");


	return count;
}

static struct proc_ops proc_fops = {
	.proc_read = procfs_read_scatter_gather,
	.proc_write = procfs_write,	
};

int __init block_io_raw_init(void)
{
	volatile struct proc_dir_entry *dir; 

	dir = proc_mkdir("bio_intf_raw", NULL);

	if (!dir)
		printk("Procfs dir create issue\n");

	dir = proc_create("start", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file create issue\n");


    return 0;
}

module_init(block_io_raw_init);

