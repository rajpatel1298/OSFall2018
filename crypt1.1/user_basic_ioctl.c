#include <stdio.h>
#include <fcntl.h>
#include "ioctl_basic.h"  //ioctl header file
#include <string.h>
//right now BUFFERSIZE is set to 10, just change it in the headerfile to whatever ya need

void create(int fd){
	
	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,CREATE, &userStruct);

}

void destroy(int fd){
	
	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,DESTROY, &userStruct);
}

void decrypt(int fd){
	
	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);
	userStruct.flag = 0;


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,DECRYPT, &userStruct);

	


	//remember that ioctl is non blocking aka have to make
	//sure that the lkm is done with is work before you
	//breaks when .flag is 1 aka the lkm has finished writing over
	while(userStruct.flag == 0);

	
	printf("New message buffer: %s\n", userStruct.messageBuffer);

}

void encrypt(int fd){
	
	
	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,ENCRYPT, &userStruct);

}





main ( ) {

 int fd;

        fd = open("/dev/temp", O_RDWR);

        if (fd == -1)
        {
                printf("Error in opening file \n");
                exit(-1);
        }
      
 decrypt(fd);

 

         close(fd);
}

