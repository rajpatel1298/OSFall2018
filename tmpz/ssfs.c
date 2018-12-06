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
#define PORT 8080 


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
    int sock = get_sock();
    int valread;

    char* pathz = (char*)malloc( sizeof(char) * 100);

    char* functionNum = "00";
    memcpy(pathz,functionNum, strlen(functionNum) );
    strcat(pathz,path);
  
   


    printf("pathz for get_attr: %s\n",pathz);  
 
    


    send(sock , pathz , strlen(pathz) , 0 ); 
    printf("path sent\n"); 




    //valread = read( sock , attrStruct, sizeof(attrStruct)); 
    char gidBuff[100];
    char uidBuff[100];
    char mtimeBuff[100];
    char atimeBuff[100];
    char modeBuff[100];
    char nlinkBuff[100];
    char sizeBuff[100];

    valread = read( sock , gidBuff, 100);
    printf("gidBuff: %s\n",gidBuff);
    int gid = atoi(gidBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 
   
    valread = read( sock , uidBuff, 100);
    printf("uidBuff: %s\n",uidBuff);
    int uid = atoi(uidBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , mtimeBuff, 100);
    printf("mtimeBuff: %s\n",mtimeBuff);
    int mtime = atoi(mtimeBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , atimeBuff, 100);
    printf("atimeBuff: %s\n",atimeBuff);
    int atime = atoi(atimeBuff);
    
    send(sock , "0" , strlen("0") , 0 ); 

   

    valread = read( sock , nlinkBuff, 100);
    printf("nlinkBuff: %s\n",nlinkBuff);
    int nlink = atoi(nlinkBuff);

    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , sizeBuff, 100);
    printf("sizeBuff: %s\n",sizeBuff);
    int size = atoi(sizeBuff);
   
    send(sock , "0" , strlen("0") , 0 ); 

    valread = read( sock , modeBuff, 100);
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
    char* functionNum = (char*)malloc( sizeof(char)*3);
    functionNum[0] = '0';
    functionNum[1] = '1';
    functionNum[2] = '\0';

    char buff[1024] = {0}; 
    char* pathz = (char*)malloc( sizeof(char) * 100);

    memcpy(pathz,functionNum, strlen(functionNum) );
    strcat(pathz,path);
  
    printf("pathz for do_readdir: %s\n",pathz); 
     
  
    send(sock , pathz , strlen(pathz) , 0 ); 
  /*
    valread = read( sock , buff, 1024); 
    printf("server sent this for do_readdir: %s\n",buff ); 
    filler(buffer,buff, NULL,0);
*/

   while(1){

    char Buff[100] = {0};

    valread = read( sock , Buff, 100);
    printf("Buff: %s\n",Buff);
    
    if(Buff[0] == '1'){break;}
    filler(buffer,Buff,NULL,0);
    
    send(sock , "0" , strlen("0") , 0 ); 

         }


	return 0;
}



static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir
    //.read		= do_read,
};

int main( int argc, char *argv[] )
{

struct sockaddr_in address; 
    
    
    

	return fuse_main( argc, argv, &operations, NULL );
}







