#include<linux/fs.h>              
#include <linux/module.h>
#include <linux/cdev.h>
#include<linux/device.h>
#include<linux/kfifo.h>
#include<linux/slab.h>

struct cdev cdev; //global
struct device *pdev;
struct class *pclass;
struct kfifo k1;
int ndevices=1;
dev_t pdevid;
#define MAX_SIZE 1024
char *pbuffer;


int pseudo_open(struct inode* inode, struct file* file);
int pseudo_close(struct inode* inode, struct file* file);
ssize_t pseudo_read(struct file * file, char __user * buf, size_t size, loff_t * off);
ssize_t pseudo_write(struct file * file, const char __user * buf, size_t size, loff_t * off);



struct file_operations fops ={
.open = pseudo_open,
.release = pseudo_close,
.write = pseudo_write,
.read = pseudo_read,
};


static int __init psuedo_init(void)
{
int ret,i=0;

pclass=class_create(THIS_MODULE,"pseudo_class");

kfifo_alloc(&k1, MAX_SIZE, GFP_KERNEL);
//Dynamically allocate device number
alloc_chrdev_region(&pdevid,0,ndevices,"pseudo_sample");

//Intialize cdev structure
cdev_init(&cdev,&fops); 
kobject_set_name(&cdev.kobj, "pdevice%d", 1);

//Register device in drivers
ret = cdev_add(&cdev, pdevid, 1);


pdev = device_create(pclass,NULL,pdevid,NULL,"psample%d",i);
if(ret)
{
printk("Pseudo: Failed to register driver\n");
return -EINVAL;
}
printk("Successfully registered, major-%d,minor=%d\n",MAJOR(pdevid),MINOR(pdevid));
printk("Pseudo Driver Sample..welcome\n");
return 0;
}
static void __exit pseudo_exit(void) {
cdev_del(&cdev);
kfifo_free(&k1);
device_destroy(pclass,pdevid);
unregister_chrdev_region(pdevid, ndevices);
class_destroy(pclass);

printk("Pseudo Driver Sample..Bye\n");

}


int pseudo_open(struct inode* inode, struct file* file)

{ 
  printk("Pseudo--open method\n"); 
  return 0;
} 
int pseudo_close(struct inode* inode, struct file* file)
 { 
 printk("Pseudo--release method\n");
 return 0;
} 
ssize_t pseudo_read(struct file * file, char __user * ubuf, size_t size, loff_t * off)
{
 int rcount = size;
if(kfifo_is_empty(&k1))
 return 0;
if(rcount > kfifo_len(&k1))
rcount =kfifo_len(&k1);
char *tbuf=kmalloc(rcount,GFP_KERNEL);
kfifo_out(&k1, tbuf, rcount);
copy_to_user(ubuf, tbuf, rcount);
kfree(tbuf);
return rcount;
}
ssize_t pseudo_write(struct file * file, const char __user * ubuf, size_t size, loff_t * off) 
{
int wcount = size;
if(kfifo_is_full(&k1))
 return -ENOSPC;
if(wcount > kfifo_avail(&k1))
wcount =kfifo_avail(&k1);
char *tbuf=kmalloc(wcount,GFP_KERNEL);
copy_from_user(tbuf, ubuf, wcount);
kfifo_in(&k1, tbuf, wcount);
kfree(tbuf);
return wcount;
}

module_init(psuedo_init);
module_exit(pseudo_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("step-5");
