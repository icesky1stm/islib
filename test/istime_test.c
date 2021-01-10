//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/istime.h"

int main(){
    printf( "timezone = [%ld]\n", timezone);
    printf("istime_version = [%s]\n", istime_version());
    printf("istime_us = [%llu]\n", istime_us());

    char isodate[20];
    memset( isodate, 0x00, sizeof(isodate));
    istime_iso8601(isodate, sizeof(isodate)-1, istime_us());
    printf( "istime_iso8601 = [%s]\n", isodate);

    printf( "istime_longdate = [%ld]\n", istime_longdate());
    printf( "istime_longtime = [%ld]\n", istime_longtime());
    printf( "timezone = [%ld]\n", timezone);

    return 0;
}

