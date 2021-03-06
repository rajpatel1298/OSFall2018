#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <errno.h>
#define PORT 8863 

char *my_itoa(int num, char *str)
{
        if(str == NULL)
        {
                return NULL;
        }
        sprintf(str, "%d", num);
        return str;
}

int get_sock(){

   int sock;
   struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

 return sock;
}
static int do_getattr( const char *path, struct stat *st )
{
/*
   if(path[1] == 'l'){
    printf("entered the forbinn dir returning -ENOENT\n");
    return -ENOENT;    }
*/

    int sock = get_sock();
    int valread;
    printf("the OG path for getattr: %s\n",path);

   // char* pathz = (char*)malloc( sizeof(char) * 25);
    char pathz[25] = {0};
    char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '0';
    pathz[1] = '0';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){pathz[i] = '\0';
       break;}
}


   


    printf("pathz for get_attr: %s\n",pathz);  
 
    


    send(sock , pathz , strlen(pathz) , 0 ); 
    printf("path sent\n"); 

    char checkBuff[25] = {0};
    valread = read(sock,checkBuff,25);
    int check = atoi(checkBuff);

    if (check != 0){//
       printf("bad check\n");
       return (check* -1);    }


    //valread = read( sock , attrStruct, sizeof(attrStruct)); 
    char gidBuff[25] = {0};
    char uidBuff[25] = {0};
    char mtimeBuff[25] = {0};
    char atimeBuff[25] = {0};
    char modeBuff[25] = {0};
    char nlinkBuff[25] = {0};
    char sizeBuff[25] = {0};

    valread = read( sock , gidBuff, 25);
    printf("gidBuff: %s\n",gidBuff);
    int gid = atoi(gidBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

   
    valread = read( sock , uidBuff, 25);
    printf("uidBuff: %s\n",uidBuff);
    int uid = atoi(uidBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , mtimeBuff, 25);
    printf("mtimeBuff: %s\n",mtimeBuff);
    int mtime = atoi(mtimeBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , atimeBuff, 25);
    printf("atimeBuff: %s\n",atimeBuff);
    int atime = atoi(atimeBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

   

    valread = read( sock , nlinkBuff, 25);
    printf("nlinkBuff: %s\n",nlinkBuff);
    int nlink = atoi(nlinkBuff);

    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , sizeBuff, 25);
    printf("sizeBuff: %s\n",sizeBuff);
    int size = atoi(sizeBuff);
   
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , modeBuff, 25);
    printf("modeBuff: %s\n",modeBuff);
    int mode = atoi(modeBuff);
    
  
    printf("finished reading valread: %d\n",valread);

    st->st_gid = (gid_t)gid;
    st->st_uid = (uid_t)uid;
    st->st_mtime = (time_t)mtime;
    st->st_atime = (time_t)atime;
    st->st_mode = (mode_t)mode;
    st->st_nlink = (nlink_t)nlink;
   
    if(size != -1  ){
         st->st_size = (off_t)size;
                }

     printf("finsihed get_attr\n");

	return 0;
 

}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
   printf("entering do_readdir the og path: %s\n",path);
   int valread;
   int sock = get_sock();

  
   // char* functionNum = "01";
  //  char* functionNum = (char*)malloc( sizeof(char)*3);
    //functionNum[0] = '0';
    //functionNum[1] = '1';
    //functionNum[2] = '\0';

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '0';
    pathz[1] = '1';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}

  
    printf("pathz for do_readdir: %s\n",pathz); 
     
  
    send(sock , pathz , strlen(pathz) , 0 ); 
  /*
    valread = read( sock , buff, 1024); 
    printf("server sent this for do_readdir: %s\n",buff ); 
    filler(buffer,buff, NULL,0);
*/

   while(1){

    char Buff[25] = {0};

    valread = read( sock , Buff, 25);
    printf("Buff: %s\n",Buff);
    
    if(Buff[0] == '1'){break;}
    filler(buffer,Buff,NULL,0);
    
    send(sock , "0" , strlen("0") , 0 ); 

         }


	return 0;
}

static int do_mkdir(const char * path , mode_t mode){


    printf("entering do_mkdir the og path: %s\n",path);
   int valread;
   int sock = get_sock();

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '1';
    pathz[1] = '1';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}

  
    printf("pathz for do_mkdir: %s\n",pathz); 
    send(sock , pathz , strlen(pathz) , 0 ); 


   char dummyBuff[25] = {0};

   valread = read( sock , dummyBuff, 25);
  
   char modeBuff[25] = {0};

   my_itoa((int)mode,modeBuff);
   printf("modeBuff: %s\n",modeBuff);
   send(sock,modeBuff,25,0);
  

   char retBuff[25] = {0};
   valread = read( sock , retBuff, 25);

	return 0;


}
static int do_releasedir(const char * path ,  struct fuse_file_info * st){


    printf("entering do_releasedir the og path: %s\n",path);
   int valread;
   int sock = get_sock();

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '0';
    pathz[1] = '2';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}

  
    printf("pathz for do_release: %s\n",pathz); 
    send(sock , pathz , strlen(pathz) , 0 ); 


    char retBuff[25] = {0};
    valread = read( sock , retBuff, 25);
    int ret = atoi(retBuff);
	//return 0;
    printf("ret values for real_dir ret: %d\n",ret);
    if (ret == 0){
        return 0;
        }
     else{return (ret*-1); }

  return ret;

}

static int do_opendir(const char * path ,  struct fuse_file_info * st){


   printf("entering do_opendir the og path: %s\n",path);
   int valread;
   int sock = get_sock();

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '1';
    pathz[1] = '2';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}

  
    printf("pathz for do_opendir: %s\n",pathz); 
    send(sock , pathz , strlen(pathz) , 0 ); 


    char retBuff[25] = {0};
    valread = read( sock , retBuff, 25);
    int ret = atoi(retBuff);
    printf("ret: %d\n",ret);

    if(ret == 0){
       return 0; }

    else{
      return (ret*-1);  }
	//return 0;
  //return ret;

}
static int do_open(const char * path, struct fuse_file_info * fp){
  
    
    printf("entering do_open the og path: %s\n",path);
   int valread;
   int sock = get_sock();

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '2';
    pathz[1] = '2';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}


    printf("pathz for do_open: %s\n",pathz); 
    send(sock , pathz , strlen(pathz) , 0 ); 
 int valread2;
    char dummyBuff[25] = {0};
    valread2 = read(sock,dummyBuff,25);
   

   
     char flagStr[25] = {0};
     //send the flags 
     my_itoa(fp->flags, flagStr);
     send(sock,flagStr, strlen(flagStr), 0);
        
     printf("Sent an open command with path %s and flags %d\n", path, fp->flags); 

     //return the file handle
     char buff2[20] = {0};
     valread2 = read(sock, buff2, 20);
     printf("Returned fd: %s\n",buff2);
     
     
     if( (atoi(buff2)) != -1) {
        return 0;
     }
     else {
         printf("else\n");
         char receiveErr[20] = {0};
         valread2 = read(sock,receiveErr, 20);
         printf("Received errno: %d\n", atoi(receiveErr));
         return -atoi(receiveErr);
     }
}


static int do_create(const char * path, mode_t mode, struct fuse_file_info * fp){

      printf("entering do_create the og path: %s\n",path);
   int valread2;
   int sock = get_sock();

    char buff[25] = {0}; 
    //char* pathz = (char*)malloc( sizeof(char) * 25);

    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
     char pathz[25] = {0};
    //char* functionNum = "00";
    //memcpy(pathz,functionNum, strlen(functionNum) );
   // strcat(pathz,path);
    int i = 2;
    int j =  0;
    pathz[0] = '0';
    pathz[1] = '3';

    while(1){
      pathz[i] = path[j];
     
      i ++;
      j++;
      if(path[j] == '\0'){
       pathz[i] = '\0';
       break;}
}


    printf("pathz for do_create: %s\n",pathz); 
    send(sock , pathz , strlen(pathz) , 0 );


    //need to send mode
    printf("Client is sending this mode: %d\n", mode);    
   char buff2[25] = {0};
     valread2 = read(sock, buff2, 25);
     printf("Returned mess from server: %s\n",buff2);
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .mkdir      = do_mkdir,
    .opendir    = do_opendir,
    .releasedir = do_releasedir,
    .open       = do_open,
    .create     = do_create
    
    //.read		= do_read,
};

int main( int argc, char *argv[] )
{

struct sockaddr_in address; 
    
    
    

	return fuse_main( argc, argv, &operations, NULL );
}







