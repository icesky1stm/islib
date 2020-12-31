//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/istime.h"

int main(){
    printf(" timezone--[%d], [%d]\n", (int)timezone, (int)daylight);
    tzset(); /* Now 'timezome' global is populated. */
    printf(" timezone--[%d], [%d]\n", (int)timezone, (int)daylight);

    printf("istime_version = [%s]\n", istime_version());
    printf("istime_us = [%llu]\n", istime_us());

    char isodate[20];
    memset( isodate, 0x00, sizeof(isodate));
    istime_iso8601(isodate, sizeof(isodate)-1, istime_us());
    printf( "istime_iso8601 = [%s]\n", isodate);

    printf( "istime_longdate = [%ld]\n", istime_longdate());
    printf( "istime_longtime = [%ld]\n", istime_longtime());

    time_t t = time(NULL);
    struct tm *aux = localtime(&t);
    int daylight_active = aux->tm_isdst;

    struct tm tm;
    char buf[1024];

    nolocks_localtime(&tm,t,timezone,daylight_active);
    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",&tm);
    printf("[timezone: %d, dl: %d %s\n", (int)timezone, (int)daylight_active, buf);
    printf(" timezone--[%d], [%d], [%d]\n", (int)timezone, (int)daylight, (int)daylight_active );

    return 0;
}

