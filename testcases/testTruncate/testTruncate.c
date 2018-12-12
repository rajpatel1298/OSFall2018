#define _POSIX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <errno.h>
#include<string.h>

int main(int argc, char ** argv){
	
	//argv[1] = path
	//argv[2] = length
	
	if(argc < 3){
		printf("Error. Not enough arguements\n");
		exit(1);
	}
	
	int length = atoi(argv[2]);
	
    if(truncate(argv[1], length) < 0){
    	printf("Error. Could not truncate path %s\n", argv[1]);
		//printf("%s\n", strerror(errno));
	}
	printf("Successfully truncate\n");
    
	return 0;
}
