//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/isfile.h"

#define ISFILE_LOCK_FILE "./isfile_lock_tmp"

int main(){
    printf("version = [%s]\n", isfile_version());
    pid_t pid = fork();
    if( pid == 0){
        // 子进程
        printf("子进程: isfile_lock success = [%d]\n", isfile_lock(ISFILE_LOCK_FILE));
        printf("子进程: isfile_lock success again = [%d]\n", isfile_lock(ISFILE_LOCK_FILE));
        sleep(2);
        printf("子进程: 退出\n");
        return 0;
    }
    // 父进程再获取则失败
    sleep(1);
    printf("父进程: isfile_lock fail = [%d]\n", isfile_lock(ISFILE_LOCK_FILE));
    sleep(2);
    printf("父进程: isfile_lock success = [%d]\n", isfile_lock(ISFILE_LOCK_FILE));

    return 0;
}