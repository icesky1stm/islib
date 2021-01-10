//
// Created by suitm on 2021/1/2.
//

#include "isfile.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include<sys/file.h>

char * isfile_version(){
    return ISFILE_VERSION_NO;
}

/** 这是一个文件锁的函数，一般用来检测进程是否存在
 *  例如 服务进程启动后，锁定一个文件，其他文件在发送时可以提前校验服务是否存在 **/
int isfile_lock( char * filename){
    int fd;
    struct  flock lock;
    fd = open(filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    if( fd < 0){
        return -5;
    }

    lock.l_type=F_WRLCK;
    lock.l_start=0;
    lock.l_whence=SEEK_SET;
    lock.l_len=0;

    if( fcntl( fd, F_SETLK, &lock)){
        // 文件锁已存在
        return 0;
    }else{
        /* 文件锁不存在，创建成功，返回fd */
        return fd;
    }
}
