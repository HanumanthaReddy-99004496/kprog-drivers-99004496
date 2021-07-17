#include<linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

dev_t pdevid;
int ndevices=1;

static int __init step1_init(void)
{
int ret;
ret=alloc_chrdev_region(&pdevid, 0, ndevices, "step1_sample");
if(ret) 
{
printk("step1: Failed to register driver\n");
return -EINVAL;
}
printk("Successfully registered,major=%d,minor=%d\n",
MAJOR(pdevid), MINOR(pdevid));
printk("step1 Driver Sample..welcome\n");
return 0;
}
static void __exit step1_exit(void) {
unregister_chrdev_region(pdevid, ndevices);
printk("step1 Driver Sample..Bye\n");
}

module_init(step1_init);
module_exit(step1_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HanumanthaReddy");
MODULE_DESCRIPTION("Driver step1 Module");
