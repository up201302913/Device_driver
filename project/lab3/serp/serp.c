
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/aio.h>
#include <linux/cdev.h>  /* cdev*/
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/ioport.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "serial_reg.h"
#include "serp.h"

MODULE_LICENSE("Dual BSD/GPL");

// clock signal with 115 200 Hz
#define CLOCKB 115200;

int serp_major = SERP_MAJOR;
int serp_minor = SERP_MINOR;

int major = 0, minor = 0;

struct serp_devs *serp_devices = NULL;

dev_t dev;


int serp_open(struct inode *inode, struct file *filep){

  struct serp_devs *dev;

  printk(KERN_NOTICE "Device driver on opening function!\n");

	nonseekable_open(inode, filep); //Non-seekable DD

  dev = container_of(inode->i_cdev,struct serp_devs, serp_cdevs);

  filep->private_data = dev;

  if(filep->private_data == NULL)
      printk(KERN_WARNING "Private data not inicialized correctly!\n");

  printk(KERN_NOTICE "Device driver successful open!\n");


  return 0;
}

int serp_release(struct inode *inode,struct file *filep){

  printk(KERN_WARNING "Device driver released!\n");

  return 0;
}

int serp_flush(struct inode *inode,fl_owner_t id){

  printk(KERN_WARNING "Driver flused!\n");

  return 0;
}

ssize_t serp_read(struct file *filep, char __user *buff, size_t count, loff_t *offp) {

  int i=0;

  unsigned char bitget = 0;
  unsigned long result;
  ssize_t rec = 0;

  char *kernelbuffer = (char *)kzalloc(count*sizeof(char), GFP_KERNEL);

  if(!kernelbuffer){
    return -ENOMEM;
  }

  printk(KERN_INFO "%d", count);
  for(i = 0; i < count; i++) {
    bitget = inb(ADR_COM1 + UART_LSR);
    while ((bitget & UART_LSR_DR) == 0) {
      msleep_interruptible(1);
      bitget = inb(ADR_COM1 + UART_LSR);
    }
    if((bitget & (UART_LSR_FE | UART_LSR_OE | UART_LSR_PE)) != 0) {
      kfree(kernelbuffer);
      return -EIO;
    } else {
      kernelbuffer[i] = inb(ADR_COM1 + UART_RX);
      rec++;
    }
  }

  result = copy_to_user(buff, kernelbuffer, rec);

  if(result == 0) {
      printk(KERN_INFO "\nCaracters received: %ld\n", (count-result));
  } else{
      if(result > 0) {
          printk(KERN_INFO "Missing %d caracters!\n", count - (int)bitget);
      }
  }

  kfree(kernelbuffer);
  return count-result;
}

ssize_t serp_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp) {

  int i=0;
	unsigned long result;

	char *kernelbuffer = (char *) kzalloc(sizeof(char)*(count+1), GFP_KERNEL);

	printk(KERN_INFO "Serp_open opened!\n");

	if(kernelbuffer == NULL) {
	     printk(KERN_ALERT "Error allocating memory for kernel buffer space!\n");
		   return -ENOMEM;
	}

	result = copy_from_user(kernelbuffer, buff, count);

	if(result == 0){
      printk(KERN_INFO "Message completed successful!\n");
	    printk(KERN_INFO "%s \n", kernelbuffer);

	} else if(result>0){
      printk(KERN_INFO "Still need to write %lu caracters to complete the message!\n", result);
	    return -EFAULT;
	}

	kernelbuffer[count]='\0';

	for(i = 0; i < strlen(kernelbuffer) ; i++){
	     while((inb(ADR_COM1+UART_LSR) & UART_LSR_THRE) == 0) {
			 msleep_interruptible(1);
		}
		outb(kernelbuffer[i], ADR_COM1+UART_TX);
	}

	kfree(kernelbuffer);

	printk(KERN_INFO "The device driver finished printing the message!\n");

	return count;
}

struct file_operations serp_fops = {
  .owner =      THIS_MODULE,
  .open =       serp_open,
  .release =    serp_release,
  .write  =     serp_write,
  .read   =     serp_read,
  .llseek =     no_llseek,
//  .flush  =     serp_flush,
};


static void serp_setup(struct serp_devs *dev, int index ){

  int err;
  int devno = MKDEV(serp_major, index);


  cdev_init(&dev->serp_cdevs, &serp_fops);
  dev->serp_cdevs.owner = THIS_MODULE;
  dev->serp_cdevs.ops = &serp_fops;

  err = cdev_add(&dev->serp_cdevs, devno, 1);

  if (err < 0) {
    printk(KERN_NOTICE "Error %d at adding serp_cdevs %d!\n", err, index);
  } else {
		printk(KERN_NOTICE "Device major %d with minor %d added successful!\n ", devno, index);
  }

}

static int serp_init(void) {

	int result, i;
  unsigned char lcr = 0;

  if (request_region(ADR_COM1, 8, "serp")) {
      printk(KERN_INFO "Serial port I/O port range reserved successful!\n");
  } else {
      printk(KERN_ALERT "Serial port I/O requested already in use!\n");
      return -EREMOTEIO;
  }

  // UART communication parameters: 8-bit chars, 2 stop bits, parity even, and 1200 bps with no interrupts

  // UART 8bit chars / 2 stop bits / Parity even / No interrupts
  lcr = UART_LCR_WLEN8 | UART_LCR_PARITY | UART_LCR_EPAR | UART_LCR_STOP;
  outb(lcr, ADR_COM1 + UART_LCR);

  // Divisor Latch acess
  lcr |= UART_LCR_DLAB;
  outb(lcr, ADR_COM1 + UART_LCR);

  //Bitrate 1200 bps
  outb(UART_DIV_1200, ADR_COM1+UART_DLL);
  outb(0x00, ADR_COM1+UART_DLM);

  //Divisor Lacth end, reset the DLAB so that the other registers can be accessed
  lcr &= ~UART_LCR_DLAB;
  outb(lcr, ADR_COM1+UART_LCR);

  //dev_t dev = MKDEV(echo_major, 0);


	// Register your major, and accept a dynamic number.

	if (serp_major)
		result = register_chrdev_region(dev, serp_minor, "serp");
	else {
		result = alloc_chrdev_region(&dev, 0, serp_minor, "serp");
		major = MAJOR(dev);
    minor = MINOR(dev);
    printk(KERN_ALERT "Major allocated : %d\n", major);
    printk(KERN_ALERT "Minor allocated : %d\n", minor);
  }
  if(result < 0){
    printk(KERN_WARNING "Error in allocating major device!");
    return result;
  }


  serp_devices = kmalloc(serp_minor * sizeof(struct serp_devs), GFP_KERNEL);

  if(serp_devices == NULL){
    printk(KERN_ALERT "Failed to allocated serp devices!\n");
    return -ENOMEM;
  }

  for(i = 0; i < serp_minor; i++){
    serp_setup(&serp_devices[i], i);
  }


	printk(KERN_ALERT "MAJOR NUMBER: %d\n",serp_major);

  return result;

}



static void serp_exit(void) {

  int i;

  //printk(KERN_ALERT "Fairwell major %d\n",serp_major);

  if (serp_devices) {
      for(i = 0; i < serp_minor; i++){
          cdev_del(&serp_devices[i].serp_cdevs);
      }
  }

  kfree(serp_devices);

  unregister_chrdev_region(MKDEV(major,0), serp_minor);

  // Release previous allocated region with address from (0x3f8-0x3ff)
  release_region(ADR_COM1, 8);

  printk(KERN_ALERT "Device driver with Major number %d was deleted!\n", major);

}

module_init(serp_init);
module_exit(serp_exit);