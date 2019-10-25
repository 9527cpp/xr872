#ifndef __YT_LOG_H__
#define __YT_LOG_H__

#include <stdio.h>
#include <string.h>
#include <task.h>




#define YT_LOG_LEVEL	3
#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#if defined(YT_LOG_LEVEL)
#define YT_LOG(_level, ...)     \
    printf("[%u][%s]%s  %s(%d): ",xTaskGetTickCount(), _level , filename(__FILE__),__FUNCTION__, __LINE__), \
    printf(__VA_ARGS__), \
    printf("\r\n");
#endif
#if defined(YT_LOG_LEVEL) && (YT_LOG_LEVEL >= 5)
#define YT_LOGV(...)          YT_LOG("V", __VA_ARGS__)
#else
#define YT_LOGV(...)
#endif

#if defined(YT_LOG_LEVEL) && (YT_LOG_LEVEL >= 4)
#define YT_LOGD(...)          YT_LOG("D", __VA_ARGS__)
#else
#define YT_LOGD(...)
#endif

#if defined(YT_LOG_LEVEL) && (YT_LOG_LEVEL >= 3)
#define YT_LOGI(...)          YT_LOG("I", __VA_ARGS__)
#else
#define YT_LOGI(...)
#endif

#if defined(YT_LOG_LEVEL) && (YT_LOG_LEVEL >= 2)
#define YT_LOGW(...)          YT_LOG("W", __VA_ARGS__)
#else
#define YT_LOGW(...)
#endif

#if defined(YT_LOG_LEVEL) && (YT_LOG_LEVEL >= 1)
#define YT_LOGE(...)          YT_LOG("E", __VA_ARGS__)
#else
#define YT_LOGE(...)
#endif


#endif
