//
// Created by suitm on 2020/12/26.
//
/**
uint32_t的长度(10-1位): 4294967296----------
uint64_t的长度(20-1位): 18446744073709551615
**/

#include <time.h>
#include <sys/time.h>

#include "istime.h"

char * istime_version(){
    return ISTIME_VERSION_NO;
}

/** 获取当前时间，到微秒 **/
uint64_t istime_us(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec * 1000000) + t.tv_usec;
}

/** 使用time_us作为输出，调用strftime **/
uint32_t istime_strftime(char * strtime, uint32_t maxsize, const char * format, uint64_t time_us){
    struct timeval t;
    t.tv_sec = time_us / 1000000;
    //t.tv_usec = time_us % 1000000;

    struct tm local_time;

    /*** TODO 可能会锁，后续改为redis的 nonblock_localtime 写法 ***/
    localtime_r( &t.tv_sec, &local_time);
    return strftime(strtime, maxsize, format, &local_time);
}

/** iso8601格式的 **/
char * istime_iso8601(char * strtime, uint32_t maxsize, uint64_t time_us){
    char format[]="%Y-%m-%dT%I:%M:%S";
    // 至少 19 位;
    // 2020-12-31T12:12:12
    istime_strftime( strtime, maxsize, format, time_us);
    return strtime;
}

long istime_longtime(){
    time_t t = time(0);
    struct tm local_time;

    localtime_r(&t, &local_time);
    return local_time.tm_hour*10000 + local_time.tm_min*100 + local_time.tm_sec;
}

long istime_longdate(){
    time_t t = time(0);
    struct tm local_time;

    localtime_r(&t, &local_time);
    return (local_time.tm_year+1900)*10000 + (local_time.tm_mon+1)*100 + local_time.tm_mday;
}
