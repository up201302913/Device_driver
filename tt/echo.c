#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#define DEVICE_NUMBER	4

MODULE_LICENSE("Dual BSD/GPL");

struct echo_dev {
	struct cdev c_dev;
	int cnt;
};

static dev_t dev_no;
static struct class *cl;
static struct echo_dev echo_d;

static int dev_open(struct inode *i, struct file *f)
{
	struct echo_dev *p = container_of(i->i_cdev, struct echo_dev, c_dev);
	f->private_data = p;
	nonseekable_open(i, f);
	printk(KERN_INFO "Device driver: open()\n");
	return 0;
}

static int dev_release(struct inode *i, struct file *f)
{
	struct echo_dev *p = container_of(i->i_cdev, struct echo_dev, c_dev);
	if(f->private_data != NULL && f->private_data != p)
		kfree(f->private_data);
	printk(KERN_INFO "Device driver: close()\n");
	return 0;
}

static ssize_t dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	char *kbuffer = f->private_data;
	unsigned long klen, i;
	if(kbuffer == NULL){
		printk(KERN_ALERT "Error\n");
		return -1;
	}
	for(i = 0; i < len - 1 && kbuffer[i] != 0; i++);
	klen = copy_to_user(buf, kbuffer, i);
	if(i - (klen = i - klen))
		printk(KERN_ALERT "Could not copy memory\n");
	kfree(kbuffer);
	f->private_data = NULL;
	printk(KERN_INFO "Device driver: read()\n");
	return klen;
}

static ssize_t dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	char *kbuffer = kmalloc(len + 1, GFP_KERNEL);
	unsigned long klen;
	if(kbuffer == NULL){
		printk(KERN_ALERT "Not enough memory\n");
		return -1;
	}
	klen = copy_from_user(kbuffer, buf, len);
	if(len - (klen = len - klen))
		printk(KERN_ALERT "Could not read memory\n");
	kbuffer[klen] = 0;
	printk(KERN_ALERT "%s", kbuffer);
	f->private_data = kbuffer;
	printk(KERN_INFO "Device driver: write()\n");
	return klen;
}

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write,
	.llseek = no_llseek,
};

static int echo_init(void)
{
	int major, minor, i, j;
	printk(KERN_INFO "Device driver module inserted in kernel\n");
	if(alloc_chrdev_region(&dev_no, 0, DEVICE_NUMBER, "echo") < 0){
		printk(KERN_WARNING "Device number allocation failed.\n");
		return -1;
	}
	printk(KERN_ALERT "Allocated device major number: %d.\n", MAJOR(dev_no));
	cl = class_create(THIS_MODULE, "echo");

	printk(KERN_WARNING "created class\n");

	if(cl == NULL){
		printk("CL NULL\n");
		goto ucdev;
	}
	minor = MINOR(dev_no);
	major = MAJOR(dev_no);
	printk("MAJOR MINOR FOUND\n");
	for(i = 0; i < DEVICE_NUMBER; i++, minor++){
		if(device_create(cl, NULL, MKDEV(major, minor), NULL, "echo%d", i) == NULL)
			goto cl_d;
		printk("INITING CDEV\n");
		cdev_init(&echo_d.c_dev, &fops);
		printk("CDEV INITED\n");

		if(cdev_add(&echo_d.c_dev, MKDEV(major, minor), 1) < 0){
			printk("ADDING CDEV\n");
			goto dev_d;
		}
	}
	printk(KERN_WARNING "ELETRO\n");
	return 0;

dev_d:	for(j = i; j >= 0; j--, minor--)
		device_destroy(cl, MKDEV(major, minor));
cl_d:	class_destroy(cl);
ucdev:	unregister_chrdev_region(dev_no, 1);
	
	printk(KERN_WARNING "ELETRO2\n");
	return -1;
}

static void echo_exit(void)
{
	int i, major;
	major = MAJOR(dev_no);
	cdev_del(&echo_d.c_dev);
	for(i = 0; i < DEVICE_NUMBER; i++)
		device_destroy(cl, MKDEV(major, i));
	class_destroy(cl);
	unregister_chrdev_region(dev_no, 1);
	printk(KERN_INFO "Device number freed.\n");
	printk(KERN_INFO "Device driver module removed from kernel\n");
}

module_init(echo_init);
module_exit(echo_exit);
