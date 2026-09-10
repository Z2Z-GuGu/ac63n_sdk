#ifndef __DEBUG_LOG_H_
#define __DEBUG_LOG_H_

#include "typedef.h"
#include "printf.h"

extern const char log_info_enable;
extern const char log_debug_enable;  //默认关闭
extern const char log_error_enable;
#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE

#define PRINTF(format, ...)         printf(format, ## __VA_ARGS__)

#define LOG_BY_MACRO        1
#define LOG_BY_CONST        2

// #define LOG_MODE            LOG_BY_MACRO   //通过宏控制
#define LOG_MODE            LOG_BY_CONST       //通过变量控制

/*
 * ******************** LOG 通过宏控制
 */
#if (LOG_MODE == LOG_BY_MACRO)

#ifdef LOG_INFO_ENABLE
#define log_info(format, ...)       PRINTF(format "\r\n", ## __VA_ARGS__)
#define log_info_hexdump(x, y)      put_buf(x, y)
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#ifdef LOG_DEBUG_ENABLE
#define log_debug(format, ...)      PRINTF("[Debug]:" format "\r\n", ## __VA_ARGS__)
#define log_debug_hexdump(x, y)     put_buf(x, y)
#else
#define log_debug(...)
#define log_debug_hexdump(x, y)
#endif

#ifdef LOG_ERROR_ENABLE
#define log_error(format, ...)      PRINTF("[Error]:" format "\r\n", ## __VA_ARGS__)
#define log_error_hexdump(x, y)     put_buf(x, y)
#else
#define log_error(...)
#define log_error_hexdump(...)
#endif
/*
 ******************** LOG 通过常量控制
 */
#elif (LOG_MODE == LOG_BY_CONST)

#define log_info(format, ...)       \
    if (log_info_enable) \
        PRINTF(format "\r\n", ## __VA_ARGS__)

#define log_info_hexdump(x, y)     \
    if (log_info_enable) \
        put_buf(x, y)

#define log_debug(format, ...)       \
    if (log_debug_enable) \
        PRINTF("[Debug]:" format "\r\n", ## __VA_ARGS__)

#define log_debug_hexdump(x, y)     \
    if (log_debug_enable) \
        put_buf(x, y)

#define log_error(format, ...)       \
    if (log_error_enable) \
        PRINTF("[Error]:" format "\r\n", ## __VA_ARGS__)

#define log_error_hexdump(x, y)     \
    if (log_error_enable) \
        put_buf(x, y)

#endif


#define r_printf(x, ...)  log_info("\e[31m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define g_printf(x, ...)  log_info("\e[32m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define y_printf(x, ...)  log_info("\e[33m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define r_f_printf(x, ...)  log_info("\e[31m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define g_f_printf(x, ...)  log_info("\e[32m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define y_f_printf(x, ...)  log_info("\e[33m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)

#define DEBUG_DEBUG_EXT(_l, _f, _li, ...) do {printf("[%s] function(%s()),line(%d) : ", _l, _f, _li); \
                                          printf(__VA_ARGS__); printf("\n\n");}while(0)

#define DEBUG_LOGV_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("V", _f, _l, __VA_ARGS__)
#define DEBUG_LOGD_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("D", _f, _l, __VA_ARGS__)
#define DEBUG_LOGI_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("I", _f, _l, __VA_ARGS__)
#define DEBUG_LOGW_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("W", _f, _l, __VA_ARGS__)
#define DEBUG_LOGE_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("E", _f, _l, __VA_ARGS__)
#define DEBUG_LOGWTF_EXT(_f, _l, ...)   DEBUG_DEBUG_EXT("WTF", _f, _l, __VA_ARGS__)

#define DEBUG_DEBUG(_l, ...)      DEBUG_DEBUG_EXT(_l, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGV(...)           DEBUG_LOGV_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGD(...)           DEBUG_LOGD_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGI(...)           DEBUG_LOGI_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGW(...)           DEBUG_LOGW_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGE(...)           DEBUG_LOGE_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGWTF(...)         DEBUG_LOGWTF_EXT(__func__, __LINE__, __VA_ARGS__)

#else
#define log_info(...)
#define log_error(...)
#define log_info_hexdump(...)

#define DEBUG_DEBUG_EXT(_l, _f, _li, ...)

#define DEBUG_LOGV_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("V", _f, _l, __VA_ARGS__)
#define DEBUG_LOGD_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("D", _f, _l, __VA_ARGS__)
#define DEBUG_LOGI_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("I", _f, _l, __VA_ARGS__)
#define DEBUG_LOGW_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("W", _f, _l, __VA_ARGS__)
#define DEBUG_LOGE_EXT(_f, _l, ...)     DEBUG_DEBUG_EXT("E", _f, _l, __VA_ARGS__)
#define DEBUG_LOGWTF_EXT(_f, _l, ...)   DEBUG_DEBUG_EXT("WTF", _f, _l, __VA_ARGS__)

#define DEBUG_DEBUG(_l, ...)      DEBUG_DEBUG_EXT(_l, __func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGV(...)           DEBUG_LOGV_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGD(...)           DEBUG_LOGD_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGI(...)           DEBUG_LOGI_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGW(...)           DEBUG_LOGW_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGE(...)           DEBUG_LOGE_EXT(__func__, __LINE__, __VA_ARGS__)
#define DEBUG_LOGWTF(...)         DEBUG_LOGWTF_EXT(__func__, __LINE__, __VA_ARGS__)

#endif

#endif//__DEBUG_LOG_H_

