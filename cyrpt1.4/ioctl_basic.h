#include <linux/ioctl.h>
	#define IOC_MAGIC 'k'
	#define IOCTL_HELLO _IO(IOC_MAGIC,0)
	#define CREATE 0
	#define DESTROY 1
	#define ENCRYPT 2
	#define DECRYPT 3
	#define BUFFERSIZE 100
	
	typedef struct s{
	char messageBuffer[BUFFERSIZE];
	char keyBuffer[BUFFERSIZE];
	int id;
	int flag;
	} argStruct;
	
	typedef struct d {
	char keyBuffer[BUFFERSIZE];
	char messageBuffer[BUFFERSIZE];
	int encryptFd;
	struct file * encryptFP;
	struct file * decryptFP;
	int id;
	} device;

