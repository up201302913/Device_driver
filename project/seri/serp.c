
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/interrupt.h>
#include <linux/kfifo.h>

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
#include <linux/wait.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "serial_reg.h"
#include "serp.h"

MODULE_LICENSE("Dual BSD/GPL");



#define SIZE_FIFO 4096
#define TIMEOUT   1024
int serp_major = SERP_MAJOR;
int serp_minor = SERP_MINOR;

int major = 0, minor = 0;
int read_flag=0,interrupted;
struct serp_devs *serp_devices = NULL;

dev_t dev;

irqreturn_t short_interrupt(int irq, void *dev_id){
  struct serp_devs *vdev = (struct serp_devs *) dev_id;
  char buffer;
  int IIR_ADDR;

  printk(KERN_INFO "\nIRQ HANDLER START\n");
  IIR_ADDR = inb(ADR_COM1+UART_IIR);
  //HANDLE READ
  if((IIR_ADDR & UART_IIR_RDI)){
    printk(KERN_INFO "\nRECEIVE\n");
    __kfifo_get(vdev->rxbuf,&buffer,1);
    kfifo_reset(vdev->rxbuf);
  }
  else if(IIR_ADDR & UART_IIR_THRI){
    printk(KERN_INFO "\nTRANSMIT\n");

  }
  wake_up_interruptible(&vdev->rxwait);
  //HANDLE WRITE

  printk(KERN_INFO "\nIRQ HANDLER END\n");
  return IRQ_HANDLED; //tava assim no livro

}

int serp_open(struct inode *inode, struct file *filep){

  struct serp_devs *serp ;
  printk(KERN_NOTICE "Device driver on serp_open function!\n");

  serp = container_of(inode->i_cdev,struct serp_devs, serp_cdevs);
  filep->private_data = serp;



  if(filep->private_data == NULL)
      printk(KERN_WARNING "Private data not initialized correctly!\n");

  nonseekable_open(inode, filep); //Non-seekable DD
  printk(KERN_NOTICE "Device driver successfully open!\n");


  return 0;
}

int serp_release(struct inode *inode,struct file *filep){

  printk(KERN_WARNING "\n\nDevice driver released!\n\n");

  return 0;
}

int serp_flush(struct inode *inode,fl_owner_t id){

  printk(KERN_WARNING "Driver flushed!\n");

  return 0;
}

ssize_t serp_read(struct file *filep, char __user *buff, size_t count, loff_t *offp) {


  int sinal = 1;
  int c;
  unsigned long result;

  char *kernelbuffer;

  struct serp_devs *devp;
  devp = filep->private_data;

  if(count > SIZE_FIFO){
    printk(KERN_WARNING "SIZE REQUESTED LARGER THA NBUFFER");
    return -1;
  }

  kernelbuffer = (char *)kmalloc((SIZE_FIFO+1)*sizeof(char), GFP_KERNEL);

  if(!kernelbuffer){
    return -ENOMEM;
  }
  printk(KERN_INFO "READ FUNCTION\n");

  while( kfifo_len(devp->rxbuf) < count){
    printk(KERN_INFO "AQI)");
    sinal = wait_event_interruptible_timeout(devp->rxwait,(0==1),TIMEOUT);
    kfifo_reset(serp_devices->rxbuf);
    if(sinal==0){
      printk(KERN_INFO "BREAK\n");
       break;
    }
    if(sinal==-ERESTARTSYS){
      kfree(kernelbuffer);
      return -ERESTARTSYS;
    }

  }
  if(sinal == 0)
    c = kfifo_len(devp->rxbuf);
  else{
    c = count;
    kernelbuffer[c] = '\0';
  }
  printk(KERN_INFO "KFIFO_GET\n");
  if(kfifo_get(devp->rxbuf,kernelbuffer,c) != c){
    printk(KERN_ALERT "KFIFO_GET ERROR\n");
		kfree(kernelbuffer);
		return -1;
  }


  printk(KERN_INFO "Reading from buffer. buffer size: %d\n", count);



  read_flag=0;
  result = copy_to_user(buff, kernelbuffer, c);

  if(result == 0) {
    printk(KERN_INFO "\nCharacters received: %d\n", c-1);
  } else{
    if(result > 0) {
      printk(KERN_INFO "Missing %d characters!\n", count);
      return -EIO;
    }
  }

  kfree(kernelbuffer);
  return c-1;

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
	    printk(KERN_INFO "WRITE %s \n", kernelbuffer);

	} else if(result>0){
      printk(KERN_INFO "Still need to write %lu caracters to complete the message!\n", result);
	    return -EFAULT;
	}

	kernelbuffer[count]='\0';

	for(i = 0; i < strlen(kernelbuffer) ; i++){
	     while((inb(ADR_COM1+UART_LSR) & UART_LSR_THRE) == 0) {
			      schedule();
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

/*
static void serp_setup(struct serp_devs *dev, int index ){

  int err;
  int devno = MKDEV(major, minor + index);


  cdev_init(&dev->serp_cdevs, &serp_fops);
  dev->serp_cdevs.owner = THIS_MODULE;
  dev->serp_cdevs.ops = &serp_fops;

  err = cdev_add(&dev->serp_cdevs, devno, 1);

  if (err < 0) {
    printk(KERN_INFO "Error %d at adding serp_cdevs %d!\n", err, index);
  } else {
		printk(KERN_INFO "Device major %d with minor %d added successful!\n ", major, index);
  }

}
*/
static int serp_init(void) {

	int result;
  unsigned char lcr = 0,ier = 0;


    int err;


	// Register your major, and accept a dynamic number.

	if (serp_major){
    dev = MKDEV(serp_major,serp_minor);
		result = register_chrdev_region(dev, serp_minor, "serp");
  }
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


  serp_devices = kmalloc(sizeof(struct serp_devs), GFP_KERNEL);

  if(serp_devices == NULL){
    printk(KERN_ALERT "Failed to allocated serp devices!\n");
    return -ENOMEM;
  }

  //memset(serp_devices, 0,sizeof(struct serp_devs));


  cdev_init(&serp_devices->serp_cdevs, &serp_fops);
  serp_devices->serp_cdevs.owner = THIS_MODULE;
  serp_devices->serp_cdevs.ops = &serp_fops;

  spin_lock_init(&serp_devices->rxlock);
  spin_lock_init(&serp_devices->txlock);

  serp_devices->rxbuf = kfifo_alloc(SIZE_FIFO,GFP_KERNEL,&(serp_devices->rxlock));
  serp_devices->txbuf = kfifo_alloc(SIZE_FIFO, GFP_KERNEL, NULL);

  init_waitqueue_head(&serp_devices->rxwait);

  err = cdev_add(&serp_devices->serp_cdevs, dev, 1);


  if (err < 0) {
    printk(KERN_INFO "Error %d at adding serp_cdevs!\n", err);
  } else {
    printk(KERN_INFO "Device major %d with minor %d added successful!\n ", major, minor);
  }


  	printk(KERN_NOTICE "MAJOR NUMBER: %d\n",major);


  //request_region
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

  // Divisor Latch access
  lcr |= UART_LCR_DLAB;
  outb(lcr, ADR_COM1 + UART_LCR);

  //Bitrate 1200 bps
  outb(UART_DIV_1200, ADR_COM1+UART_DLL);
  outb(0x00, ADR_COM1+UART_DLM);

  //Divisor Lacth end, reset the DLAB so that the other registers can be accessed
  lcr &= ~UART_LCR_DLAB;
  outb(lcr, ADR_COM1+UART_LCR);

  //ier = inb(ADR_COM1+UART_IER);
  ier = UART_IER_RDI | UART_IER_THRI |UART_IER_RLSI;
  outb(ier,ADR_COM1+UART_IER);

/*
  if((inb(ADR_COM1+UART_LSR) & UART_LSR_THRE)==0){
  		schedule();
  }
  else{
  		outb('W',ADR_COM1+UART_TX);
  }*/
  if(request_irq(IRQ_NUM, short_interrupt ,SA_INTERRUPT, "serp", (void*) serp_devices) != 0){
      printk(KERN_ALERT "ERRO no REQUEST IRQ");
      return -1;
  }



  return result;
}



static void serp_exit(void) {

  //printk(KERN_ALERT "Fairwell major %d\n",serp_major);
  kfifo_free(serp_devices->rxbuf);
  free_irq(IRQ_NUM, (void*) serp_devices);

  if (serp_devices) {
      cdev_del(&serp_devices->serp_cdevs);

  }

  kfree(serp_devices);

  unregister_chrdev_region(MKDEV(major,0), serp_minor);

  // Release previous allocated region with address from (0x3f8-0x3ff)
  release_region(ADR_COM1, 8);



  printk(KERN_ALERT "Device driver with Major number %d was deleted!\n", major);

}

module_init(serp_init);
module_exit(serp_exit);
