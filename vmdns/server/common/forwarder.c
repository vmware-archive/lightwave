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
* Module Name:  forward.c
*
* Abstract: VMware Domain Name Service.
*
* DNS forwarding routines
*/

#include "includes.h"

static
DWORD
VmDnsGetForwarders_inlock(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    );

static
int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwardHost
    );

static
DWORD
VmDnsForwarderAppend(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarders
    );

static
DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    int                         nIndex
    );

static
DWORD
VmDnsForwardRequest(
    PCSTR       pszForwarder,
    BOOL        bUseUDP,
    DWORD       dwQuerySize,
    PBYTE       pQueryBuffer,
    PDWORD      pdwResponseSize,
    PBYTE*      ppResopnse
    );

static
DWORD
VmDnsPeekResponseCode(
    DWORD       dwResponseSize,
    PBYTE       pResponseBytes,
    PDWORD      pdwResponseCode
    );

static
BOOLEAN
VmDnsValidateForwarder(
    PCSTR       pszForwarder
    );

static
VOID
VmDnsFreeForwarderEntry(
    PVMDNS_FORWARDER_ENTRY pForwarder
    );

static
DWORD
VmDnsForwarderMetricsInit(
    PVMDNS_FORWARDER_ENTRY   pForwarder
    );

static
VOID
VmDnsForwarderMetricsUpdate(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry,
    UINT64                    duration,
    DWORD                     opCode
    );

static
DWORD
VmDnsForwarderMetricsDelete(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry
    );

DWORD
VmDnsForwarderInit(
    PVMDNS_FORWARDER_CONTEXT*   ppForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_CONTEXT pForwarderContext = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppForwarder, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_CONTEXT),
                        (PVOID*)&pForwarderContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pForwarderContext->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppForwarder = pForwarderContext;

cleanup:
    return dwError;

error:

    if (pForwarderContext)
    {
        VmDnsForwarderCleanup(pForwarderContext);
    }

    goto cleanup;
}

VOID
VmDnsForwarderCleanup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder
    )
{
    DWORD i = 0;

    if (pForwarder)
    {
        for (i = 0; i < pForwarder->dwCount; ++i)
        {
            if (pForwarder->pForwarderEntries[i])
            {
            VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[i]);
            }
        }
        VMDNS_FREE_RWLOCK(pForwarder->pLock);
        VMDNS_SAFE_FREE_MEMORY(pForwarder);
    }
}

PVMDNS_FORWARDER_CONTEXT
VmDnsGetForwarderContext(
    )
{
    return gpSrvContext->pForwarderContext;
}

DWORD
VmDnsGetForwarders(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    VMDNS_LOCKREAD(pForwarder->pLock);
    bLocked = TRUE;

    dwError = VmDnsGetForwarders_inlock(
                         pForwarder,
                         pppszForwarders,
                         pdwCount
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(pForwarder->pLock);
    }
    return dwError;

error:

    goto cleanup;
}


DWORD
VmDnsSetForwarders(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    DWORD                       dwCount,
    PSTR*                       ppszForwarders
    )
{
    DWORD dwError = 0;
    DWORD dwCurrentCount, i = 0;
    BOOL bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszForwarders, dwError);

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    dwCurrentCount = pForwarder->dwCount;
    for (i = 0; i < dwCurrentCount; ++i)
    {
        VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[i]);
    }

    pForwarder->dwCount = 0;

    for (i = 0; i < dwCount; ++i)
    {
        dwError = VmDnsForwarderAppend(
                            pForwarder,
                            ppszForwarders[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }
    return dwError;

error:

    goto cleanup;
}


DWORD
VmDnsAddForwarder(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    if (!VmDnsValidateForwarder(pszForwarder))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    int index = VmDnsForwarderLookup(
                        pForwarder,
                        pszForwarder);
    if (index < 0)
    {
        dwError = VmDnsForwarderAppend(
                        pForwarder,
                        pszForwarder);
        BAIL_ON_VMDNS_ERROR(dwError);

        VMDNS_UNLOCKWRITE(pForwarder->pLock);
        bLocked = FALSE;

        dwError = VmDnsGetForwarders_inlock(
                        pForwarder,
                        &ppszForwarders,
                        &dwCount);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsStoreSaveForwarders(
                        pForwarder->dwCount,
                        ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDeleteForwarder(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    DWORD index = 0;
    BOOL bLocked = FALSE;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    index = VmDnsForwarderLookup(
                            pForwarder,
                            pszForwarder);
    if (index != -1)
    {
        dwError = VmDnsForwarderRemoveAt(
                            pForwarder,
                            index);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsGetForwarders_inlock(
                            pForwarder,
                            &ppszForwarders,
                            &dwCount);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsStoreSaveForwarders(
                            pForwarder->dwCount,
                            ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsGetForwarders_inlock(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* pszForwarders = NULL;
    DWORD dwCount = 0, i = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppszForwarders, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwCount, dwError);

    dwCount = pForwarder->dwCount;

    dwError = VmDnsAllocateMemory(
                        (dwCount + 1) * sizeof(PSTR),
                        (PVOID*)&pszForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; i < dwCount; ++i)
    {
        dwError = VmDnsAllocateStringA(
                            pForwarder->pForwarderEntries[i]->pszForwarder,
                            &pszForwarders[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pdwCount = dwCount;
    *pppszForwarders = pszForwarders;

cleanup:

    return dwError;

error:
    VmDnsFreeStringArrayA(pszForwarders);

    goto cleanup;
}

static int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwardHost
    )
{
    int index = -1;
    DWORD i = 0;

    for (i = 0; i < pForwarder->dwCount; ++i)
    {
        int match = VmDnsStringCompareA(
                                pForwarder->pForwarderEntries[i]->pszForwarder,
                                pszForwardHost,
                                FALSE);
        if (match == 0)
        {
            index = i;
            break;
        }
    }

    return index;
}

DWORD
VmDnsForwarderAppend(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR szForwarderCopy = NULL;
    DWORD dwCount = pForwarder->dwCount;

    if (dwCount >= VMDNS_MAX_NUM_FORWARDS)
    {
        dwError = ERROR_OUT_OF_RANGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(pszForwarder, &szForwarderCopy);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_ENTRY),
                        (void**)&pForwarder->pForwarderEntries[dwCount]);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarder->pForwarderEntries[dwCount]->pszForwarder = szForwarderCopy;
    szForwarderCopy = NULL;

    dwError = VmDnsForwarderMetricsInit(pForwarder->pForwarderEntries[dwCount]);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarder->dwCount++;

cleanup:

    return dwError;
error:

    if (pForwarder->pForwarderEntries[dwCount])
    {
        VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[dwCount]);
    }
    VMDNS_SAFE_FREE_STRINGA(szForwarderCopy);

    goto cleanup;
}

DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    int                         nIndex
    )
{
    DWORD dwError = 0;

    if ((DWORD)nIndex >= pForwarder->dwCount)
    {
        dwError = ERROR_OUT_OF_RANGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsForwarderMetricsDelete(pForwarder->pForwarderEntries[nIndex]);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarder->pForwarderEntries[nIndex] = NULL;


#ifndef WIN32
    memmove(&pForwarder->pForwarderEntries[nIndex],
            &pForwarder->pForwarderEntries[nIndex+1],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - (nIndex + 1)));
#else
    memmove_s(&pForwarder->pForwarderEntries[nIndex],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - nIndex),
            &pForwarder->pForwarderEntries[nIndex + 1],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - (nIndex + 1)));
#endif
    pForwarder->pForwarderEntries[--pForwarder->dwCount] = NULL;

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsForwarderResolveRequest(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    BOOL                        bUseUDP,
    BOOL                        bRecusive,
    DWORD                       dwQuerySize,
    PBYTE                       pQueryBuffer,
    PDWORD                      pdwResponseSize,
    PBYTE*                      ppResopnse,
    PUCHAR                      prCode
    )
{
    DWORD dwError = 0;
    DWORD index = 0;
    DWORD dwResponseSize = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseCode = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    PVMDNS_HEADER pDnsHeader = NULL;
    UINT64 startTime = 0;
    UINT64 endTime = 0;

    startTime = VmDnsGetTimeInMilliSec();

    if ((bUseUDP && dwQuerySize > VMDNS_UDP_PACKET_SIZE) ||
        !pQueryBuffer ||
        !pdwResponseSize ||
        !ppResopnse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDnsMessageBuffer(
                                    pQueryBuffer,
                                    dwQuerySize,
                                    &pDnsMessageBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDnsHeaderFromBuffer(
                                    pDnsMessageBuffer,
                                    &pDnsHeader);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; index < pForwarder->dwCount; ++index)
    {
        startTime = VmDnsGetTimeInMilliSec();

        if (pResponse)
        {
            VmDnsFreeMemory(pResponse);
            pResponse = NULL;
        }

        dwError = VmDnsForwardRequest(
                        pForwarder->pForwarderEntries[index]->pszForwarder,
                        bUseUDP,
                        dwQuerySize,
                        pQueryBuffer,
                        &dwResponseSize,
                        &pResponse);
        if (dwError != ERROR_SUCCESS)
        {
            endTime = VmDnsGetTimeInMilliSec();
            VmDnsForwarderMetricsUpdate(
                          pForwarder->pForwarderEntries[index],
                          endTime-startTime,
                          pDnsHeader->codes.opcode);
            continue;
        }

        if (dwResponseSize > 0 && pResponse)
        {
            dwError = VmDnsPeekResponseCode(
                            dwResponseSize,
                            pResponse,
                            &dwResponseCode);

            if (dwError != ERROR_SUCCESS ||
                dwResponseCode != VM_DNS_RCODE_NOERROR)
            {
                endTime = VmDnsGetTimeInMilliSec();
                VmDnsForwarderMetricsUpdate(
                              pForwarder->pForwarderEntries[index],
                              endTime-startTime,
                              pDnsHeader->codes.opcode);
                continue;
            }
            else
            {
                endTime = VmDnsGetTimeInMilliSec();
                VmDnsForwarderMetricsUpdate(
                              pForwarder->pForwarderEntries[index],
                              endTime-startTime,
                              pDnsHeader->codes.opcode);
                break;
            }
        }
    }

    *ppResopnse = pResponse;
    *pdwResponseSize = dwResponseSize;
    *prCode = (UCHAR)dwResponseCode;

cleanup:

    return dwError;

error :

    if (pResponse)
    {
        VmDnsFreeMemory(pResponse);
    }

    goto cleanup;
}

DWORD
VmDnsForwardRequest(
    PCSTR       pszForwarder,
    BOOL        bUseUDP,
    DWORD       dwQuerySize,
    PBYTE       pQueryBuffer,
    PDWORD      pdwResponseSize,
    PBYTE*      ppResponse
    )
{
    DWORD dwError = 0;
    PBYTE pResponse = NULL;
    VM_SOCK_CREATE_FLAGS falgs = (bUseUDP) ? VM_SOCK_CREATE_FLAGS_UDP : VM_SOCK_CREATE_FLAGS_TCP;
    PVM_SOCKET pSocket = NULL;
    UINT16 usExpectedSize = 0;
    struct sockaddr_storage address;
    socklen_t addLenth = sizeof address;
    PVM_SOCK_IO_BUFFER pIoRequest = NULL;
    PVM_SOCK_IO_BUFFER pIoSizeResponse = NULL;
    PVM_SOCK_IO_BUFFER pIoDataResponse = NULL;

    if ((bUseUDP && dwQuerySize > VMDNS_UDP_PACKET_SIZE)||
        !pszForwarder ||
        !pQueryBuffer ||
        !pdwResponseSize ||
        !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockOpenClient(
                    pszForwarder,
                    VMW_DNS_PORT,
                    falgs,
                    &pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockSetTimeOut(
                    pSocket,
                    VMDNS_FORWARDER_TIMEOUT
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockGetAddress(
                    pSocket,
                    &address,
                    &addLenth);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockAllocateIoBuffer(
                    VM_SOCK_EVENT_TYPE_UNKNOWN,
                    dwQuerySize,
                    &pIoRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                    pIoRequest->pData,
                    pIoRequest->dwExpectedSize,
                    pQueryBuffer,
                    dwQuerySize);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockWrite(
                    pSocket,
                    (struct sockaddr*)&address,
                    addLenth,
                    pIoRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bUseUDP)
    {
        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        VMDNS_UDP_PACKET_SIZE,
                        &pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);


        dwError = VmDnsCopyMemory(
                          &pIoDataResponse->clientAddr,
                          sizeof pIoDataResponse->clientAddr,
                          &address,
                          addLenth);
        BAIL_ON_VMDNS_ERROR(dwError);

        pIoDataResponse->addrLen = addLenth;

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoDataResponse);
       BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        sizeof(UINT16),
                        &pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        usExpectedSize = htons(*((UINT*)pIoSizeResponse->pData));

        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        usExpectedSize,
                        &pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        pIoDataResponse->dwCurrentSize,
                        (PVOID*)&pResponse);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                      pResponse,
                      pIoDataResponse->dwCurrentSize,
                      pIoDataResponse->pData,
                      pIoDataResponse->dwCurrentSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppResponse = pResponse;
    *pdwResponseSize = pIoDataResponse->dwCurrentSize;

cleanup:

    if (pSocket)
    {
        VmDnsSockRelease(pSocket);
    }
    if (pIoRequest)
    {
        VmDnsSockReleaseIoBuffer(pIoRequest);
    }
    if (pIoSizeResponse)
    {
        VmDnsSockReleaseIoBuffer(pIoSizeResponse);
    }
    if (pIoDataResponse)
    {
        VmDnsSockReleaseIoBuffer(pIoDataResponse);
    }

    return dwError;

error:

    if (pdwResponseSize)
    {
        *pdwResponseSize = 0;
    }

    if (ppResponse)
    {
        *ppResponse = NULL;
    }

    if (pResponse)
    {
        VmDnsFreeMemory(pResponse);
    }

    goto cleanup;
}

static
DWORD
VmDnsPeekResponseCode(
    DWORD dwResponseSize,
    PBYTE pResponseBytes,
    PDWORD pdwResponseCode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HEADER pDnsHeader = NULL;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;

    if (!pdwResponseCode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    dwError = VmDnsGetDnsMessageBuffer(
                        pResponseBytes,
                        dwResponseSize,
                        &pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDnsHeaderFromBuffer(
                        pDnsMessageBuffer,
                        &pDnsHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *pdwResponseCode =  pDnsHeader->codes.RCODE;

cleanup:

    if (pDnsMessageBuffer)
    {
       VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    VMDNS_SAFE_FREE_MEMORY(pDnsHeader);

    return dwError;

error :

    goto cleanup;

}

static
BOOLEAN
VmDnsValidateForwarder(
    PCSTR       pszForwarder
    )
{
    if (!pszForwarder ||
        pszForwarder[0] == '\0' ||
        VmDnsStringChrA(pszForwarder, '.') == NULL
        )
    {
        return FALSE;
    }

    return TRUE;
}

static
VOID
VmDnsFreeForwarderEntry(
    PVMDNS_FORWARDER_ENTRY pForwarder
    )
{
    if (pForwarder)
    {
        VMDNS_SAFE_FREE_STRINGA(pForwarder->pszForwarder);
        VMDNS_SAFE_FREE_MEMORY(pForwarder);
    }
}
static
DWORD
VmDnsForwarderMetricsInit(
    PVMDNS_FORWARDER_ENTRY   pForwarder
    )
{
    DWORD dwError = 0;
    UINT64 buckets[] = {1, 5, 10, 100, 300};
    VM_METRICS_LABEL labelDurationOps[2][2] = {{{"operation","query"},{"forwarder",""}},
                                               {{"operation","update"},{"forwarder",""}}};

    labelDurationOps[0][1].pszValue = pForwarder->pszForwarder;
    labelDurationOps[1][1].pszValue = pForwarder->pszForwarder;

    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
                gVmDnsMetricsContext,
                "vmdns_forwarder_request_duration",
                labelDurationOps[0],
                2,
                "Forwarder Process Request Duration",
                buckets,
                5,
                &pForwarder->ForwarderMetricsContext.pQueryDuration
                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
                gVmDnsMetricsContext,
                "vmdns_forwarder_request_duration",
                labelDurationOps[1],
                2,
                "Forwarder Process Request Duration",
                buckets,
                5,
                &pForwarder->ForwarderMetricsContext.pUpdateDuration
                );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
VOID
VmDnsForwarderMetricsUpdate(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry,
    UINT64                    duration,
    DWORD                     opCode
    )
{
    if (opCode == VM_DNS_OPCODE_QUERY)
    {
        VmMetricsHistogramUpdate(
                    pForwarderEntry->ForwarderMetricsContext.pQueryDuration,
                    VDNS_RESPONSE_TIME(duration)
                    );
    }
    else if (opCode == VM_DNS_OPCODE_UPDATE)
    {
        VmMetricsHistogramUpdate(
                    pForwarderEntry->ForwarderMetricsContext.pUpdateDuration,
                    VDNS_RESPONSE_TIME(duration)
                    );
    }
}

static
DWORD
VmDnsForwarderMetricsDelete(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry
    )
{
    DWORD dwError;

    dwError = VmMetricsHistogramDelete(
                          gVmDnsMetricsContext,
                          pForwarderEntry->ForwarderMetricsContext.pQueryDuration);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramDelete(
                          gVmDnsMetricsContext,
                          pForwarderEntry->ForwarderMetricsContext.pUpdateDuration);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
