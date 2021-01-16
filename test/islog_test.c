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

pthread_t threads[20];

int main(){
    printf("version = [%s]\n", islog_version());
    printf("pid = [%ld]\n", getpid());

    islog_level(ISLOG_LVL_INFO);
    islog_output_type(ISLOG_OUTPUT_FILE|ISLOG_OUTPUT_CONSOLE);
    islog_mask_off();
    islog_filecutsize(5000000);

    islogtag_set("TAG");
//    islogtag_output_type("TAG", ISLOG_OUTPUT_CONSOLE);

    int i;
    for( i = 0; i < 10; i++ ){
        pthread_create(&threads[i], NULL, &thread_main, i);
    }

    usleep(100000);
    wait_flag = 1;
    for ( i = 0; i < 100 ; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}
void * thread_main(void * arg){
    pthread_t *tt;
    char isodate[100] = "TESTTESTSTETSTSETEST";
    int i = (int)arg;

    int j = 0;
    for( j = 0 ; j <10000; j++){
        /* 设置mask */
        islog_mask(5, 1000);
        islog_debug("MASK:%s(%d)", isodate, i);
        islog_warn("MASK:%s(%d)", isodate, i);

        islog_mask(5, 1000);
        islog_warn_t("TAG", "%sTAG(%d)", isodate, i);
        islog_debug_t("TAG", "%sTAG(%d)", isodate, i);
    }
    return 0;
}