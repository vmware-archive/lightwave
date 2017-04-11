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
int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwardHost
    );

static
DWORD
VmDnsForwarderAppend(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwarders
    );

static
DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
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

DWORD
VmDnsForwarderInit(
    PVMDNS_FORWARDER_CONETXT*   ppForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_CONETXT pForwarderContext = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppForwarder, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_CONETXT),
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
    PVMDNS_FORWARDER_CONETXT    pForwarder
    )
{
    if (pForwarder)
    {
        DWORD i;
        for (i = 0; i < pForwarder->dwCount; ++i)
        {
            VMDNS_SAFE_FREE_STRINGA(pForwarder->ppszForwarders[i]);
        }

        VMDNS_FREE_RWLOCK(pForwarder->pLock);
        VMDNS_SAFE_FREE_MEMORY(pForwarder);
    }
}

PVMDNS_FORWARDER_CONETXT
VmDnsGetForwarderContext(
    )
{
    return gpSrvContext->pForwarderContext;
}

DWORD
VmDnsGetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* pszForwarders = NULL;
    DWORD dwCount = 0, i = 0;
    BOOL bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppszForwarders, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwCount, dwError);

    VMDNS_LOCKREAD(pForwarder->pLock);
    bLocked = TRUE;

    dwCount = pForwarder->dwCount;

    dwError = VmDnsAllocateMemory(
                        (dwCount + 1) * sizeof(PSTR),
                        (PVOID*)&pszForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; i < dwCount; ++i)
    {
        dwError = VmDnsAllocateStringA(
                            pForwarder->ppszForwarders[i],
                            &pszForwarders[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pdwCount = dwCount;
    *pppszForwarders = pszForwarders;

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(pForwarder->pLock);
    }
    return dwError;

error:
    VmDnsFreeStringArrayA(pszForwarders);

    goto cleanup;
}

DWORD
VmDnsSetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
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
        VMDNS_SAFE_FREE_STRINGA(pForwarder->ppszForwarders[i]);
        pForwarder->ppszForwarders[i] = 0;
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
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

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

        dwError = VmDnsStoreSaveForwarders(
                        pForwarder->dwCount,
                        pForwarder->ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
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
VmDnsDeleteForwarder(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    DWORD index = 0;
    BOOL bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    VMDNS_LOCKREAD(pForwarder->pLock);
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

        dwError = VmDnsStoreSaveForwarders(
                            pForwarder->dwCount,
                            pForwarder->ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(pForwarder->pLock);
    }
    return dwError;

error:

    goto cleanup;
}

static int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwardHost
    )
{
    int index = -1;
    DWORD i = 0;

    for (i = 0; i < pForwarder->dwCount; ++i)
    {
        int match = VmDnsStringCompareA(
                                pForwarder->ppszForwarders[i],
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
    PVMDNS_FORWARDER_CONETXT    pForwarder,
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

    pForwarder->ppszForwarders[dwCount++] = szForwarderCopy;
    pForwarder->dwCount = dwCount;
    szForwarderCopy = NULL;

cleanup:

    return dwError;
error:

    VMDNS_SAFE_FREE_STRINGA(szForwarderCopy);

    goto cleanup;
}

DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    int                         nIndex
    )
{
    DWORD dwError = 0;
    PSTR szTemp = NULL;

    if ((DWORD)nIndex >= pForwarder->dwCount)
    {
        dwError = ERROR_OUT_OF_RANGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    szTemp = pForwarder->ppszForwarders[nIndex];
    pForwarder->ppszForwarders[nIndex] = NULL;

#ifndef WIN32
    memmove(&pForwarder->ppszForwarders[nIndex],
            &pForwarder->ppszForwarders[nIndex+1],
            sizeof(PSTR) * (pForwarder->dwCount - nIndex + 1));
#else
    memmove_s(&pForwarder->ppszForwarders[nIndex],
            sizeof(PSTR) * (pForwarder->dwCount - nIndex),
            &pForwarder->ppszForwarders[nIndex + 1],
            sizeof(PSTR) * (pForwarder->dwCount - (nIndex + 1)));
#endif
    pForwarder->ppszForwarders[--pForwarder->dwCount] = NULL;

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsForwarderResolveRequest(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
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

    if ((bUseUDP && dwQuerySize > VMDNS_UDP_PACKET_SIZE) ||
        !pQueryBuffer ||
        !pdwResponseSize ||
        !ppResopnse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    for (; index < pForwarder->dwCount; ++index)
    {
        if (pResponse)
        {
            VmDnsFreeMemory(pResponse);
            pResponse = NULL;
        }

        dwError = VmDnsForwardRequest(
                        pForwarder->ppszForwarders[index],
                        bUseUDP,
                        dwQuerySize,
                        pQueryBuffer,
                        &dwResponseSize,
                        &pResponse);
        if (dwError != ERROR_SUCCESS)
        {
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
                continue;
            }
            else
            {
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

    dwError = VmwSockOpenClient(
                    pszForwarder,
                    VMW_DNS_PORT,
                    falgs,
                    &pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockSetTimeOut(
                    pSocket,
                    VMDNS_FORWARDER_TIMEOUT
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockGetAddress(
                    pSocket,
                    &address,
                    &addLenth);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockAllocateIoBuffer(
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

    dwError = VmwSockWrite(
                    pSocket,
                    (struct sockaddr*)&address,
                    addLenth,
                    pIoRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bUseUDP)
    {
        dwError = VmwSockAllocateIoBuffer(
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

        dwError = VmwSockRead(
                        pSocket,
                        pIoDataResponse);
       BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        sizeof(UINT16),
                        &pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockRead(
                        pSocket,
                        pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        usExpectedSize = htons(*((UINT*)pIoSizeResponse->pData));

        dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        usExpectedSize,
                        &pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockRead(
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
        VmwSockRelease(pSocket);
    }
    if (pIoRequest)
    {
        VmwSockReleaseIoBuffer(pIoRequest);
    }
    if (pIoSizeResponse)
    {
        VmwSockReleaseIoBuffer(pIoSizeResponse);
    }
    if (pIoDataResponse)
    {
        VmwSockReleaseIoBuffer(pIoDataResponse);
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
    //PVMDNS_MESSAGE pDnsMessage = NULL;
    //PVMDNS_UPDATE_MESSAGE pUpdateDnsMessage = NULL;

    if (!pdwResponseCode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    //dwError = VmDnsGetDnsMessage(
    //                      pResponseBytes,
    //                      dwResponseSize,
    //                      &pDnsMessage,
    //                      &pUpdateDnsMessage
    //                      );
    //BAIL_ON_VMDNS_ERROR(dwError);

    //*pdwResponseCode = pDnsMessage->pHeader->codes.RCODE;

cleanup:

    //VmDnsFreeDnsMessage(pDnsMessage);
    //VmDnsFreeDnsUpdateMessage(pUpdateDnsMessage);

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
