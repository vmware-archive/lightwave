/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

static
DWORD
VmDnsReadHeaderFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER *ppHeader
    );

static
DWORD
VmDnsReadQuestionsFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    DWORD dwCount,
    PVMDNS_QUESTION **pppQuestions
    );

static
DWORD
VmDnsReadRecordsFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    DWORD dwCount,
    PVMDNS_RECORD **pppRecords
    );

DWORD
VmDnsParseMessage(
      PVMDNS_MESSAGE_BUFFER pMessageBuffer,
      PVMDNS_MESSAGE *ppMessage
      )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE pMessage = NULL;
    PVMDNS_HEADER pHeader = NULL;
    PVMDNS_QUESTION *pQuestions = NULL;
    PVMDNS_RECORD *pRecords = NULL;

    if (!pMessageBuffer || !ppMessage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadHeaderFromBuffer(
                              pMessageBuffer,
                              &pHeader
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadQuestionsFromBuffer(
                              pMessageBuffer,
                              pHeader->usQDCount,
                              &pQuestions
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadRecordsFromBuffer(
                              pMessageBuffer,
                              pHeader->usANCount,
                              &pRecords
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                              sizeof(VMDNS_MESSAGE),
                              (PVOID)&pMessage
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    pMessage->pHeader = pHeader;
    pHeader = NULL;
    pMessage->pQuestions = pQuestions;
    pQuestions = NULL;
    pMessage->pRecords = pRecords;
    pRecords = NULL;

    *ppMessage = pMessage;

cleanup:

    return dwError;

error:
    if (ppMessage)
    {
        *ppMessage = NULL;
    }
    if (pMessage)
    {
      VmDnsFreeDnsMessage(pMessage);
    }
    if (pHeader && pQuestions)
    {
        VmDnsFreeQuestions(pQuestions, pHeader->usQDCount);
    }
    if (pHeader && pRecords)
    {
        VmDnsFreeRecordsArray(pRecords, pHeader->usANCount);
    }
    VMDNS_SAFE_FREE_MEMORY(pHeader);
    goto cleanup;
}

DWORD
VmDnsWriteHeaderToBuffer(
    PVMDNS_HEADER pHeader,
    PVMDNS_MESSAGE_BUFFER pMessageBuffer
    )
{
    DWORD dwError = 0;
    UINT16 usCodes = 0;

    if (!pHeader || !pMessageBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteUINT16ToBuffer(
                        pHeader->usId,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    usCodes = VMDNS_FORM_HEADER(pHeader);

    dwError = VmDnsWriteUINT16ToBuffer(
                        usCodes,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pHeader->usQDCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pHeader->usANCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pHeader->usNSCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pHeader->usARCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsReadHeaderFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER *ppHeader
    )
{
    DWORD dwError = 0;
    PVMDNS_HEADER pHeader = NULL;

    if (!pMessageBuffer || !ppHeader)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_HEADER),
                        (PVOID)&pHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        &pHeader->usId
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        (PUINT16)&pHeader->codes
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        &pHeader->usQDCount
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        &pHeader->usANCount
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        &pHeader->usNSCount
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                        pMessageBuffer,
                        &pHeader->usARCount
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppHeader = pHeader;


cleanup:

    return dwError;
error:

    if (ppHeader)
    {
        *ppHeader = NULL;
    }
    VMDNS_SAFE_FREE_MEMORY(pHeader);
    goto cleanup;
}

static
DWORD
VmDnsReadQuestionsFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    DWORD dwCount,
    PVMDNS_QUESTION **pppQuestions
    )
{
    DWORD dwError = 0;
    PVMDNS_QUESTION *ppQuestions = NULL;
    DWORD dwIndex = 0;

    if (!pMessageBuffer || !pppQuestions)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwCount)
    {
        dwError = VmDnsAllocateMemory(
                              sizeof(PVMDNS_QUESTION)*dwCount,
                              (PVOID)&ppQuestions
                              );
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmDnsAllocateMemory(
                             sizeof(VMDNS_QUESTION),
                             (PVOID)&ppQuestions[dwIndex]
                             );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsReadDomainNameFromBuffer(
                                          pMessageBuffer,
                                          &ppQuestions[dwIndex]->pszQName
                                          );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsReadUINT16FromBuffer(
                                      pMessageBuffer,
                                      &ppQuestions[dwIndex]->uQType
                                      );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsReadUINT16FromBuffer(
                                     pMessageBuffer,
                                     &ppQuestions[dwIndex]->uQClass
                                     );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *pppQuestions = ppQuestions;
cleanup:

    return dwError;
error:

    if (pppQuestions)
    {
        *pppQuestions = NULL;
    }
    if (ppQuestions)
    {
        VmDnsFreeQuestions(ppQuestions,dwCount);
    }
    goto cleanup;
}

static
DWORD
VmDnsReadRecordsFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    DWORD dwCount,
    PVMDNS_RECORD **pppRecords
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD *ppRecords = NULL;
    DWORD dwIndex = 0;

    if (!pMessageBuffer || !pppRecords)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwCount)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(PVMDNS_RECORD)*dwCount,
                            (PVOID)&ppRecords
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmDnsReadRecordFromBuffer(
                            pMessageBuffer,
                            &ppRecords[dwIndex]
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *pppRecords = ppRecords;

cleanup:

    return dwError;
error:

    if (pppRecords)
    {
        *pppRecords = NULL;
    }
    if (ppRecords)
    {
        VmDnsFreeRecordsArray(ppRecords,dwCount);
    }

    goto cleanup;
}

