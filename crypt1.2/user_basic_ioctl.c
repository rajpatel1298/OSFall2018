#include <stdio.h>
#include <fcntl.h>
#include "ioctl_basic.h"  //ioctl header file
#include <string.h>
#include <stdlib.h>
//right now BUFFERSIZE is set to 10, just change it in the headerfile to whatever ya need

void create(int fd){
		
	char id[10];
	printf("ENTER THE ID TO BE CREATED: ");
	fgets(id,10,stdin);

	printf("THE ID TO BE CREATED IS: %s\n",id);
	
	int idValue = atoi(id);

	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);
	userStruct.id = idValue;


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,CREATE, &userStruct);

	ioctl(fd,10,&userStruct);

}

void destroy(int fd){
	
	char id[10];
	printf("ENTER THE ID TO BE DESTROYED: ");
	fgets(id,10,stdin);

	printf("THE ID TO BE DESTORYED IS: %s\n",id);
	
	int idValue = atoi(id);
	

	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);
	userStruct.id = idValue;



	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,DESTROY, &userStruct);
	ioctl(fd,10,&userStruct);
}

void decrypt(int fd){
	
	char id[10];
	printf("ENTER THE ID TO BE DECRYPT: ");
	fgets(id,10,stdin);

	printf("THIS IS THE ID TO BE DECRYPT: %s\n",id);
	
	int idValue = atoi(id);

	char key[10];
	printf("ENTER THE KEY: ");
	fgets(key,10,stdin);

	printf("THIS IS THE KEY: %s\n",id);
	
	

	//creating the user struct
	argStruct userStruct;	
	char hello[10] = {'h','e','l','l','o','\n'};
	strcpy( userStruct.messageBuffer, hello);
	userStruct.flag = 0;
        userStruct.id = idValue;
	strcpy(userStruct.keyBuffer,key);


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,DECRYPT, &userStruct);
	ioctl(fd,10,&userStruct);
	


	//remember that ioctl is non blocking aka have to make
	//sure that the lkm is done with is work before you
	//breaks when .flag is 1 aka the lkm has finished writing over
	while(userStruct.flag == 0);

	
	//printf("New message buffer: %s\n", userStruct.messageBuffer);

}

void encrypt(int fd){
	
	char id[10];
	printf("ENTER THE ID TO BE ENCRYPT: ");
	fgets(id,10,stdin);

	printf("THIS IS THE ID TO BE ENCRYPT: %s\n",id);
	
	int idValue = atoi(id);

	char key[10];
	printf("ENTER THE KEY: ");
	fgets(key,10,stdin);

	printf("THIS IS THE KEY: %s\n",id);
	
	char message[10];
	printf("ENTER THE MESSAGE: ");
	fgets(message,10,stdin);

	printf("THIS IS THE MESSAGE: %s\n",id);
	

	//creating the user struct
	argStruct userStruct;	
	
	strcpy(userStruct.keyBuffer,key);
	strcpy(userStruct.messageBuffer,message);
	userStruct.id = idValue;

	


	//calling the apporiate lkm function and passing the user struct
	ioctl(fd,69, &userStruct);
	ioctl(fd,10,&userStruct);
}


void getUserCommands(int fd){
	
	char command[10];
	char message[10];
	char key[10];		

	
	while(1){

		printf("CREATE:0 DESTROY:1 ENCRYPT:2 DECRPYT:3\n");
		printf("Enter your command: ");
		fgets(command,10,stdin);
		if(command[0] == '0'){
			printf("ENTERED CREATE\n");
			create(fd);	}
			
		else if(command[0] == '1'){
			printf("ENTERED DESTROY\n");
			destroy(fd);	}

		else if(command[0] == '2'){
			printf("ENTERED ENCRYPT\n");
			encrypt(fd);	}

		else if(command[0] == '3'){
			printf("ENTERED DECRYPT\n");
			decrypt(fd);	}
			
		}




}



main ( ) {

 int fd;

        fd = open("/dev/temp", O_RDWR);

        if (fd == -1)
        {
                printf("Error in opening file \n");
                exit(-1);
        }




      
 	getUserCommands(fd);

 

         close(fd);
}

