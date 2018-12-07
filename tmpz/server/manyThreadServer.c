#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <fuse.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
 

char *my_itoa(int num, char *str)
{
        if(str == NULL)
        {
                return NULL;
        }
        sprintf(str, "%d", num);
        return str;
}

int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
    {
     return 1;
    }

    return -1;
}
//the thread function
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8863 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 5);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}

char* path = "./mount";

void get_attr(char* buffer,int sock){


    int  sta;
   struct stat bu;
   sta = stat(buffer, &bu);
   if(sta != 0 ) {
      char failBuff[25] = {0};
      my_itoa(-1,failBuff);
      printf("stat could not find file/dir in get_attr\n");
      send(sock,failBuff,25,0);
                 }

    else{  
      char goodBuff[25] = {0};
      my_itoa(1,goodBuff);
  
      send(sock,goodBuff,25,0);  }
   
 
   
   char gidBuff[25];
   char uidBuff[25];
   char mtimeBuff[25];
   char atimeBuff[25];
   char modeBuff[25];
   char nlinkBuff[25];
   char sizeBuff[25];
   int valread;

   uid_t uid = getuid();
   gid_t gid = getgid();
   mode_t mode;
   time_t atime = time( NULL );
   time_t mtime = time( NULL );
   off_t size;
   nlink_t nlink;

     //send struct to user
     printf("new path: %s\n", buffer); 

	//uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	//gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	//atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	//mtime = time( NULL ); // The last "m"odification of the file/directory is right now
  	


        if ( isFile(buffer) == 0 ){
                printf("IS A DIR\n");
                //mode = S_IFDIR | 0755;
                struct stat buf;
                int  status;

                status = stat(buffer, &buf);
                if(status == 0) {
                   mode = buf.st_mode;
                   nlink = buf.st_nlink;
                   uid = buf.st_uid;
                   gid = buf.st_gid;
                   atime = buf.st_atim.tv_sec;
                   mtime = buf.st_mtim.tv_sec;
                 
                      }
   
	        //nlink = 2; 
                size = -1;

        }

	else
	{       printf("IS A FILE\n");
		//mode = S_IFREG | 0644;
		//nlink = 1;
                struct stat buf;
                int  status;

                status = stat(buffer, &buf);
                if(status == 0) {
             // size of file is in member buffer.st_size;
                  printf("stat() worked");
                  size = buf.st_size;
                  
                      }
                //need this else for those trash can things
                 else{
                    size = 0;
                    //mode = buf.st_mode;
                     }
                  mode = buf.st_mode;
                  nlink = buf.st_nlink;
                  uid = buf.st_uid;
                  gid = buf.st_gid;
                  atime = buf.st_atim.tv_sec;
                  mtime = buf.st_mtim.tv_sec;
	}

    
    char dummyBuff[25] = {0};

    my_itoa((int)gid,gidBuff);
    printf("gidBuff: %s\n",gidBuff);
    send(sock,gidBuff,25,0);
    valread = read( sock , dummyBuff, 10);

    my_itoa((int)uid,uidBuff);
    printf("uidBuff: %s\n",uidBuff);
    send(sock,uidBuff,25,0);
    valread = read( sock , dummyBuff, 10);
    
    my_itoa((int)mtime,mtimeBuff);
    printf("mtimeBuff: %s\n",mtimeBuff);
    send(sock,mtimeBuff,25,0);
    valread = read( sock , dummyBuff, 10);

    my_itoa((int)atime,atimeBuff);
    printf("atimeBuff: %s\n",atimeBuff);
    send(sock,atimeBuff,25,0);
    valread = read( sock , dummyBuff, 10);
    

    my_itoa((int)nlink,nlinkBuff);
    printf("nlinkBuff: %s\n",nlinkBuff);
    send(sock,nlinkBuff,25,0);
    valread = read( sock , dummyBuff, 10);


   

    my_itoa((int)size,sizeBuff);
    printf("sizeBuff: %s\n",sizeBuff);
    send(sock,sizeBuff,25,0);
    valread = read( sock , dummyBuff, 10);



    my_itoa((int)mode,modeBuff);
    printf("modeBuff: %s\n",modeBuff);
    send(sock,modeBuff,25,0);
    //valread = read( sock , dummyBuff, 10);

	
    //send(sock , (void*)attrStruct, sizeof(attrStruct) , 0 );
  printf("\n\n");
}

void get_dir(char* buffer, int sock){
     int valread;
     //char * pathz// = (char*)malloc(sizeof(char) * 100 );
     char pathz[25] = {0};
     int i = 0;
     while(1){
         pathz[i] = path[i];
         i++;
         if ( path[i] == '\0'){break;}
              }
     int j = 0;

     while(1){
         pathz[i] = buffer[j];
         i++;
         j++;
         if(buffer[j] == '\0'){
            pathz[i] = '\0';
            break;}
             }
     

     printf("in the server get_dir\n");
     printf("pathz: %s \n",pathz);

    char dummyBuff[25] = {0};
    DIR * dirp;
    struct dirent * dp;

    dirp = opendir(pathz);  

    errno = 0;
    
    while ( (dp = readdir(dirp)) != NULL ) { 

        char fileName[25] = {0};

        if (dp->d_type == DT_REG) { //currently reading a file, so index it
                strcpy(fileName, dp->d_name);
                send(sock,fileName, 25,0 );  
                valread = read( sock , dummyBuff, 10);            
                
        } else if (dp->d_type == DT_DIR) {
                strcpy(fileName, dp->d_name);
                send(sock,fileName, 25,0 ); 
                //send(sock,dp->d_name, strlen(dp->d_name),0 );
                valread = read( sock , dummyBuff, 10);
            } 
                                            }
     

     //char * test = "file1";
     //send(sock, test,strlen(test), 0);
     char dummy[25] = {0};
     strcpy(dummy,"1");
     //send(sock,"1", strlen("1"),0 );
     send(sock,dummy, 25,0 );
 
     printf("\n\n");
}

void mk_dir(char* buffer, int sock){
     int valread;
     //char * pathz// = (char*)malloc(sizeof(char) * 100 );
     char pathz[25] = {0};
     int i = 0;
     while(1){
         pathz[i] = path[i];
         i++;
         if ( path[i] == '\0'){break;}
              }
     int j = 0;

     while(1){
         pathz[i] = buffer[j];
         i++;
         j++;
         if(buffer[j] == '\0'){
            pathz[i] = '\0';
            break;}
             }
     

     printf("in the server mk_dir\n");
     printf("pathz: %s \n",pathz);

     char dummyBuffer[25] = {0};
     dummyBuffer[0] = '1';
     send(sock,dummyBuffer,25,0);


     int mode = 0;
     char modeBuff[25] = {0};
     valread = read( sock , modeBuff, 25);
     printf("modeBuff string: %s\n",modeBuff);
     mode = atoi(modeBuff);
     printf("mode int: %d\n",mode);

     int ret = mkdir(pathz, (mode_t)mode);
     
    
     char retBuff[25] = {0};
     my_itoa(ret,retBuff);
     printf("retBuff: %s\n",retBuff);
     send(sock,retBuff,25,0);
     
}


/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    //char *message , client_message[2000];
    char* message;
    char client_message[25] = {0};
    

   // int valread = read( sock,client_message, 1024); 

    int valread = read( sock,client_message, 25); 

    char* pathz = (char*)malloc( sizeof(char) * 25);
    memcpy(pathz,path, strlen(path) );
     
    strcat(pathz,client_message + 2);
          

    

     ////client_message[5] = '\0';
     //printf("client_message: %s\n",client_message);
    if(  (client_message[0] == '0') && (client_message[1] == '0') ){
         get_attr(pathz, sock);
          }

    if(  (client_message[0] == '0') && (client_message[1] == '1') ){
         printf("message before cat for get_dir: %s\n",client_message);
         get_dir(client_message + 2, sock);
          }
    
     
    if(  (client_message[0] == '1') && (client_message[1] == '1') ){
         printf("message before cat for mk_dir: %s\n",client_message);
         mk_dir(client_message + 2, sock);
          }

     //send(sock , "yo from server", 14 , 0 ); 
    

    


    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
} 






