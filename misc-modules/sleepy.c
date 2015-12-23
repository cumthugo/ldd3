/*
 * sleepy.c -- the writers awake the readers
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: sleepy.c,v 1.7 2004/09/26 07:02:43 gregkh Exp $
 */

#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>  /* current and everything */
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/types.h>  /* size_t */
#include <linux/wait.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

static int sleepy_major = 0;
static struct cdev *sleepy_dev = NULL;

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

ssize_t sleepy_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) going to sleep\n",
			current->pid, current->comm);
	wait_event_interruptible(wq, flag != 0);
	flag = 0;
	printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	return 0; /* EOF */
}

ssize_t sleepy_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
			current->pid, current->comm);
	flag = 1;
	wake_up_interruptible(&wq);
	return count; /* succeed, to avoid retrial */
}


struct file_operations sleepy_fops = {
	.owner = THIS_MODULE,
	.read =  sleepy_read,
	.write = sleepy_write,
};


int sleepy_init(void)
{
	int result;
        dev_t dev = 0;

	/*
	 * Register your major, and accept a dynamic number
	 */
    	 if (sleepy_major) {
		dev = MKDEV(sleepy_major, 0);
		result = register_chrdev_region(dev, 1, "sleepy");
	} else {/*dynamic*/
		result = alloc_chrdev_region(&dev, 0, 1,
				"sleepy");
		sleepy_major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "sleepy: can't get major %d\n", sleepy_major);
		return result;
	}

	sleepy_dev = cdev_alloc( );
	if (sleepy_dev == NULL) {
		printk(KERN_WARNING "cannot allocate cdev struct\n");
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	sleepy_dev->ops = &sleepy_fops;
	sleepy_dev->owner = THIS_MODULE;
	result = cdev_add(sleepy_dev, dev, 1);
	if (result) {
		printk(KERN_WARNING "cannot add sleepy char device. \n");
		unregister_chrdev_region(dev, 1);
		return -2;
	}
        
	return 0;
}

void sleepy_cleanup(void)
{
  	dev_t devno = MKDEV(sleepy_major, 0);
	cdev_del(sleepy_dev);
	unregister_chrdev_region(devno, 1);
}

module_init(sleepy_init);
module_exit(sleepy_cleanup);

