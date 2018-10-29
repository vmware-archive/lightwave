/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include <includes.h>

BOOLEAN
LwCAUtilIsValueIPAddr(
    PCSTR           pcszValue
    )
{
    BOOLEAN         bIsIP = FALSE;

    if (!IsNullOrEmptyString(pcszValue))
    {
        unsigned char buf[sizeof(struct in_addr)];

        bIsIP = (inet_pton(AF_INET, pcszValue, &buf[0]) == 1);
    }

    return bIsIP;
}

/*
 * The IP Address in pcszValue can be in either dot-decimal notation
 * or hexadecimal notation
 */
BOOLEAN
LwCAUtilIsValuePrivateOrLocalIPAddr(
    PCSTR           pcszValue
    )
{
    uint32_t        unIp = 0x0;

    if (!IsNullOrEmptyString(pcszValue))
    {
        unIp = inet_network(pcszValue);

        if (LWCA_IS_IP_IN_PRIVATE_NETWORK(unIp))
        {
            return TRUE;
        }
        else if (LWCA_IS_IP_IN_LOCAL_NETWORK(unIp))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}

DWORD
LwCAUtilIsValueFQDN(
    PCSTR           pcszValue,
    PBOOLEAN        pbIsValid
    )
{
    DWORD           dwError = 0;
    DWORD           dwIdx = 0;
    PSTR            pszTempVal = NULL;
    PSTR            pszLabel = NULL;
    PSTR            pszNextTok = NULL;
    BOOLEAN         bIsValid = FALSE;
    int             count = 0;
    char            cTemp = 0;
    size_t          szLabelLen = 0;

    if (IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAStringCountSubstring(
                        pcszValue,
                        ".",
                        &count);
    BAIL_ON_LWCA_ERROR(dwError);
    if (count == 0)
    {
        bIsValid = FALSE;
        goto ret;
    }

    if (LwCAStringLenA(pcszValue) > 255)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(
                            &pszTempVal,
                            pcszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    pszLabel = LwCAStringTokA(pszTempVal, ".", &pszNextTok);
    while (pszLabel)
    {
        szLabelLen = LwCAStringLenA(pszLabel);

        if (szLabelLen > 63)
        {
            bIsValid = FALSE;
            goto ret;
        }

        if (!isalpha(pszLabel[0]) &&
            !isalpha(pszLabel[szLabelLen - 1]) &&
            !isdigit(pszLabel[szLabelLen - 1]))
        {
            bIsValid = FALSE;
            goto ret;
        }

        while ((cTemp = *(pszLabel + dwIdx)) != '\0')
        {
            if (!isalpha(cTemp) &&
                !isdigit(cTemp) &&
                (cTemp != '-'))
            {
                bIsValid = FALSE;
                goto ret;
            }

            ++dwIdx;
        }

        pszLabel = LwCAStringTokA(NULL, ".", &pszNextTok);
    }

    bIsValid = TRUE;


ret:

    *pbIsValid = bIsValid;

cleanup:

    LWCA_SAFE_FREE_STRINGA(pszTempVal);

    return dwError;

error:

    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

BOOLEAN
LwCAUtilDoesValueHaveWildcards(
    PCSTR            pcszValue
    )
{
    DWORD           dwError = 0;
    int             count = 0;

    if (IsNullOrEmptyString(pcszValue))
    {
        return FALSE;
    }

    dwError = LwCAStringCountSubstring(
                        pcszValue,
                        "*",
                        &count);
    if (count != 0)
    {
        return TRUE;
    }

    return FALSE;
}

DWORD
LwCADbCreateCertData(
    PCSTR                   pcszSerialNumber,
    PCSTR                   pcszTimeValidFrom,
    PCSTR                   pcszTimeValidTo,
    DWORD                   revokedReason,
    PCSTR                   pcszRevokedDate,
    LWCA_CERT_STATUS        status,
    PLWCA_DB_CERT_DATA      *ppCertData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    if (IsNullOrEmptyString(pcszSerialNumber) || !ppCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszSerialNumber, &(pCertData->pszSerialNumber));
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszTimeValidFrom))
    {
        dwError = LwCAAllocateStringA(pcszTimeValidFrom, &(pCertData->pszTimeValidFrom));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszTimeValidTo))
    {
        dwError = LwCAAllocateStringA(pcszTimeValidTo, &(pCertData->pszTimeValidTo));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszRevokedDate))
    {
        dwError = LwCAAllocateStringA(pcszRevokedDate, &(pCertData->pszRevokedDate));
        BAIL_ON_LWCA_ERROR(dwError);
    }
    pCertData->revokedReason = revokedReason;
    pCertData->status = status;

    *ppCertData = pCertData;

cleanup:
    return dwError;

error:
    LwCADbFreeCertData(pCertData);
    if (ppCertData)
    {
        *ppCertData = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbCopyCertData(
    PLWCA_DB_CERT_DATA pCertData,
    PLWCA_DB_CERT_DATA *ppCertData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pTempCertData = NULL;

    if (pCertData || !ppCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pTempCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pCertData->pszSerialNumber))
    {
        dwError = LwCAAllocateStringA(pCertData->pszSerialNumber, &pTempCertData->pszSerialNumber);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidFrom))
    {
        dwError = LwCAAllocateStringA(pCertData->pszTimeValidFrom, &(pTempCertData->pszTimeValidFrom));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszTimeValidTo))
    {
        dwError = LwCAAllocateStringA(pCertData->pszTimeValidTo, &(pTempCertData->pszTimeValidTo));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertData->pszRevokedDate))
    {
        dwError = LwCAAllocateStringA(pCertData->pszRevokedDate, &(pTempCertData->pszRevokedDate));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pTempCertData->revokedReason = pCertData->revokedReason;

    *ppCertData = pTempCertData;

cleanup:
    return dwError;

error:
    LwCADbFreeCertData(pTempCertData);
    if (ppCertData)
    {
        *ppCertData = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbCopyCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray,
    PLWCA_DB_CERT_DATA_ARRAY *ppCertDataArray
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY pTempCertDataArray = NULL;
    DWORD iEntry = 0;

    if (!pCertDataArray || pCertDataArray->dwCount <= 0 || !ppCertDataArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA_ARRAY), (PVOID*)&pTempCertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    pTempCertDataArray->dwCount = pCertDataArray->dwCount;

    dwError = LwCAAllocateMemory(sizeof(PSTR) * pCertDataArray->dwCount, (PVOID*)&pTempCertDataArray->ppCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    for(; iEntry < pCertDataArray->dwCount; ++iEntry)
    {
        dwError = LwCADbCopyCertData(pCertDataArray->ppCertData[iEntry], &pTempCertDataArray->ppCertData[iEntry]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertDataArray = pTempCertDataArray;

cleanup:
    return dwError;

error:
    LwCADbFreeCertDataArray(pTempCertDataArray);
    if (ppCertDataArray)
    {
        *ppCertDataArray = NULL;
    }

    goto cleanup;
}

VOID
LwCADbFreeCertData(
    PLWCA_DB_CERT_DATA pCertData
    )
{
    if (pCertData != NULL)
    {
        LWCA_SAFE_FREE_STRINGA(pCertData->pszSerialNumber);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszTimeValidFrom);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszTimeValidTo);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszRevokedDate);
        LWCA_SAFE_FREE_MEMORY(pCertData);
    }
}

VOID
LwCADbFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
    DWORD iEntry = 0;

    if (pCertDataArray != NULL)
    {
        if (pCertDataArray->dwCount > 0 && pCertDataArray->ppCertData)
        {
            for(; iEntry < pCertDataArray->dwCount; ++iEntry)
            {
                LwCADbFreeCertData(pCertDataArray->ppCertData[iEntry]);
            }
        }
        LWCA_SAFE_FREE_MEMORY(pCertDataArray->ppCertData);
        LWCA_SAFE_FREE_MEMORY(pCertDataArray);
    }
}
