#include<linux/fs.h>              
#include <linux/module.h>
#include <linux/cdev.h>
#include<linux/device.h>
#include<linux/kfifo.h>
#include<linux/slab.h>

#define MAX_BUF_SIZE 1024
#define MAX_SIZE 1024


typedef struct priv_obj{
	struct cdev cdev; //global
	struct kfifo k1;
	struct device *pdev;
}PRIV_OBJ;

PRIV_OBJ* pobj;
struct class *pclass;
int ndevices=1;
dev_t pdevid;
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
pobj=kmalloc(sizeof(PRIV_OBJ),GFP_KERNEL);
kfifo_alloc(&pobj->k1, MAX_SIZE, GFP_KERNEL);
//Dynamically allocate device number
alloc_chrdev_region(&pdevid,0,ndevices,"pseudo_sample");

//Intialize cdev structure
cdev_init(&pobj->cdev,&fops); 
kobject_set_name(&pobj->cdev.kobj, "pdevice%d", 1);

//Register device in drivers
ret = cdev_add(&pobj->cdev, pdevid, 1);


pobj->pdev = device_create(pclass,NULL,pdevid,NULL,"psample%d",i);
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
cdev_del(&pobj->cdev);
kfifo_free(&pobj->k1);
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
 int ret,rcount = size;
if(kfifo_is_empty(&pobj->k1))
 return 0;
if(rcount > kfifo_len(&pobj->k1))
rcount =kfifo_len(&pobj->k1);
char *tbuf=kmalloc(rcount,GFP_KERNEL);
kfifo_out(&pobj->k1, tbuf, rcount);
ret=copy_to_user(ubuf, tbuf, rcount);
if(ret) {
     printk("Erorr inn copy_to_user,failed");
     return -EINVAL;
   }
   
      
   printk("Pseudo--read method, transferred %d bytes\n",rcount);
kfree(tbuf);
return rcount;
}
ssize_t pseudo_write(struct file * file, const char __user * ubuf, size_t size, loff_t * off) 
{
int ret,wcount = size;
if(kfifo_is_full(&pobj->k1))
 return -ENOSPC;
if(wcount > kfifo_avail(&pobj->k1))
wcount =kfifo_avail(&pobj->k1);
char *tbuf=kmalloc(wcount,GFP_KERNEL);
ret=copy_from_user(tbuf, ubuf, wcount);
if(ret) {
     printk("Erorr inn copy_from_user");
     return -EINVAL;
   }   

   
   printk("Pseudo--write method, transferred %d bytes\n",wcount);
kfifo_in(&pobj->k1, tbuf, wcount);
kfree(tbuf);
return wcount;
}

module_init(psuedo_init);
module_exit(pseudo_exit);
module_exit(pseudo_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("step-6");


MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("private objects step");
