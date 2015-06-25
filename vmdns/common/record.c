/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

BOOLEAN
VmDnsCompareRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord1, &idx);
    if(!dwError)
    {
        return gRecordMethods[idx].pfnCompare(pRecord1, pRecord2);
    }

    return dwError;
}

BOOLEAN
VmDnsMatchRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pTemplate
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        return gRecordMethods[idx].pfnMatch(pRecord, pTemplate);
    }

    return dwError;
}

VOID
VmDnsClearRecord(
    PVMDNS_RECORD       pRecord
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        gRecordMethods[idx].pfnClear(pRecord);
    }
}

VOID
VmDnsRpcClearRecord(
    PVMDNS_RECORD   pRecord
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        gRecordMethods[idx].pfnRpcClear(pRecord);
    }
}

DWORD
VmDnsDuplicateRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnDuplicate(pRecord, ppDest);
    }

    return dwError;
}

DWORD
VmDnsRpcDuplicateRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnRpcDuplicate(pRecord, ppDest);
    }

    return dwError;
}

DWORD
VmDnsCopyRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnCopy(pRecord, pDest);
    }

    return dwError;
}

DWORD
VmDnsRpcCopyRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnRpcCopy(pRecord, pDest);
    }

    return dwError;
}

DWORD
VmDnsRecordToString(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnToString(pRecord, ppStr);
    }

    return dwError;
}

DWORD
VmDnsRecordGetCN(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnGetCN(pRecord, ppStr);
    }

    return dwError;
}

VOID
VmDnsClearRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    int i = 0;
    if (pRecordArray)
    {
        for (; i < pRecordArray->dwCount; ++i)
        {
            VmDnsClearRecord(&(pRecordArray->Records[i]));
        }
    }
}

DWORD
VmDnsCreateSoaRecord(
    PVMDNS_ZONE_INFO    pZoneInfo,
    PVMDNS_RECORD*      ppRecord
    )
{
    DWORD           dwError = 0;
    PVMDNS_RECORD   pRecord = NULL;
    PSTR            pszName = NULL;
    PSTR            pszPrimaryDnsName = NULL;
    PSTR            pszRName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecord, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecord->dwType = VMDNS_RR_TYPE_SOA;
    dwError = VmDnsAllocateStringA(VMDNS_SOA_RECORD_NAME, &pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszPrimaryDnsSrvName,
                                    &pszPrimaryDnsName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszRName, &pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecord->pszName = pszName;
    pRecord->Data.SOA.pNameAdministrator = pszRName;
    pRecord->Data.SOA.pNamePrimaryServer = pszPrimaryDnsName;
    pRecord->Data.SOA.dwDefaultTtl = pZoneInfo->minimum;
    pRecord->Data.SOA.dwExpire = pZoneInfo->expire;
    pRecord->Data.SOA.dwRefresh = pZoneInfo->refreshInterval;
    pRecord->Data.SOA.dwRetry = pZoneInfo->retryInterval;
    pRecord->Data.SOA.dwSerialNo = pZoneInfo->serial;
    pszName = NULL;
    pszRName = NULL;

    *ppRecord = pRecord;

cleanup:

    return dwError;

error:
    VmDnsFreeMemory(pszName);
    VmDnsFreeMemory(pszRName);
    VMDNS_FREE_RECORD(pRecord);
    goto cleanup;
}

DWORD
VmDnsWriteRecordToBuffer(
    PVMDNS_RECORD pDnsRecord,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    if (!pDnsRecord || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDomainNameToBuffer(
                                  pDnsRecord->pszName,
                                  pVmDnsBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                  pDnsRecord->dwType,
                                  pVmDnsBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                  pDnsRecord->iClass,
                                  pVmDnsBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                                  pDnsRecord->dwTtl,
                                  pVmDnsBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < gRecordMethodMapSize; ++dwIndex)
    {
        if (pDnsRecord->dwType == gRecordMethods[dwIndex].type)
        {
            dwError = gRecordMethods[dwIndex].pfnSerialize(
                                                    pDnsRecord->Data,
                                                    pVmDnsBuffer
                                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
            break;
        }
    }
cleanup:

    return dwError;
error:

    goto cleanup;
}
DWORD
VmDnsSerializeDnsRecord(
    PVMDNS_RECORD pDnsRecord,
    PBYTE* ppBytes,
    DWORD* pdwSize)
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;
    PBYTE pBytes = NULL;
    DWORD dwSize =  0;

    if (!pDnsRecord ||
        !ppBytes ||
        !pdwSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAllocateBufferStream(
                                    0,
                                    &pVmDnsBuffer
                                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteRecordToBuffer(
                                   pDnsRecord,
                                   pVmDnsBuffer
                                   );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                                      pVmDnsBuffer,
                                      NULL,
                                      &dwSize
                                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                              dwSize,
                              (PVOID *)&pBytes
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                                      pVmDnsBuffer,
                                      pBytes,
                                      &dwSize
                                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppBytes = pBytes;
    *pdwSize = dwSize;

cleanup:
    if (pVmDnsBuffer)
    {
        VmDnsFreeBufferStream(pVmDnsBuffer);
    }

    return dwError;
error:
    if (ppBytes)
    {
        *ppBytes = NULL;
    }
    if (pdwSize)
    {
        *pdwSize = 0;
    }
    VMDNS_SAFE_FREE_MEMORY(pBytes);

    goto cleanup;
}

DWORD
VmDnsReadRecordFromBuffer(
          PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
          PVMDNS_RECORD *ppDnsRecord
          )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pDnsRecord = NULL;
    DWORD dwIndex =  0;

    if (!pVmDnsBuffer || !ppDnsRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                              sizeof(VMDNS_RECORD),
                              (PVOID *)&pDnsRecord
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDomainNameFromBuffer(
                              pVmDnsBuffer,
                              &pDnsRecord->pszName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                             pVmDnsBuffer,
                             &pDnsRecord->dwType
                             );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                             pVmDnsBuffer,
                             &pDnsRecord->iClass
                             );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                             pVmDnsBuffer,
                             &pDnsRecord->dwTtl
                             );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < gRecordMethodMapSize; ++dwIndex)
    {
        if (pDnsRecord->dwType == gRecordMethods[dwIndex].type)
        {
            dwError = gRecordMethods[dwIndex].pfnDeSerialize(
                                                    pVmDnsBuffer,
                                                    &pDnsRecord->Data
                                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *ppDnsRecord = pDnsRecord;

cleanup:

    return dwError;
error:

    if (ppDnsRecord)
    {
        *ppDnsRecord = NULL;
    }
    VMDNS_FREE_RECORD(pDnsRecord);
    goto cleanup;
}


DWORD
VmDnsDeserializeDnsRecord(
    PBYTE pBytes,
    DWORD dwSize,
    PVMDNS_RECORD *ppDnsRecord
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pDnsRecord = NULL;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;

    if (!pBytes ||
        !dwSize ||
        !ppDnsRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBufferStreamWithBuffer(
                                        pBytes,
                                        dwSize,
                                        0,
                                        FALSE,
                                        &pVmDnsBuffer
                                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadRecordFromBuffer(
                                    pVmDnsBuffer,
                                    &pDnsRecord
                                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsRecord = pDnsRecord;

cleanup:

    if (pVmDnsBuffer)
    {
        VmDnsFreeBufferStream(pVmDnsBuffer);
    }
    return dwError;
error:

    if (ppDnsRecord)
    {
        *ppDnsRecord = NULL;
    }
    VMDNS_FREE_RECORD(pDnsRecord);
    goto cleanup;
}

DWORD
VmDnsRecordTypeToString(
    VMDNS_RR_TYPE       type,
    PCSTR*              ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gRecordTypeMapSize; ++idx)
    {
        if (type == gRecordTypeMap[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gRecordTypeMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsServiceTypeToString(
    VMDNS_SERVICE_TYPE  service,
    PCSTR*              ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gServiceNameMapSize; ++idx)
    {
        if (service == gServiceNameMap[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gServiceNameMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsProtocolToString(
    VMDNS_SERVICE_PROTOCOL  protocol,
    PCSTR*                  ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gProtocolNameMapSize; ++idx)
    {
        if (protocol == gProtocolNameMap[idx].protocol)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gProtocolNameMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

BOOLEAN
VmDnsFindRecordMethods(
    PVMDNS_RECORD   pRecord,
    DWORD           *pIdx
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gRecordMethodMapSize; ++idx)
    {
        if (pRecord->dwType == gRecordMethods[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *pIdx = idx;
            break;
        }
    }

    return dwError;
}

BOOLEAN
VmDnsCompareRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    BOOLEAN result = TRUE;
    if (!pRecord1 && !pRecord2)
    {
        result = TRUE;
    }
    else if (!pRecord1 || !pRecord2)
    {
        result = FALSE;
    }
    else if (pRecord1->dwType != pRecord2->dwType)
    {
        result = FALSE;
    }
    else if (pRecord1->iClass != pRecord2->iClass)
    {
        result = FALSE;
    }
    else if (VmDnsStringCompareA(pRecord1->pszName, pRecord2->pszName, TRUE))
    {
        result = FALSE;
    }

    return result;
}

DWORD
VmDnsGetDomainNameLength(
    PSTR pszDomainName,
    PUINT16 puSize
    )
{
    DWORD dwError = 0;
    PSTR pszTempString = NULL;
    PSTR  pToken = NULL;
    PSTR  pNextToken = NULL;
    UINT16 uTotalStringLength = 0;

    if (!puSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                            pszDomainName,
                            &pszTempString
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pToken = VmDnsStringTokA(
                      pszTempString,
                      ".",
                      &pNextToken
                      );

    while(++uTotalStringLength && pToken)
    {
        UINT16 uStringLength = 0;

        uStringLength = VmDnsStringLenA(pToken);

        uTotalStringLength += uStringLength;

        pToken = VmDnsStringTokA(
                             NULL,
                             ".",
                             &pNextToken
                             );
    }

    *puSize = uTotalStringLength;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    return dwError;
error:

    if (puSize)
    {
        *puSize = 0;
    }
    goto cleanup;
}

DWORD
VmDnsWriteDomainNameToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PSTR pszTempString = NULL;
    PSTR  pToken = NULL;
    PSTR  pNextToken = NULL;
    DWORD dwTotalStringLength = 0;

    if (!pszDomainName ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                            pszDomainName,
                            &pszTempString
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pToken = VmDnsStringTokA(
                      pszTempString,
                      ".",
                      &pNextToken
                      );

    while(pToken)
    {
        DWORD dwStringLength = 0;

        dwStringLength = VmDnsStringLenA(pToken);

        if (dwStringLength > VMDNS_LABEL_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsWriteStringToBuffer(
                                pToken,
                                dwStringLength,
                                pVmDnsBuffer
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwTotalStringLength += dwStringLength+1;

        if (dwTotalStringLength > VMDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        pToken = VmDnsStringTokA(
                             NULL,
                             ".",
                             &pNextToken
                             );
    }

    if (++dwTotalStringLength > VMDNS_NAME_LENGTH_MAX)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteCharToBuffer(
                                  0,
                                  pVmDnsBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsReadDomainNameFromBuffer(
      PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
      PSTR *ppszDomainName
      )
{
    DWORD dwError = 0;
    DWORD dwTotalStringLength = 0;
    DWORD dwLabelLength = 0;
    PSTR pszTempString = NULL;
    PSTR pszTempStringCursor = NULL;
    PSTR pszLabels = NULL;
    PSTR pszDomainName = NULL;

    if (!pVmDnsBuffer||
        !ppszDomainName
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                          256,
                          (PVOID *)&pszTempString
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    pszTempStringCursor = pszTempString;

    do
    {
        dwError = VmDnsReadStringFromBuffer(
                                        pVmDnsBuffer,
                                        &pszLabels,
                                        &dwLabelLength
                                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (dwLabelLength)
        {

            memcpy(
                    pszTempStringCursor,
                    pszLabels,
                    dwLabelLength
                    );

            pszTempStringCursor[dwLabelLength]='.';
            dwLabelLength++;
        }

        pszTempStringCursor = &pszTempStringCursor[dwLabelLength];
        VMDNS_SAFE_FREE_STRINGA(pszLabels);
        dwTotalStringLength += dwLabelLength;

        if (dwTotalStringLength > 255)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }while(dwLabelLength);

    if (dwTotalStringLength > 0)
    {
        pszTempString[dwTotalStringLength - 1] = 0;
    }

    dwError = VmDnsAllocateStringA(
                              pszTempString,
                              &pszDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszDomainName = pszDomainName;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    VMDNS_SAFE_FREE_STRINGA(pszLabels);

    return dwError;

error:

    if (ppszDomainName)
    {
        *ppszDomainName = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsParseRecordType(
    PSTR            pszRecordType,
    VMDNS_RR_TYPE*  pType
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    for (; idx < gRecordTypeMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszRecordType,
                                gRecordTypeMap[idx].pszName,
                                FALSE))
        {
            *pType = gRecordTypeMap[idx].type;
            dwError = ERROR_SUCCESS;
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsParseServiceType(
    PSTR                pszServiceType,
    VMDNS_SERVICE_TYPE* pType,
    PSTR*               ppszName
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    for (; idx < gServiceNameMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszServiceType,
                                gServiceNameMap[idx].pszUserFriendlyName,
                                FALSE))
        {
            dwError = ERROR_SUCCESS;
            if (pType)
            {
                *pType = gServiceNameMap[idx].type;
            }
            if (ppszName)
            {
                dwError = VmDnsAllocateStringA(gServiceNameMap[idx].pszName, ppszName);
            }
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsParseServiceProtocol(
    PSTR                    pszServiceType,
    VMDNS_SERVICE_PROTOCOL* pProtocol,
    PSTR*                   ppszName
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    for (; idx < gProtocolNameMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszServiceType,
                                gProtocolNameMap[idx].pszUserFriendlyName,
                                FALSE))
        {
            dwError = ERROR_SUCCESS;
            if (pProtocol)
            {
                *pProtocol = gProtocolNameMap[idx].protocol;
            }
            if (ppszName)
            {
                dwError = VmDnsAllocateStringA(gProtocolNameMap[idx].pszName, ppszName);
            }
            break;
        }
    }

    return dwError;
}
