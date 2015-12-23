/*
 * complete.c -- the writers awake the readers
 *
 * Copyright (C) 2003 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2003 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: complete.c,v 1.2 2004/09/26 07:02:43 gregkh Exp $
 */

#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>  /* current and everything */
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/types.h>  /* size_t */
#include <linux/completion.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

static int complete_major = 0;
static struct cdev *complete_dev = NULL;

DECLARE_COMPLETION(comp);

ssize_t complete_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) going to sleep\n",
			current->pid, current->comm);
	wait_for_completion(&comp);
	printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	return 0; /* EOF */
}

ssize_t complete_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
			current->pid, current->comm);
	complete(&comp);
	return count; /* succeed, to avoid retrial */
}


struct file_operations complete_fops = {
	.owner = THIS_MODULE,
	.read =  complete_read,
	.write = complete_write,
};


int complete_init(void)
{
	int result;
	dev_t dev = 0;

	/*
	 * Register your major, and accept a dynamic number
	 */ 
	 if (complete_major) {
		dev = MKDEV(complete_major, 0);
		result = register_chrdev_region(dev, 1, "complete");
	} else {/*dynamic*/
		result = alloc_chrdev_region(&dev, 0, 1,
				"complete");
		complete_major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "complete: can't get major %d\n", complete_major);
		return result;
	}

	complete_dev = cdev_alloc( );
	if (complete_dev == NULL) {
		printk(KERN_WARNING "cannot allocate cdev struct\n");
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	complete_dev->ops = &complete_fops;
	complete_dev->owner = THIS_MODULE;
	result = cdev_add(complete_dev, dev, 1);
	if (result) {
		printk(KERN_WARNING "cannot add complete char device. \n");
		unregister_chrdev_region(dev, 1);
		return -2;
	}

	return 0;
}

void complete_cleanup(void)
{
	dev_t devno = MKDEV(complete_major, 0);
	cdev_del(complete_dev);
	unregister_chrdev_region(devno, 1);
}

module_init(complete_init);
module_exit(complete_cleanup);

