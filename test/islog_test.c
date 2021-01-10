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
        pthread_create(&threads[i], NULL, &thread_main, &threads[i]);
    }

    usleep(100000);
    wait_flag = 1;
//    pthread_join(threads[19], NULL);
    usleep(100000);

    return 0;
}
void * thread_main(void * arg){
    pthread_t *tt;
    char isodate[20] = "LOGBUFFFFFFFF";

    tt = (pthread_t *) arg;
    while( wait_flag == 0){
        ;
    }
    islog_debug("%s", isodate);
    islog_info("%s", isodate);
    islog_error("%s", isodate);
    islog_warn_t("TAG1", "%s", isodate);
    islog_warn_t("TAG2", "%s", isodate);
    islog_warn_t("TAG3", "%s", isodate);
    islog_warn("%s", isodate);
    return 0;
}