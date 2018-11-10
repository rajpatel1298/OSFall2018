#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/fs.h> // required for various structures related to files liked fops
#include <linux/semaphore.h>
#include <linux/cdev.h> 
#include <linux/version.h>
#include <linux/buffer_head.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include "ioctl_basic.h"    //ioctl header file
#include <linux/gfp.h>

int currentId = 0;
int createFlag = 0;
device devices[10];

int numDevices = 0;
static dev_t first; 
static struct cdev c_dev;
static struct class * deviceClass;



int open(struct inode *inode, struct file *filp)
{
 
 printk(KERN_INFO "Inside open \n");
 return 0;
}

int release(struct inode *inode, struct file *filp) {
 printk (KERN_INFO "Inside close \n");
 return 0;
}



struct file * file_open(const char * path, int flags, int rights){
	struct file * filp = NULL;
	mm_segment_t oldFs;

	int err = 0;
	oldFs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path,flags,rights);
	set_fs(oldFs);

	if(IS_ERR(filp)){
		err = PTR_ERR(filp);
		return NULL;
	}

	return filp;
}

void file_close(struct file * file){
	filp_close(file,NULL);
}

int file_read(struct file * file, unsigned long long offset, unsigned char * data, unsigned int size){
	mm_segment_t oldFs;
	int ret;

	oldFs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file,data,size, &offset);

	set_fs(oldFs);
	return ret;
}

int file_write(struct file * file, unsigned long long offset, unsigned char * data, unsigned int size){

	mm_segment_t oldFs;
	int ret;

	oldFs = get_fs();
        set_fs(get_ds());

	ret = vfs_write(file,data,size,&offset);
	set_fs(oldFs);
	
	return ret; 	
}

char * cipher(char msg[], char key[]){

        int msgSize = strlen(msg)-1; //changed
        int keySize = strlen(key)-1; //changed
	int i;
        char newKey[msgSize]; //msg + newKey = encrypted key
        char encryptedMsg[msgSize];
//	char encryptedMsgProper[msgSize];
//	msg[msgSize] = '\0';
//	key[keySize] = '\0';

	printk("Message %s. Key %s inside of cipher", msg, key);
        
        for(i = 0; i < msgSize; i++){
		if(key[i] >= 65 && key[i] <= 90){
                	newKey[i] = key[i % keySize];
		}else{ //turns the key captial if lowercase
			key[i] -= 32;
			newKey[i] = key[i % keySize];
		}
        }
	
        for(i = 0; i < msgSize; i++){
		if(msg[i] >= 65 && msg[i] <= 90){
                	encryptedMsg[i] = ((msg[i] + newKey[i]) % 26) + 'A';
//			encryptedMsgProper[i] = ((msg[i] + newKey[i]) % 26) + 'A';
			
		}else{	
			printk("msg[i] = %c",msg[i]);
			
			
			msg[i] -= 32;
			printk("msg[i]-32 = %c", msg[i]);			
			encryptedMsg[i] = ((msg[i] + newKey[i]) % 26) + 'A' + 32;
//			encryptedMsgProper[i] = ((msg[i] + newKey[i]) % 26) + 'A' + 32;
			
		}	
        }

        encryptedMsg[i] = '\0';
        strcpy(msg,encryptedMsg);
	char * ptr = kmalloc(strlen(msg)+1, 0);
	strcpy(ptr,msg);
        return ptr;
}
char * decipher (char msg[], char key[]){
       
        int msgSize = strlen(msg);
        int keySize = strlen(key)-1;
	int i;
        char newKey[msgSize]; //msg + newKey = encrypted key
        char decryptedMsg[msgSize];

//	msg[msgSize] = '\0';
//	key[keySize] = '\0';

        
        for(i = 0; i < msgSize; i++){
                if(key[i] >= 65 && key[i] <= 90){
                	newKey[i] = key[i % keySize];
		}else{ //turns the key captial if lowercase
			key[i] -= 32;
			newKey[i] = key[i % keySize];
		}
        }

        for(i = 0; i < msgSize; i++){
		if(msg[i] >= 65 && msg[i] <= 90){
                	decryptedMsg[i] = ((msg[i] - newKey[i] + 26) % 26) + 'A';
		}else{
			msg[i] -= 32;
			decryptedMsg[i] = ((msg[i] - newKey[i] + 26) % 26) + 'A' + 32;
		}
        }
        decryptedMsg[i] = '\0';
	printk("BEFORE DECRYPTION: %s", msg);
        strcpy(msg,decryptedMsg);
	printk("DECTRYPTED: %s", msg); 
        return msg;
}

const char * fetch_create_string(char buf[]){ //0 = encrypt, 1 = decrypt
	
	char idBuf [2];
	idBuf[0] = currentId + '0'; //only cycles from 0-9, so can use ascii value to convert	
	idBuf[1] = '\0';
	strcat(buf,idBuf);


	if(createFlag == 2) {
		currentId++;
		if(currentId == 10){
			currentId = 0;
		}
		createFlag = 0;
	} else {
		createFlag++;
	}
	return buf;
}


int make_device(char buf[]){

	printk(KERN_INFO "Registering New Device...");

 	if(device_create(deviceClass, NULL,MKDEV(MAJOR(first), numDevices+1), NULL, fetch_create_string(buf)) == NULL) 	{
		class_destroy(deviceClass);
		cdev_del(&c_dev);
		unregister_chrdev_region(first,numDevices);
		printk(KERN_INFO "Registering New Device Failed.");
		return -1;
 	}
	 numDevices++;
	 return 0;	
}


void create(unsigned long arg){

	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	char encryptBuf[25] = "/dev/cryptEncrypt";
	char decryptBuf[25] = "/dev/cryptDecrypt";
	int dev1,dev2;
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	dev1 = make_device(encryptBuf);
	dev2 = make_device(decryptBuf);

	if(dev1 == 0 && dev2 == 0){
		printk("Both devices succeeded in  creation");
	}
	else {
		printk("Device registration failed");
	}
	
	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);
	
}

void destroy(unsigned long arg){

	
	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	

	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	filp_close(devices[kernStruct.id].encryptFP, NULL);
	filp_close(devices[kernStruct.id].decryptFP, NULL);

	

	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);
}

	void encrypt(unsigned long arg){
	
	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	char * finalMessage;
	int write;
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);
	printk(KERN_INFO "keyBuffer for device: %s", devices[kernStruct.id].keyBuffer);

	int id = kernStruct.id;

	strcpy(	devices[id].messageBuffer, kernStruct.messageBuffer);
	
	finalMessage = cipher(kernStruct.messageBuffer, devices[kernStruct.id].keyBuffer);
	printk("After cipher() finalmessage = %s", finalMessage);
	devices[kernStruct.id].encryptedBuffer = finalMessage;
	printk("After strcpy= %s", kernStruct.encryptedBuffer);
	
	write = file_write(devices[kernStruct.id].encryptFP, 0,finalMessage, strlen(kernStruct.messageBuffer));
	
	printk("Wrote %d bytes into file", write);
	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "messageBuffer: %s",kernStruct.messageBuffer);
	}
	
	void decrypt(unsigned long arg){
	
	//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	char * finalMessage;
	int write;
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );	
	
	//At this point have a copy of user buffer in kernBuffer
	printk(KERN_INFO "id for device: %d", kernStruct.id);
	printk(KERN_INFO "keyBuffer for device: %s", devices[kernStruct.id].keyBuffer);
	
	
	//Do the decrpyt stuff
	printk("Before decipher called encryptedBuffer %s",devices[kernStruct.id].encryptedBuffer);
	printk("MessgeBuffer %s", devices[kernStruct.id].messageBuffer);
	finalMessage = decipher(devices[kernStruct.id].encryptedBuffer, devices[kernStruct.id].keyBuffer);
	write = file_write(devices[kernStruct.id].decryptFP,0,finalMessage, strlen(kernStruct.messageBuffer));
	
	printk("Wrote %d bytes into file", write);
	
	
	}

	void confg(unsigned long arg){

		//Getting copy of the struct passed by user
	argStruct kernStruct;
	argStruct * userStruct = (argStruct *)arg;
	
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	printk(KERN_INFO "keyBuffer: %s",kernStruct.keyBuffer);

	int id = kernStruct.id;

	strcpy(	devices[id].keyBuffer, kernStruct.keyBuffer);

	printk(KERN_INFO "keyBuffer for device: %s", devices[kernStruct.id].keyBuffer);

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
	
	case 13:
	printk(KERN_INFO "CASE CONF");
	confg(arg);
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

 printk(KERN_INFO "LKM Registered");
/*
 if(alloc_chrdev_region(&first,0,10,"cryptctl") < 0){
	return -1;
 } 

 if( (deviceClass = class_create(THIS_MODULE, "chardev")) == NULL){
	unregister_chrdev_region(first,10);
	return -1;
 }

 if(device_create(deviceClass, NULL, first, NULL, "cryptctl") == NULL) {
	class_destroy(deviceClass);
	unregister_chrdev_region(first,10);
	return -1;
 }

 cdev_init(&c_dev, &fops);
 
 if(cdev_add(&c_dev, first,10) == -1){
	device_destroy(deviceClass,first);
	class_destroy(deviceClass);
	unregister_chrdev_region(first,10);
	return -1;
 }
 */


 if(alloc_chrdev_region(&first,0,10,"cryptctl") < 0){
	return -1;
 }

 cdev_init(&c_dev, &fops);
 if( cdev_add(&c_dev, first, 10) < 0){
	return -1;
 } 

 if( (deviceClass = class_create(THIS_MODULE, "chardrvr")) == NULL){
	class_destroy(deviceClass);
	unregister_chrdev_region(first, 10);
	return -1;
 }

 if( device_create(deviceClass, NULL, MKDEV(MAJOR(first), 0), NULL, "cryptctl") == NULL) {
	class_destroy(deviceClass);
	cdev_del(&c_dev);
	unregister_chrdev_region(first,10);
	return -1;
 }

 return 0;
}

void char_arr_cleanup(void) {
 printk(KERN_INFO " Inside cleanup_module\n");
 cdev_del(&c_dev);
 device_destroy(deviceClass,first);
 class_destroy(deviceClass);
 unregister_chrdev_region(first,1);
}
MODULE_LICENSE("GPL"); 
module_init(char_arr_init);
module_exit(char_arr_cleanup);


