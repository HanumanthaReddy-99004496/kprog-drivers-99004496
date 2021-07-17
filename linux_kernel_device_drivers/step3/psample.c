#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>

struct cdev cdev;
dev_t pdevid;
int ndevices=1;

struct device *pdev; //global
struct class *pclass; //global

int pseudo_open(struct inode* inode , struct file* file)
{
  printk("Pseudo--open method\n");
  return 0;
}
int pseudo_close(struct inode* inode , struct file* file)
{
  printk("Pseudo--release method\n");
  return 0;
}
ssize_t pseudo_read(struct file * file, char __user * buf , size_t size, loff_t * off)
{
  printk("Pseudo--read method\n");
  return 0;
}
ssize_t pseudo_write(struct file * file, const char __user * buf , size_t size, loff_t * off)
{
  printk("Pseudo--write method\n");
  return -ENOSPC;
}


struct file_operations fops = {
   .open    = pseudo_open,
   .release = pseudo_close,
   .write   = pseudo_write,
   .read    = pseudo_read
};

static int __init psuedo_init(void)
{
   int ret;
   int i=0;
   pclass = class_create(THIS_MODULE, "pseudo_class");
   ret=alloc_chrdev_region(&pdevid, 0, ndevices, "myfiledriver_sample");
   if(ret) 
   {
      printk("Pseudo: Failed to register driver\n");
      return -EINVAL;
   }
   cdev_init(&cdev, &fops);
   kobject_set_name(&cdev.kobj,"pdevice%d", i);
   ret = cdev_add(&cdev, pdevid, 1);
   pdev = device_create(pclass, NULL, pdevid, NULL, "psample%d",i);
   
   printk("Successfully registered,major=%d,minor=%d\n",MAJOR(pdevid), MINOR(pdevid));//MAKEDEV
   printk("Pseudo Driver Sample..welcome\n");
   
   return 0;
}

static void __exit psuedo_exit(void)
{
  device_destroy(pclass, pdevid);
  cdev_del(&cdev);
  unregister_chrdev_region(pdevid, ndevices);
  class_destroy(pclass);
  printk("Pseudo Driver Sample..Bye\n");
}


module_init(psuedo_init);
module_exit(psuedo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("A Simple Driver");
