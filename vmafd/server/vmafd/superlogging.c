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

//
// The default log size.
//
#define VMAFD_SUPERLOGGING_DEFAULT_CAPACITY     10000
#define VMAFD_THREAD_RETRY_WAIT_TIME            1000

//
// Related registry values.
//
#define VMAFD_REG_KEY_SUPER_LOGGING_ENABLE      "SuperLoggingEnabled"
#define VMAFD_REG_KEY_SUPER_LOGGING_CAPACITY    "SuperLoggingCapacity"
#define VMAFD_MAX_EAGAIN_RETRY                  5

static
VOID
_VmAfdSuperLoggingReadRegistry(
    PDWORD pdwCapacity,
    PBOOLEAN pbEnabled
    );

static
DWORD
_VmAfdSrvThrInit(
    PVMSUPERLOG_THREAD_INFO   pThrInfo,
    pthread_mutex_t*    pAltMutex,
    PVMAFD_COND         pAltCond,
    BOOLEAN             bJoinFlag
    );

static
DWORD
_VmAfdCreateThread(
    pthread_t* pThread,
    BOOLEAN bDetached,
    VmAfdStartRoutine* pStartRoutine,
    PVOID pArgs
    );

static
PVOID
ThreadFunction(
    PVOID pArgs
    );

static
VOID
_VmAfdSrvThrAdd(
    PVMSUPERLOG_THREAD_INFO   pThrInfo
    );


static
DWORD
_VmAfdCopyLogEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *puEnumerationHandle,
    DWORD dwCount,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    );

static
VOID
_VmAfdSuperLoggingReadRegistry(
    PDWORD pdwCapacity,
    PBOOLEAN pbEnabled
    );

static
DWORD
_VmAfdWriteSuperlogEntryToEventLog(
    PVMAFD_SUPERLOG_ENTRY superlogEntry
    );

static
DWORD
_VmAfdEventLogPublisherThrFun(
    PVOID pArg
    );

static
DWORD
_VmAfdInitEventLogPublisherThread(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

static
VOID
_VmAfdSrvThrAdd(
    PVMSUPERLOG_THREAD_INFO   pThrInfo
    );

static
DWORD
_VmAfdCopyLogEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *puEnumerationHandle,
    DWORD dwCount,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    );


static
BOOLEAN
CopyLogEntryCallback(
    PVOID Element,
    PVOID Context
    );



DWORD
VmAfdSuperLoggingInit(
    PVMSUPERLOGGING *ppLogger
    )
{

    PVMSUPERLOGGING pLogger = NULL;
    DWORD dwError = 0;
    DWORD dwCapacity = 0;

    dwError = VmAfdAllocateMemory(sizeof(*pLogger), (PVOID)&pLogger);
    BAIL_ON_VMAFD_ERROR(dwError);

    _VmAfdSuperLoggingReadRegistry(
                    &dwCapacity,
                    &pLogger->bEnabled);

    dwError = VmAfdCircularBufferCreate(
                    dwCapacity,
                    sizeof(VMAFD_SUPERLOG_ENTRY), &pLogger->pCircularBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Note: Keep event log publisher disabled until we have a run-time on/off switch
    if (0)
    {
        dwError = _VmAfdInitEventLogPublisherThread(pLogger->pCircularBuffer);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppLogger = pLogger;

   VmAfdLog(VMAFD_DEBUG_ANY, "Super Logger object is created.");

cleanup:
    return dwError;

error:
    if (pLogger != NULL)
    {
        VmAfdCircularBufferFree(pLogger->pCircularBuffer);
        VmAfdFreeMemory(pLogger);
        pLogger = NULL;
    }

    goto cleanup;
}



BOOLEAN
VmAfdIsSuperLoggingEnabled(
    PVMSUPERLOGGING pLogger
    )
{
    return pLogger->bEnabled;
}

//
// This silently succeeds if logging's already enabled.
//
DWORD
VmAfdEnableSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    DWORD dwError = 0;
    pLogger->bEnabled = TRUE;

    dwError = _VmAfdConfigSetInteger(
                    VMAFD_REG_KEY_SUPER_LOGGING_ENABLE,
                    1);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

//
// This silently succeeds if logging's already disabled.
//
DWORD
VmAfdDisableSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    DWORD dwError = 0;
    pLogger->bEnabled = FALSE;

    dwError = _VmAfdConfigSetInteger(
                    VMAFD_REG_KEY_SUPER_LOGGING_ENABLE,
                    0);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdFlushSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    VmAfdCircularBufferReset(pLogger->pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmAfdGetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    PDWORD pSize
    )
{
    return VmAfdCircularBufferGetSize(pLogger->pCircularBuffer, pSize);
}

DWORD
VmAfdSetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    DWORD dwSize
    )
{
    DWORD dwError = 0;

    dwError = VmAfdCircularBufferSetCapacity(pLogger->pCircularBuffer, dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    _VmAfdConfigSetInteger(VMAFD_REG_KEY_SUPER_LOGGING_CAPACITY, dwSize);

error:
    return dwError;
}


typedef struct {
    //
    // The number of entries that we've found.
    //
    DWORD dwCount;

    //
    // The number of entries that the caller has requested.
    //
    DWORD dwDesiredCount;

    //
    // Used to propagate any error out of the callback.
    //
    DWORD dwError;

    //
    // Where to write the next matching entry.
    //
    PVMAFD_SUPERLOG_ENTRY DestinationBuffer;

    //
    // The passed in start time; we want entries newer than this.
    //
    UINT64 StartTime;
} LOG_SELECT_CONTEXT, *PLOG_SELECT_CONTEXT;

/**
*  To add heart beat events super log entry
**/
DWORD
VmAfdAddHBSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PVMAFD_HB_STATUS_W pHBEntry,
    DWORD dwErrorCode
    )
{
    PVMAFD_SUPERLOG_ENTRY pLogEntry = NULL;
    DWORD dwError = 0;

    // AFD should not fail if super logging is disabled. This is why returning 0 here
    if (!VmAfdIsSuperLoggingEnabled(pLogger))
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(!pHBEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pLogEntry = (PVMAFD_SUPERLOG_ENTRY)VmAfdCircularBufferGetNextEntry(pLogger->pCircularBuffer);

    memset(pLogEntry, 0, sizeof(*pLogEntry));

    pLogEntry->iStartTime = iStartTime;
    pLogEntry->iEndTime = iEndTime;
    pLogEntry->dwErrorCode = dwErrorCode;

    pLogEntry->bHBIsAlive = pHBEntry->bIsAlive;

cleanup:
    return dwError;
error:
    goto cleanup;
}

/**
*  To add CDC DB events super log entry
**/
DWORD
VmAfdAddDBSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DB_ENTRY_W pDBEntry,
    DWORD dwErrorCode
    )
{
    PVMAFD_SUPERLOG_ENTRY pLogEntry = NULL;
    DWORD dwError = 0;
    PSTR pszDCName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszSiteName = NULL;

    // AFD should not fail if super logging is disabled. This is why returning 0 here
    if (!VmAfdIsSuperLoggingEnabled(pLogger))
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(!pDBEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pLogEntry = (PVMAFD_SUPERLOG_ENTRY)VmAfdCircularBufferGetNextEntry(pLogger->pCircularBuffer);

    memset(pLogEntry, 0, sizeof(*pLogEntry));

    pLogEntry->iStartTime = iStartTime;
    pLogEntry->iEndTime = iEndTime;
    pLogEntry->dwErrorCode = dwErrorCode;

    pLogEntry->bCDCIsAlive = pDBEntry->bIsAlive;
    pLogEntry->dwCDCLastPing = pDBEntry->dwLastPing;
    pLogEntry->dwCDCPingTime = pDBEntry->dwPingTime;

    if(pDBEntry->pszDCName)
    {
        dwError = VmAfdAllocateStringAFromW(pDBEntry->pszDCName, &pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDCName, VMAFD_MAX_DN_LEN, pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDBEntry->pszDomainName)
    {
        dwError = VmAfdAllocateStringAFromW(pDBEntry->pszDomainName, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDomainName, VMAFD_MAX_DN_LEN, pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDBEntry->pszSiteName)
    {
        dwError = VmAfdAllocateStringAFromW(pDBEntry->pszSiteName, &pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszSiteName, VMAFD_MAX_DN_LEN, pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszSiteName);

    return dwError;
error:
    goto cleanup;
}

/**
* To add log entry for PCDC_DC_INFO_W
**/
DWORD
VmAfdAddDCSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DC_INFO_W pDCEntry,
    DWORD dwErrorCode
    )
{
    PVMAFD_SUPERLOG_ENTRY pLogEntry = NULL;
    DWORD dwError = 0;
    PSTR pszDCName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszSiteName = NULL;
    PSTR pszDCAddress = NULL;

    // AFD should not fail if super logging is disabled. This is why returning 0 here
    if (!VmAfdIsSuperLoggingEnabled(pLogger))
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(!pDCEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pLogEntry = (PVMAFD_SUPERLOG_ENTRY)VmAfdCircularBufferGetNextEntry(pLogger->pCircularBuffer);

    memset(pLogEntry, 0, sizeof(*pLogEntry));

    pLogEntry->iStartTime = iStartTime;
    pLogEntry->iEndTime = iEndTime;
    pLogEntry->dwErrorCode = dwErrorCode;

    //pLogEntry->dwDCAddressType = pDCEntry->DcAddressType;

    if(pDCEntry->pszDCName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDCName, &pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDCName, VMAFD_MAX_DN_LEN, pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry->pszDomainName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDomainName, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDomainName, VMAFD_MAX_DN_LEN, pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry->pszDcSiteName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDcSiteName, &pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszSiteName, VMAFD_MAX_DN_LEN, pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry->pszDCAddress)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDCAddress, &pszDCAddress);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDCAddress, VMAFD_MAX_DN_LEN, pszDCAddress);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszSiteName);
    VMAFD_SAFE_FREE_STRINGA(pszDCAddress);

    return dwError;

error:

    goto cleanup;
}

/**
* To add log entry for PCDC_DC_INFO_W
**/
DWORD
VmAfdAddCDCSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DC_INFO_W pDCEntry,
    CDC_DC_STATE dwState,
    DWORD dwErrorCode
    )
{
    PVMAFD_SUPERLOG_ENTRY pLogEntry = NULL;
    DWORD dwError = 0;
    PSTR pszDCName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszSiteName = NULL;
    PSTR pszDCAddress = NULL;

    // AFD should not fail if super logging is disabled. This is why returning 0 here
    if (!VmAfdIsSuperLoggingEnabled(pLogger))
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(!pDCEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pLogEntry = (PVMAFD_SUPERLOG_ENTRY)VmAfdCircularBufferGetNextEntry(pLogger->pCircularBuffer);

    if(!pLogEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    memset(pLogEntry, 0, sizeof(*pLogEntry));

    pLogEntry->iStartTime = iStartTime;
    pLogEntry->iEndTime = iEndTime;
    pLogEntry->dwErrorCode = dwErrorCode;
    pLogEntry->dwState = dwState;


    if(pDCEntry && pDCEntry->pszDCName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDCName, &pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDCName, VMAFD_MAX_DN_LEN, pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry && pDCEntry->pszDomainName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDomainName, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDomainName, VMAFD_MAX_DN_LEN, pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry && pDCEntry->pszDcSiteName)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDcSiteName, &pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszSiteName, VMAFD_MAX_DN_LEN, pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pDCEntry && pDCEntry->pszDCAddress)
    {
        dwError = VmAfdAllocateStringAFromW(pDCEntry->pszDCAddress, &pszDCAddress);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCpyA(pLogEntry->pszDCAddress, VMAFD_MAX_DN_LEN, pszDCAddress);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszSiteName);
    VMAFD_SAFE_FREE_STRINGA(pszDCAddress);

    return dwError;

error:

    goto cleanup;
}


DWORD
VmAfdSuperLoggingGetEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *pEnumerationCookie,
    DWORD dwCount,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    )
{
    DWORD dwError = 0;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pEntries = NULL;

    if(!pLogger || !pEnumerationCookie || !ppEntries)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    //
    // If the client requests zero entries then we give them everything we've got. Note
    // that we do this without the lock. Since the circular buffer can't shrink on its
    // own there are only three possibilities: First, that the count doesn't change
    // when we grab the lock later in _VmAfdCopyLogEntries. Second, dwCount is smaller
    // than the actual size. In this case the client won't get all the entries they
    // could, but they can get them on the next call (and this is conceptually fine
    // since the client is always racing with incoming connections / log entries).
    // Third, someone could reset the log and there'll be no entries in the buffer
    // (but dwCount will be non-zero). This case is also fine as the circular buffer
    // protects itself from requests for more data than is present.
    //
    if (dwCount == 0)
    {
        dwError = VmAfdCircularBufferGetSize(pLogger->pCircularBuffer, &dwCount);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = _VmAfdCopyLogEntries(pLogger, pEnumerationCookie, dwCount, &pEntries);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppEntries = pEntries;

cleanup:
    return dwError;

error:
    if(pEntries)
    {
        pEntries = NULL;
    }

    goto cleanup;
}


VOID
VmAfdFreeSuperLogEntryArray(
    PVMAFD_SUPERLOG_ENTRY_ARRAY pEntries
    )
{
    if (pEntries)
    {
        VMAFD_SAFE_FREE_MEMORY(pEntries->entries);
        VMAFD_SAFE_FREE_MEMORY(pEntries);
    }
}

/*****
*
* Beginning of static functions implementation
*
*****/

static
DWORD
_VmAfdCreateThread(
    pthread_t* pThread,
    BOOLEAN bDetached,
    VmAfdStartRoutine* pStartRoutine,
    PVOID pArgs
)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PVMAFD_THREAD_START_INFO    pThreadStartInfo = NULL;
    pthread_attr_t              thrAttr;
    BOOLEAN                     bThreadAttrInited = FALSE;
    int                         iRetryCnt = 0;

    if ( ( pThread == NULL ) || ( pStartRoutine == NULL ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if( bDetached != FALSE )
    {
        pthread_attr_init(&thrAttr);
        bThreadAttrInited = TRUE;
        pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    }

    dwError = VmAfdAllocateMemory(
        sizeof(VMAFD_THREAD_START_INFO),
        ((PVOID*)&pThreadStartInfo)
    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pThreadStartInfo->pStartRoutine = pStartRoutine;
    pThreadStartInfo->pArgs = pArgs;

    do
    {
        dwError = pthread_create(
                        pThread,
                        ((bDetached == FALSE) ? NULL : &thrAttr),
                        ThreadFunction,
                        pThreadStartInfo
                        );
        if ( dwError == EAGAIN )    // no resources, retry after 1 second pause
        {
            iRetryCnt++ ;
            VmAfdSleep(VMAFD_THREAD_RETRY_WAIT_TIME); // sleep one second
        }
        else
        {
            iRetryCnt = VMAFD_MAX_EAGAIN_RETRY;
        }
    } while ( iRetryCnt < VMAFD_MAX_EAGAIN_RETRY );
    BAIL_ON_VMAFD_ERROR(dwError);

    // we started successfully -> pThreadStartInfo is now owned by
    // ThreadFunction
    pThreadStartInfo = NULL;

error:

    if(bThreadAttrInited != FALSE)
    {
        pthread_attr_destroy(&thrAttr);
    }

    VMAFD_SAFE_FREE_MEMORY( pThreadStartInfo );

    return dwError;
}

static
PVOID
ThreadFunction(
    PVOID pArgs
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMAFD_START_ROUTINE pThreadStart = NULL;
    PVOID pThreadArgs = NULL;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal;

    memset(&retVal, 0, sizeof(retVal));
    if( pArgs == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pThreadStart = ((PVMAFD_THREAD_START_INFO)pArgs)->pStartRoutine;
    pThreadArgs = ((PVMAFD_THREAD_START_INFO)pArgs)->pArgs;

    if( pThreadStart == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMAFD_SAFE_FREE_MEMORY( pArgs );

    dwError = pThreadStart( pThreadArgs );
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    VMAFD_SAFE_FREE_MEMORY( pArgs );

    retVal.dwError = dwError;
    return retVal.pvRet;
}


static
DWORD
_VmAfdSrvThrInit(
    PVMSUPERLOG_THREAD_INFO   pThrInfo,
    pthread_mutex_t*    pAltMutex,
    PVMAFD_COND         pAltCond,
    BOOLEAN             bJoinFlag
    )
{
    DWORD dwError = 0;

    if (pAltMutex && pAltMutex != pThrInfo->mutex)
    {
        pThrInfo->mutexUsed = pAltMutex;
    }
    else
    {
        dwError = pthread_mutex_init(pThrInfo->mutex, NULL);
        BAIL_ON_VMAFD_ERROR(dwError);
        pThrInfo->mutexUsed = pThrInfo->mutex;
    }

    if (pAltCond && pAltCond != pThrInfo->condition)
    {
        pThrInfo->conditionUsed = pAltCond;
    }
    else
    {
        dwError = VmAfdAllocateMemory(sizeof(VMAFD_COND), ((PVOID*)&pAltCond));
        BAIL_ON_VMAFD_ERROR(dwError);
        pThrInfo->conditionUsed = pThrInfo->condition;
    }

    pThrInfo->bJoinThr = bJoinFlag;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
VOID
_VmAfdSuperLoggingReadRegistry(
    PDWORD pdwCapacity,
    PBOOLEAN pbEnabled
    )
{
    DWORD dwEnabled = 0;
    DWORD dwCapacity = 0;
    DWORD dwError = 0;

    dwError = _VmAfdConfigGetInteger(
                    VMAFD_REG_KEY_SUPER_LOGGING_ENABLE,
                    &dwEnabled);
    if(dwError != ERROR_SUCCESS)
    {
        dwEnabled = 0;
    }

    VmAfdLog(VMAFD_DEBUG_ANY, "Registry key value for Super Logging: %d", dwEnabled);

    dwError = _VmAfdConfigGetInteger(
                    VMAFD_REG_KEY_SUPER_LOGGING_CAPACITY,
                    &dwCapacity);
    if(dwError != ERROR_SUCCESS)
    {
        dwCapacity = VMAFD_SUPERLOGGING_DEFAULT_CAPACITY;
    }

    *pdwCapacity = dwCapacity;
    *pbEnabled = dwEnabled;
}

static
DWORD
_VmAfdWriteSuperlogEntryToEventLog(
    PVMAFD_SUPERLOG_ENTRY superlogEntry
    )
{
    DWORD dwError = 0;
    PCSTR pszDelimiter = "||";
    PSTR pszMarshalledEntry = NULL;

    dwError = VmAfdAllocateStringPrintf(
            &pszMarshalledEntry,
            "%u%s%u%s%u%s%lu%s",
            superlogEntry->dwErrorCode, pszDelimiter,
            superlogEntry->dwCDCLastPing, pszDelimiter,
            superlogEntry->iStartTime, pszDelimiter,
            superlogEntry->iEndTime, pszDelimiter
            );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMarshalledEntry);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmAfdEventLogPublisherThrFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;

    PVMAFD_CIRCULAR_BUFFER pCircularBuffer = (PVMAFD_CIRCULAR_BUFFER)pArg;
    DWORD dwCur = pCircularBuffer->dwHead;
    PVMAFD_SUPERLOG_ENTRY superlogEntry = NULL;

    while (VmAfdStatus() != VMAFD_STATUS_STOPPED)
    {
        if (!dwError)
        {
            if (dwCur == pCircularBuffer->dwHead)
            {
                VmAfdSleep(VMAFD_THREAD_RETRY_WAIT_TIME);
                continue;
            }
            superlogEntry = (PVMAFD_SUPERLOG_ENTRY)&pCircularBuffer->CircularBuffer[dwCur * pCircularBuffer->dwElementSize];
        }

        if ((dwError = _VmAfdWriteSuperlogEntryToEventLog(superlogEntry)))
        {
            VmAfdSleep(VMAFD_THREAD_RETRY_WAIT_TIME);
            continue;
        }

        dwCur = (dwCur + 1) % pCircularBuffer->dwCapacity;
    }

    return 0;
}

static
DWORD
_VmAfdInitEventLogPublisherThread(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    DWORD dwError = 0;
    PVMSUPERLOG_THREAD_INFO pThrInfo = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(*pThrInfo),
                    (PVOID)&pThrInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdSrvThrInit(pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdCreateThread(&pThrInfo->tid, FALSE, _VmAfdEventLogPublisherThrFun, (PVOID)pCircularBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    _VmAfdSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:

    if (pThrInfo)
    {
        VMAFD_SAFE_FREE_MEMORY(pThrInfo);
    }

    goto cleanup;
}

static
VOID
_VmAfdSrvThrAdd(
    PVMSUPERLOG_THREAD_INFO   pThrInfo
    )
{
    pthread_mutex_lock(&gVmafdGlobals.mutex);

    if (gVmafdGlobals.pSrvThrInfo)
    {
        pThrInfo->pNext = gVmafdGlobals.pSrvThrInfo;
    }
    gVmafdGlobals.pSrvThrInfo = pThrInfo;

    pthread_mutex_unlock(&gVmafdGlobals.mutex);
}

static
DWORD
_VmAfdCopyLogEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *puEnumerationHandle,
    DWORD dwCount,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    )
{
    DWORD dwError = 0;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pCollection = NULL;
    LOG_SELECT_CONTEXT Context = { 0 };

    dwError = VmAfdAllocateMemory(sizeof(VMAFD_SUPERLOG_ENTRY_ARRAY), (PVOID*)&pCollection);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCollection->entries = NULL;
    pCollection->dwCount = dwCount;

    if (dwCount > 0)
    {
        //
        // We allocate a buffer for the total size the client requested, even though
        // there might not be that many eligible records. This is faster than walking
        // the list twice (once to count how many we have, and again to copy them).
        //
        dwError = VmAfdAllocateMemory(sizeof(VMAFD_SUPERLOG_ENTRY) * dwCount, (PVOID*)&pCollection->entries);
        pCollection->dwCount = dwCount;
        BAIL_ON_VMAFD_ERROR(dwError);

        Context.DestinationBuffer = pCollection->entries;
        Context.StartTime = *puEnumerationHandle;
        Context.dwDesiredCount = dwCount;
        Context.dwCount = 0;

        //
        // We have to pass zero for count to walk every element, as we don't know which ones we want (based on puEnumerationHandle).
        //
        dwError = VmAfdCircularBufferSelectElements(pLogger->pCircularBuffer, 0, CopyLogEntryCallback, &Context);
        BAIL_ON_VMAFD_ERROR(dwError);

        Context.dwError = 0;
        pCollection->dwCount = pLogger->pCircularBuffer->dwSize;

    }

    *ppEntries = pCollection;
cleanup:
    return dwError;

error:
    VmAfdFreeSuperLogEntryArray(pCollection);
    goto cleanup;
}

static
BOOLEAN
CopyLogEntryCallback(
    PVOID Element,
    PVOID Context
    )
{
    PVMAFD_SUPERLOG_ENTRY Entry = (PVMAFD_SUPERLOG_ENTRY)Element;
    PLOG_SELECT_CONTEXT LogContext = (PLOG_SELECT_CONTEXT)Context;
    PVMAFD_SUPERLOG_ENTRY row = NULL;
    DWORD dwError = 0;

    //
    // We're not interested in this entry, but keep enumerating.
    //
    if (LogContext->StartTime >= Entry->iStartTime)
    {
        return TRUE;
    }

    row = LogContext->DestinationBuffer;

    dwError = VmAfdStringCpyA((PSTR)&row->pszDCName, VMAFD_MAX_DN_LEN, Entry->pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringCpyA((PSTR)&row->pszDomainName, VMAFD_MAX_DN_LEN, Entry->pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringCpyA((PSTR)&row->pszDCAddress, VMAFD_MAX_DN_LEN, Entry->pszDCAddress);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringCpyA((PSTR)&row->pszSiteName, VMAFD_MAX_DN_LEN, Entry->pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    row->bCDCIsAlive = Entry->bCDCIsAlive;
    row->bHBIsAlive = Entry->bHBIsAlive;
    row->dwCDCLastPing = Entry->dwCDCLastPing;
    row->dwCDCPingTime = Entry->dwCDCPingTime;
    row->dwHBCount = Entry->dwHBCount;
    row->dwState = Entry->dwState;
    row->dwErrorCode = Entry->dwErrorCode;
    row->iStartTime = Entry->iStartTime;
    row->iEndTime = Entry->iEndTime;

    LogContext->dwCount += 1;
    LogContext->dwDesiredCount -= 1;
    LogContext->DestinationBuffer += 1;

    //
    // Stop enumerating once we've read as many entries as the user requested.
    //
    return (LogContext->dwDesiredCount != 0);

error:
    LogContext->dwError = dwError;

    return FALSE;
}
