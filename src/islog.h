//
// Created by suitm on 2020/12/15.
//

#ifndef ISLOG_H
#define ISLOG_H

/** 版本信息 **/
#define ISLOG_VERSION_NO "21.01.1"
char * islog_version();

/** 函数声明区 **/
void islog_output(int level, const char *tag, const char *file, const char * func,
                  const long line, char *fmtstr, ...);
/***********************************************
 * 一、 通过宏的方式控制日志是否输出，
 * 因为在预编译时执行,对效率完全没有影响
 * ********************************************/
#define ISLOG_LVL ISLOG_LVL_DEBUG
/* 定义级别顺序 */
#define ISLOG_LVL_ERROR                       1
#define ISLOG_LVL_WARN                        2
#define ISLOG_LVL_INFO                        3
#define ISLOG_LVL_DEBUG                       4
#define ISLOG_LVL_TOTAL_NUM                   4


#ifdef ISLOG_DISABLE
#define islog_error_t(tag, ...)
#define islog_warn_t(tag, ...)
#define islog_info_t(tag, ...)
#define islog_debug_t(tag, ...)
#else /*ISLOG_DIASABLE*/
/* 判断开启的日志级别ERROR */
#if ISLOG_LVL >= ISLOG_LVL_ERROR
#define islog_error_t(tag, ...)  \
    do{                      \
        islog_output(ISLOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }while(0)
#else
#define islog_error_t(tag, ...)
#endif

/* 判断开启的日志级别WARN */
#if ISLOG_LVL >= ISLOG_LVL_WARN
#define islog_warn_t(tag, ...)  \
    do{                      \
        islog_output(ISLOG_LVL_WARN, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }while(0)
#else
#define islog_warn_t(tag, ...)
#endif
/* 判断开启的日志级别INFO */
#if ISLOG_LVL >= ISLOG_LVL_INFO
#define islog_info_t(tag, ...)  \
    do{                      \
        islog_output(ISLOG_LVL_INFO, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }while(0)
#else
#define islog_info_t(tag, ...)
#endif
/* 判断开启的日志级别DEBUG */
#if ISLOG_LVL >= ISLOG_LVL_DEBUG
#define islog_debug_t(tag, ...)  \
    do{                      \
        islog_output(ISLOG_LVL_DEBUG, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);\
    }while(0)
#else
#define islog_debug_t(tag, ...)
#endif /*ISLOG_LVL*/
#endif/*ISLOG_DISABLE*/


/***********************************************
 * 二、日志的输出格式定义
 **********************************************/
/* all formats index */
typedef enum {
    ISLOG_FMT_TAG    = 1 << 0, /**< tag */
    ISLOG_FMT_LVL    = 1 << 1, /**< level */
    ISLOG_FMT_TIME   = 1 << 2, /**< current time */
    ISLOG_FMT_P_INFO = 1 << 3, /**< process info */
    ISLOG_FMT_T_INFO = 1 << 4, /**< thread info */
    ISLOG_FMT_FILE    = 1 << 5, /**< file directory and name */
    ISLOG_FMT_FUNC   = 1 << 6, /**< function name */
    ISLOG_FMT_LINE   = 1 << 7, /**< line number */
} islog_fmtidx_enum;

/* macro definition for all formats */
#define ISLOG_FMT_ALL    (ISLOG_FMT_LVL|ISLOG_FMT_TAG|ISLOG_FMT_TIME|ISLOG_FMT_P_INFO|ISLOG_FMT_T_INFO| \
    ISLOG_FMT_FILE|ISLOG_FMT_FUNC|ISLOG_FMT_LINE)

/***********************************************
 * 三、日志的输出方式
 **********************************************/
 typedef enum {
     ISLOG_OUTPUT_CONSOLE = 1 << 0,
     ISLOG_OUTPUT_FILE    = 1 << 1,
 } islog_output_enum;

#define islog_error(...)  islog_error_t("DEFAULT", __VA_ARGS__)
#define islog_warn(...)   islog_warn_t("DEFAULT", __VA_ARGS__)
#define islog_info(...)   islog_info_t("DEFAULT", __VA_ARGS__)
#define islog_debug(...)  islog_debug_t("DEFAULT", __VA_ARGS__)
#endif //ISLOG_H
