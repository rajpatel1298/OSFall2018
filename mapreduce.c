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

    FILE *f = fopen(infile, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char * buffer = malloc(fsize+1); //error check
    fread(buffer,fsize,1,f); //error check
    fclose(f);

    buffer[fsize] = 0;
    int * delimArr = (int*)malloc(sizeof(int) * (num_maps +1));
    
    //loop through buffer and splits into num_maps -1, then the last thread will handle the rest
    // if nummaps == 1 no need for delim
    
    int position;
    int delimPos = 1;
    delimArr[0] = -1;
    
    if(num_maps == 1){
    	position = fsize - 1; // -1 because the size is 1 greater than the position
    	delimArr[delimPos] = position;
    } else { //if num_maps(# of threads) > 1
    	int size = fsize/num_maps; // gets the ideal sizes of each thread
    	int i;
    	for(i = 0; i < num_maps; i++){
    		
    		if(i == num_maps - 1){ //this is created for the last iteration of the loop
    			delimArr[delimPos] = fsize - 1;
    			break;
    		}
    		
    		position = ((i+1) * size) - 1; //gets the position at the (i+1)th thread    	

    		while(buffer[position] != '\n'){
    			position--;
    		}
//    		printf("At position %d is this char %c\n", position, buffer[position]);
    		delimArr[delimPos] = position;
    		delimPos++;
    		
    		
    	}
    	
    
    }

    printf("%s\n", buffer);
	int i;
	printf("Delim Arr:\n");
	for(i = 0; i < num_maps +1; i++){
		printf("%d\n",delimArr[i]);
	}
	printf("%ld\n", fsize -1);
    return 0;
}
