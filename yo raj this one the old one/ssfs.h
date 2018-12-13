#ifndef SSFS_H_
#define SSFS_H_


typedef struct openArgument {
    const char * path;
    int flags;
} openArg;

typedef struct closeArgument {
    int fd;
} closeArg;

typedef struct readWriteArgument {
    int fd;
    void * buf;
    size_t cnt;
    int flag; //0 = read, 1 = write
} readArg;


#endif
