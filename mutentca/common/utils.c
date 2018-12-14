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

#include "includes.h"

DWORD
LwCAGetInstallDirectory(
    PSTR *ppszInstallDir
    )
{
    DWORD dwError = 0;
    dwError = LwCAAllocateStringA(LWCA_INSTALL_DIR, ppszInstallDir);
    return dwError;
}


DWORD
LwCAGetDataDirectory(
    PSTR *ppszDataDir
    )
{
    DWORD dwError = 0;
    dwError = LwCAAllocateStringA(LWCA_ROOT_CERT_DIR, ppszDataDir);
    return dwError;
}

DWORD
LwCAGetLogDirectory(
    PSTR *ppszLogDir
    )
{
    DWORD dwError = 0;
    dwError = LwCAAllocateStringA(LWCA_LOG_DIR, ppszLogDir);
    return dwError;
}

DWORD
LwCACreateKey(
    PBYTE     pData,
    DWORD     dwLength,
    PLWCA_KEY *ppKey
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pKey = NULL;

    if (!pData || dwLength <= 0 || !ppKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_KEY), (PVOID*)&pKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(dwLength * sizeof(BYTE), (PVOID*)&pKey->pData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACopyMemory((PVOID) pKey->pData,
                             dwLength * sizeof(BYTE),
                             pData,
                             (size_t)dwLength
                             );
    BAIL_ON_LWCA_ERROR(dwError);

    pKey->dwLength = dwLength;

    *ppKey = pKey;

cleanup:
    return dwError;

error:
    LwCAFreeKey(pKey);
    if (ppKey)
    {
        *ppKey = NULL;
    }

    goto cleanup;
}

VOID
LwCAFreeKey(
    PLWCA_KEY pKey
    )
{
    if (pKey)
    {
        LWCA_SECURE_SAFE_FREE_MEMORY(pKey->pData, pKey->dwLength);
        LWCA_SAFE_FREE_MEMORY(pKey);
    }
}

DWORD
LwCACopyKey(
    PLWCA_KEY pKey,
    PLWCA_KEY *ppKey
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pTempKey = NULL;

    if (!pKey || !ppKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCACreateKey(pKey->pData, pKey->dwLength, &pTempKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppKey = pTempKey;

cleanup:
    return dwError;

error:
    LwCAFreeKey(pTempKey);
    if (ppKey)
    {
        *ppKey = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbCreateCAData(
    PCSTR                       pcszSubjectName,
    PLWCA_CERTIFICATE_ARRAY     pCertificates,
    PLWCA_KEY                   pEncryptedPrivateKey,
    PCSTR                       pcszCRLNumber,
    PCSTR                       pcszLastCRLUpdate,
    PCSTR                       pcszNextCRLUpdate,
    PCSTR                       pcszAuthBlob,
    LWCA_CA_STATUS              status,
    PLWCA_DB_CA_DATA            *ppCAData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;

    if (!ppCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CA_DATA), (PVOID*)&pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCertificates)
    {
        dwError = LwCACopyCertArray(pCertificates, &pCAData->pCertificates);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszSubjectName))
    {
        dwError = LwCAAllocateStringA(pcszSubjectName,
                                      &pCAData->pszSubjectName
                                      );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pEncryptedPrivateKey)
    {
        dwError = LwCACopyKey(pEncryptedPrivateKey,
                              &pCAData->pEncryptedPrivateKey
                              );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszCRLNumber))
    {
        dwError = LwCAAllocateStringA(pcszCRLNumber, &pCAData->pszCRLNumber);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszLastCRLUpdate))
    {
        dwError = LwCAAllocateStringA(pcszLastCRLUpdate,
                                      &pCAData->pszLastCRLUpdate
                                      );
        BAIL_ON_LWCA_ERROR(dwError);
    }
    if (!IsNullOrEmptyString(pcszNextCRLUpdate))
    {
        dwError = LwCAAllocateStringA(pcszNextCRLUpdate,
                                      &pCAData->pszNextCRLUpdate
                                      );
        BAIL_ON_LWCA_ERROR(dwError);
    }
    if (!IsNullOrEmptyString(pcszAuthBlob))
    {
        dwError = LwCAAllocateStringA(pcszAuthBlob, &pCAData->pszAuthBlob);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pCAData->status = status;

    *ppCAData = pCAData;

cleanup:
    return dwError;

error:
    LwCADbFreeCAData(pCAData);
    if (ppCAData)
    {
        *ppCAData = NULL;
    }
    goto cleanup;
}

VOID
LwCADbFreeCAData(
    PLWCA_DB_CA_DATA pCAData
    )
{
    if (pCAData)
    {
        LWCA_SAFE_FREE_STRINGA(pCAData->pszSubjectName);
        LwCAFreeCertificates(pCAData->pCertificates);
        LwCAFreeKey(pCAData->pEncryptedPrivateKey);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszCRLNumber);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszLastCRLUpdate);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszNextCRLUpdate);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszAuthBlob);
        LWCA_SAFE_FREE_MEMORY(pCAData);
    }
}

DWORD
LwCACreateStringArrayFromCertArray(
    PLWCA_CERTIFICATE_ARRAY     pCertArray,
    PLWCA_STRING_ARRAY          *ppStrArray
    )
{
    DWORD                   dwError = 0;
    PLWCA_STRING_ARRAY      pStrArray = NULL;
    DWORD                   dwEntry = 0;

    if (!pCertArray || !ppStrArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(PLWCA_STRING_ARRAY),
                                 (PVOID*)&pStrArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pStrArray->dwCount = pCertArray->dwCount;

    dwError = LwCAAllocateMemory(sizeof(PSTR) * pStrArray->dwCount,
                                 (PVOID*)&pStrArray->ppData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    for (; dwEntry < pCertArray->dwCount; ++dwEntry)
    {
        dwError = LwCAAllocateStringA(pCertArray->ppCertificates[dwEntry],
                                      &pStrArray->ppData[dwEntry]
                                      );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppStrArray = pStrArray;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pStrArray);
    if (ppStrArray)
    {
        *ppStrArray = NULL;
    }
    goto cleanup;
}

DWORD
LwCACreateCertArray(
    PSTR                        *ppszCertificates,
    DWORD                       dwCount,
    PLWCA_CERTIFICATE_ARRAY     *ppCertArray
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    PLWCA_CERTIFICATE_ARRAY  pCertArray = NULL;

    if (!ppszCertificates || dwCount <=0 || !ppCertArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_CERTIFICATE_ARRAY),
                                 (PVOID*)&pCertArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pCertArray->dwCount = dwCount;

    dwError = LwCAAllocateMemory(sizeof(PLWCA_CERTIFICATE) * dwCount,
                                 (PVOID*)&pCertArray->ppCertificates
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    for(; iEntry < dwCount; ++iEntry)
    {
        dwError = LwCACreateCertificate(ppszCertificates[iEntry],
                                        &pCertArray->ppCertificates[iEntry]
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

   *ppCertArray = pCertArray;

cleanup:
    return dwError;

error:
    LwCAFreeCertificates(pCertArray);
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCACopyCertArray(
    PLWCA_CERTIFICATE_ARRAY     pCertArray,
    PLWCA_CERTIFICATE_ARRAY     *ppCertArray
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    PLWCA_CERTIFICATE_ARRAY  pTempCertArray = NULL;

    if (!pCertArray ||
        !pCertArray->ppCertificates ||
        pCertArray->dwCount <=0 ||
        !ppCertArray
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_CERTIFICATE_ARRAY),
                                 (PVOID*)&pTempCertArray
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    pTempCertArray->dwCount = pCertArray->dwCount;

    dwError = LwCAAllocateMemory(sizeof(PLWCA_CERTIFICATE) * pCertArray->dwCount,
                                 (PVOID*)&pTempCertArray->ppCertificates
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    for(; iEntry < pCertArray->dwCount; ++iEntry)
    {
        dwError = LwCACreateCertificate(pCertArray->ppCertificates[iEntry],
                                        &pTempCertArray->ppCertificates[iEntry]
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertArray = pTempCertArray;

cleanup:
    return dwError;

error:
    LwCAFreeCertificates(pTempCertArray);
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }

   goto cleanup;
}

DWORD
LwCACreateCertificate(
    PCSTR               pcszCertificate,
    PLWCA_CERTIFICATE   *ppCertificate
    )
{
    DWORD dwError = 0;
    PSTR pszCert = NULL;

    if (IsNullOrEmptyString(pcszCertificate) || !ppCertificate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pcszCertificate, &pszCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertificate = (PLWCA_CERTIFICATE)pszCert;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_MEMORY(pszCert);

    if (ppCertificate)
    {
        *ppCertificate = NULL;
    }

    goto cleanup;
}

VOID
LwCAFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    )
{
    DWORD iEntry = 0;

    if (pCertArray != NULL)
    {
        if (pCertArray->dwCount > 0 && pCertArray->ppCertificates)
        {
            for(; iEntry < pCertArray->dwCount; ++iEntry)
            {
                LwCAFreeCertificate(pCertArray->ppCertificates[iEntry]);
            }
        }
        LWCA_SAFE_FREE_MEMORY(pCertArray->ppCertificates);
        LWCA_SAFE_FREE_MEMORY(pCertArray);
    }
}

VOID
LwCAFreeCertificate(
    PLWCA_CERTIFICATE pCertificate
    )
{
    if (pCertificate)
    {
        LWCA_SAFE_FREE_MEMORY(pCertificate);
    }
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

    if (!pCertData || !ppCertData)
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

    dwError = LwCAAllocateMemory(sizeof(PLWCA_DB_CERT_DATA) * pCertDataArray->dwCount, (PVOID*)&pTempCertDataArray->ppCertData);
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

PCSTR
LwCAGetErrorDescription(
    DWORD dwErrorCode
    )
{
    DWORD i;
    for (i=0; i < LWCA_ERROR_Table_size; i++)
    {
        if ( dwErrorCode == LWCA_ERROR_Table[i].code )
        {
            return LWCA_ERROR_Table[i].desc;
        }
    }
    return NULL;
}

DWORD
LwCAUuidGenerate(
    PSTR    *ppszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszUuid = NULL;
    uuid_t  uuid = {0};

    if (!ppszUuid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    uuid_generate(uuid);
    if (uuid_is_null(uuid))
    {
        dwError = LWCA_ERROR_UUID_GENERATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(*pszUuid) * LWCA_UUID_LEN,
                                 (PVOID *)&pszUuid
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    uuid_unparse_lower(uuid, pszUuid);

    *ppszUuid = pszUuid;

cleanup:
    uuid_clear(uuid);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    if (ppszUuid)
    {
        *ppszUuid = NULL;
    }
    goto cleanup;
}
