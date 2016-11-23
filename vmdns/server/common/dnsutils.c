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

static
VOID
VmDnsFreeBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );


DWORD
VmDnsCreateBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT *ppMessage
    )
{
    DWORD dwError = 0;
    PVMDNS_SOCK_BUF_CONTEXT pMessage = NULL;

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_SOCK_BUF_CONTEXT),
                    (PVOID*)&pMessage
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pMessage->refCount = 1;

    *ppMessage = pMessage;

cleanup:

    return dwError;

error:

    *ppMessage = NULL;

    if (pMessage)
    {
      VmDnsFreeBufferContext(pMessage);
    }
    goto cleanup;
}

PVMDNS_SOCK_BUF_CONTEXT
VmDnsAcquireBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    InterlockedIncrement(&pMessage->refCount);

    return pMessage;
}

DWORD
VmDnsCopyRecords(
    PVMDNS_RECORD   *ppSrc,
    DWORD           dwCount,
    PVMDNS_RECORD   **pppDst
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDNS_RECORD *ppDst = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pppDst, dwError);

    if (dwCount)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_RECORD) * dwCount,
                            (PVOID *)&ppDst
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmDnsDuplicateRecord(
                                ppSrc[dwIndex],
                                &ppDst[dwIndex]
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *pppDst = ppDst;


cleanup:

    return dwError;

error:

    VmDnsFreeRecordsArray(ppDst, dwCount);
    if (pppDst)
    {
        *pppDst = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCopyQuestion(
    PVMDNS_QUESTION     pSrc,
    PVMDNS_QUESTION     *ppDst
    )
{
    DWORD dwError = 0;
    PVMDNS_QUESTION pDst = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDst, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_QUESTION),
                        (PVOID)&pDst
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                        pSrc->pszQName,
                        &pDst->pszQName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pDst->uQType = pSrc->uQType;
    pDst->uQClass = pSrc->uQClass;

    *ppDst = pDst;


cleanup:

    return dwError;

error:

    VmDnsFreeQuestion(pDst);
    if (ppDst)
    {
        *ppDst = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCopyQuestions(
    PVMDNS_QUESTION     *ppSrc,
    DWORD               dwCount,
    PVMDNS_QUESTION     **pppDst
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDNS_QUESTION *ppDst = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppDst, dwError);

    if (dwCount)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_QUESTION) * dwCount,
                            (PVOID*)&ppDst
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmDnsCopyQuestion(
                                ppSrc[dwIndex],
                                &ppDst[dwIndex]
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *pppDst = ppDst;


cleanup:

    return dwError;

error:

    VmDnsFreeQuestions(ppDst, dwCount);
    if (pppDst)
    {
        *pppDst = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCopyZone(
    PVMDNS_UPDATE_ZONE  pSrcZone,
    PVMDNS_UPDATE_ZONE  *ppDstZone
    )
{
    DWORD dwError = 0;
    PVMDNS_UPDATE_ZONE pDstZone = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrcZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDstZone, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_UPDATE_ZONE),
                        (PVOID)&pDstZone
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                        pSrcZone->pszName,
                        &pDstZone->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pDstZone->uType = pSrcZone->uType;
    pDstZone->uClass = pSrcZone->uClass;

    *ppDstZone = pDstZone;


cleanup:

    return dwError;

error:

    VmDnsFreeZone(pDstZone);
    if (ppDstZone)
    {
        *ppDstZone = NULL;
    }

    goto cleanup;
}

VOID
VmDnsReleaseBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    if (InterlockedDecrement(&pMessage->refCount) == 0)
    {
        VmDnsFreeBufferContext(pMessage);
    }
}


VOID
VmDnsFreeQuestion(
    PVMDNS_QUESTION pQuestion
    )
{
    if (pQuestion)
    {
        VMDNS_SAFE_FREE_STRINGA(pQuestion->pszQName);
        VMDNS_SAFE_FREE_MEMORY(pQuestion);
    }
}

VOID
VmDnsFreeQuestions(
    PVMDNS_QUESTION *pQuestions,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;
    if (pQuestions)
    {
        for (; dwIndex < dwCount; ++dwIndex)
        {
            if (pQuestions[dwIndex])
            {
                VmDnsFreeQuestion(pQuestions[dwIndex]);
            }
        }
        VMDNS_SAFE_FREE_MEMORY(pQuestions);
    }
}

VOID
VmDnsFreeZone(
    PVMDNS_UPDATE_ZONE  pZone
    )
{
    if (pZone)
    {
        VMDNS_SAFE_FREE_STRINGA(pZone->pszName);
        VMDNS_SAFE_FREE_MEMORY(pZone);
    }
}

VOID
VmDnsFreeRecordsArray(
    PVMDNS_RECORD *pRecords,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;
    if (pRecords)
    {
        for (; dwIndex < dwCount; ++dwIndex)
        {
            if (pRecords[dwIndex])
            {
                VmDnsClearRecord(pRecords[dwIndex]);
                VMDNS_SAFE_FREE_MEMORY(pRecords[dwIndex]);
            }
        }
        VMDNS_SAFE_FREE_MEMORY(pRecords);
    }
}

VOID
VmDnsCleanupDnsMessage(
    PVMDNS_MESSAGE  pVmDnsMessage
    )
{
    if (pVmDnsMessage)
    {
        if (pVmDnsMessage->pQuestions)
        {
            VmDnsFreeQuestions(
                        pVmDnsMessage->pQuestions,
                        pVmDnsMessage->pHeader->usQDCount
                        );
        }
        if (pVmDnsMessage->pAnswers)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pAnswers,
                        pVmDnsMessage->pHeader->usANCount
                        );
        }
        if (pVmDnsMessage->pAuthority)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pAuthority,
                        pVmDnsMessage->pHeader->usNSCount
                        );
        }
        if (pVmDnsMessage->pAdditional)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pAdditional,
                        pVmDnsMessage->pHeader->usARCount
                        );
        }
        VmDnsFreeBlob(pVmDnsMessage->pRawDnsMessage);
    }
}

VOID
VmDnsFreeDnsMessage(
    PVMDNS_MESSAGE pVmDnsMessage
    )
{
    if (pVmDnsMessage)
    {
        VmDnsCleanupDnsMessage(pVmDnsMessage);
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage->pHeader);
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage);
    }
}

VOID
VmDnsCleanupDnsUpdateMessage(
    PVMDNS_UPDATE_MESSAGE   pVmDnsMessage
    )
{
    if (pVmDnsMessage)
    {
        VmDnsFreeZone(pVmDnsMessage->pZone);
        if (pVmDnsMessage->pPrerequisite)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pPrerequisite,
                        pVmDnsMessage->pHeader->usPRCount
                        );
        }
        if (pVmDnsMessage->pUpdate)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pUpdate,
                        pVmDnsMessage->pHeader->usUPCount
                        );
        }
        if (pVmDnsMessage->pAdditional)
        {
            VmDnsFreeRecordsArray(
                        pVmDnsMessage->pAdditional,
                        pVmDnsMessage->pHeader->usADCount
                        );
        }
        VmDnsFreeBlob(pVmDnsMessage->pRawDnsMessage);
    }
}

VOID
VmDnsFreeDnsUpdateMessage(
    PVMDNS_UPDATE_MESSAGE pVmDnsMessage
)
{
    if (pVmDnsMessage)
    {
        VmDnsCleanupDnsUpdateMessage(pVmDnsMessage);
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage->pHeader);
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage);
    }
}

DWORD
VmDnsGetDnsMessageBuffer(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PVMDNS_MESSAGE_BUFFER *ppDnsMessageBuffer
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;


    if (!pDnsRequest || !dwDnsRequestSize || !ppDnsMessageBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBufferStreamWithBuffer(
                                          pDnsRequest,
                                          dwDnsRequestSize,
                                          0,
                                          FALSE,
                                          &pDnsMessageBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsMessageBuffer = pDnsMessageBuffer;

cleanup:

    return dwError;
error:

    if (ppDnsMessageBuffer)
    {
        *ppDnsMessageBuffer = NULL;
    }
    if (pDnsMessageBuffer)
    {
        VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    goto cleanup;
}


static
VOID
VmDnsFreeBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    if (pMessage->pData)
    {
        VmDnsFreeMemory(pMessage->pData);
    }
    VmDnsFreeMemory(pMessage);
}
