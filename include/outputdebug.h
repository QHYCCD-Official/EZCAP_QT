#ifndef OUTPUTDEBUG_H
#define OUTPUTDEBUG_H

#include "include/dllqhyccd.h"

#define DBG_FLAG 1

#define DBG_TYPE_ERROR   "EZCAPERROR"   //fatal error, got unexcepted status or wrong return value of SDK
#define DBG_TYPE_WARNING "EZCAPWARNING" //lesser error, maybe has no bad influence for capturing
#define DBG_TYPE_INFO    "EZCAPINFO"    //debug information

#define DBGOPT_ERROR(fmt, ...)   OutputDebug("%s | %s | %s | " fmt, DBG_TYPE_ERROR, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define DBGOPT_WARNING(fmt, ...) OutputDebug("%s | %s | %s | " fmt, DBG_TYPE_WARNING, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define DBGOPT_INFO(fmt, ...)    OutputDebug("%s | %s | %s | " fmt, DBG_TYPE_INFO, __FILE__, __FUNCTION__, ##__VA_ARGS__)

void OutputDebug(const char *strOutputString,...);

#endif // OUTPUTDEBUG_H
