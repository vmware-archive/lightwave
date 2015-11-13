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


/*
 * Module Name:  util.c
 *
 * Abstract: Common utility functions.
 *
 */

#include "includes.h"

#define ENTRYID int

/*
 * Qsort comparison function for char** data type
 */
int
VmDnsQsortPPCHARCmp(
    const void*		ppStr1,
    const void*		ppStr2
    )
{

    if ((ppStr1 == NULL || *(char * const *)ppStr1 == NULL) &&
        (ppStr2 == NULL || *(char * const *)ppStr2 == NULL))
    {
        return 0;
    }

    if (ppStr1 == NULL || *(char * const *)ppStr1 == NULL)
    {
       return -1;
    }

    if (ppStr2 == NULL || *(char * const *)ppStr2 == NULL)
    {
       return 1;
    }

   return VmDnsStringCompareA(* (char * const *) ppStr1, * (char * const *) ppStr2, TRUE);
}

int
VmDnsQsortPEIDCmp(
    const void * pEID1,
    const void * pEID2)
{
    if (pEID1 == NULL  && pEID2 == NULL)
    {
        return 0;
    }

    if (pEID1 == NULL && pEID2 != NULL)
    {
       return -1;
    }

    if (pEID1 != NULL && pEID2 == NULL)
    {
       return 1;
    }

    if (*((ENTRYID *)pEID1) < *((ENTRYID*)pEID2))
    {
        return -1;
    }
    else if (*((ENTRYID *)pEID1) == *((ENTRYID*)pEID2))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}



/*
 * VmDnsGenOriginatingTimeStr(): Generates the current Universal Time (UTC) in YYYYMMDDHHMMSSmmm format
 * Return value:
 *      0: Indicates that the call succeeded.
 *     -1: Indicates that an error occurred, and errno is set to indicate the error.
 */

#define NSECS_PER_MSEC        1000000

int
VmDnsGenOriginatingTimeStr(
    char * timeStr)
{
#ifndef _WIN32
    struct timespec tspec = {0};
    struct tm       tmTime = {0};
    int             retVal = 0;

    retVal = clock_gettime( CLOCK_REALTIME, &tspec );
    BAIL_ON_VMDNS_ERROR(retVal);

    if (gmtime_r(&tspec.tv_sec, &tmTime) == NULL)
    {
        retVal = errno;
        BAIL_ON_VMDNS_ERROR(retVal);
    }
    snprintf( timeStr, VMDNS_ORIG_TIME_STR_LEN, "%4d%02d%02d%02d%02d%02d.%03d",
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
        VMDNS_ORIG_TIME_STR_LEN,
        VMDNS_ORIG_TIME_STR_LEN-1,
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
VmDnsCurrentGeneralizedTime(
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
VmDnsSleep(
    DWORD dwMilliseconds
)
{
#ifndef _WIN32
    sleep( dwMilliseconds/1000 );
#else
    Sleep( dwMilliseconds );
#endif
}

DWORD
VmDnsConfigGetDword(
    PCSTR   pcszValueName,
    DWORD*  pdwOutput
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;
    PVMDNS_CFG_KEY pRootKey = NULL;
    PVMDNS_CFG_KEY pParamsKey = NULL;
    CHAR  szParamsKeyPath[] = VMDNS_CONFIG_PARAMETER_KEY_PATH;

    dwError = VmDnsConfigOpenConnection(&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadDWORDValue(
                    pParamsKey,
                    NULL,
                    pcszValueName,
                    &dwValue);
    BAIL_ON_VMDNS_ERROR(dwError);

    *pdwOutput = dwValue;

cleanup:

    if (pParamsKey)
    {
        VmDnsConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmDnsConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmDnsConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsConfigGetStringA(
    PCSTR   pcszKeyPath,
    PCSTR   pcszValueName,
    PSTR*   ppszOutput
    )
{
    DWORD dwError = 0;
    PSTR  pszValue = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;
    PVMDNS_CFG_KEY pRootKey = NULL;
    PVMDNS_CFG_KEY pParamsKey = NULL;

    dwError = VmDnsConfigOpenConnection(&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pcszKeyPath,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    pcszValueName,
                    &pszValue);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszOutput = pszValue;

cleanup:

    if (pParamsKey)
    {
        VmDnsConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmDnsConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmDnsConfigCloseConnection(pConnection);
    }

    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pszValue);
    goto cleanup;
}
