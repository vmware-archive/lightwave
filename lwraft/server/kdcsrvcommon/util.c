/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */



#include "includes.h"

/*
 * Linux/Win32 thread-safe version of ctime.
 * Make sure input "buf" is 26 characters or more.
 * The WIN32 implementation assumes 32 bytes.
 */
const char *VmKdcCtimeTS(
    const time_t *tvalIn,
    char buf[])
{
    const char *ctimeFmt = NULL;
#ifdef _WIN32
    if (ctime_s(buf, 32, tvalIn) != 0)
    {
        ctimeFmt = (const char *) tvalIn;
    }
#else
    ctimeFmt = ctime_r(tvalIn, buf);
#endif
    return ctimeFmt;
}

/*
 * VmKdcGenOriginatingTimeStr(): Generates the current Universal Time (UTC) in YYYYMMDDHHMMSSmmm format
 * Return value:
 *      0: Indicates that the call succeeded.
 *     -1: Indicates that an error occurred, and errno is set to indicate the error.
 */

#define NSECS_PER_MSEC        1000000

int
VmKdcGenOriginatingTimeStr(
    char * timeStr)
{
#ifndef _WIN32
    struct timespec tspec = {0};
    struct tm       tmTime = {0};
    int             retVal = 0;

    retVal = clock_gettime( CLOCK_REALTIME, &tspec );
    BAIL_ON_VMKDC_ERROR(retVal);

    if (gmtime_r(&tspec.tv_sec, &tmTime) == NULL)
    {
        retVal = errno;
        BAIL_ON_VMKDC_ERROR(retVal);
    }
    snprintf( timeStr, VMKDC_ORIG_TIME_STR_LEN, "%4d%02d%02d%02d%02d%02d.%03d",
              tmTime.tm_year+1900,
              tmTime.tm_mon+1,
              tmTime.tm_mday,
              tmTime.tm_hour,
              tmTime.tm_min,
              tmTime.tm_sec,
              (int)(tspec.tv_nsec/NSECS_PER_MSEC));

cleanup:
    return retVal;

error:
    goto cleanup;
#else
    int retVal = 0;
    SYSTEMTIME sysTime = {0};

    GetSystemTime( &sysTime );

    if( _snprintf_s(
        timeStr,
        VMKDC_ORIG_TIME_STR_LEN,
        VMKDC_ORIG_TIME_STR_LEN-1,
        "%4d%02d%02d%02d%02d%02d.%03d",
        sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
        sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds
        ) == -1 )
    {
        retVal = -1;
    }

    return retVal;
#endif
}

void
VmKdcCurrentGeneralizedTime(
    PSTR    pszTimeBuf,
    int     iBufSize)
{
#ifndef _WIN32
    time_t      tNow = time(NULL);
    struct tm   tmpTm = {0};

    assert (pszTimeBuf);

    gmtime_r(&tNow, &tmpTm);

    snprintf(pszTimeBuf, iBufSize, "%04d%02d%02d%02d%02d%02d.0Z",
            tmpTm.tm_year + 1900,
            tmpTm.tm_mon + 1,
            tmpTm.tm_mday,
            tmpTm.tm_hour,
            tmpTm.tm_min,
            tmpTm.tm_sec);

    return;
#else
    SYSTEMTIME sysTime = {0};

    GetSystemTime( &sysTime );

    _snprintf_s(
        pszTimeBuf,
        iBufSize,
        iBufSize-1,
        "%04d%02d%02d%02d%02d%02d.0Z",
        sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
        sysTime.wMinute, sysTime.wSecond
        );

#endif

}

VOID
VmKdcSleep(
    DWORD dwMilliseconds
)
{
#ifndef _WIN32
    sleep( dwMilliseconds/1000 );
#else
    Sleep( dwMilliseconds );
#endif
}
