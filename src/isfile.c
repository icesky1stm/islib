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

/** ����һ���ļ����ĺ�����һ�������������Ƿ����
 *  ���� �����������������һ���ļ��������ļ��ڷ���ʱ������ǰУ������Ƿ���� **/
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
        // �ļ����Ѵ���
        return 0;
    }else{
        /* �ļ��������ڣ������ɹ�������fd */
        return fd;
    }
}
