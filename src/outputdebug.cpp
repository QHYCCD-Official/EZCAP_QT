#include "outputdebug.h"
#include <stdarg.h>

void OutputDebug(const char *strOutputString,...)
{
#if DBG_FLAG
    char strBuffer[4096] = {0};

    va_list vlArgs;
    va_start(vlArgs,strOutputString);
#if WIN32 || WIN64
    _vsnprintf(strBuffer,sizeof(strBuffer)-1,strOutputString,vlArgs);
#else
    vsnprintf(strBuffer,sizeof(strBuffer)-1,strOutputString,vlArgs);
#endif
    va_end(vlArgs);

    libqhyccd->OutputQHYCCDDebug(strBuffer);
#endif
}
