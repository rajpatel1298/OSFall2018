#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


int main(int argc, char ** argv){
	
	//test open, take in path arg that has the path to open
    
    if(argc < 2){
        printf("Not enough arguments\n");
        exit(1);
    }

    char * path = argv[1];

    int fd;
    fd = open(path, O_RDWR);

    printf("Opened file at %s with fd %d\n", path, fd);

    if(fd != -1){
        close(fd);
    }
    else {
        printf("Failed to open file at %s\n", path);
    }
	

	return 0;
}
