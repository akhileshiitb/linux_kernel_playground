#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/bio.h>
#include <linux/blk-crypto.h>

/* 
 * Module to understand and emulate crypto context passing from BIO to block driver layer.
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand blockdevices and block crypto layer");

static struct  block_device *block_dev;
static struct page *page;
static char *buffer;
static struct bio *bio;

#define BLOCK_DEVICE_PATH	"/dev/diskAmazon"

struct bio_crypto_ctx *crypto_ctx;

static void bio_done_callback(struct bio *bio)
{
	/* BIO request completion callback */
	printk("Requested bio crypto is complete\n");
	complete(bio->bi_private);

	/* Deallocate crypto context memory */
	//vfree(bio->bi_crypt_context->bc_key);
	
}

static void set_bio_crypto_context(struct bio * bio)
{
	int ret;
	struct blk_crypto_key *bc_key;
	const u8 raw_key[32];
	const u64 dun[BLK_CRYPTO_DUN_ARRAY_SIZE];

	const struct blk_crypto_config cfg = {
		.crypto_mode = BLK_ENCRYPTION_MODE_AES_256_XTS,
		.data_unit_size =  1 << 5,
		.dun_bytes = 4,
	};

	if (blk_crypto_config_supported(bio->bi_bdev, &cfg))
		printk("crypto config supported by device\n");
	else
		printk("crypto config not supported by device\n");

	bc_key = vzalloc(sizeof(struct blk_crypto_key));

	memset(raw_key, 0xDE, 32);	

	ret = blk_crypto_init_key(bc_key, raw_key,
			cfg.crypto_mode,
			cfg.dun_bytes,
			cfg.data_unit_size);

	if (ret) {
		printk("Crypto init key failed\n");
		return;
	}

	blk_crypto_start_using_key(bio->bi_bdev, bc_key);

	// set up the context
	memset(dun, 0xAA, sizeof(dun)); // fill in data unit number
	bio_crypt_set_ctx(bio, bc_key, dun, GFP_KERNEL); //uses mempool to allocate
	
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
	bio = bio_alloc(block_dev, 4, REQ_OP_READ | REQ_OP_WRITE, GFP_KERNEL);
	if (!bio)
		printk("bio init failed");

	/* Set up the bio: this bio needs to submit to this device */
	bio_set_dev(bio, block_dev);
	bio->bi_iter.bi_sector = 0;
	//1bio_set_op_attrs(bio, REQ_OP_READ, 0); // TODO
	bio->bi_opf = REQ_OP_READ;
	bio_add_page(bio, page, PAGE_SIZE, 0);

	// setup completion 
	init_completion(&done);
	bio->bi_private = &done;
	bio->bi_end_io = bio_done_callback;

	// set up the crypto context in bio
	set_bio_crypto_context(bio);
	
	// submit bio
	submit_bio(bio);

	/* Wait for completion */
	wait_for_completion(&done);

	if (bio->bi_status) {
		printk("Block read operation fails\n");	
	}

	/* Print */
	print_hex_dump(KERN_INFO, "Data: ", DUMP_PREFIX_OFFSET, 16,1, buffer, 32, true);


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
	block_dev = blkdev_get_by_path(BLOCK_DEVICE_PATH, FMODE_READ | FMODE_WRITE, NULL, NULL);

	/* Allocate some page on RAM to read data on */
	pageWrite = alloc_page(GFP_KERNEL);
	if (!pageWrite)
		printk("page allocation error");
	/* Get virtual address of page */
	bufferWrite = page_address(pageWrite);

	/* Fill in buffer data */
	for (int i = 0; i < 50; i++) {
		bufferWrite[i] = i;
	}

	/* Initilize the bio strcture: block IO strcture */
	bio = bio_alloc(block_dev, 4, REQ_OP_READ | REQ_OP_WRITE, GFP_KERNEL);
	if (!bio)
		printk("bio init failed");

	/* Set up the bio: this bio needs to submit to this device */
	bio_set_dev(bio, block_dev);
	bio->bi_iter.bi_sector = 0;
	//1bio_set_op_attrs(bio, REQ_OP_READ, 0); // TODO
	bio->bi_opf = REQ_OP_WRITE;  // set bio operation flag.
	bio_add_page(bio, pageWrite, PAGE_SIZE, 0);

	// setup completion 
	init_completion(&done);
	bio->bi_private = &done;
	bio->bi_end_io = bio_done_callback;

	// set up the crypto context in bio
	set_bio_crypto_context(bio);
	
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
	.proc_read = procfs_read,
	.proc_write = procfs_write,	
};

int __init block_io_crypto_init(void)
{
	volatile struct proc_dir_entry *dir; 

	dir = proc_mkdir("bio_crypto_intf", NULL);

	if (!dir)
		printk("Procfs dir create issue\n");

	dir = proc_create("start", 0777, dir, &proc_fops);
	if (!dir)
		printk("Procfs file create issue\n");


    return 0;
}

module_init(block_io_crypto_init);

