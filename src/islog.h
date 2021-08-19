//
// Created by suitm on 2020/12/15.
//

#ifndef ISLOG_H
#define ISLOG_H

/** 版本信息 **/
#define ISLOG_VERSION_NO "21.01.3"
char * islog_version();


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
#define ISLOG_OUTPUT_NUM    2

#define islog_error(...)  islog_error_t("", __VA_ARGS__)
#define islog_warn(...)   islog_warn_t("", __VA_ARGS__)
#define islog_info(...)   islog_info_t("", __VA_ARGS__)
#define islog_debug(...)  islog_debug_t("", __VA_ARGS__)

void islog_output(int level, const char *tag, const char *file, const char * func,
                  const long line, char *fmtstr, ...);

/***********************************************
 * 四、对外函数
 **********************************************/
/*** tag值设置，多tag输出的场景必须设置,以下所有参数都有默认值 ***/
int islogtag_set( char * tag);

/** 开关对应tag的日志显示，默认tag为"" **/
int islogtag_on( char * tag);
int islogtag_off( char * tag);

/*** 脱敏处理系列,无islog开头为无tag模式 ***/
/** 设置脱敏位置 **/
int islog_mask( int beg, int end);
/** 设置脱敏字符，默认为'*' **/
int islog_mask_symbol( char symbol);
/** 按tag关闭脱敏处理，使islog_mask失效 **/
int islogtag_mask_off(char * tag);
int islog_mask_off();
/** 按tag打开脱敏处理，使islog_mask生效，默认为打开 **/
int islogtag_mask_on(char * tag);
int islog_mask_on();
/** 按tag设置日志输出级别,默认为debug **/
int islogtag_level(char * tag, int level);
int islog_level(int level);

/*** 前缀格式定义 ***/
/** 设置前缀显示哪些，默认都显示 **/
int islogtag_format(char * tag, int fmtidx);
int islog_format(char * tag, int fmtidx);
/** 设置前缀显示格式的回调函数，不设置走默认值 **/
int islogtag_formatfunc( char* tag, int(*func_format)(islog_fmtidx_enum fmttype, char * old, char * new));
int islog_formatfunc(int(*func_format)(islog_fmtidx_enum fmttype, char * old, char * new));

/*** 输出信息的定义 ***/
/** 输出类型的定义，默认为终端输出 **/
int islogtag_output_type(char * tag, int output_idx);
int islog_output_type(int output_idx);
/** 设置输出的回调函数，不设置走默认值 **/
int islogtag_outputfunc( char* tag, int(*func_output)(islog_output_enum fmttype, char * buff, int len,char * attr, int i));
int islog_outputfunc(int(*func_output)(islog_output_enum fmttype, char * buff, int len,char * attr, int i ));
/** 设置日志文件的路径，不设置为./log **/
int islogtag_filepath( char * tag, char *  filepath);
int islog_filepath(char *  filepath);
/** 设置日志文件的名字，不设置为app.$TAG.log **/
int islogtag_filename( char * tag, char *  filename);
int islog_filename( char * filename);
/** 设置日志文件的切换文件大小，不设置为20M **/
int islogtag_filecutsize( char * tag, int cut_size);
int islog_filecutsize( int cut_size);

#endif //ISLOG_H
