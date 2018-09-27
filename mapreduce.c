#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

const char * app; //wordcount or sort
const char * impl; //procs or threads
int num_maps; //number of maps
int num_reduces; //number of reduces
const char * infile; //name of input file
const char * outfile; //name of output file


int main(int argc, char ** argv){
    
    if(argc < 13){
        printf("Error: not enough arguments\n");
        exit(1);
    }
    app = argv[2];
    
    //basic error check
    if( (strcmp(app,"wordcount") != 0) && (strcmp(app,"sort") != 0) ){
        printf("Error: Invalid app name\n");
        exit(1);
    }
    impl = argv[4];

    //basic error check
    if( (strcmp(impl,"procs") != 0) && (strcmp(impl,"threads") != 0) ){
        printf("Error: Invalid argument\n");
        exit(1);
    }
    num_maps = atoi(argv[6]);
    num_reduces = atoi(argv[8]);
    infile = argv[10];
    outfile = argv[12];

    int fd = open(infile, O_RDONLY);
    if(fd == -1){
        printf("Error: %d\n", errno);
        exit(1);
    }

    char fileBuf[1000000];
    while( read(fd,fileBuf,100) != 0){
       
    }
    printf("%s\n", fileBuf);

    close(fd);
    return 0;
}
