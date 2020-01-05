/**
 * @file RTE_Log.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 0.1
 * @date 2019-12-22
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __RTE_LOG_H
#define __RTE_LOG_H
#include "RTE_Config.h"
/* Define logging levels */
typedef uint8_t log_level_t;
enum {
    LOG_LEVEL_FATAL     =  0,    // A fatal error has occurred: program will exit immediately
    LOG_LEVEL_ERROR     =  1,    // An error has occurred: program may not exit
    LOG_LEVEL_INFO      =  2,    // Necessary information regarding program operation
    LOG_LEVEL_WARN      =  3,    // Any circumstance that may not affect normal operation
    LOG_LEVEL_DEBUG     =  4,    // Standard debug messages
    LOG_LEVEL_VERBOSE   =  5,    // All debug messages
};
typedef uint16_t log_format_t;
enum {
    LOG_FMT_FUNCTION    =  0x0001,    //
    LOG_FMT_LINE        =  0x0002,    //
    LOG_FMT_FILE        =  0x0004,    //
    LOG_FMT_TIME        =  0x0008,    //
    LOG_FMT_DEFAULT     =  0x000B,    //
    LOG_FMT_FULL        =  0x000F,    //
};
typedef uint8_t log_command_t;
enum{
    LOG_CMD_ENABLE      = 0,    //
    LOG_CMD_DISABLE     = 1,    //
    LOG_CMD_LEVEL_UP    = 2,    //
    LOG_CMD_LEVEL_DOWN  = 3,    //
    LOG_CMD_GET_LEVEL   = 4,    //
    LOG_CMD_SET_FORMAT  = 5,    //
    LOG_CMD_GET_FORMAT  = 6,    //
    LOG_CMD_ADD_FILTER  = 7,    //
    LOG_CMD_RMV_FILTER  = 8,    //
};
/* Define colors for printing to terminal */
#define COL_FATAL   "\x1B[31m"  // Red
#define COL_ERROR   "\x1B[91m"  // Light Red
#define COL_INFO    "\x1B[37m"  // White
#define COL_WARN    "\x1B[33m"  // Yellow
#define COL_DEBUG   "\x1B[94m"  // Light Blue
#define COL_VERBOSE "\x1B[36m"  // Cyan
#define CLR_RESET   "\033[0m"
typedef size_t (*log_output_f)(uint8_t *data, size_t length);
typedef uint64_t (*log_get_tick_f)(void);
#define LOG_STR1(R)  #R
#define LOG_STR2(R)  LOG_STR1(R)
#define LOG_FATAL(MODULE, ...) log_out(LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_ERROR(MODULE, ...) log_out(LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_INFO(MODULE, ...) log_out(LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_WARN(MODULE, ...) log_out(LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_DEBUG(MODULE, ...) log_out(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_VERBOSE(MODULE, ...) log_out(LOG_LEVEL_VERBOSE, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_ASSERT(MODULE, v)   do{														        \
                                    if(!(v)) {											        \
                                        LOG_FATAL(MODULE, "assert [%s] fail!", LOG_STR2(v));    \
                                        while(1);                                               \
                                    }													        \
						        }while(0)
#define LOG_LOCK(v)   do{                                                                       \
                            if(((v)->mutex_lock_func) && ((v)->mutex))                          \
                                (((v)->mutex_lock_func))((v)->mutex);                           \
                      }while(0)

#define LOG_UNLOCK(v) do{                                                                       \
                            if(((v)->mutex_lock_func) && ((v)->mutex))                          \
                                (((v)->mutex_unlock_func))((v)->mutex);                         \
                      }while(0)
/**
 * @brief
 *
 */
typedef struct __log_config_t_{
    bool enable;
    log_level_t level;
    log_format_t format;
    uint8_t filter_cnt;
    char *filter_tbl[LOG_MAX_FILTER_CNT];
    void *mutex;
    log_output_f out_func;
    mutex_lock_f mutex_lock_func;
    mutex_unlock_f mutex_unlock_func;
    log_get_tick_f get_tick_func;
}log_config_t;
/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Default lock function for a null mutex.
 *
 * @param mutex
 * @return int8_t
 */
static inline int8_t log_default_mutex_lock(void *mutex)
{
    UNUSED(mutex);
    return 0;
}
/**
 * @brief Default unlock function for a null mutex.
 *
 * @param mutex
 * @return int8_t
 */
static inline int8_t log_default_mutex_unlock(void *mutex)
{
    UNUSED(mutex);
    return 0;
}
/**
 * @brief
 *
 * @param mutex
 * @param out_func
 * @param mutex_lock_func
 * @param mutex_unlock_func
 * @param get_tick_func
 * @return int8_t
 */
extern int8_t log_init(void *mutex, log_output_f out_func,
                        mutex_lock_f mutex_lock_func,
                        mutex_unlock_f mutex_unlock_func,
                        log_get_tick_f get_tick_func);
/**
 * @brief
 *
 * @param level
 * @param file
 * @param function
 * @param line
 * @param module
 * @param msg
 * @param ...
 * @return size_t
 */
extern size_t log_out(log_level_t level, const char *file, const char *function, int line,
                        const char *module, const char *msg, ...);
/**
 * @brief
 *
 * @param command
 * @param param
 * @return int8_t
 */
extern int8_t log_control(log_command_t command, void *param);
/**
 * @brief
 *
 * @param timer
 * @param tm
 */
extern void log_sec2time(time_t timer, struct tm *tm);
/**
 * @brief
 *
 * @param s
 * @param n
 * @param format
 * @param ...
 * @return int
 */
extern int log_snprintf(char * restrict s, size_t n, const char * restrict format, ...);
/**
 * @brief
 *
 * @param s
 * @param format
 * @param ...
 * @return int
 */
extern int log_sprintf(char * restrict s, const char *format, ...);

/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
#endif