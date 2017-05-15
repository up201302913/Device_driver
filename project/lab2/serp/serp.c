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
 #include "serial_reg.h"
#include "serp.h"
MODULE_LICENSE("Dual BSD/GPL");

int echo_major = ECHO_MAJOR;
int echo_devs = ECHO_DEVS;

struct echo_dev *echo_devices = NULL;

int echo_open(struct inode *inode, struct file *filep){

  struct echo_dev *dev;
  dev = container_of(inode->i_cdev,struct echo_dev,cdev);

  filep->private_data = dev;
  if(filep->private_data == NULL)
    printk(KERN_WARNING "PRIVATE DATA NOT INIT");

  printk(KERN_WARNING "INITED private_data of file\n");

  nonseekable_open(inode,filep);
  return 0;
}

int echo_release(struct inode *inode,struct file *filep){
  printk(KERN_WARNING "INVOKES echo_release\n");

  return 0;
}

int echo_flush(struct inode *inode,fl_owner_t id){

  printk(KERN_WARNING "XAU\n");

  return 0;
}

ssize_t echo_read(struct file *filep,
                  char __user *buff,
                  size_t count,
                  loff_t *offp){

  struct echo_dev *dev = filep->private_data;

  printk("NUMERO CHARS LIDOS %d\n", dev->cnt);

  return 0;
}
ssize_t echo_write(struct file *filep,
                   const char __user *buff,
                   size_t count,
                   loff_t *offp){
  int num;
  char *kern_buff = kmalloc(sizeof(char)*(count+1),GFP_KERNEL);

  struct echo_dev *dev = filep->private_data;

  if(kern_buff == NULL){
    printk(KERN_WARNING "FAILED TO ALLOC KERNEL BUFFER\n");
    return -1;
  }
  printk("COUNT %d \n",count);

  num = copy_from_user(kern_buff,buff,count);
  if(num == 0){
    printk(KERN_WARNING "COPY_FROM_USER SUCCESSFUL\n");
  }
  else{
    printk(KERN_WARNING "COULD NOT COPY %d BYTES FROM USER\n",num);
  }

   kern_buff[count-num] = '\0';

  if(kern_buff[count-num] != '\0'){
    printk("CANT PRINT; EXITING");
    return -1;
  }

  dev->cnt = count-num;
//  printk("CHARS NO DEIVCE %d",dev->cnt);

  while(*(kern_buff) != '\0'){
    printk("%c",*(kern_buff));
    kern_buff++;
  }
  return count-num;
}

struct file_operations echo_fops = {
  .owner =      THIS_MODULE,
  .open =       echo_open,
  .release =    echo_release,
  .write  =     echo_write,
  .read   =     echo_read,
  .llseek =     no_llseek,
//  .flush  =     echo_flush,

};

static void echo_setup(struct echo_dev *dev, int index ){
  int devno = MKDEV(echo_major,index);
  int err;

  cdev_init(&dev->cdev,&echo_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &echo_fops;
  err = cdev_add(&dev->cdev,devno,1);

  if (err)
		printk(KERN_NOTICE "Error %d adding echo%d", err, index);

}

static int echo_init(void)
{
	int result;
	dev_t dev = MKDEV(echo_major, 0);
  int i=0;
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
  echo_devices = kmalloc(echo_devs*sizeof(struct echo_dev), GFP_KERNEL);
  if(echo_devices == NULL){
    printk(KERN_ALERT "FAILED TO ALLOC CDEV\n");
    return -1;
  }

  for(i = 0;i < echo_devs;i++){
    echo_setup(echo_devices + i,i);
  }


	printk(KERN_ALERT "MAJOR NUMBER: %d\n",echo_major);
	return result;
}



static void echo_exit(void)
{
  int i;
  printk(KERN_ALERT "Fairwell major %d\n",echo_major);

  for(i = 0;i < echo_devs;i++){
    cdev_del(&echo_devices[i].cdev);
  }

    kfree(echo_devices);

  unregister_chrdev_region( MKDEV(echo_major,0),echo_devs);

}

module_init(echo_init);
module_exit(echo_exit);
