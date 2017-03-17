/*
 * $Id: echo.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */

 #include <linux/module.h>
 #include <linux/moduleparam.h>
 #include <linux/init.h>
 #include <linux/kernel.h>	/* printk() */
 #include <linux/slab.h>		/* kmalloc() */
 #include <linux/fs.h>		/* everything... */
 #include <linux/errno.h>	/* error codes */
 #include <linux/types.h>	/* size_t */
 #include <linux/proc_fs.h>
 #include <linux/fcntl.h>	/* O_ACCMODE */
 #include <linux/aio.h>
 #include <linux/cdev.h>  /* cdev*/
 #include <asm/uaccess.h>
#include "echo.h"
MODULE_LICENSE("Dual BSD/GPL");

int echo_major = ECHO_MAJOR;
int echo_devs = ECHO_DEVS;

struct cdev *echo_cdev = NULL;

int echo_open(struct inode *inode, struct file *filep){

//  struct cdev *dev;
  //dev = container_of(inode->i_cdev,struct cdev,cdev);
  filep->private_data = inode->i_cdev;

  printk(KERN_WARNING "INITED private_data of file\n");

  return 1;
}

int echo_release(struct inode *inode,struct file *filep){
  printk(KERN_WARNING "INVOKES echo_release\n");

  return 0;
}

int echo_flush(struct inode *inode,fl_owner_t id){

  printk(KERN_WARNING "XAU\n");

  return 0;
}

struct file_operations echo_fops = {
  .owner =      THIS_MODULE,
  .open =       echo_open,
//  .release =    echo_release,
//  .flush  =     echo_flush,

};



static int echo_init(void)
{
	int result;
	dev_t dev = MKDEV(echo_major, 0);

	/*
	 * Register your major, and accept a dynamic number.
	 */
	if (echo_major)
		result = register_chrdev_region(dev, echo_devs, "echo");
	else {
		result = alloc_chrdev_region(&dev, 0, echo_devs, "echo");
		echo_major = MAJOR(dev);
	}
  if(result < 0){
    printk(KERN_WARNING "echo: can't get major %d\n",echo_major);
    return result;
  }

  echo_cdev = cdev_alloc();
  if(echo_cdev == NULL){
    printk(KERN_ALERT "FAILED TO ALLOC CDEV\n");
    return -1;
  }
  echo_cdev->owner = THIS_MODULE; //por defeito, definido em /modules.h
  echo_cdev->ops =   &echo_fops;

  if(cdev_add(echo_cdev,dev,1) < 0){
    printk(KERN_ALERT "FAILED CDEV_ADD\n");
    return -1;
  }

	printk(KERN_ALERT "MAJOR NUMBER: %d\n",echo_major);
	return 0;
}



static void echo_exit(void)
{
	printk(KERN_ALERT "Fairwell major %d\n",echo_major);
  cdev_del(echo_cdev);
	unregister_chrdev_region( MKDEV(echo_major,0),echo_devs);
}

module_init(echo_init);
module_exit(echo_exit);
