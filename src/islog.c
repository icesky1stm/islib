//
// Created by suitm on 2020/12/15.
//
/**
 * 极简的约定大于配置的c语言日志库
 *
 * 参考了zlog和EasyLogger的写法，考虑到这是一个工具库
 * 因此目标是写了一个简单、高可用的、无配置的日志组件框架和库.
 * 使用springboot的约定大于配置的历年，不需要任何设置init，start等，直接islog_debug即可输出日志.
 * 当然，也可以通过各种暴露的对外函数,对各种属性进行配置以及自定义回调函数等
 *
 * islog日志库包含如下几点功能:
 * 1. 基于tag的信息配置
 *      1) 默认使用的默认tag，即DEFAULT,此时默认的fmt不显示.
 *      2) 可以基于不同的tag，使用不同的所有配置,默认最多可以设置ISLOG_TAG_NUM个tag
 *      3) 如果使用tag时，使用islog_xxxx_t系列函数
 *         使用扩展宏定义define，来区分不同的tag,如XIPLOG,APPLOG
 * 2. 日志的输出级别:
 *      控制方式:
 *      1) 静态、通过宏定义来实现，编译时使用。(因为是类库，也很重要)
 *      2) 动态、通过API接口来实现，运行时使用。
 *      级别:
 *      1) debug, 调试信息
 *      2) info, 正常信息
 *      3) warn, 警告信息
 *      4) error, 错误信息
 * 3. 输出的格式定义支持：
 *      1) 支持 日志级别、打印时间、业务标签(MDC?)、进程信息、线程信息、文件路径、行号、方法名
 *      2) TODO hex 16进制格式，主要用于展示不可见报文
 *      3) TODO raw 原始格式,是啥就是啥，完全不加工
 * 4. 输出内容的过滤，有如下两方面:
 *      1) 支持敏感信息如证件号、手机号中间字段的星号加密模式
 * 5. 输出方式：
 *      1. 内嵌支持输出到终端
 *      2. 内嵌支持文件输出，需要支持：
 *          1）文件转档
 *      3. 支持扩展输出到远程的信息上，比如syslog或者其他上面，通过回调函数来实现
 *      4. 支持以上几种输出方式的组合
 *      5. TODO 日志输出，研究一下ringbuffer，支持异步日志输出
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

#include "islog.h"

#include "isstr.h"
#include "istime.h"

/** 版本信息 **/
char * islog_version(){
    return ISLOG_VERSION_NO;
}

/* 默认的TAG数量是5个,一般是一个业务级、一个系统级、一个监控级就够使用 */
#ifndef ISLOG_TAG_NUM
#define ISLOG_TAG_NUM          5
#endif
/* TAG的长度和默认TAG */
#define ISLOG_TAGNAME_LEN      32
#define ISLOG_TAGNAME_DEFAULT  ""

/* 报文head的总长度定义 */
#define ISLOG_HEAD_LEN 1024
/* 单个类型的定义 */
#define ISLOG_FMT_LEN 128
/* BUFF的最大长度, 10M */
#define ISLOG_BUFF_MAX_LEN 10240000
#define ISLOG_NEWLINE_SIGN                        "\n"

/* 默认20M一个文件 */
#define ISLOG_OUTPUT_FILE_CUTSIZE 20000000

/* FILE默认设置 */
#define ISLOG_FILE_DEFAULT_SETTING "./log, app%s.log, 20000000"

/* level的输出定义，简化写法 */
static const char *level_string[] = {
        [ISLOG_LVL_ERROR-1]   = "ERROR",
        [ISLOG_LVL_WARN-1]    = "WARN",
        [ISLOG_LVL_INFO-1]    = "INFO",
        [ISLOG_LVL_DEBUG-1]   = "DEBUG",
};

/*** 全局的日志控制配置，如果想加配置文件，可以修改这个结构体的信息 ***/
static struct _islog_logger{
    char tag[ISLOG_TAGNAME_LEN + 1];
    int isinit;
    int off;
    int level;
    struct _format{
        int format_idx;
        int (*func_format)(islog_fmtidx_enum fmttype, char * old, char * new);
    }format;
    struct _filter {
        int maskoff;
    }filter;
    struct _output{
        int (*func_output)(islog_output_enum type, char * buff, int len, char * attr, int i);
        int output_idx;
        FILE * fp;
        char file_path[256];
        char file_name[256];
        int file_cutsize;
        char attr[1024]; /*附加属性,用于自定义函数*/
        pthread_mutex_t mutexs[ISLOG_OUTPUT_NUM]; /*输出锁*/
    }output;
}islogger[ISLOG_TAG_NUM];
pthread_mutex_t g_islogger_mutex = PTHREAD_MUTEX_INITIALIZER; /*islog配置锁*/

/** mask线程本地变量,主要用于对日志中关键位置的进行加密 **/
static __thread struct _islogger_mask{
    int ismask;
    int beg;
    int end;
    char symbol;
}thread_islogger_mask;

/* 内部使用函数定义 */
static int islogger_tag_find(const char * tag);
static int islogger_init(int i);
static int default_func_format(islog_fmtidx_enum fmttype, char * old, char * new);
static int default_func_output(islog_output_enum type, char * buff, int len,char * attr, int i);
void islog_log(int level, const char * file, const char *func, const long line, char *fmtstr, ...);
#define isloglog_d(...)  islog_log(ISLOG_LVL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define isloglog_e(...)  islog_log(ISLOG_LVL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define isloglog_i(...)  islog_log(ISLOG_LVL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define isloglog_w(...)  islog_log(ISLOG_LVL_WARN, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)


/*********************************************************************************
 * 主函数
 * ******************************************************************************/
void islog_output(int level, const char *tag, const char *file, const char * func,
                  const long line, char *fmtstr, ...)
{
    va_list ap;
    char * buff = NULL;
    char head[ISLOG_HEAD_LEN + 1];
    char oldbuf[128];
    char newbuf[128];
    int n = 0;

    /*** 零、根据tag获取当前的配置信息 ***/
    int i = 0;
    i = islogger_tag_find( tag);
    if( i < 0){
        return ;
    }
    if( islogger->off != 0){
        return;
    }

    /*** 一、如果没有初始化过，则设置默认值 ***/
    islogger_init(i);

    /*** 二、动态判断日志级别是否可用 ***/
//    isloglog_d("level --- [%d] > [%d][%d][%s][%s]0[%s]1[%s]2[%s][%ld]", level, islogger[i].level, i, tag, islogger[i].tag,
//    islogger[0].tag, islogger[1].tag, islogger[2].tag, __LINE__);

    if (level > islogger[i].level){
        return;
    }

    /*** 三、按照format的信息，组成日志的buff ***/
    /** 1. 组成日志头 **/
    memset( head, 0x00, sizeof(head));
    /* TAG */
    if( islogger[i].format.format_idx & ISLOG_FMT_TAG){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s", islogger[i].tag);
        islogger[i].format.func_format( ISLOG_FMT_TAG, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* LVL */
    if( islogger[i].format.format_idx & ISLOG_FMT_LVL){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s", level_string[level-1]);
        islogger[i].format.func_format( ISLOG_FMT_LVL, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* TIME */
    if( islogger[i].format.format_idx & ISLOG_FMT_TIME){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s.%llu", istime_iso8601(oldbuf, sizeof(oldbuf), istime_us()), istime_us()%1000000);
        islogger[i].format.func_format( ISLOG_FMT_TIME, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* PID */
    if( islogger[i].format.format_idx & ISLOG_FMT_P_INFO){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%d", getpid());
        islogger[i].format.func_format( ISLOG_FMT_P_INFO, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* TID */
    if( islogger[i].format.format_idx & ISLOG_FMT_T_INFO){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%04ld", ((unsigned long)pthread_self())%10000);
        islogger[i].format.func_format( ISLOG_FMT_T_INFO, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* FUNC */
    if( islogger[i].format.format_idx & ISLOG_FMT_FUNC){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s", func);
        islogger[i].format.func_format( ISLOG_FMT_FUNC, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* FILE */
    if( islogger[i].format.format_idx & ISLOG_FMT_FILE){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s", file);
        islogger[i].format.func_format( ISLOG_FMT_FILE, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* LINE */
    if( islogger[i].format.format_idx & ISLOG_FMT_LINE){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%ld", line);
        islogger[i].format.func_format( ISLOG_FMT_LINE, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /** 2. 组成日志体 **/
    va_start(ap, fmtstr);
    n = vsnprintf(NULL, 0, fmtstr, ap) + strlen(head) + 1;
    va_end(ap);
    if( n >= ISLOG_BUFF_MAX_LEN){
        /** 如果太长，则截断 **/
        n = ISLOG_BUFF_MAX_LEN;
    }
    buff = calloc(n + 1, sizeof(char));
    if( buff == NULL){
        isloglog_e("can't calloc buff, [%s]", strerror(errno));
        return;
    }
    memcpy( buff, head, strlen(head));
    va_start(ap, fmtstr);
    vsnprintf(buff + strlen(head), n - strlen(head) + 1, fmtstr, ap);
    va_end(ap);
    strcat( buff, ISLOG_NEWLINE_SIGN);

    /*** 四、按照mask进行过滤 ***/
    int mask_cur = 0;
    /** 全局开关 **/
    if( islogger[i].filter.maskoff != 1 ){
        /** 线程变量，每次设置 **/
        if( thread_islogger_mask.ismask == 1){
            mask_cur = strlen(head) + thread_islogger_mask.beg;
            while( buff[mask_cur] != 0x00 && buff[mask_cur] != '\n' ){
                if( mask_cur - strlen(head) >  thread_islogger_mask.end ){
                    break;
                }else{
                    buff[mask_cur] = thread_islogger_mask.symbol;
                }
                mask_cur++;
            }
            /** 每次要重新设置 **/
            thread_islogger_mask.ismask = 0;
            thread_islogger_mask.beg = 0;
            thread_islogger_mask.end = 0;
        }
    }

    /*** 五、输出 ***/
    if( islogger[i].output.output_idx & ISLOG_OUTPUT_CONSOLE){
        islogger[i].output.func_output(ISLOG_OUTPUT_CONSOLE, buff, n, islogger[i].output.attr, i);
    }
    if( islogger[i].output.output_idx & ISLOG_OUTPUT_FILE){
        islogger[i].output.func_output(ISLOG_OUTPUT_FILE, buff, n, islogger[i].output.attr, i);
    }

    /***  结束 ***/
    free(buff);
    buff = NULL;
    return ;
}
/*** 辅助函数 ***/
static int islogger_tag_find(const char * tag){
    int i = 0;
    int isfind = 0;

    while(i < ISLOG_TAG_NUM){
        /* 如果tag是空且不是第一个，则没找到 */
        if( strlen( islogger[i].tag) == 0 && i != 0){
            break;
        }

        /* 比较是否找到*/
        if( strncmp( islogger[i].tag, tag, ISLOG_TAGNAME_LEN) == 0) {
            isfind = 1;
            break;
        }else{
            /** 继续查找 **/
            i++;
        }
    }
    if( isfind != 1){
        isloglog_e("can't find tag[%s]", tag);
        return -5;
    }

    return i;
}

/*********************************************************************************
 * 供外部系统设置参数的对外函数
 * ******************************************************************************/
/** 设置日志的tag **/
int islogtag_set( char * tag){
    if( strlen(tag) == 0){
        return 0;
    }
    int isset = 0;
    int i = 1;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    while(i < ISLOG_TAG_NUM){
        /* 如果tag是空，说明要新设置 */
        if( strlen( islogger[i].tag) == 0){
            strncpy( islogger[i].tag, tag, ISLOG_TAGNAME_LEN);
            isset = 1;
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    if( isset != 1){
        isloglog_e("[%s] index[%d] is large than ISLOG_TAG_NUM[%d]", tag, i+1, ISLOG_TAG_NUM);
        return -5;
    }
    return i;
};
/** 设置当前线程的下一行日志，是否进行脱敏处理 **/
int islog_mask( int beg, int end){
    if( beg > end){
        return -5;
    }
    thread_islogger_mask.beg = beg - 1;
    thread_islogger_mask.end = end - 1;
    if( thread_islogger_mask.symbol == 0){
        thread_islogger_mask.symbol = '*';
    }
    thread_islogger_mask.ismask = 1;
    return 0;
}
int islog_mask_symbol( char symbol){
    thread_islogger_mask.symbol = symbol;
    return 0;
}
int islogtag_mask_off(char * tag){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].filter.maskoff = 1;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_mask_off(){
    return islogtag_mask_off("");
}
int islogtag_mask_on(char * tag){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].filter.maskoff = 0;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_mask_on(){
    return islogtag_mask_on("");
}

int islogtag_output_type(char * tag, int output_idx){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].output.output_idx = output_idx;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_output_type(int output_idx){
    return islogtag_output_type("", output_idx);
}

int islogtag_level(char * tag, int level){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].level = level;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_level(int level){
    return islogtag_level("", level);
}

int islogtag_format(char * tag, int fmtidx){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].format.format_idx = fmtidx;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_format(char * tag, int fmtidx) {
    return islogtag_format("", fmtidx);
}

int islogtag_formatfunc( char* tag, int(*func_format)(islog_fmtidx_enum fmttype, char * old, char * new)){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].format.func_format = func_format;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_formatfunc(int(*func_format)(islog_fmtidx_enum fmttype, char * old, char * new)){
    return islogtag_formatfunc("", func_format);
}
int islogtag_outputfunc( char* tag, int(*func_output)(islog_output_enum fmttype, char * buff, int len,char * attr, int i)){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].output.func_output = func_output;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_outputfunc(int(*func_output)(islog_output_enum fmttype, char * buff, int len,char * attr, int i )){
    return islogtag_outputfunc("", func_output);
}

int islogtag_filepath( char * tag, char *  filepath){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    strncpy( islogger[i].output.file_path, filepath, 256);
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_filepath(char *  filepath){
    return islogtag_filepath("", filepath);
}

int islogtag_filename( char * tag, char *  filename){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    strncpy( islogger[i].output.file_path, filename, 256);
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_filename( char * filename){
    return islogtag_filename( "", filename);
}

int islogtag_filecutsize( char * tag, int cut_size){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].output.file_cutsize = cut_size;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islog_filecutsize( int cut_size){
    return islogtag_filecutsize("", cut_size);
}
int islogtag_off( char * tag){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].off = 1;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}
int islogtag_on( char * tag){
    int i = islogger_tag_find(tag);
    if( i < 0) return -5;
    pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
    islogger[i].off = 0;
    pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    return i;
}

/*********************************************************************************
 * 供islog内部使用的函数，都是static的
 * ******************************************************************************/
static int islogger_init(int i){
    int j;
    if( islogger[i].isinit == 0){
        /** 此处加锁的目的是防止后续有人改动,即使不加锁，由于每个赋值都是可以重入的，其实没有影响 **/
        pthread_mutex_lock(&g_islogger_mutex); /*全局配置锁*/
        if( islogger[i].isinit == 0) {
            /** 日志级别-默认为DEBUG **/
            if (islogger[i].level == 0) {
                islogger[i].level = ISLOG_LVL_DEBUG;
            }

            /** 格式信息-默认 **/
            if (islogger[i].format.format_idx == 0) //默认输出格式
                islogger[i].format.format_idx = ISLOG_FMT_ALL;
            if (islogger[i].format.func_format == NULL)
                islogger[i].format.func_format = default_func_format;
            /** 输出方式-默认为file **/
            if (islogger[i].output.output_idx == 0) {
                islogger[i].output.output_idx = ISLOG_OUTPUT_FILE;
            }
            if (islogger[i].output.func_output == NULL) {
                islogger[i].output.func_output = default_func_output;
            }
            if (strlen(islogger[i].output.attr) == 0) {
                snprintf(islogger[i].output.attr, 1024 , ISLOG_FILE_DEFAULT_SETTING, islogger[i].tag);
            }
            if( strlen(islogger[i].output.file_path) == 0){
                strcpy( islogger[i].output.file_path, "./log");
            }
            if( strlen(islogger[i].output.file_name) == 0){
                if( i == 0){
                    snprintf( islogger[i].output.file_name, 255, "app.log");
                }else{
                    snprintf( islogger[i].output.file_name, 255, "app.%s.log", islogger[i].tag);
                }
            }
            if( islogger[i].output.file_cutsize == 0){
                islogger[i].output.file_cutsize = ISLOG_OUTPUT_FILE_CUTSIZE;
            }

            for( j  = 0; j< ISLOG_OUTPUT_NUM; j++){
                pthread_mutex_init(&(islogger[i].output.mutexs[j]), NULL);
            }

            /** 必须要放到最后面,要不然就加锁，多线程高并发的情况下，可能被执行多遍，不过没关系 **/
            islogger[i].isinit = 1;
        }
        pthread_mutex_unlock(&g_islogger_mutex); /*全局配置锁*/
    }

    return 0;
}

/** islog内部日志 **/
pthread_mutex_t  g_islog_log_mutex = PTHREAD_MUTEX_INITIALIZER;
void islog_log(int level, const char * file, const char *func, const long line, char *fmtstr, ...){
    va_list ap;
    va_start(ap, fmtstr);
    pthread_mutex_lock( &g_islog_log_mutex );
    fprintf(stdout,"#islog# [%s][%ld][%ld]", level_string[level-1], ((unsigned long)pthread_self())%10000, line);
    vfprintf(stdout, fmtstr, ap);
    fprintf(stdout,"\n");
    pthread_mutex_unlock( &g_islog_log_mutex );
    va_end(ap);
    return ;
}
/**********************************************
 * 默认的回调函数
 *********************************************/
/** 默认的日志格式,可以自定义该函数进行回调（重写）**/
static int default_func_format(islog_fmtidx_enum fmttype, char * old, char * new){
    switch( fmttype) {
        case ISLOG_FMT_TAG:
            /* 如果是default，默认不显示 */
            if (strcmp(old, ISLOG_TAGNAME_DEFAULT) == 0) {
                strcpy(new, "");
            } else {
                snprintf(new, ISLOG_FMT_LEN - 1, "[%s]", old);
            }
            break;
        case ISLOG_FMT_LVL:
            snprintf(new, ISLOG_FMT_LEN - 1, "[%s]", old);
            break;
        case ISLOG_FMT_TIME:
            snprintf(new, ISLOG_FMT_LEN - 1, "[%s]", old);
            break;
        case ISLOG_FMT_P_INFO:
            snprintf(new, ISLOG_FMT_LEN - 1, "[%s", old);
            break;
        case ISLOG_FMT_T_INFO:
            snprintf(new, ISLOG_FMT_LEN - 1, "-%s]", old);
            break;
        case ISLOG_FMT_FUNC:
            snprintf(new, ISLOG_FMT_LEN - 1, "[%s", old);
            break;
        case ISLOG_FMT_FILE:
            snprintf(new, ISLOG_FMT_LEN - 1, "-%s:", old);
            break;
        case ISLOG_FMT_LINE:
            snprintf(new, ISLOG_FMT_LEN - 1, "%s]", old);
            break;
        default:
            snprintf(new, ISLOG_FMT_LEN - 1, "[%s]", old);
            break;
    }

    return strlen(new);
}

/** 默认输出方式，可以回调重写 **/
static int default_func_file_cut(FILE ** p_fp, char * path, char * name, int i);
static int default_func_output(islog_output_enum type, char * buff, int len, char * attr, int i){
    if( type == ISLOG_OUTPUT_CONSOLE) {
        pthread_mutex_lock( &islogger[i].output.mutexs[(int)log2(ISLOG_OUTPUT_CONSOLE)]);
        fprintf(stdout, "%s", buff);
        pthread_mutex_unlock( &islogger[i].output.mutexs[(int)log2(ISLOG_OUTPUT_CONSOLE)]);
    }else if( type == ISLOG_OUTPUT_FILE){
        pthread_mutex_lock( &islogger[i].output.mutexs[(int)log2(ISLOG_OUTPUT_FILE)]);
        char filename[512];
        int file_size = 0;

        /** 如果fp为空的话，加锁重新打开文件 **/
        if( islogger[i].output.fp == NULL){
            snprintf( filename, sizeof(filename) - 1, "%s/%s", islogger[i].output.file_path, islogger[i].output.file_name);
            islogger[i].output.fp = fopen(filename, "a+");
            if (islogger[i].output.fp == NULL) {
                isloglog_e("open logfile error[%s],[%s]", filename, strerror(errno));
                pthread_mutex_unlock( &islogger[i].output.mutexs[(int)log2(ISLOG_OUTPUT_FILE)]);
                return -5;
            }
        }
        /** 写文件 **/
        fwrite(buff, len , sizeof(char), islogger[i].output.fp);
        fflush(islogger[i].output.fp);

        /** 判断文件大小，决定是否切换文件 **/
        fseek(islogger[i].output.fp, 0L, SEEK_END);
        file_size = ftell(islogger[i].output.fp);
        if( file_size >= islogger[i].output.file_cutsize){
            if( islogger[i].output.fp != NULL){
                default_func_file_cut(&islogger[i].output.fp, islogger[i].output.file_path, islogger[i].output.file_name, i);
            }
        }

        pthread_mutex_unlock( &islogger[i].output.mutexs[(int)log2(ISLOG_OUTPUT_FILE)]);
    }
    return 0;
}

static int default_func_file_cut(FILE ** p_fp, char * path, char * name, int i){
    char oldpath[256], newpath[256];
    FILE *tmp_fp = NULL;
    int ret = 0;

    fclose(*p_fp);
    *p_fp = NULL;

    memset( oldpath, 0x00, sizeof( oldpath));
    memset( newpath, 0x00, sizeof( newpath));
    snprintf(oldpath, sizeof(oldpath),"%s/%s.%ld%ld.%06llu", path, name, istime_longdate(), istime_longtime(), istime_us()%1000000);
reopen:
    snprintf(newpath, sizeof(newpath),"%s/%s", path, name);
    tmp_fp = fopen( oldpath, "r");
    if( tmp_fp == NULL){
        /*** 不存在才正常，则rename，如果存在，说明被其他人rename过了 ***/
        ret = rename(newpath, oldpath);
        if( ret != 0){
            isloglog_e("rename error!!![%s]->[%s]", newpath, oldpath);
        }
    }else{
        fclose(tmp_fp);
        tmp_fp = NULL;
        isloglog_w("back log file conflict, add thread id[%s]", oldpath);
        usleep(1); //等1us再重试
        goto reopen;
    }

    *p_fp = fopen(newpath, "a+");

    return 0;
}
