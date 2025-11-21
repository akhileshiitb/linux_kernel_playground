#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/blk-mq.h>
#include <linux/blk-crypto.h>
#include <linux/blk-crypto-profile.h>

/* 
 * This module checks out basic linux block device 
 * and related operations.
 * */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akhilesh Patil");
MODULE_DESCRIPTION("Module to understand block devices and block layer");

#define DEVICE_NAME	"diskAmazon"
/* 8MB disk */
#define BLOCK_SIZE_MB	8 


#define DEVICE_SIZE (1 * 1024 *1024) // 1 MB of device
#define NUM_SECTORS	DEVICE_SIZE/512
#define DISK_MINOR 0

/* Define strcture defining my block device */
struct amazon_block_dev {
	sector_t capacity; // device capacity in sectors
	uint8_t *data;	   // some memoery to store the data. Storage data for device
	struct blk_mq_tag_set tag_set; // tag set which does queue management for this device
	struct gendisk *gd; // Disk associated with this block device. similar to VFS. Its virtual disk
};

struct amazon_block_dev *dev = NULL;

static unsigned int major_num = 0;

int amazon_keyslot_program(struct blk_crypto_profile *profile,
			       const struct blk_crypto_key *key,
			       unsigned int slot)
{
	
	printk("Programming the key\n");

	return 0;
}


int amazon_keyslot_evict(struct blk_crypto_profile *profile,
			     const struct blk_crypto_key *key,
			     unsigned int slot)
{
	printk("Evicting the key\n");	

	return 0;
}

struct blk_crypto_ll_ops diskAmazonCryptoOps = {
	.keyslot_program = amazon_keyslot_program,
	.keyslot_evict = amazon_keyslot_evict,
};

void register_inline_crypto_profile(void)
{
	struct blk_crypto_profile *amazon_crypto_profile;
	amazon_crypto_profile = vzalloc(sizeof(struct blk_crypto_profile));
	

	// initlize the crypto profile for the device.
	blk_crypto_profile_init(amazon_crypto_profile, 16);
	amazon_crypto_profile->modes_supported[0] = 0xFFFFFFFF;
	amazon_crypto_profile->modes_supported[1] = 0xFFFFFFFF;
	amazon_crypto_profile->modes_supported[2] = 0xFFFFFFFF;
	amazon_crypto_profile->modes_supported[3] = 0xFFFFFFFF;
	amazon_crypto_profile->max_dun_bytes_supported = 8;
	amazon_crypto_profile->ll_ops = diskAmazonCryptoOps;

	/* Register the crypto profile in the request queue of this disk */
	dev->gd->queue->crypto_profile = amazon_crypto_profile;

}


blk_status_t amazon_block_queue_rq(struct blk_mq_hw_ctx *hctx,
				 const struct blk_mq_queue_data *bd) {
	printk("blk mq queue request entry\n");
	return BLK_STS_OK;
}
void amazon_block_process_bio(struct bio *bio)
{
	printk("[amazon block device BIO submit] processing bio\n");

	/* Print block crypto stracture */
	if (bio_has_crypt_ctx(bio)) {
		printk("[dev blk] bio request has crypto context\n");	
		printk("[dev blk] crypto DUN: %llx\n", bio->bi_crypt_context->bc_dun[0]);
	}

	if (bio->bi_opf == REQ_OP_READ)
		printk("[dev blk -> read] decrypting content\n");
	else if (bio->bi_opf == REQ_OP_WRITE)
		printk("[dev blk -> write] encrypting content\n");

	// disable crypto context here, before ending the io
	if (bio_has_crypt_ctx(bio))
		printk("[blk dev ]Disable inline crypto\n");

	bio_endio(bio);

}

const struct blk_mq_ops	amazon_block_mq_opts = {
	.queue_rq = amazon_block_queue_rq,
};

const struct block_device_operations amazon_block_ops = {
	.owner = THIS_MODULE,
	.submit_bio = amazon_block_process_bio,
};

int __init block_device_init(void)
{
	int ret = 0;

	/* Allocate device strcuture */
	dev = kzalloc(sizeof(struct amazon_block_dev), GFP_KERNEL);

	/* Assign some data buffer to out block device */
	dev->data = vzalloc(DEVICE_SIZE);

	/* Initialize the tag set for this device */
	dev->tag_set.ops = &amazon_block_mq_opts;
	dev->tag_set.nr_hw_queues = 1; /* Only 1 hardware queue */
	dev->tag_set.queue_depth = 8; /* Queue depth is 8 */
	dev->tag_set.numa_node = NUMA_NO_NODE;
	dev->tag_set.cmd_size = 0;
	dev->tag_set.flags = 0;
	ret = blk_mq_alloc_tag_set(&dev->tag_set);

	/* Register this block device driver */
	major_num = register_blkdev(major_num, DEVICE_NAME);

	/* Allocate the gendisk strcture for this block device */
	dev->gd = blk_mq_alloc_disk(&dev->tag_set, dev);
	dev->gd->private_data = dev; // link disk with device
	dev->gd->fops = &amazon_block_ops;
	dev->gd->major = major_num;
	dev->gd->first_minor = 0;
	dev->gd->minors = 1; // only 1 partition on our disk
	snprintf(dev->gd->disk_name, 12, DEVICE_NAME);
	set_capacity(dev->gd, NUM_SECTORS);

	/* Create direct bio request queue */
	blk_queue_logical_block_size(dev->gd->queue, 512);
	dev->gd->queue->queuedata = dev;

	/* Register crypto profile for this device */
	register_inline_crypto_profile();

	/* Add the generated disk to the system */
	ret = add_disk(dev->gd);

	printk( KERN_EMERG "This is block device init\n");

    	return 0;
}

module_init(block_device_init);

