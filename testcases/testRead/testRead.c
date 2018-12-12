#define _POSIX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <errno.h>

int main(int argc, char ** argv){
	
	if(argc < 2){
		printf("Error. Not enough arguements\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDONLY);
    if(fd == -1){
    	printf("Error. Could not open %s\n", argv[1]);
		//printf("%s\n", strerror(errno));
	}
    printf("Successfully opened %s with fd: %d.\n", argv[1], fd);
    
    size_t bytes = 50;
    char buff[50] = {0};
    
    if(read(fd, buff, bytes) < 0){
    	printf("Error. Could not read from %s\n", argv[1]);
		//printf("%s\n", strerror(errno));
	}
	printf("Successfully read into the buffer: \n\t%s\n", buff);
    close(fd);
    
	return 0;
}
