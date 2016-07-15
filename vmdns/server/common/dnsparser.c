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

static
DWORD
VmDnsReadZoneFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_UPDATE_ZONE *ppZone
    );


DWORD
VmDnsParseMessage(
      PVMDNS_MESSAGE_BUFFER pMessageBuffer,
      PVMDNS_MESSAGE *ppMessage,
      PVMDNS_UPDATE_MESSAGE *ppUpdateMessage
      )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE pMessage = NULL;
    PVMDNS_HEADER pHeader = NULL;
    PVMDNS_QUESTION *pQuestions = NULL;
    PVMDNS_RECORD *pRecords = NULL;
    PVMDNS_UPDATE_MESSAGE pUpdateMessage = NULL;
    PVMDNS_UPDATE_ZONE pZone = NULL;
    PVMDNS_RECORD *pPrereqs = NULL;
    PVMDNS_RECORD *pUpdates = NULL;
    PVMDNS_RECORD *pAdds = NULL;

    if (!pMessageBuffer || !ppMessage || !ppUpdateMessage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadHeaderFromBuffer(
                              pMessageBuffer,
                              &pHeader
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pHeader && pHeader->codes.opcode == VM_DNS_OPCODE_QUERY)
    {
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
    }
    else if (pHeader && pHeader->codes.opcode == VM_DNS_OPCODE_UPDATE)
    {
        dwError = VmDnsReadZoneFromBuffer(
                                    pMessageBuffer,
                                    &pZone
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        //   To read prerequisites
        dwError = VmDnsReadRecordsFromBuffer(
                                    pMessageBuffer,
                                    pHeader->usPRCount,
                                    &pPrereqs
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        //   To read updates
        dwError = VmDnsReadRecordsFromBuffer(
                                    pMessageBuffer,
                                    pHeader->usUPCount,
                                    &pUpdates
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        //   To read additional data
        dwError = VmDnsReadRecordsFromBuffer(
                                    pMessageBuffer,
                                    pHeader->usADCount,
                                    &pAdds
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_UPDATE_MESSAGE),
                            (PVOID)&pUpdateMessage
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        pUpdateMessage->pHeader = pHeader;
        pHeader = NULL;
        pUpdateMessage->pZone = pZone;
        pZone = NULL;
        pUpdateMessage->pPrerequisite = pPrereqs;
        pPrereqs = NULL;
        pUpdateMessage->pUpdate = pUpdates;
        pUpdates = NULL;
        pUpdateMessage->pAdditional = pAdds;
        pAdds = NULL;

        *ppUpdateMessage = pUpdateMessage;
    }

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
    if (pUpdateMessage)
    {
        VmDnsFreeDnsUpdateMessage(pUpdateMessage);
    }
    if (pHeader && pQuestions)
    {
        VmDnsFreeQuestions(pQuestions, pHeader->usQDCount);
    }
    if (pHeader && pRecords)
    {
        VmDnsFreeRecordsArray(pRecords, pHeader->usANCount);
    }
    if (pHeader && pAdds)
    {
        VmDnsFreeRecordsArray(pAdds, pHeader->usADCount);
    }
    if (pHeader && pPrereqs)
    {
        VmDnsFreeRecordsArray(pPrereqs, pHeader->usPRCount);
    }
    if (pHeader && pUpdates)
    {
        VmDnsFreeRecordsArray(pUpdates, pHeader->usUPCount);
    }
    VMDNS_SAFE_FREE_MEMORY(pZone);
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



// It is one one zone in the request package for update
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
        sizeof(PVMDNS_UPDATE_ZONE),
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

    VmDnsFreeMemory(pZone);

    goto cleanup;
}
