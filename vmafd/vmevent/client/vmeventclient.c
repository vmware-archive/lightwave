/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

// EventLogClient.c : Defines the exported functions for the DLL application.
#include "stdafx.h"

static
DWORD
EventLogDCEGetErrorCode(
    dcethread_exc* pDceException
    );

static
DWORD
EventLogMarshallEventContainer(
    PEVENTLOG_CONTAINER pEventContainer,
    PBYTE               pBuffer,
    size_t              curOffset,
    size_t*             pBytesRequired,
    size_t*             pBytesWritten
    );

static
DWORD
EventLogMarshallEventEntry(
    PEVENTLOG_ENTRY pAppEntry,     /* IN              */
    PEVENTLOG_ENTRY pAppEntryCopy, /* IN OUT OPTIONAL */
    PBYTE           pBuffer,       /* IN OUT OPTIONAL */
    size_t          curOffset,     /* IN              */
    size_t*         pBytesRequired,/* IN OUT OPTIONAL */
    size_t*         pBytesWritten  /* IN OUT OPTIONAL */
    );


static
VOID
EventLogRpcFreeEventContainer(
    PEVENTLOG_CONTAINER pEventContainer
    );

static
VOID
EventLogRpcFreeEventEntryContents(
    PEVENTLOG_ENTRY pEventEntry
    );


static
DWORD
EventLogDCEGetErrorCode(
    dcethread_exc* pDceException
    )
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus (pDceException);

    return dwError;
}

#ifndef _WIN32
__attribute__ ((constructor))
#endif
DWORD
EventLogInitialize()
{
    setlocale(LC_ALL, "");
    return 0;
}

DWORD
EventLogAdd(
    RP_PCSTR pszServerName,
    DWORD dwEventID,
    DWORD dwEventType,
    RP_PCSTR pszMessage
    )
{
    handle_t BindingHandle = NULL;
    DWORD dwError = 0;
    RP_PWSTR pwszMessage = NULL;

    dwError = EventLogCreateBindingHandleA(
                    pszServerName,
                    NULL,
                    &BindingHandle
                    );
    BAIL_ON_ERROR(dwError);

    dwError = EventLogAllocateStringWFromA(
                        pszMessage,
                        &pwszMessage
                        );
    BAIL_ON_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = RpcEventLogAdd(
                BindingHandle,
                dwEventID,
                dwEventType,
                pwszMessage);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = EventLogDCEGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_ERROR(dwError);

error:

    if (BindingHandle) {
        EventLogFreeBindingHandle(&BindingHandle);
    }
    if (pwszMessage) {
        EventLogFreeStringW(pwszMessage);
    }
    return dwError;
}

DWORD
EventLogInitEnumEventsHandle(
    RP_PCSTR pszServerName,
    PDWORD   pdwHandle
    )
{
    handle_t BindingHandle = NULL;
    DWORD dwError = 0;

    dwError = EventLogCreateBindingHandleA(
                    pszServerName,
                    NULL,
                    &BindingHandle
                    );
    BAIL_ON_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = RpcEventLogInitEnumHandle(
                BindingHandle,
                pdwHandle);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = EventLogDCEGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_ERROR(dwError);

error:

    if (BindingHandle) {
        EventLogFreeBindingHandle( &BindingHandle);
    }
    return dwError;
}

DWORD
EventLogEnumEvents(
    RP_PCSTR pszServerName,
    DWORD dwHandle,
    DWORD dwStartIndex,
    DWORD dwNumEvents,
    PEVENTLOG_CONTAINER * ppEventContainer
    )
{
    DWORD dwError = 0;
    handle_t BindingHandle = NULL;
    PEVENTLOG_CONTAINER pEventContainer = NULL;
    PBYTE  pBuffer = NULL;
    size_t bufferSize = 0;

    if (!ppEventContainer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = EventLogCreateBindingHandleA(
                    pszServerName,
                    NULL,
                    &BindingHandle
                    );
    BAIL_ON_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = RpcEventLogEnumEvents(
                BindingHandle,
                dwHandle,
                dwStartIndex,
                dwNumEvents,
                &pEventContainer);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = EventLogDCEGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_ERROR(dwError);


    if(pEventContainer->dwCount > dwNumEvents)
    {
        dwError = pEventContainer->dwCount;
        BAIL_ON_ERROR(dwError);
    }

    dwError = EventLogMarshallEventContainer(
                    pEventContainer,
                    NULL,
                    0,
                    &bufferSize,
                    NULL);
    BAIL_ON_ERROR(dwError);

    dwError = EventLogAllocateMemory((DWORD)bufferSize, (PVOID*)&pBuffer);
    BAIL_ON_ERROR(dwError);

    dwError = EventLogMarshallEventContainer(
                    pEventContainer,
                    pBuffer,
                    0,
                    NULL,
                    NULL);
    BAIL_ON_ERROR(dwError);

    *ppEventContainer = (PEVENTLOG_CONTAINER)pBuffer;

cleanup:

    if (pEventContainer)
    {
        EventLogRpcFreeEventContainer(pEventContainer);
    }

    if (BindingHandle)
    {
        EventLogFreeBindingHandle( &BindingHandle);
    }

    return dwError;

error:

    if (ppEventContainer)
    {
        *ppEventContainer = NULL;
    }

    if (pBuffer)
    {
        EventLogFreeMemory(pBuffer);
    }

    goto cleanup;
}

static
DWORD
EventLogMarshallEventContainer(
    PEVENTLOG_CONTAINER pEventContainer,   /* IN              */
    PBYTE               pBuffer,           /* IN OUT OPTIONAL */
    size_t              curOffset,         /* IN              */
    size_t*             pBytesRequired,    /* IN OUT OPTIONAL */
    size_t*             pBytesWritten      /* IN OUT OPTIONAL */
    )
{
    DWORD dwError = 0;
    size_t bytesWritten = 0;
    size_t requiredSize = curOffset;
    PBYTE  pCursor = pBuffer;

    if (pEventContainer)
    {
        PEVENTLOG_CONTAINER pContainer = NULL;

        requiredSize += EVENTLOG_ALIGN_BYTES(requiredSize);
        requiredSize += sizeof(*pEventContainer);

        if (pCursor)
        {
            pCursor += EVENTLOG_ALIGN_BYTES(requiredSize);

            pContainer = (PEVENTLOG_CONTAINER)pCursor;

            pContainer->dwCount = pEventContainer->dwCount;
            pContainer->dwTotalCount = pEventContainer->dwTotalCount;

            bytesWritten += sizeof(*pEventContainer);
            pCursor      += sizeof(*pEventContainer);
        }

        if (pEventContainer->dwCount)
        {
            DWORD  iEntry = 0;
            size_t tmpSize = 0;

            tmpSize = EVENTLOG_ALIGN_BYTES(requiredSize);

            if (pCursor)
            {
                pCursor += tmpSize;
                bytesWritten += tmpSize;

                pContainer->pPkgEntries = (PEVENTLOG_ENTRY)pCursor;
            }

            requiredSize += tmpSize;

            tmpSize = sizeof(pEventContainer->pPkgEntries[0]) *
                        pEventContainer->dwCount;

            requiredSize += tmpSize;

            if (pCursor)
            {
                pCursor += tmpSize;
                bytesWritten += tmpSize;
            }

            for (; iEntry < pEventContainer->dwCount ; iEntry++)
            {
                size_t nWritten = 0;

                dwError = EventLogMarshallEventEntry(
                                &pEventContainer->pPkgEntries[iEntry],
                                pContainer ?
                                &pContainer->pPkgEntries[iEntry] : NULL,
                                pCursor,
                                requiredSize,
                                &requiredSize,
                                &nWritten);
                BAIL_ON_ERROR(dwError);

                if (pCursor)
                {
                    pCursor += nWritten;
                    bytesWritten += nWritten;
                }
            }
        }
    }

    if (pBytesRequired)
    {
        *pBytesRequired = requiredSize;
    }
    if (pBytesWritten)
    {
        *pBytesWritten = bytesWritten;
    }

cleanup:

    return dwError;

error:

    if (pBytesRequired)
    {
        *pBytesRequired = 0;
    }
    if (pBytesWritten)
    {
        *pBytesWritten = 0;
    }

    goto cleanup;
}

static
DWORD
EventLogMarshallEventEntry(
    PEVENTLOG_ENTRY pEventEntry,      /* IN              */
    PEVENTLOG_ENTRY pEventEntryCopy,  /* IN OUT OPTIONAL */
    PBYTE           pBuffer,          /* IN OUT OPTIONAL */
    size_t          curOffset,        /* IN              */
    size_t*         pBytesRequired,   /* IN OUT OPTIONAL */
    size_t*         pBytesWritten     /* IN OUT OPTIONAL */
    )
{
    DWORD  dwError = 0;
    size_t bytesWritten = 0;
    size_t requiredSize = curOffset;
    typedef enum
    {
        MEMBER_TYPE_W16STRING
    } MemberType;

    struct
    {
        MemberType type;
        PWSTR  pwszSrc;
        PWSTR* ppwszDst;
        size_t     size;
    }
    values[] =
    {
        {
            VMEVENT_SF_INIT(.type, MEMBER_TYPE_W16STRING),
            VMEVENT_SF_INIT(.pwszSrc, pEventEntry->pszMessage),
            VMEVENT_SF_INIT(.ppwszDst, (pEventEntryCopy ? &pEventEntryCopy->pszMessage : NULL))
        }
    };
    DWORD iValue = 0;
    PBYTE pCursor = pBuffer;

    for (; iValue < sizeof(values)/sizeof(values[0]); iValue++)
    {
        switch (values[iValue].type)
        {
            case MEMBER_TYPE_W16STRING:

                if (values[iValue].pwszSrc)
                {
                    PWSTR pwszSrc = values[iValue].pwszSrc;
                    size_t tmpSize = 0;
                    size_t valueLength = 0;

                    tmpSize += EVENTLOG_ALIGN_BYTES(requiredSize);

                    dwError = EventLogGetStringLengthW(pwszSrc, &valueLength);

                    BAIL_ON_ERROR(dwError);

                    tmpSize += (valueLength + 1) * sizeof(WCHAR);

                    if (pCursor)
                    {
                        pCursor += EVENTLOG_ALIGN_BYTES(requiredSize);

                        *values[iValue].ppwszDst = (PWSTR)pCursor;

                        if (valueLength > 0)
                        {
                            memcpy(
                                pCursor,
                                (PBYTE)pwszSrc,
                                valueLength * sizeof(WCHAR));

                            pCursor += valueLength * sizeof(WCHAR);
                        }

                        pCursor += sizeof(WCHAR);

                        bytesWritten += tmpSize;
                    }

                    requiredSize += tmpSize;
                }

                break;
        }
    }

    if (pEventEntryCopy)
    {
        pEventEntryCopy->dwEventType = pEventEntry->dwEventType;
        pEventEntryCopy->dwEventId   = pEventEntry->dwEventId;
    }

    if (pBytesRequired)
    {
        *pBytesRequired = requiredSize;
    }
    if (pBytesWritten)
    {
        *pBytesWritten = bytesWritten;
    }

cleanup:

    return dwError;

error:

    if (pBytesRequired)
    {
        *pBytesRequired = 0;
    }
    if (pBytesWritten)
    {
        *pBytesWritten = 0;
    }

    goto cleanup;
}

static
VOID
EventLogRpcFreeEventContainer(
    PEVENTLOG_CONTAINER pEventContainer
    )
{
    if (pEventContainer->dwCount > 0)
    {
        DWORD iEntry = 0;

        for (; iEntry < pEventContainer->dwCount; iEntry++)
        {
            PEVENTLOG_ENTRY pEventEntry = &pEventContainer->pPkgEntries[iEntry];

            EventLogRpcFreeEventEntryContents(pEventEntry);
        }
    }

    EVENTLOG_RPC_SAFE_FREE_MEMORY(pEventContainer);
}

static
VOID
EventLogRpcFreeEventEntryContents(
    PEVENTLOG_ENTRY pEventEntry
    )
{
    EVENTLOG_RPC_SAFE_FREE_MEMORY(pEventEntry->pszMessage);
}

