#include<linux/fs.h>              
#include <linux/module.h>
#include <linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

struct cdev cdev; //global
struct device *pdev;
struct class *pclass;
int ndevices=1;
dev_t pdevid;
char *pbuffer;
int wr_offset=0;
int rd_offset=0;
int buflen=0;
#define MAX_BUF_SIZE 1024
#define MAX_SIZE 1024

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

pbuffer = kmalloc(MAX_BUF_SIZE,GFP_KERNEL);
if(pbuffer == NULL)
{
 printk("Pseudo : Kmalloc failed\n");
 return -ENOMEM;
}
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
kfree(pbuffer);
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
ssize_t pseudo_read(struct file * file, char __user * ubuf, size_t usize, loff_t * off)
{
int ret, rlen;
   rlen=usize;
   
   if(buflen==0) {    //(wr_offset - rd_offset)==0
     printk("pseudo read -- buffer is empty\n");
     return 0;       //buffer is empty
   }    
   if(rlen > buflen)
       rlen = buflen;  // min of uspace request, buffer length
    
   ret=copy_to_user(ubuf, pbuffer, rlen);
   if(ret) {
     printk("Erorr inn copy_to_user,failed");
     return -EINVAL;
   }
   
   rd_offset += rlen;
   buflen -= rlen;
      
   printk("Pseudo--read method, transferred %d bytes\n",rlen);
   return rlen;
}
ssize_t pseudo_write(struct file * file, const char __user * ubuf, size_t usize, loff_t * off) 
{
 int ret, wlen, kremain;
   
   wlen=usize;
   kremain = MAX_SIZE - wr_offset;
   
   if(kremain==0)    // wr_offset >= MAX_SIZE
      return -ENOSPC;     //buffer is full
         
   if(wlen > kremain)
      wlen = kremain;    //min of user request, remaining space
      
   ret=copy_from_user(pbuffer, ubuf, wlen);
   if(ret) {
     printk("Erorr inn copy_from_user");
     return -EINVAL;
   }   
   
   buflen += wlen;
   wr_offset += wlen;
   
   printk("Pseudo--write method, transferred %d bytes\n",wlen);
   return wlen;
}

module_init(psuedo_init);
module_exit(pseudo_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
