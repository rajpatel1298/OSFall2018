#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>
#include<netdb.h>


//serverSNFS, accepts parameters in this format: serverSNFS -port 1245 -mount /tmp/path
int main(int argc, char ** argv){

    if(argc < 5){
        printf("Too few arguments...\n");
        exit(1);
    }

    //read in command line arguments for server
    int  PORT_NUM = atoi(argv[2]);
    char * PATH = argv[4];
    int sockDesc, clientSock,* newSock;
    struct sockaddr_in server, client;
    int on = 1;

    //get socket descriptor
    if ((sockDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Server socket error...\n");
        exit(1);
    }

    printf("Server socket created...\n");

    //set socket descriptor for reuse
    if( (setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on))) < 0) {
        printf("Set socket options error");
        close(sockDesc);
        exit(1);
    }

    printf("Server socket set for resuse...\n");

    //init sockaddr_in struct, bind to address
    memset(&server, 0x00, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT_NUM);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("Listening at %d\n",(PORT_NUM));

    //bind 
    if( bind(sockDesc, (struct sockaddr *) &server, sizeof(server)) < 0){
        printf("Bind error...\n");
        close(sockDesc);
        exit(1);
    }
    printf("Server binded successfully\n");

    //listen
    if(listen(sockDesc, 5) < 0) {
        printf("Server listen error...\n");
    }

    printf("Waiting for client connections...\n");

    int size = sizeof(struct sockaddr_in);
    if( (clientSock = accept(sockDesc, (struct sockddr *)&client, &size) ) < 0){
        printf("Server accept error...\n");
        close(sockDesc);
        exit(1);
    }

    printf("Connection accepted!");
    sleep(10);

    close(clientSock);
    close(sockDesc);
}






















