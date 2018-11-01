#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // required for various structures related to files liked fops. 
#include <linux/semaphore.h>
#include <linux/cdev.h> 
#include "ioctl_basic.h"    //ioctl header file
#include <linux/version.h>
static int Major;


int open(struct inode *inode, struct file *filp)
{
 
 printk(KERN_INFO "Inside open \n");
 return 0;
}

int release(struct inode *inode, struct file *filp) {
 printk (KERN_INFO "Inside close \n");
 return 0;
}

static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg){

int data=10,ret;

switch(cmd) {

case IOCTL_HELLO: 
 printk(KERN_INFO "Hello ioctl world");
 break;
 } 
 
return ret;
 
}

struct file_operations fops = {
 open:   open,
 unlocked_ioctl:   my_ioctl,
 release: release
};


struct cdev *kernel_cdev; 


int char_arr_init (void) {
 int ret;
 dev_t dev_no,dev;

 kernel_cdev = cdev_alloc(); 
  kernel_cdev->ops = &fops;
 kernel_cdev->owner = THIS_MODULE;
 printk (" Inside init module\n");
  ret = alloc_chrdev_region( &dev_no , 0, 1,"char_arr_dev");
    if (ret < 0) {
  printk("Major number allocation is failed\n");
  return ret; 
 }
 
    Major = MAJOR(dev_no);
    dev = MKDEV(Major,0);
 printk (" The major number for your device is %d\n", Major);
 ret = cdev_add( kernel_cdev,dev,1);
 if(ret < 0 ) 
 {
 printk(KERN_INFO "Unable to allocate cdev");
 return ret;
 }

 return 0;
}

void char_arr_cleanup(void) {
 printk(KERN_INFO " Inside cleanup_module\n");
 cdev_del(kernel_cdev);
 unregister_chrdev_region(Major, 1);
}
MODULE_LICENSE("GPL"); 
module_init(char_arr_init);
module_exit(char_arr_cleanup);


