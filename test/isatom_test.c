//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/isatom.h"

#include <pthread.h>
long var = 0;
long step = 1;
long wait_flag = 0;

void * thread_main(void * arg);

int main(){
    pthread_t threads[20];

    printf("version = [%s]\n", isatom_version());

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
    tt = (pthread_t *) arg;
    while( wait_flag == 0){
        ;
    }
//    isatom_incr(var, step);
    var = var + step;
    printf("tid[%ld], var = [%ld] \n", (long)*tt, var);
    return 0;
}
