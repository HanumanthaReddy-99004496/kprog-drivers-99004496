#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/dev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>

#define mem_size 1024

//Define the ioctl code
#define WR_DATA _IOW('a','a','int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)

int32_t val=0;


dev_t dev=0;
static struct class *dev_class;
static struct cdev my_cdev;

uint8_t *kernel_buffer;

static int __init chr_driver_init(void);
static void __exit chr_driver_exit(void);

//declaration of file operations
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops=
{
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.open = my_open,
	.unlocked_ioctl = chr_ioctl, 
	.release = my_release,
};

static int my_open(struct inode *inode, struct file *file)
{
		/*creating the physical memory*/
		//loopback driver using global buffer
		kenel_buffer=kmalloc(mem_size, GFP_KERNEL)
		
		if(kernel_buffer==0)
		{
				printk(KERNEL_INFO "cannot allocate memory to the kernel");
				return -1;
				//return -ENOMEM;
		}
		
		printk(KERN_INFO "device file found");
		return 0;
}

static my_release(struct inode *inode, struct file *file)
{
		kfree(kenel_buffer);
		printk(KERN_INFO "device file closed");
		return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	int ret=copy_to_user(buf,kernel_buffer,mem_size);
	printk(KERN_INFO "data read");
	return mem_size;
}


static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
		int ret=copy_from_user(kenel_buffer, buf, len);
		if(ret)
		{
			printk("error in copy_from_user");
			return -EINVAL;
		}
		printk(KERN_INFO "data written");
		return len;
}

static long chr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case WR_DATA:
			copy_from_user(&val, (int32_t*)arg,sizeof(val));
			printk(KERN_INFO " val= %d\n", val);
			break;
			
		case RD_DATA:
			copy_to_user((int32_t*)arg,&val,sizeof(val));
			break;
	}
	
	return 0;
}		

static int __init chr_driver_init(void)
{
		/* Allocating major number dynamically*/
		int a=alloc_chrdev_region(&dev,0,1,"my_dev");
		
		if(a<0)
		{
			printk(KERR_INFO"Cannot allocate major number");
			return -1;
		}
		
		/*printing major number and minor number*/
		printf(KERN_INFO"major number=%d and minor number=%d", MAJOR(dev), MINOR(dev));
		
		/*creating cdev structure*/
		cdev_init(&my_cdev, &fops);
		
		/*Adding character device to the system*/
		int b=cdev_add(&my_cdev, dev,1);
		
		if(b<0)
		{
				printk(KERN_INFO "cannot add the device to the system");
				goto r_class;
		}
		
		/*creating struct class*/
		
		int c=dev_class=class_create(THIS_MODULE,"my_class");
		
		if(c==NULL)
		{
			printk(KERN_INFO "cannot create the struct class");
			goto r_class;
		}
		
		/*creating device*/
		int d=device_create(dev_class,NULL,dev,NULL,"my_device");
		
		if(d==NULL)
		{
			printk(KERN_INFO "cannot create the device");
			goto r_device;
		}
	
	printk(KERN_INFO "device driver inserted properly");
	return 0;	
}


r_device:
	class_destroy(dev_class);
	
r_class:
	unregister_chrdev_region(dev,1);
	return -1;

static void __exit chr_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "device driver is removed successfully");
}

module_init(chr_driver_init);
module_exit(chr_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("The character device driver");
