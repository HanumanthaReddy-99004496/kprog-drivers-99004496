#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/list.h>

#define MAX_SIZE 1024

typedef struct priv_obj{
	struct cdev cdev;
	struct device *pdev;
	struct kfifo pkfifo;
	struct list_head node;
}PRIV_OBJ;

dev_t pdevid;
struct class *pclass;	// shouldn't be private??

int ndevices=1;
module_param(ndevices,int,S_IRUGO);
//PRIV_OBJ* pobj;

LIST_HEAD(pobjlist);
unsigned char *pbuffer;

int pseudo_open(struct inode* inode, struct file* file)
{
	// printk("Pseudo--open method\n");
	PRIV_OBJ *pobj=container_of(inode->i_cdev,PRIV_OBJ,cdev);
	file->private_data=pobj;
	return 0;
}

int pseudo_close(struct inode* inode, struct file* file)
{
	printk("Pseudo--release method\n");
	return 0;
}

ssize_t pseudo_read(struct file * file, char __user * ubuf , size_t usize, loff_t * off)
{
   
   int ret, rlen;
   char* tbuf;
   PRIV_OBJ *pobj=file->private_data;
   rlen=usize;
   
   if(kfifo_is_empty(&(pobj->pkfifo))) 
   {
     printk("pseudo read -- buffer is empty\n");
     return 0;       //buffer is empty
   }    
   
   if(rlen > kfifo_len(&(pobj->pkfifo)))
       rlen = kfifo_len(&(pobj->pkfifo));  // min of uspace request, buffer length
   
   
   tbuf=kmalloc(rlen,GFP_KERNEL);
   ret=kfifo_out(&(pobj->pkfifo),tbuf,rlen);
   ret=copy_to_user(ubuf,tbuf,rlen);
   if(ret) {
     printk("Erorr inn copy_to_user,failed");
     return -EINVAL;
   }

   printk("Pseudo--read method, transferred %d bytes\n",rlen);
   kfree(tbuf);
   return rlen;

}
ssize_t pseudo_write(struct file * file, const char __user * ubuf , size_t usize, loff_t * off)
{
   
   int ret, wlen;
   char* tbuf;
   PRIV_OBJ *pobj=file->private_data;
   wlen=usize;
   
   if(kfifo_is_full(&(pobj->pkfifo)))    // wr_offset >= MAX_SIZE
      return -ENOSPC;     //buffer is full
         
   if(wlen > kfifo_avail(&(pobj->pkfifo)))
      wlen = kfifo_avail(&(pobj->pkfifo));    //min of user request, remaining space
   
   
   tbuf = kmalloc(wlen,GFP_KERNEL);
   ret=copy_from_user(tbuf, ubuf, wlen);
   if(ret) {
     printk("Erorr inn copy_from_user");
     return -EINVAL;
   }
   
   kfifo_in(&(pobj->pkfifo),tbuf,wlen);

   printk("Pseudo--write method, transferred %d bytes\n",wlen);
   kfree(tbuf);
   return wlen;
}

struct file_operations fops={
	.open		= pseudo_open,
	.release	= pseudo_close,
	.write		= pseudo_write,
	.read		= pseudo_read
};

static int __init pseudo_init(void)
{
	PRIV_OBJ *pobj;
	int ret,i=0;
		
	ret=alloc_chrdev_region(&pdevid,0,ndevices,"pseudo_sample");
	if(ret)
	{
		printk("Pseudo: Failed to register driver\n");
		return -EINVAL;
	}
	
	pclass=class_create(THIS_MODULE,"pseudo_class");
	
	for(i=0;i<ndevices;i++)
	{
		pobj=kmalloc(sizeof(PRIV_OBJ),GFP_KERNEL);
		if(pobj==NULL)
		{
			printk("pobj kmalloc failed!\n");
			return -ENOMEM;
		}
		pbuffer=kmalloc(MAX_SIZE,GFP_KERNEL);
		if(pbuffer==NULL)
		{
			printk("pbuffer kmalloc failed!\n");
			return -ENOMEM;
		}
		kfifo_init(&(pobj->pkfifo),pbuffer,GFP_KERNEL);

		cdev_init(&(pobj->cdev),&fops);
		kobject_set_name(&(pobj->cdev.kobj),"pdevice%d",i);
		ret=cdev_add(&(pobj->cdev),pdevid+i,1);								//minor number
		if(ret)
		{
			printk("Successfully registered, major=%d, minor=%d\n",MAJOR(pdevid),MINOR(pdevid));
		}

		pobj->pdev=device_create(pclass,NULL,pdevid+i,NULL,"psample%d",i);
		if(pobj->pdev==NULL)
		{
			printk("error creating device.\n");
			return 0;
		}
		list_add_tail(&pobj->node,&pobjlist);
	}
	
	return 0;
}

static void __exit pseudo_exit(void)
{
	struct list_head *pcur,*qcur;
	PRIV_OBJ *pobj;
	int i=0;

	list_for_each_safe(pcur,qcur,&pobjlist)
	{
		pobj=list_entry(pcur,PRIV_OBJ,node);
		kfifo_free(&(pobj->pkfifo));
		cdev_del(&(pobj->cdev));
		device_destroy(pclass,pdevid+i);
		i++;
		kfree(pobj);
	}
	unregister_chrdev_region(pdevid,ndevices);
	class_destroy(pclass);
	
	kfree(pbuffer);
	printk("Pseudo Driver Sample..Bye!\n");
}

module_init(pseudo_init);
module_exit(pseudo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanumantha Reddy");
MODULE_DESCRIPTION("Driver for multiple devices ");
