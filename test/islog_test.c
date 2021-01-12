//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../src/islog.h"

long var = 0;
long step = 1;
long wait_flag = 0;

void * thread_main(void * arg);

int main(){
    printf("version = [%s]\n", islog_version());
    pthread_t threads[20];

    int i;
    for( i = 0; i < 10; i++ ){
        pthread_create(&threads[i], NULL, &thread_main, i);
    }

    usleep(100000);
    wait_flag = 1;
    for ( i = 0; i < 10 ; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}
void * thread_main(void * arg){
    pthread_t *tt;
    char isodate[20] = "LOGBUFFFFFFFF";
    int i = (int)arg;

    islog_debug("%s(%d)", isodate, i);
    islog_warn("%s(%d)", isodate, i);
    islog_warn_t("TAG", "%s(%d)", isodate, i);
    return 0;
}