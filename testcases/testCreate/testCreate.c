#define _POSIX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


int main(int argc, char ** argv){

//test create

 //takes in mount path + name of file to create
	char * path = argv[1];
	

	if(argc < 2){
		printf("Not enough arguments");
		exit(-1);
	}

	int fd;

	fd = creat(path, S_IRWXU);
	printf("Created file at %s with fd %d\n",path,fd);

	if(fd != -1){
		close(fd);
	}
	else {
		printf("Failed to create file with path %s\n", path);
	}
	return 0;
}
