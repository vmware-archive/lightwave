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
#define VMDIR_SUPERLOGGING_DEFAULT_CAPACITY 10000

//
// Related registry values.
//
#define VMDIR_REG_KEY_SUPER_LOGGING_ENABLE "SuperLoggingEnabled"
#define VMDIR_REG_KEY_SUPER_LOGGING_CAPACITY "SuperLoggingCapacity"

DWORD
VmDirSuperLogQueryServerState(
    PVMDIR_SUPERLOG_SERVER_DATA *ppServerData
    )
{
    PVMDIR_SUPERLOG_SERVER_DATA pServerData = NULL;
    DWORD dwError = 0;

    dwError = VmDirRpcAllocateMemory(sizeof(VMDIR_SUPERLOG_SERVER_DATA), (PVOID*)&pServerData);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerData->iServerStartupTime = gVmdirGlobals.iServerStartupTime;
    pServerData->iAddCount = VmDirOPStatisticGetCount(LDAP_REQ_ADD);
    pServerData->iBindCount = VmDirOPStatisticGetCount(LDAP_REQ_BIND);
    pServerData->iDeleteCount = VmDirOPStatisticGetCount(LDAP_REQ_DELETE);
    pServerData->iModifyCount = VmDirOPStatisticGetCount(LDAP_REQ_MODIFY);
    pServerData->iSearchCount = VmDirOPStatisticGetCount(LDAP_REQ_SEARCH);
    pServerData->iUnbindCount = VmDirOPStatisticGetCount(LDAP_REQ_UNBIND);

    *ppServerData = pServerData;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VmDirSuperLoggingReadRegistry(
    PDWORD pdwCapacity,
    PBOOLEAN pbEnabled
    )
{
    DWORD dwEnabled = 0;
    DWORD dwCapacity = 0;

    (VOID)VmDirGetRegKeyValueDword(
            VMDIR_CONFIG_PARAMETER_KEY_PATH,
            VMDIR_REG_KEY_SUPER_LOGGING_ENABLE,
            &dwEnabled,
            FALSE);
    (VOID)VmDirGetRegKeyValueDword(
            VMDIR_CONFIG_PARAMETER_KEY_PATH,
            VMDIR_REG_KEY_SUPER_LOGGING_CAPACITY,
            &dwCapacity,
            VMDIR_SUPERLOGGING_DEFAULT_CAPACITY);

    *pdwCapacity = dwCapacity;
    *pbEnabled = !!dwEnabled;
}

static
DWORD
_VmDirWriteSuperlogEntryToEventLog(
    PVMSUPERLOGGING_ENTRY superlogEntry
    )
{
    DWORD dwError = 0;
    PCSTR pszDelimiter = "||";
    PCSTR pszServerName = "localhost";
    PSTR pszMarshalledEntry = NULL;
    static PFEVENTLOG_ADD pfEventLogAdd = NULL;

    if (pfEventLogAdd == NULL)
    {
        dwError = VmDirLoadEventLogLibrary(&pfEventLogAdd);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszMarshalledEntry,
            "%u%s%u%s%u%s%u%s%lu%s%lu%s%s%s%s%s%s%s%s%s",
            superlogEntry->dwOperation, pszDelimiter,
            superlogEntry->dwErrorCode, pszDelimiter,
            superlogEntry->dwClientPort, pszDelimiter,
            superlogEntry->dwServerPort, pszDelimiter,
            superlogEntry->iStartTime, pszDelimiter,
            superlogEntry->iEndTime, pszDelimiter,
            superlogEntry->szLoginDN, pszDelimiter,
            superlogEntry->szClientIPAddress, pszDelimiter,
            superlogEntry->szServerIPAddress, pszDelimiter,
            superlogEntry->Parameter
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pfEventLogAdd(pszServerName, 1, 1, pszMarshalledEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszMarshalledEntry);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirEventLogPublisherThrFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;

    // TODO - replace this hack with proper cookie-based version
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = (PVMDIR_CIRCULAR_BUFFER)pArg;
    DWORD dwCur = pCircularBuffer->dwHead;
    PVMSUPERLOGGING_ENTRY superlogEntry = NULL;

    while (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        if (!dwError)
        {
            if (dwCur == pCircularBuffer->dwHead)
            {
                VmDirSleep(1000);
                continue;
            }
            superlogEntry = (PVMSUPERLOGGING_ENTRY)&pCircularBuffer->CircularBuffer[dwCur * pCircularBuffer->dwElementSize];
        }

        if ((dwError = _VmDirWriteSuperlogEntryToEventLog(superlogEntry)))
        {
            VmDirSleep(1000);
            continue;
        }

        dwCur = (dwCur + 1) % pCircularBuffer->dwCapacity;
    }

    return 0;
}

static
DWORD
_VmDirInitEventLogPublisherThread(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            pThrInfo->bJoinThr,
            _VmDirEventLogPublisherThrFun,
            (PVOID)pCircularBuffer);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

DWORD
VmDirSuperLoggingInit(
    PVMSUPERLOGGING *ppLogger
    )
{
    PVMSUPERLOGGING pLogger = NULL;
    DWORD dwError = 0;
    DWORD dwCapacity = 0;

    dwError = VmDirAllocateMemory(sizeof(*pLogger), (PVOID)&pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

    _VmDirSuperLoggingReadRegistry(&dwCapacity, &pLogger->bEnabled);

    dwError = VmDirCircularBufferCreate(dwCapacity, sizeof(VMSUPERLOGGING_ENTRY), &pLogger->pCircularBuffer);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Note: Keep event log publisher disabled until we have a run-time on/off switch
    if (0)
    {
        dwError = _VmDirInitEventLogPublisherThread(pLogger->pCircularBuffer);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppLogger = pLogger;

cleanup:
    return dwError;

error:
    if (pLogger != NULL)
    {
        VmDirCircularBufferFree(pLogger->pCircularBuffer);
        VmDirFreeMemory(pLogger);
        pLogger = NULL;
    }

    goto cleanup;
}

BOOLEAN
VmDirIsSuperLoggingEnabled(
    PVMSUPERLOGGING pLogger
    )
{
    return pLogger->bEnabled;
}

//
// This silently succeeds if logging's already enabled.
//
DWORD
VmDirEnableSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    pLogger->bEnabled = TRUE;

    (VOID)VmDirSetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_SUPER_LOGGING_ENABLE, TRUE);

    return ERROR_SUCCESS;
}

//
// This silently succeeds if logging's already disabled.
//
DWORD
VmDirDisableSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    pLogger->bEnabled = FALSE;

    (VOID)VmDirSetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_SUPER_LOGGING_ENABLE, FALSE);

    return ERROR_SUCCESS;
}

DWORD
VmDirFlushSuperLogging(
    PVMSUPERLOGGING pLogger
    )
{
    VmDirCircularBufferReset(pLogger->pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmDirGetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    PDWORD pSize
    )
{
    return VmDirCircularBufferGetCapacity(pLogger->pCircularBuffer, pSize);
}

DWORD
VmDirSetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    DWORD dwSize
    )
{
    DWORD dwError = 0;

    dwError = VmDirCircularBufferSetCapacity(pLogger->pCircularBuffer, dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    (VOID)VmDirSetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_SUPER_LOGGING_CAPACITY, dwSize);

error:
    return dwError;
}

static
DWORD
_VmDirLogSearchInformation(
    PVMSUPERLOGGING_ENTRY   pLogEntry,
    VDIR_SUPERLOG_RECORD    srcRec
    )
{
    DWORD dwError = 0;

    if (!pLogEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(srcRec.opInfo.searchInfo.pszAttributes))
    {
        dwError = VmDirStringNCpyA(
                pLogEntry->opInfo.searchInfo.szAttributes,
                sizeof(pLogEntry->opInfo.searchInfo.szAttributes),
                srcRec.opInfo.searchInfo.pszAttributes,
                sizeof(pLogEntry->opInfo.searchInfo.szAttributes) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(srcRec.opInfo.searchInfo.pszBaseDN))
    {
        dwError = VmDirStringNCpyA(
                pLogEntry->opInfo.searchInfo.szBaseDN,
                sizeof(pLogEntry->opInfo.searchInfo.szBaseDN),
                srcRec.opInfo.searchInfo.pszBaseDN,
                sizeof(pLogEntry->opInfo.searchInfo.szBaseDN) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(srcRec.opInfo.searchInfo.pszScope))
    {
        dwError = VmDirStringNCpyA(
                pLogEntry->opInfo.searchInfo.szScope,
                sizeof(pLogEntry->opInfo.searchInfo.szScope),
                srcRec.opInfo.searchInfo.pszScope,
                sizeof(pLogEntry->opInfo.searchInfo.szScope) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(srcRec.opInfo.searchInfo.pszIndexResults))
    {
        dwError = VmDirStringNCpyA(
                pLogEntry->opInfo.searchInfo.szIndexResults,
                sizeof(pLogEntry->opInfo.searchInfo.szIndexResults),
                srcRec.opInfo.searchInfo.pszIndexResults,
                sizeof(pLogEntry->opInfo.searchInfo.szIndexResults) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pLogEntry->opInfo.searchInfo.dwScanned = srcRec.opInfo.searchInfo.dwScanned;
    pLogEntry->opInfo.searchInfo.dwReturned = srcRec.opInfo.searchInfo.dwReturned;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirLogOperation(
    PVMSUPERLOGGING pLogger,
    DWORD dwOperation, // LDAP_REQ_XXXX
    VDIR_CONNECTION *pConn,
    DWORD dwErrorCode
    )
{
    PVMSUPERLOGGING_ENTRY pLogEntry = NULL;
    DWORD dwError = 0;

    //
    // Because the structures involved are always valid we don't need to have the lock here.
    // If logging is enabled here but then disabled by the time we grab the lock below we'll
    // unnecessarily write a log entry but there's no other harm involved. This allows us
    // to not grab the lock at all when logging is disabled, which will be the most
    // common state.
    //
    if (!VmDirIsSuperLoggingEnabled(pLogger))
    {
        return;
    }

    pLogEntry = (PVMSUPERLOGGING_ENTRY)VmDirCircularBufferGetNextEntry(pLogger->pCircularBuffer);

    memset(pLogEntry, 0, sizeof(*pLogEntry));
    pLogEntry->dwOperation = dwOperation;
    pLogEntry->dwErrorCode = dwErrorCode;
    pLogEntry->iStartTime = pConn->SuperLogRec.iStartTime;
    pLogEntry->iEndTime = pConn->SuperLogRec.iEndTime;

    dwError = VmDirStringCpyA(pLogEntry->szClientIPAddress, sizeof(pLogEntry->szClientIPAddress), pConn->szClientIP);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringCpyA(pLogEntry->szServerIPAddress, sizeof(pLogEntry->szServerIPAddress), pConn->szServerIP);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLogEntry->dwClientPort = pConn->dwClientPort;
    pLogEntry->dwServerPort = pConn->dwServerPort;

    if (pConn->SuperLogRec.pszBindID != NULL)
    {
        dwError = VmDirStringNCpyA(pLogEntry->szLoginDN, sizeof(pLogEntry->szLoginDN), pConn->SuperLogRec.pszBindID, sizeof(pLogEntry->szLoginDN) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pConn->SuperLogRec.pszOperationParameters != NULL)
    {
        dwError = VmDirStringNCpyA(pLogEntry->Parameter, sizeof(pLogEntry->Parameter), pConn->SuperLogRec.pszOperationParameters, sizeof(pLogEntry->Parameter) - 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    switch (dwOperation)
    {
    case LDAP_REQ_SEARCH:
        dwError = _VmDirLogSearchInformation(pLogEntry, pConn->SuperLogRec);
        BAIL_ON_VMDIR_ERROR(dwError);
        break;
    default:
        break;
    }

cleanup:
    return;

error:
    goto cleanup;
}

static
DWORD
_VmDirSuperLoggingAllocString(
    PWSTR *ppDestinationString,
    PCSTR pSourceString
    )
{
    DWORD dwError = 0;
    PWSTR pwsz = NULL;

    assert(ppDestinationString && pSourceString);

    dwError = VmDirAllocateStringWFromA(pSourceString, &pwsz);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateStringW(pwsz, ppDestinationString);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VMDIR_SAFE_FREE_MEMORY(pwsz);
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
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION DestinationBuffer;

    //
    // The passed in start time; we want entries newer than this.
    //
    UINT64 StartTime;
} LOG_SELECT_CONTEXT, *PLOG_SELECT_CONTEXT;

static
DWORD
_CopyLogSearchInformation(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION row,
    VMSUPERLOGGING_SEARCH_INFO searchInfo
    )
{
    DWORD dwError = 0;

    if (!row)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirSuperLoggingAllocString(
            &row->opInfo.searchInfo.pwszAttributes,
            VDIR_SAFE_STRING(searchInfo.szAttributes));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(
            &row->opInfo.searchInfo.pwszBaseDN,
            VDIR_SAFE_STRING(searchInfo.szBaseDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(
            &row->opInfo.searchInfo.pwszScope,
            VDIR_SAFE_STRING(searchInfo.szScope));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(
            &row->opInfo.searchInfo.pwszIndexResults,
            VDIR_SAFE_STRING(searchInfo.szIndexResults));
    BAIL_ON_VMDIR_ERROR(dwError);

    row->opInfo.searchInfo.dwScanned = searchInfo.dwScanned;
    row->opInfo.searchInfo.dwReturned = searchInfo.dwReturned;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
BOOLEAN
CopyLogEntryCallback(
    PVOID Element,
    PVOID Context)
{
    PVMSUPERLOGGING_ENTRY Entry = (PVMSUPERLOGGING_ENTRY)Element;
    PLOG_SELECT_CONTEXT LogContext = (PLOG_SELECT_CONTEXT)Context;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION row = NULL;
    DWORD dwError = 0;

    //
    // We're not interested in this entry, but keep enumerating.
    //
    if (LogContext->StartTime >= Entry->iStartTime)
    {
        return TRUE;
    }

    row = LogContext->DestinationBuffer;

    dwError = _VmDirSuperLoggingAllocString(&row->pwszLoginDN, Entry->szLoginDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(&row->pwszOperation, VmDirGetOperationStringFromTag(Entry->dwOperation));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(&row->pwszString, Entry->Parameter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(&row->pwszClientIP, Entry->szClientIPAddress);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSuperLoggingAllocString(&row->pwszServerIP, Entry->szServerIPAddress);
    BAIL_ON_VMDIR_ERROR(dwError);

    row->dwClientPort = Entry->dwClientPort;
    row->dwServerPort = Entry->dwServerPort;
    row->dwErrorCode = Entry->dwErrorCode;
    row->iStartTime = Entry->iStartTime;
    row->iEndTime = Entry->iEndTime;
    row->opType = Entry->dwOperation;

    switch (Entry->dwOperation)
    {
    case LDAP_REQ_SEARCH:
        dwError = _CopyLogSearchInformation(row, Entry->opInfo.searchInfo);
        BAIL_ON_VMDIR_ERROR(dwError);
        break;
    default:
        break;
    }

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

VOID
UpdateEnumerationCookie(
    UINT64 *puEnumerationHandle,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pCollection
    )
{
    //
    // Update the cookie to the start time of the last entry we copied out.
    //
    if (pCollection->dwCount != 0)
    {
        *puEnumerationHandle = pCollection->entries[pCollection->dwCount - 1].iStartTime;
    }
}

static
DWORD
_VmDirCopyLogEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *puEnumerationHandle,
    DWORD dwCount,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries
    )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pCollection = NULL;
    LOG_SELECT_CONTEXT Context = { 0 };

    dwError = VmDirRpcAllocateMemory(sizeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY), (PVOID*)&pCollection);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCollection->entries = NULL;
    pCollection->dwCount = 0;

    if (dwCount != 0)
    {
        //
        // We allocate a buffer for the total size the client requested, even though
        // there might not be that many eligible records. This is faster than walking
        // the list twice (once to count how many we have, and again to copy them).
        //
        dwError = VmDirRpcAllocateMemory(sizeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION) * dwCount, (PVOID*)&pCollection->entries);
        pCollection->dwCount = dwCount;
        BAIL_ON_VMDIR_ERROR(dwError);

        Context.DestinationBuffer = pCollection->entries;
        Context.StartTime = *puEnumerationHandle;
        Context.dwDesiredCount = dwCount;
        Context.dwCount = 0;

        //
        // We have to pass zero for count to walk every element, as we don't know which ones we want (based on puEnumerationHandle).
        //
        dwError = VmDirCircularBufferSelectElements(pLogger->pCircularBuffer, 0, CopyLogEntryCallback, &Context);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = Context.dwError;
        BAIL_ON_VMDIR_ERROR(dwError);

        assert(Context.dwCount <= dwCount);
        pCollection->dwCount = Context.dwCount;

        UpdateEnumerationCookie(puEnumerationHandle, pCollection);
    }

    *ppEntries = pCollection;
cleanup:
    return dwError;

error:
    VmDirFreeSuperLogEntryLdapOperationArray(pCollection);
    goto cleanup;
}

DWORD
VmDirSuperLoggingGetEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *pEnumerationCookie,
    DWORD dwCount,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries
    )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries = NULL;

    //
    // If the client requests zero entries then we give them everything we've got. Note
    // that we do this without the lock. Since the circular buffer can't shrink on its
    // own there are only three possibilities: First, that the count doesn't change
    // when we grab the lock later in _VmDirCopyLogEntries. Second, dwCount is smaller
    // than the actual size. In this case the client won't get all the entries they
    // could, but they can get them on the next call (and this is conceptually fine
    // since the client is always racing with incoming connections / log entries).
    // Third, someone could reset the log and there'll be no entries in the buffer
    // (but dwCount will be non-zero). This case is also fine as the circular buffer
    // protects itself from requests for more data than is present.
    //
    if (dwCount == 0)
    {
        dwError = VmDirCircularBufferGetSize(pLogger->pCircularBuffer, &dwCount);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirCopyLogEntries(pLogger, pEnumerationCookie, dwCount, &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    *ppEntries = pEntries;
    return dwError;

error:
    pEntries = NULL;
    goto cleanup;
}
