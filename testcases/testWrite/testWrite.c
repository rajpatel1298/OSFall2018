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
	
	if(argc < 3){
		printf("Error. Not enough arguements\n");
		exit(1);
	}
	if(strlen(argv[2]) > 50){
		printf("Error. String length of buffer must be less than or equal to 50.");
	}

	int fd = open(argv[1], O_WRONLY);
    if(fd == -1){
    	printf("Error. Could not open %s\n", argv[1]);
		//printf("%s\n", strerror(errno));
	}
    printf("Successfully opened %s with fd: %d.\n", argv[1], fd);
    
    size_t bytes = 50;
    
    if(write(fd, argv[2], bytes) < 0){
    	printf("Error. Could not write from %s\n", argv[1]);
		//printf("%s\n", strerror(errno));
	}
	printf("Successfully write\n");
    close(fd);
    
	return 0;
}
