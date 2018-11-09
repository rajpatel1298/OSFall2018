 	#include <linux/module.h>
	#include <linux/kernel.h>
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
	#include "ioctl_basic.h" //ioctl header file
	
	
	static int Major;
	int currentId = 0;
	int createFlag = 0;
	device devices[10];
	

	


	
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
	
	int msgSize = strlen(msg);
	int keySize = strlen(key);
	
	char newKey[msgSize]; //msg + newKey = encrypted key
	char encryptedMsg[msgSize];
	
	int i;
	for(i = 0; i < msgSize; i++){
	newKey[i] = key[i % keySize];
	}
	newKey[i] = '\0';
	
	for(i = 0; i < msgSize; i++){
	encryptedMsg[i] = ((msg[i] + newKey[i]) % 26) + 'A';
	}
	
	encryptedMsg[i] = '\0';
	strcpy(msg,encryptedMsg);
	
	return msg;
	}
	char * decipher (char msg[], char key[]){
	
	int msgSize = strlen(msg);
	int keySize = strlen(key);
	
	char newKey[msgSize]; //msg + newKey = encrypted key
	char decryptedMsg[msgSize];
	
	int i;
	for(i = 0; i < msgSize; i++){
	newKey[i] = key[i % keySize];
	}
	
	for(i = 0; i < msgSize; i++){
	decryptedMsg[i] = ((msg[i] - newKey[i] + 26) % 26) + 'A';
	}
	decryptedMsg[i] = '\0';
	strcpy(msg,decryptedMsg);
	return msg;
	}
	
	const char * fetch_create_string(char buf[]){ //0 = encrypt, 1 = decrypt
	
	char id [2];
	id[0] = currentId + '0'; //only cycles from 0-9, so can use ascii value to convert
	id[1] = '\0';
	strcat(buf,id);
	
	if(createFlag == 2){
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
	
	void create(unsigned long arg){
	
	struct file * filp = NULL;
	struct file * filp2 = NULL;
	char encryptBuf[25] = "/dev/cryptEncrypt";
	char decryptBuf[25] = "/dev/cryptDecrypt";
	//Getting copy of the struct passed by user
	argStruct kernStruct;
	device newDevice;
	argStruct * userStruct = (argStruct *)arg;
	
	raw_copy_from_user(&kernStruct,userStruct, sizeof(argStruct) );
	
	
	filp = file_open(fetch_create_string(encryptBuf), O_RDWR | O_CREAT, S_IRWXU);
	filp2 = file_open(fetch_create_string(decryptBuf), O_RDWR | O_CREAT, S_IRWXU);
	
	if( filp && filp2) {
	printk("Successfully created files...");
	strcpy(newDevice.keyBuffer, kernStruct.keyBuffer);
	newDevice.encryptFP = filp;
	newDevice.decryptFP = filp2;
	newDevice.id = currentId;
	devices[currentId] = newDevice;
	printk("Device struct saved");
	kernStruct.flag = 1;
	raw_copy_to_user(userStruct, &kernStruct, sizeof(argStruct));
	} else {
	printk("Error opening files...");
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
	
	finalMessage = decipher(devices[kernStruct.id].messageBuffer, devices[kernStruct.id].keyBuffer);
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
	//decrypt(arg);
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
	open: open,
	unlocked_ioctl: my_ioctl,
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
	
