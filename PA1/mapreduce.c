#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define keySize 20

const char * app; //wordcount or sort
const char * impl; //procs or threads
int num_maps; //number of maps
int num_reduces; //number of reduces
const char * infile; //name of input file
char * outfile; //name of output file
int maxBytes = 20;


pthread_barrier_t map_barrier; //barrier used to wait for map threads to finish

//this marks the size of each index in the shared memory, so each map function has that much memory to play around with/
//helps sync issues since each reduce/map func has its own spot in the shared memory

long INDEX_SIZE;
long SHMEM_SIZE;

int SIZE = 1;

struct threadInput
{ 
   void* shareMem;
   int   threadID;
   char* partialBuffer;
};

//For word part you'd have to make the key the ascii value of that number and make the value the number since
//the reduce organization part sorts by the key

struct pair
{
    //char * key;
	char key[keySize];
	int value;
};
//Prints input array of pairs
//Debug function
void printPairArr( struct pair* pairArr ){
	printf("Printing Arr: \n");
	int i = 0;

	while(1){

		printf("Key  : %s\n",pairArr[i].key);
		printf("Value: %d\n",pairArr[i].value);
		
		printf("\n");
		i ++;

		if(pairArr[i].value == -1){
			break;
			}
		

	}

}

//converts a string that contains an int to a new string that strcmp() can use. numOfBytes tells the func the size of the new string
//For example-> target = "123" and numOfBytes = 5, the result would be-> "00123"
//remember to free the string that you sent into this function since you'll probs not use it again and to free this string
//also note every string you sent this must contain the same numOfBytes arg for strcmp to work
char * convertIntForStrcmp(char* target, int numOfBytes){
    
    char* final = (char*)malloc(numOfBytes * sizeof(char));
    int size = (int)strlen(target);
    int i,j;
                                    //zeroing out the newly created string
    for(i = 0; i < numOfBytes; i++){
        final[i] = '0';
    }
                                    //copy the arg string into the newly created string starting from right to left
    for(i = size -1 , j = numOfBytes -2 ; i >= 0; i --, j--){
        final[j] = target[i];
    }
    
    final[numOfBytes - 1] = '\0';
    return final;
}

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_ANONYMOUS | MAP_SHARED;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, 0, 0);
}

//will write the input pair array to the location marked by the threadID
//Note the pair array end node must be an end node aka the key must be -1 to mark the end of the array
//example-> pairArr = [ (1,"hello"), (2,"whats good"), (-1,"end") ]
void writeToSharedMem(void* shareMem, struct pair * pairArr, int threadID){

	int x = threadID * INDEX_SIZE;			//finding the index to write to
	int y = 0;
	int i = 0;

	while(1){
		
		memcpy(shareMem + y + x, &(pairArr[i]), sizeof(struct pair));
		
		y+= sizeof(struct pair);	//incr the next position in the pair array store in share memory

		if(pairArr[i].value == -1)	//marks the end of the pair array
		{
			break;
		}

		i++;
		}	

}
//will print all pairs located in the index marked by the threadID
//Debug function
void printContentsShareMem(int threadID, void* shareMem){
	
	int x = threadID * INDEX_SIZE;
	int y = 0;


	printf("\nContents of thread: %d\n",threadID);
	while(1){
		struct pair* pairArr = (struct pair*)(shareMem + x + y); //have to incr shareMem first before type casting
									//cant type cast first then treat as array for whatever reason

		if(pairArr->value == -1)	//marks the end of the pair array
		{
			break;
		}
		
		printf("Key  : %s\n", pairArr->key);
		printf("Value: %d\n", pairArr->value);
		printf("\n");
		

		y += sizeof(struct pair);
		
		}

}

//returns a COPY of a pair array located by the threadID
//thus you gonna have to free() this copy after your done with it
struct pair* getCopyOfPairArr( void* shareMem, int threadID){

	
	int x = threadID * INDEX_SIZE;			//finding the index to write to
	int y = 0;
	int size = 0;
	int i = 0;
						//have to first find the size of the array in order to allocate enough space
	while(1){
		size ++;
		struct pair* pairArr = (struct pair*)(shareMem + x + y);
		
		if(pairArr->value == -1){
			break;
				}

		y += sizeof(struct pair);
		
		}

	struct pair* pairArrCopy = (struct pair*)malloc( size * sizeof( struct pair) );
	
	y = 0;
						//once we have enough space, we can then go thru again and copy its contents
	while(1){
		
		struct pair* pairArr = (struct pair*)(shareMem + x + y);
		
		//pairArrCopy[i].key = pairArr->key;
                strcpy(pairArrCopy[i].key, pairArr->key);/////new
		pairArrCopy[i].value = pairArr->value;

		if(pairArr->value == -1){
			break;
				}

		y += sizeof(struct pair);
		i ++;
		
		}

	return pairArrCopy;


}

//just a function to check if your at the cap of the array, if so then extends is by times 2
struct pair * checkArrlist(struct pair* pairArr, int current){	
				//getting size of array
	int size = SIZE;

	if( current >= size ){
					//doubles the size of the array
		struct pair * nwPairArr = (struct pair*)malloc( (size * 2) * sizeof(struct pair));
		memcpy(nwPairArr, pairArr, (size) * sizeof(struct pair) );
		SIZE = size * 2;
	//	free(pairArr);

		return nwPairArr;
			}

	else{
		return pairArr;
	}
	
}


//function divides up the pair array located in the first index of shared memory and writes matching pairs to indiv indexes with the last
//index having the rest of the pairs. Takes in shared memory pointer and the max number of reduce threads
void writeBackOrganize(void* shareMem, int maxThreadID){

						//getting the sorted pair array
	struct pair* organizeArr = getCopyOfPairArr(shareMem, 0);

	int currentThread = 0;
	int currentPos = 1;
	int currentPairArr = 1;
	SIZE = 1;
						//the array that holds matching pairs to be writen to shared memory
	struct pair* pairArr = (struct pair *)malloc( sizeof(struct pair));

						//making the first entry in the org array the first entry in the matching array
	//pairArr[0].key   = organizeArr[0].key;
        strcpy(pairArr[0].key , organizeArr[0].key); //new
	pairArr[0].value = organizeArr[0].value;
	
	while(1){			
						//if were on the last reduce thread then we just have to write the rest of the org array to that
						//threads spot in shared memory
		if(currentThread == (maxThreadID -1)){
			writeToSharedMem(shareMem, organizeArr += (currentPos -1), currentThread );
		//	free(pairArr);
		//	free(organizeArr);
			return;
						}
						//if were at the limit of the org array thus the matching pair array is written to shared memory
		if(organizeArr[currentPos].value == -1){
			pairArr = checkArrlist(pairArr, currentPos);
			//pairArr[currentPairArr].key   = organizeArr[currentPos].key;
                         strcpy( pairArr[currentPairArr].key, organizeArr[currentPos].key);   ///new

			pairArr[currentPairArr].value = organizeArr[currentPos].value;

			writeToSharedMem(shareMem, pairArr, currentThread );
		//	free(pairArr);
		//	free(organizeArr);

					     //since were at the limit of shared memory we have to write the def end node to the remaining reduces
			int i;
			for(i = currentThread + 1; i < maxThreadID; i += 1){
				pairArr = (struct pair*)malloc(sizeof(struct pair));
				pairArr[0].value = -1;
				pairArr[0].key[0] = 'e';    //new
                                pairArr[0].key[1] = 'd';
                                pairArr[0].key[2] = '\0';

				writeToSharedMem(shareMem, pairArr, i);
		//		free(pairArr);
					}	
			return;  
			}
					     //the next node on the org array matching the matching pair thus we add it to that array
		if( strcmp(pairArr[0].key, organizeArr[currentPos].key )== 0){
			pairArr = checkArrlist(pairArr, currentPairArr);
			//pairArr[currentPairArr].key   = organizeArr[currentPos].key;
                         strcpy( pairArr[currentPairArr].key, organizeArr[currentPos].key); //new
			pairArr[currentPairArr].value = organizeArr[currentPos].value;
			currentPairArr += 1;
			}

					//if the next pair doesnt match so we write the matching pair array to its location and start anew
		else{ 
			pairArr = checkArrlist(pairArr, currentPos);
			pairArr[currentPairArr].value   = -1;
			//pairArr[currentPairArr].key = "end";
                        //pairArr[currentPairArr].key[0] = 'e';    //new
                       // pairArr[currentPairArr].key[1] = 'd';
                       // pairArr[currentPairArr].key[2] = '\0';
                         strcpy( pairArr[currentPairArr].key, "end"); 
			writeToSharedMem(shareMem, pairArr, currentThread );
			

			currentThread += 1;
			currentPairArr = 1;
			//pairArr[0].key   = organizeArr[currentPos].key;
                        strcpy( pairArr[0].key, organizeArr[currentPos].key);   //new

			pairArr[0].value = organizeArr[currentPos].value;
		}
		currentPos += 1;
	}
}

//will merge all of the pair arrs located in the shared memory in order and write a new pair arr back to the first index in the shared memory
//max threadID tells the system how many different indexes to merge from
void mergeShareMem(void* shareMem, int maxThreadID){
	
	//char* minKey;
	int i, size,current,minValue;
	size = 1;
	current = 0;
        char minKey[keySize];
	struct pair* mergePairArr = (struct pair*)malloc( size * sizeof(struct pair) );
	
		
								//this holds the current position for each thread
	int* positionArr = (int*)malloc( maxThreadID * sizeof(int)); 
	
								//making the starting position for each thread zero
	for(i = 0; i < maxThreadID ; i ++)				
	{
		positionArr[i] = 0;
	}
	while(1)
	{
										//making the def min the first pair in the first thread
	      struct pair* pairArr = (struct pair*)(shareMem + positionArr[0]);
              strcpy( minKey, pairArr->key);   ///new
	      minValue = pairArr->value;
	     // minKey   = pairArr->key;
		
								//have to first find the most min value
		for( i = 0; i < maxThreadID ; i ++)
		{
		
			int x = i * INDEX_SIZE; 				//calcing the index
									//getting the pair at the current ith index
			struct pair* pairArr = (struct pair*)(shareMem + x + positionArr[i]);	
			if(pairArr->value == -1){
				continue;
						}
			
			if( 0 > strcmp(pairArr->key, minKey) ){
				minValue = pairArr->value;
				//minKey   = pairArr->key; 
                                strcpy( minKey, pairArr->key); //new
					}
			if(minValue == -1){			//if the current min is a end node then change it to the current node
				minValue = pairArr->value;
				//minKey   = pairArr->key; 
                                 strcpy( minKey, pairArr->key);
				}
			
		}
	
							//if the mid is the end node, then your done
		if(minValue == -1){
			break;
				}
		
							//Now that we have the mind value we have to go back and see which min values
							//match, and if they match add them to the merge arr and incr the position for that thread
		
		for(i = 0; i < maxThreadID; i ++)
		{
		   int x = i * INDEX_SIZE;
		   struct pair* pairArr = (struct pair*)(shareMem + x + positionArr[i]);
		
		   if(pairArr->value == -1)
			{
				continue;
			}
			
		   if(0 == strcmp(pairArr->key, minKey )  ) //found a match min so add it the merge arr
			{
		        mergePairArr = checkArrlist(mergePairArr, current);
			//mergePairArr[current].key   = pairArr->key;
                        strcpy( mergePairArr[current].key , pairArr->key);  //new
			mergePairArr[current].value = pairArr->value;
			
			current += 1;
								//now have to incr the posistion of that index
			
			positionArr[i] += sizeof(struct pair);
			
			
			}
			
			
		}
							

	}
	
	mergePairArr = checkArrlist(mergePairArr, current);
							//giving the merge arr a end node
	//mergePairArr[current].key  = "end";
        strcpy( mergePairArr[current].key , "end");  //new
	mergePairArr[current].value = -1;
							//writing the new merged arr back to the first index in shared memory 
	writeToSharedMem(shareMem, mergePairArr, 0);
	//free(mergePairArr);
	//free(positionArr);

}

//sorts the array of pairs based on their key values on the index based on the threadID and writes the newly sorted array back to shared memory
void sortPairArr(void* shareMem, int threadID)
{
   
   int  value, j;
   int size = 0;
   int i = 0;
  // char* key;
    char key[keySize];	
   struct pair * pairArr = getCopyOfPairArr(shareMem, threadID);

  					 //getting the size of the pairArr
	while(1){
		size ++;
		if(pairArr[i].value == -1){
			break;
			}
		i ++;

		}

  					//sorting the array using standard insertion sort, not the best but the easiest way to do it
   for (i = 1; i < (size -1); i++)	//(size -1) b/c we dont want to include the end node in sorting
   {
      // key = pairArr[i].key;
      strcpy( key, pairArr[i].key); //new
      value = pairArr[i].value;

       j = i-1;
 
      
       while (j >= 0 && (0 < strcmp(pairArr[j].key, key)) )  
       {
           //pairArr[j+1].key = pairArr[j].key;
             strcpy( pairArr[j+1].key, pairArr[j].key);  //new
	   pairArr[j+1].value = pairArr[j].value;

           j = j-1;
       }
       //pairArr[j+1].key = key;
         strcpy( pairArr[j+1].key, key); //new
       pairArr[j+1].value = value;
   }

					//finished sorting now writing the new array of pairs back to the shared memory
   writeToSharedMem(shareMem, pairArr, threadID );

}


int BiggestNumByte = 10;

void writePairsToFile(char* filename, int numOfReduces, void* shareMem){
    printf("w1\n");
    FILE *fp;
    int i;
    int j;
    fp = fopen( filename, "w" );

     printf("w2\n");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
 printf("w3\n");
    for(i = 0; i < numOfReduces; i++){

    struct pair* pairArr = getCopyOfPairArr( shareMem, i);
     
    j = 0;
  printf("yooo\n");
    while(1){
       
       if(pairArr[j].value == -1){break;}
        
      char* snum = (char*)malloc( BiggestNumByte * sizeof(char));
      sprintf(snum, "%d", pairArr[j].value);    
      fprintf(fp, pairArr[j].key );
      fprintf(fp, " ");
      fprintf(fp, snum );
      
      

      fprintf(fp,"\n");
   //   free(snum);
       j++;
     
        }
   //  free(pairArr);
    } 
   
    
    fclose(fp);
}

int isNotDelim(char c){
	if(c != ' ' && c != '.' && c != ',' && c != ';' && c != ':' && c != '!' && c != '-' && c != '\n' && c != '\r'){
		return 1;
	}
	return 0;
}

void * map_wordcount(void * threadArg){
	struct threadInput * input = (struct threadInput *)threadArg;
	char * buffer = (*input).partialBuffer;
	void * shmem = (*input).shareMem;
	int threadID = (*input).threadID;
	int i;
	int start = 0, end = 0, totalWords = 0, pairPos = 0;
	int keyPos = 0;
	int rule = 0;
	
	for(i = 0; i < strlen(buffer); i++){

        if(isNotDelim(buffer[i]) == 1){

        	rule = 0;
            continue;
        }
        else {
        	if(rule == 1){
        		continue;
        	}
        	rule = 1;        	
            totalWords++;
    
        }
    }
    struct pair* pairArr = (struct pair *)malloc( (totalWords+1) * sizeof(struct pair));
    char * key;
	printf("total words: %d\n", totalWords);

    while(start < strlen(buffer)){
        if(isNotDelim(buffer[start]) == 1){
            start++;
        }
        else {
            key = malloc(start-end+1);
            for(i = end; i < start; i++){ //copy current string into key
                if(buffer[i] <= 90 && buffer[i] >= 65){
               		key[keyPos] = buffer[i] + 32;
               		keyPos++;
            	}else{
               		key[keyPos] = buffer[i];
                	keyPos++;
                }	
              
            }
            keyPos = 0; //reset keyPos for next key
            key[start-end] = '\0';

            //add key and value to pairArr
            pairArr[pairPos].value = 1;
            strcpy(pairArr[pairPos].key, key);
           // pairArr[pairPos].key = key;
            
            pairPos++;
            
            while(isNotDelim(buffer[start]) == 0){
            	start++;
					if(start == strlen(buffer)){
						break;
					}
            }
            end = start;
            start = end;
    	}
    }
    
    if(isNotDelim(buffer[start -1]) == 1){
    	key = malloc(start-end+1);
        for(i = end; i < start; i++){ //copy current string into key
        	if(buffer[i] <= 90 && buffer[i] >= 65){
               		key[keyPos] = buffer[i] + 32;
               		keyPos++;
            }else{
          		key[keyPos] = buffer[i];
            	keyPos++;
            }	
        }
        keyPos = 0; //reset keyPos for next key
        key[start-end] = '\0';

            //add key and value to pairArr
        pairArr[pairPos].value = 1;
        strcpy(pairArr[pairPos].key, key);
     //   pairArr[pairPos].key = key;
            
        pairPos++;
    }
    
    strncpy(pairArr[totalWords].key, "end\0", 4);
  //  pairArr[totalWords].key = "end";
    pairArr[totalWords].value = -1;
    

//    printPairArr(pairArr);
    
    writeToSharedMem(shmem, pairArr,threadID);
    if(strcmp(impl,"threads") == 0){ 
        pthread_barrier_wait(&map_barrier);
    }
    
    return NULL;
}


void * map_sort(void * threadArg){
    struct threadInput * input  = (struct threadInput *)threadArg;
    char * buffer = (*input).partialBuffer;
    void * shmem = (*input).shareMem;
    int threadID = (*input).threadID;
    int i;
    int start = 0, end = 0, totalNumbers = 0, pairPos = 0;
    int keyPos = 0;
    int rule = 0;
    
    for(i = 0; i < strlen(buffer); i++){

        if(buffer[i] >= '0' && buffer[i] <= '9'){
        	rule = 0;
            continue;
        }
        else {
        	if(rule == 1){
        		continue;
        	}
        	rule = 1;        	
            totalNumbers++;
        }
    }
   
    struct pair* pairArr = (struct pair *)malloc( (totalNumbers+1) * sizeof(struct pair));
    char * key;
    while(start < strlen(buffer)){
        if(buffer[start] >= '0' && buffer[start] <= '9'){
            start++;
        }
        else {
            key = malloc(start-end+1);
            for(i = end; i < start; i++){ //copy current string into key
                key[keyPos] = buffer[i];
                keyPos++;
            }
            keyPos = 0; //reset keyPos for next key
            key[start-end] = '\0';
            int value = atoi(key);
            if(strlen(key) > maxBytes){
                maxBytes = strlen(key);
            }
            pairArr[pairPos].value = value;
       //   pairArr[pairPos].key = key;
            memset(pairArr[pairPos].key, '\0', sizeof(pairArr[pairPos].key));
            strncpy(pairArr[pairPos].key, key, start-end+1);

            pairPos++;
				while(buffer[start] < '0' || buffer[start] > '9'){
					start++;
					if(start == strlen(buffer)){
						break;
					}
				}
            end = start;
            start = end;
        }
    }
    //need to save last number somehow
   // pairArr[totalNumbers].key = "end";
    strncpy(pairArr[totalNumbers].key, "end\0", 4);
    pairArr[totalNumbers].value = -1;
    
    
    //update keys so that they work for strcmp
    for(i = 0; i < totalNumbers; i++){
        //char * newKey = convertIntForStrcmp(pairArr[i].key, maxBytes);
       // memset(pairArr[i].key, '\0', sizeof(pairArr[i].key));
       // strncpy(pairArr[i].key, newKey, maxBytes);
		  
     	 int j = 0, k = 0;
		 char final[keySize];
		 int size = (int)strlen(pairArr[i].key);
//		 int i,j;
		                                 //zeroing out the newly created string
		 for(k = 0; k < keySize; k++){
		     final[k] = '0';
		 }
		                                 //copy the arg string into the newly created string starting from right to left
		 for(k = size -1 , j = keySize -2 ; k >= 0; k --, j--){
		     final[j] = pairArr[i].key[k];
		 }
		 
		 final[keySize - 1] = '\0';
		 strcpy(pairArr[i].key, final);
    }
    

  //  printPairArr(pairArr);
    
    writeToSharedMem(shmem, pairArr,threadID);

    if(strcmp(impl, "threads") == 0) {
        pthread_barrier_wait(&map_barrier);
    }

    return NULL;
}

void * reduce_wc(void * threadArg){
    struct threadInput * input  = (struct threadInput *)threadArg;
    void * shmem = (*input).shareMem;
    int threadID = (*input).threadID;
	printf("we in threadID: %d\n", threadID);
    //for the case that the number of maps = the number of reduces
    int i = 0;
    int numDiffWords = 1;

    struct pair * shmemBlock = getCopyOfPairArr(shmem, threadID);

	char * currentKey = shmemBlock[0].key;
	while(1){

		if(strcmp(currentKey, shmemBlock[i].key) == 0){
			i++;

            continue;
        }else{

        	if(shmemBlock[i].value == -1){
        		break;
        	}
        	numDiffWords++;
        	currentKey = shmemBlock[i].key;
        	i++;
        }    
	
	}

    //new pair array with the correct size malloc'd
    struct pair* newPairArr = (struct pair *)malloc( (numDiffWords+1) * sizeof(struct pair));
    int pos1 = 0, pos2 = 0;

    for(i = 0; i < numDiffWords; i++){

        strncpy(newPairArr[pos1].key, shmemBlock[pos2].key, sizeof(shmemBlock[pos2].key));
     //   newPairArr[pos1].key = shmemBlock[pos2].key;
        newPairArr[pos1].value = 0;
        currentKey = shmemBlock[pos2].key;
        while( strcmp(currentKey, shmemBlock[pos2].key) == 0 ){
            newPairArr[pos1].value += 1;
            pos2++;
        }
        printf("key: %s\nvalue:%d\n",newPairArr[pos1].key, newPairArr[pos1].value);
        pos1++;

    }
    //add last pair struct

    strncpy(newPairArr[numDiffWords].key, "end\0", 4);
    newPairArr[numDiffWords].value = -1;
	
    writeToSharedMem(shmem, newPairArr, threadID);
	printf(":gsag\n");
    return NULL;
}


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

    pthread_t map_threads[num_maps];
    pthread_t reduce_threads[num_reduces];

    pid_t map_procs[num_maps];
    pid_t reduce_procs[num_reduces];
   
    int i,j;

    FILE *f = fopen(infile, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    SHMEM_SIZE = fsize * 20 * 20;
    INDEX_SIZE = SHMEM_SIZE / num_maps;

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
    		if(strcmp(app,"sort") == 0){
    			while(buffer[position] != '\n'){
    				position--;
    			}
    		}else{
    			while(buffer[position] != ' ' && buffer[position] != '.' && buffer[position] != ',' && buffer[position] != ';' && buffer[position] != ':' && buffer[position] != '!' && buffer[position] != '-'){
    				position--;
    			}
    		}	
//    		printf("At position %d is this char %c\n", position, buffer[position]);
    		delimArr[delimPos] = position;
    		delimPos++;	
    	}
    }

    void* shmem = create_shared_memory((size_t)SHMEM_SIZE);
    if(strcmp(impl,"threads") == 0){

        int pos;
        struct threadInput * arg;
        char * buffSplit;
        int prev = 0;
        pthread_barrier_init(&map_barrier,NULL,num_maps+1);
        for(i = 0; i < num_maps; i++){ //creating threads 
           arg = malloc(sizeof(struct threadInput));
           (*arg).threadID = i;
           (*arg).shareMem = shmem;
           buffSplit = (char*) malloc(delimArr[i+1]-prev+1+1);
           pos = 0;
           for(j = prev; j <= delimArr[i+1]; j++){
              buffSplit[pos] = buffer[j];
              pos++;
           }
           
           prev = delimArr[i+1]+1;        	
           
           (*arg).partialBuffer = buffSplit;
           if(strcmp(app,"sort") == 0){

              pthread_create(&map_threads[i], NULL, map_sort, (void*)arg);
           }else{

           	  pthread_create(&map_threads[i], NULL, map_wordcount, (void*)arg);
           }  
        }
        
        pthread_barrier_wait(&map_barrier);
        for(i = 0; i < num_maps; i++){
            pthread_join(map_threads[i], NULL);
        }
        pthread_barrier_destroy(&map_barrier);
        for(i = 0; i < num_maps; i++){
            sortPairArr(shmem, i);
        }
        mergeShareMem(shmem,num_maps);
        writeBackOrganize(shmem, num_reduces);
       	
		for(i = 0; i < num_reduces; i++){ //creating threads 
           arg = malloc(sizeof(struct threadInput));
           (*arg).threadID = i;
           (*arg).shareMem = shmem;
           
           if(strcmp(app,"sort") == 0){
    //          pthread_create(&reduce_threads[i], NULL, reduce_sort, (void*)arg);
           }else{
              printf("num reduc: %d\n", num_reduces);
           	  printf("pthread: %d\n",pthread_create(&reduce_threads[i], NULL, reduce_wc, (void*)arg));
           }
           
       	}
  /*      
        for(i = 0; i < num_reduces; i++){
            pthread_join(reduce_threads[i], NULL); 
           
        }   
  */   	
        writePairsToFile(outfile, num_reduces, shmem);
    }
    else { //use processes instead of threads

        int pos;
        struct threadInput * arg;
        char * buffSplit;
        int prev = 0;

        for(i = 0; i < num_maps; i++){ //creating threads 

           arg = malloc(sizeof(struct threadInput));
           (*arg).threadID = i;
           (*arg).shareMem = shmem;
           buffSplit = (char*) malloc(delimArr[i+1]-prev+1+1);
           pos = 0;
           for(j = prev; j <= delimArr[i+1]; j++){
              buffSplit[pos] = buffer[j];
              pos++;
           }
           
           prev = delimArr[i+1]+1;        	
           
           (*arg).partialBuffer = buffSplit;

            if( (map_procs[i] = fork()) < 0){
                perror("fork");
                exit(1);
            }
            else if( (map_procs[i] == 0) ){
                //do work in child
                printf("process werk werk %d, passing in this ID to arg: %d\n", map_procs[i], i);
                if(strcmp(app, "sort") == 0){
                    map_sort((void*)arg);
                }
                else {
                    map_wordcount((void*)arg);
                }
                exit(0);
            }
            else {
                //parent process
            }
       }
       int status;
       pid_t pid;
       int n = num_maps;
       while(n > 0){
            pid = wait(&status);
            printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
            --n;
       }
        
       //SEGFaults on this sortPairArr underneath
        for(i = 0; i < num_maps; i++){
            sortPairArr(shmem, i);
        }
		  
		  
       mergeShareMem(shmem,num_maps);
       writeBackOrganize(shmem, num_reduces);

	   for(i = 0; i < num_maps; i++){
            printContentsShareMem(i,shmem);
       }

       //call reduce processes

        for(i = 0; i < num_reduces; i++){  

            arg = malloc(sizeof(struct threadInput));
            (*arg).threadID = i;
            (*arg).shareMem = shmem;

            if( (reduce_procs[i] = fork()) < 0){
                perror("fork");
                exit(1);
            }
            else if( (reduce_procs[i] == 0) ){
                //do work in child
                printf("process werk werk %d, passing in this ID to arg: %d\n", map_procs[i], i);
                if(strcmp(app, "sort") == 0){
                  //  reduce_sort((void*)arg);
                }
                else {
                    reduce_wc((void*)arg);
                }
                exit(0);
            }
            else {
                //parent process
            }
        }

        //wait for processes to finish

       int r = num_maps;
       while(r > 0){
            pid = wait(&status);
            printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
            --r;
       }

		writePairsToFile(outfile, num_reduces, shmem);
		 
    }
    return 0;
}
