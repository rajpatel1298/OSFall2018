


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


int main(int argc, char ** argv){

    int fd;

    fd = open("/tmp/yo/file1", O_RDWR);
    printf("Opened file with fd %d\n", fd);

    if(fd != -1){
        close(fd);
    }
    return 0;
}
