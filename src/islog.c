//
// Created by suitm on 2020/12/15.
//
/**
 * 参考了zlog和EasyLogger的写法，考虑到这是一个工具库
 * 因此目标是写了一个简单、高可用的、无配置的日志组件框架和库.
 * 使用springboot的理念，约定大于配置，不需要设置大量的init，start等，直接使用日志即可.
 * 当然，也可以对各种属性进行配置，也支持对各种属性进行设置，自定义回调函数等
 * 初步规划islog日志库一般要有如下几点功能:
 * 1. 基于tag的信息配置
 *      1) 默认使用的默认tag，即DEFAULT,此时默认的fmt不显示.
 *      2) 可以基于不同的tag，使用不同的所有配置,默认最多可以设置3个tag
 *      3) 如果使用tag时，建议使用define，来区分不同的tag,如XIPLOG,APPLOG
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
 *      2) hex 16进制格式，主要用于展示不可见报文
 *      3) raw 原始格式,是啥就是啥，完全不加工
 * 4. 输出内容的过滤，有如下两方面:
 *      1) 类似log4j的filter，可以先实现一个简单的
 *      2) 支持金融行业如证件号字段的星号加密模式
 * 5. 输出方式：
 *      1. 内嵌支持输出到终端
 *      2. 内嵌支持文件输出，需要支持：
 *          1）文件转档
 *          2）文件循环保存
 *      3. 支持扩展输出到远程的信息上，比如syslog或者其他上面，通过回调函数来实现
 *      4. 支持以上几种输出方式的组合
 *      5. 日志输出，研究一下ringbuffer，支持异步日志输出
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "islog.h"
#include "istime.h"

/** 版本信息 **/
char * islog_version(){
    return ISLOG_VERSION_NO;
}

/* 最大输出行数 */
#define ISLOG_LINE_DEFAULT_LEN 1024

/* 默认的TAG数量是3个,一般是一个系统级、一个业务级、一个监控级 */
#ifndef ISLOG_TAG_NUM
#define ISLOG_TAG_NUM          3
#endif

/* TAG的长度和默认TAG */
#define ISLOG_TAGNAME_LEN      32
#define ISLOG_TAGNAME_DEFAULT  "DEFAULT"

/* 报文head的总长度定义 */
#define ISLOG_HEAD_LEN 1024
/* 单个类型的定义 */
#define ISLOG_FMT_LEN 128
/* BUFF的最大长度, 10M */
#define ISLOG_BUFF_MAX_LEN 10240000
#define ISLOG_NEWLINE_SIGN                        "\n"

/* level的输出定义，简化写法 */
static const char *level_string[] = {
        [ISLOG_LVL_ERROR-1]   = "ERROR",
        [ISLOG_LVL_WARN-1]    = "WARN",
        [ISLOG_LVL_INFO-1]    = "INFO",
        [ISLOG_LVL_DEBUG-1]   = "DEBUG",
};

/*** 全局的日志控制配置，如果想加配置文件，可以修改这个结构体的信息 ***/
static struct _islog_logger{
    char tag[ISLOG_TAGNAME_LEN];
    int isinit;
    int level;
    struct _format{
        int format_idx;
        int (*func_format)(islog_fmtidx_enum fmttype, char * old, char * new);
    }format;
    struct _filter {
        char kw[10][100];
        int kw_pos;
        int (*func_mask)(char * buf);
    }filter;
    struct _output{
        int (*func_output)(islog_output_enum type, char * buff, int len);
        int output_idx;
        char para1[256]; /*文件名称*/
        char para2[64];  /*切换路径*/
        char para3[64];  /*保留日期*/
        char attr[1024]; /*其他属性*/
        pthread_mutex_t mutex; /*输出锁*/
    }output;
}islogger[ISLOG_TAG_NUM];

/** mask线程本地变量,主要用于对日志的某个信息进行加密 **/
static __thread struct _islogger_mask{
    int ismask;
    int beg;
    int end;
    char symbol;
}islogger_mask;

static int default_func_format(islog_fmtidx_enum fmttype, char * old, char * new);
static int default_func_output(islog_output_enum type, char * buff, int len);

/*********************************************************************************
 * 主函数
 * ******************************************************************************/
void islog_output(int level, const char *tag, const char *file, const char * func,
                  const long line, char *fmtstr, ...)
{
    va_list ap;
    char * buff = NULL;
    char head[ISLOG_HEAD_LEN];
    char oldbuf[64];
    char newbuf[128];
    int n = 0;

    /*** 零、根据tag获取当前的配置信息 ***/
    int i = 0;
    int isfind = 0;
    while(i < ISLOG_TAG_NUM){
        /* 找到了则退出 */
        if( strcmp( islogger[i].tag, tag) == 0) {
            isfind = 1;
            break;
        }
        /* 从未设置,则第一次获取 */
        if( strlen( islogger[i].tag) == 0){
            isfind = 1;
            break;
        }
        /** 继续查找 **/
        i++;
    }
    if( isfind != 1){
        printf("tag can't find and tag num is full--[%s]!!!\n", tag);
        return;
    }

    /*** 一、如果没有初始化过，则设置默认值 ***/
    if( islogger[i].isinit == 0){

        /** tag 设置 **/
        strncpy( islogger[i].tag, tag, ISLOG_TAGNAME_LEN);
        /** 日志级别-默认为DEBUG **/
        if( islogger[i].level == 0){
            islogger[i].level = ISLOG_LVL_DEBUG;
        }

        /** 格式信息-默认 **/
        if( islogger[i].format.format_idx == 0) //默认输出格式
            islogger[i].format.format_idx = ISLOG_FMT_ALL;
        if( islogger[i].format.func_format == NULL)
            islogger[i].format.func_format = default_func_format;

        /** 输出方式-默认为console **/
        if( islogger[i].output.output_idx == 0){
            islogger[i].output.output_idx = ISLOG_OUTPUT_CONSOLE;
        }
        if( islogger[i].output.func_output == NULL){
            islogger[i].output.func_output  = default_func_output;
        }

        /** 必须要放到最后面,要不然就加锁，多线程高并发的情况下，可能被执行多遍，不过没关系 **/
        islogger[i].isinit = 1;
    }

    /*** 二、动态判断日志级别是否可用 ***/
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
        sprintf(oldbuf, "%s", level_string[islogger[i].level-1]);
        islogger[i].format.func_format( ISLOG_FMT_LVL, oldbuf, newbuf);
        strcat( head, newbuf);
    }
    /* TIME */
    if( islogger[i].format.format_idx & ISLOG_FMT_TIME){
        memset( oldbuf, 0x00, sizeof( oldbuf));
        memset( newbuf, 0x00, sizeof( newbuf));
        sprintf(oldbuf, "%s", istime_iso8601(oldbuf, sizeof(oldbuf), istime_us()));
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
        sprintf(oldbuf, "%d", (int)pthread_self());
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
    n = vsnprintf(NULL, 0, fmtstr, ap) + 1 + strlen(head);
    va_end(ap);
    if( n >= ISLOG_BUFF_MAX_LEN){
        /** 如果太长，则截断 **/
        n = ISLOG_BUFF_MAX_LEN;
    }
    buff = calloc(n, sizeof(char));
    if( buff == NULL){
        printf("can't calloc buff, [%s]\n", strerror(errno));
        return;
    }
    memcpy( buff, head, strlen(head));
    va_start(ap, fmtstr);
    vsnprintf(buff + strlen(head), n - strlen(head), fmtstr, ap);
    va_end(ap);

    /*** 四、按照filter进行过滤 ***/
    //TODO

    /*** 五、输出 ***/
    if( islogger[i].output.output_idx & ISLOG_OUTPUT_CONSOLE){
        islogger[i].output.func_output(ISLOG_OUTPUT_CONSOLE, buff, n);
    }
    if( islogger[i].output.output_idx & ISLOG_OUTPUT_FILE){
        islogger[i].output.func_output(ISLOG_OUTPUT_FILE, buff, n);
    }

    /***  结束 ***/
    free(buff);
    buff = NULL;
    return ;
}

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
static int default_func_output(islog_output_enum type, char * buff, int len){
    if( type == ISLOG_OUTPUT_CONSOLE) {
        fprintf(stdout, "%s%s", buff, ISLOG_NEWLINE_SIGN);
    }else if( type == ISLOG_OUTPUT_FILE){
        //TODO
        fprintf(stdout, "%s%s", buff, ISLOG_NEWLINE_SIGN);
    }
    return 0;
}

#if 0
/* REDIS的serverlog, TODO待修改 */
void serverLogRaw(int level, const char *msg) {
    const int syslogLevelMap[] = { LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING };
    const char *c = ".-*#";
    FILE *fp;
    char buf[64];
    int rawmode = (level & LL_RAW);
    int log_to_stdout = server.logfile[0] == '\0';

    level &= 0xff; /* clear flags */
    if (level < server.verbosity) return;

    fp = log_to_stdout ? stdout : fopen(server.logfile,"a");
    if (!fp) return;

    if (rawmode) {
        fprintf(fp,"%s",msg);
    } else {
        int off;
        struct timeval tv;
        int role_char;
        pid_t pid = getpid();

        gettimeofday(&tv,NULL);
        struct tm tm;
        nolocks_localtime(&tm,tv.tv_sec,server.timezone,server.daylight_active);
        off = strftime(buf,sizeof(buf),"%d %b %Y %H:%M:%S.",&tm);
        snprintf(buf+off,sizeof(buf)-off,"%03d",(int)tv.tv_usec/1000);
        if (server.sentinel_mode) {
            role_char = 'X'; /* Sentinel. */
        } else if (pid != server.pid) {
            role_char = 'C'; /* RDB / AOF writing child. */
        } else {
            role_char = (server.masterhost ? 'S':'M'); /* Slave or Master. */
        }
        fprintf(fp,"%d:%c %s %c %s\n",
                (int)getpid(),role_char, buf,c[level],msg);
    }
    fflush(fp);

    if (!log_to_stdout) fclose(fp);
    if (server.syslog_enabled) syslog(syslogLevelMap[level], "%s", msg);
}
#endif