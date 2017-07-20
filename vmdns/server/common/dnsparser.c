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


#include "includes.h"

static
DWORD
VmDnsExtractHeaderCodes(
    UINT16 uData,
    PVMDNS_HEADER pHeader
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

static
DWORD
VmDnsReadZoneFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_UPDATE_ZONE *ppZone
    );

static
DWORD
VmDnsWriteRecordsToBuffer(
    PVMDNS_RECORD           *ppRecords,
    DWORD                   dwCount,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );

static
DWORD
VmDnsWriteQuestionToBuffer(
    PVMDNS_QUESTION         pQuestion,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );

static
DWORD
VmDnsWriteZoneToBuffer(
    PVMDNS_UPDATE_ZONE      pZone,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );


DWORD
VmDnsReadDnsHeaderFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER *ppHeader
    )
{
    DWORD dwError = 0;
    PVMDNS_HEADER pHeader = NULL;
    UINT16 uTempHeaderBits = 0x00;

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
                        &uTempHeaderBits
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsExtractHeaderCodes(
                        uTempHeaderBits,
                        pHeader
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

    VMDNS_SAFE_FREE_MEMORY(pHeader);
    if (ppHeader)
    {
        *ppHeader = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsParseQueryMessage(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER pHeader,
    PVMDNS_MESSAGE *ppMessage
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE pMessage = NULL;
    PVMDNS_QUESTION *pQuestions = NULL;
    PVMDNS_RECORD *pAnswers = NULL;
    PVMDNS_RECORD *pAuthority = NULL;
    PVMDNS_RECORD *pAdditional = NULL;
    PBYTE pStartOfMessage = NULL;
    UINT16 unRawMsgSize = 0;
    PVMDNS_BLOB pRawDnsMessage = NULL;

    if (!pMessageBuffer || !pHeader || !ppMessage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_MESSAGE),
                        (PVOID)&pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pStartOfMessage = pMessageBuffer->pMessage + pMessageBuffer->szCursor;

    // Read questions section
    dwError = VmDnsReadQuestionsFromBuffer(
                        pMessageBuffer,
                        pHeader->usQDCount,
                        &pQuestions
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read answers section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usANCount,
                        &pAnswers
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read authority section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usNSCount,
                        &pAuthority
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read additional section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usARCount,
                        &pAdditional
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pHeader->usARCount > 0)
    {
        if (pAdditional[pHeader->usARCount - 1]->dwType == VMDNS_RR_MTYPE_TSIG)
        {
            unRawMsgSize = pAdditional[pHeader->usARCount - 1]->Data.TSIG.pRawTsigPtr - pStartOfMessage;

            dwError = VmDnsAllocateBlob(
                                unRawMsgSize,
                                &pRawDnsMessage
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsCopyMemory(
                            pRawDnsMessage->pData,
                            unRawMsgSize,
                            pStartOfMessage,
                            unRawMsgSize
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    pMessage->pHeader = pHeader;
    pMessage->pQuestions = pQuestions;
    pQuestions = NULL;
    pMessage->pAnswers = pAnswers;
    pAnswers = NULL;
    pMessage->pAuthority = pAuthority;
    pAuthority = NULL;
    pMessage->pAdditional = pAdditional;
    pAdditional = NULL;
    pMessage->pRawDnsMessage = pRawDnsMessage;

    *ppMessage = pMessage;


cleanup:

    return dwError;
error:

    if (pHeader && pQuestions)
    {
        VmDnsFreeQuestions(pQuestions, pHeader->usQDCount);
    }
    if (pHeader && pAnswers)
    {
        VmDnsFreeRecordsArray(pAnswers, pHeader->usANCount);
    }
    if (pHeader && pAuthority)
    {
        VmDnsFreeRecordsArray(pAuthority, pHeader->usNSCount);
    }
    if (pHeader && pAdditional)
    {
        VmDnsFreeRecordsArray(pAdditional, pHeader->usNSCount);
    }
    VmDnsFreeBlob(pRawDnsMessage);
    if (pMessage)
    {
        VmDnsFreeDnsMessage(pMessage);
    }
    if (ppMessage)
    {
        *ppMessage = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsParseUpdateMessage(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER pHeader,
    PVMDNS_UPDATE_MESSAGE *ppUpdateMessage
    )
{
    DWORD dwError = 0;
    PVMDNS_UPDATE_MESSAGE pUpdateMessage = NULL;
    PVMDNS_UPDATE_ZONE pZone = NULL;
    PVMDNS_RECORD *pPrereqs = NULL;
    PVMDNS_RECORD *pUpdates = NULL;
    PVMDNS_RECORD *pAdds = NULL;
    PBYTE pStartOfMessage = NULL;
    UINT16 unRawMsgSize = 0;
    PVMDNS_BLOB pRawDnsMessage = NULL;

    if (!pMessageBuffer || !pHeader || !ppUpdateMessage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_UPDATE_MESSAGE),
                    (PVOID)&pUpdateMessage
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pStartOfMessage = pMessageBuffer->pMessage + pMessageBuffer->szCursor;

    // Read zone section
    dwError = VmDnsReadZoneFromBuffer(
                        pMessageBuffer,
                        &pZone
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read prerequisite section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usPRCount,
                        &pPrereqs
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read update section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usUPCount,
                        &pUpdates
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Read additional data section
    dwError = VmDnsReadRecordsFromBuffer(
                        pMessageBuffer,
                        pHeader->usADCount,
                        &pAdds
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pHeader->usADCount > 0)
    {
        if (pAdds[pHeader->usADCount - 1]->dwType == VMDNS_RR_MTYPE_TSIG)
        {
            unRawMsgSize = pAdds[pHeader->usADCount - 1]->Data.TSIG.pRawTsigPtr - pStartOfMessage;

            dwError = VmDnsAllocateBlob(
                                unRawMsgSize,
                                &pRawDnsMessage
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsCopyMemory(
                            pRawDnsMessage->pData,
                            unRawMsgSize,
                            pStartOfMessage,
                            unRawMsgSize
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    pUpdateMessage->pHeader = pHeader;
    pUpdateMessage->pZone = pZone;
    pZone = NULL;
    pUpdateMessage->pPrerequisite = pPrereqs;
    pPrereqs = NULL;
    pUpdateMessage->pUpdate = pUpdates;
    pUpdates = NULL;
    pUpdateMessage->pAdditional = pAdds;
    pAdds = NULL;
    pUpdateMessage->pRawDnsMessage = pRawDnsMessage;

    *ppUpdateMessage = pUpdateMessage;


cleanup:

    return dwError;
error:

    if (pHeader && pPrereqs)
    {
        VmDnsFreeRecordsArray(pPrereqs, pHeader->usPRCount);
    }
    if (pHeader && pUpdates)
    {
        VmDnsFreeRecordsArray(pUpdates, pHeader->usUPCount);
    }
    if(pHeader && pAdds)
    {
        VmDnsFreeRecordsArray(pAdds, pHeader->usADCount);
    }
    VmDnsFreeZone(pZone);
    VmDnsFreeBlob(pRawDnsMessage);
    if (pUpdateMessage)
    {
        VmDnsFreeDnsUpdateMessage(pUpdateMessage);
    }
    if (ppUpdateMessage)
    {
        *ppUpdateMessage = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsWriteDnsHeaderToBuffer(
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

DWORD
VmDnsWriteQueryMessageToBuffer(
    PVMDNS_MESSAGE          pDnsMessage,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;

    if (!pDnsMessage ||
        !pDnsMessage->pHeader ||
        !pMessageBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Write question section
    if (pDnsMessage->pHeader->usQDCount != 0)
    {
        dwError = VmDnsWriteQuestionToBuffer(
                            pDnsMessage->pQuestions[0],
                            pMessageBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Write answers section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsMessage->pAnswers,
                        pDnsMessage->pHeader->usANCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Write authority section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsMessage->pAuthority,
                        pDnsMessage->pHeader->usNSCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Write additional data section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsMessage->pAdditional,
                        pDnsMessage->pHeader->usARCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsWriteUpdateMessageToBuffer(
    PVMDNS_UPDATE_MESSAGE   pDnsUpdateMessage,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;

    if (!pDnsUpdateMessage || !pMessageBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Write zone section
    dwError = VmDnsWriteZoneToBuffer(
                        pDnsUpdateMessage->pZone,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Write prerequisite section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsUpdateMessage->pPrerequisite,
                        pDnsUpdateMessage->pHeader->usPRCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Write update section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsUpdateMessage->pUpdate,
                        pDnsUpdateMessage->pHeader->usUPCount,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Write additional data section
    dwError = VmDnsWriteRecordsToBuffer(
                        pDnsUpdateMessage->pAdditional,
                        pDnsUpdateMessage->pHeader->usADCount,
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
VmDnsExtractHeaderCodes(
    UINT16 uData,
    PVMDNS_HEADER pHeader
    )
{
    DWORD dwError = 0;

    if (!pHeader)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pHeader->codes.QR = VMDNS_GET_BITS(uData,0x0f,0x0f);
    pHeader->codes.opcode = VMDNS_GET_BITS(uData,0x0b,0x0e);
    pHeader->codes.AA = VMDNS_GET_BITS(uData,0x0a,0x0a);
    pHeader->codes.TC = VMDNS_GET_BITS(uData,0x09,0x09);
    pHeader->codes.RD = VMDNS_GET_BITS(uData,0x08,0x08);
    pHeader->codes.RA = VMDNS_GET_BITS(uData,0x07,0x07);
    pHeader->codes.Z = VMDNS_GET_BITS(uData,0x04,0x06);
    pHeader->codes.RCODE = VMDNS_GET_BITS(uData,0x00,0x03);


cleanup:

    return dwError;
error:

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

static
DWORD
VmDnsReadZoneFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_UPDATE_ZONE *ppZone
)
{
    DWORD dwError = 0;
    PVMDNS_UPDATE_ZONE pZone = NULL;

    if (!pMessageBuffer || !ppZone)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_UPDATE_ZONE),
                    (PVOID)&pZone
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDomainNameFromBuffer(
                        pMessageBuffer,
                        &pZone->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                    pMessageBuffer,
                    &pZone->uType
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                    pMessageBuffer,
                    &pZone->uClass
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZone = pZone;

cleanup:

    return dwError;
error:

    VmDnsFreeZone(pZone);

    goto cleanup;
}

static
DWORD
VmDnsWriteRecordsToBuffer(
    PVMDNS_RECORD           *ppRecords,
    DWORD                   dwCount,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    if (!pMessageBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwCount && ppRecords)
    {
        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmDnsWriteRecordToBuffer(
                                ppRecords[dwIndex],
                                pMessageBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }


cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsWriteQuestionToBuffer(
    PVMDNS_QUESTION         pQuestion,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;

    if (!pMessageBuffer || !pQuestion)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDomainNameToBuffer(
                        pQuestion->pszQName,
                        pMessageBuffer,
                        TRUE
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pQuestion->uQType,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pQuestion->uQClass,
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
VmDnsWriteZoneToBuffer(
    PVMDNS_UPDATE_ZONE      pZone,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;

    if (!pMessageBuffer || !pZone)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDomainNameToBuffer(
                            pZone->pszName,
                            pMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pZone->uType,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pZone->uClass,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}
