#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // required for various structures related to files liked fops. 
#include <linux/semaphore.h>
#include <linux/cdev.h> 
#include "ioctl_basic.h"    //ioctl header file
#include <linux/version.h>
#include <asm/uaccess.h>

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




void create(unsigned long arg){

	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	


	//have id
	printk(KERN_INFO "ID: %d",kernStruct.id);

}

void destroy(unsigned long arg){

	
	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	

	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	

	

	printk(KERN_INFO "ID: %d",kernStruct.id);
}

void encrypt(unsigned long arg){

	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);

	printk(KERN_INFO "KeyBuffer: %s",kernStruct.keyBuffer);

	printk(KERN_INFO "ID: %d",kernStruct.id);
}

void decrypt(unsigned long arg){

	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);

	printk(KERN_INFO "keyeBuffer: %s",kernStruct.keyBuffer);
	
	printk(KERN_INFO "ID: %d",kernStruct.id);







	//Do the decrpyt stuff

	//this is just a dummy decrpyt, remove this when actually implimenting the decrpyt logic
	kernStruct.messageBuffer[0] = 'F';	
	kernStruct.messageBuffer[1] = 'o';
	kernStruct.messageBuffer[2] = 'o';
	kernStruct.messageBuffer[3] = 'k';
	kernStruct.messageBuffer[4] = '\n';

	//Writing to the back to the user buffer
	//Remember to change to the flag to 1 in order to let user no lkm is done
	kernStruct.flag = 1;
	raw_copy_to_user(userStruct, &kernStruct, sizeof(argStruct));
}



static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg){

int data=10,ret;



switch(cmd) {

case CREATE: 
 printk(KERN_INFO "CASE CREATE");
 create(arg);
 break;

case DESTROY:
 printk(KERN_INFO "CASE DESTROY");
 destroy(arg);
 break;

case ENCRYPT:
 printk(KERN_INFO "CASE ENCRYPT");
 encrypt(arg);
 break;

case DECRYPT:
 printk(KERN_INFO "CASE DECRYPT");
 decrypt(arg);
 break;
case 69:
 printk(KERN_INFO "CASE ENCRYPT");
 encrypt(arg);
 break;

default:
 printk(KERN_INFO "DEF");
 

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


