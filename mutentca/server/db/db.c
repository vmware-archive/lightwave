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

/*
 * To initialize db context, the Mutent CA config file must provide db plugin with absolute path
 * Example:
 * {
      "dbPlugin": "/usr/lib/libdbplugin.so"
 * }
 */
DWORD
LwCADbInitCtx(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PSTR pszPlugin = NULL;

    if (gpDbCtx != NULL)
    {
        dwError = LWCA_DB_ALREADY_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pConfig, CONFIG_DB_PLUGIN_KEY_NAME, &pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CONTEXT), (PVOID*)&gpDbCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_FUNCTION_TABLE), (PVOID*)&gpDbCtx->pFt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszPlugin, &gpDbCtx->pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitialize(pszPlugin, gpDbCtx->pFt, &gpDbCtx->pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gpDbCtx->pFt->pFnInit(&gpDbCtx->pDbHandle);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPlugin);
    return dwError;

error:
    LwCADbFreeCtx();
    goto cleanup;
}

DWORD
LwCADbCreateCertArray(
    PSTR                        *ppCertificates,
    DWORD                       dwCount,
    PLWCA_DB_CERTIFICATE_ARRAY  *ppCertArray
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    PLWCA_DB_CERTIFICATE_ARRAY  pCertArray = NULL;

    if (!ppCertificates || dwCount <=0 || !ppCertArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERTIFICATE_ARRAY), (PVOID*)&pCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwCount, (PVOID*)&pCertArray->ppCertificates);
    BAIL_ON_LWCA_ERROR(dwError);


    for(; iEntry < dwCount; ++iEntry)
    {
        if (!IsNullOrEmptyString(ppCertificates[iEntry]))
        {
            dwError = LwCAAllocateStringA(ppCertificates[iEntry], &pCertArray->ppCertificates[iEntry]);
            BAIL_ON_LWCA_ERROR(dwError);
            pCertArray->dwCount++;
        }
    }

    *ppCertArray = pCertArray;

cleanup:
    return dwError;

error:
    LwCADbFreeCertificates(pCertArray);
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbCreateEncryptedKey(
    PBYTE                   pData,
    DWORD                   dwLength,
    PLWCA_DB_ENCRYPTED_KEY  *ppEncryptedKey
    )
{
    DWORD dwError = 0;
    PLWCA_DB_ENCRYPTED_KEY pEncryptedKey = NULL;

    if (!pData || dwLength <= 0 || !ppEncryptedKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_ENCRYPTED_KEY), (PVOID*)&pEncryptedKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(dwLength * sizeof(BYTE), (PVOID*) &pEncryptedKey->pData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACopyMemory((PVOID) pEncryptedKey->pData, dwLength * sizeof(BYTE), pData, (size_t)dwLength);
    BAIL_ON_LWCA_ERROR(dwError);

    pEncryptedKey->dwLength = dwLength;

    *ppEncryptedKey = pEncryptedKey;

cleanup:
    return dwError;

error:
    LwCADbFreeEncryptedKey(pEncryptedKey);
    if (ppEncryptedKey)
    {
        *ppEncryptedKey = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbCreateCAData(
    PCSTR                       pcszIssuer,
    PCSTR                       pcszSubject,
    PLWCA_DB_CERTIFICATE_ARRAY  pCertificates,
    PLWCA_DB_ENCRYPTED_KEY      pEncryptedPrivateKey,
    PLWCA_DB_ENCRYPTED_KEY      pEncryptedEncryptionKey,
    PCSTR                       pcszTimeValidFrom,
    PCSTR                       pcszTimeValidTo,
    LWCA_DB_CA_STATUS           status,
    PLWCA_DB_CA_DATA            *ppCAData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;

    if (!pCertificates || !pEncryptedPrivateKey || !pEncryptedEncryptionKey ||
            IsNullOrEmptyString(pcszIssuer) || IsNullOrEmptyString(pcszSubject) || !ppCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CA_DATA), (PVOID*)&pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCertArray(pCertificates->ppCertificates, pCertificates->dwCount, &pCAData->pCertificates);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszIssuer, &pCAData->pszIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszSubject, &pCAData->pszSubject);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszTimeValidFrom))
    {
        dwError = LwCAAllocateStringA(pcszTimeValidFrom, &pCAData->pszTimeValidFrom);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszTimeValidTo))
    {
        dwError = LwCAAllocateStringA(pcszTimeValidTo, &pCAData->pszTimeValidTo);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbCopyEncryptedKey(pEncryptedPrivateKey, &pCAData->pEncryptedPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCopyEncryptedKey(pEncryptedEncryptionKey, &pCAData->pEncryptedEncryptionKey);
    BAIL_ON_LWCA_ERROR(dwError);

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

DWORD
LwCADbCreateCertData(
    PCSTR                   pcszSerialNumber,
    PCSTR                   pcszIssuer,
    PCSTR                   pcszTimeValidFrom,
    PCSTR                   pcszTimeValidTo,
    PCSTR                   pcszRevokedReason,
    PCSTR                   pcszRevokedDate,
    LWCA_DB_CERT_STATUS     status,
    PLWCA_DB_CERT_DATA      *ppCertData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    if (IsNullOrEmptyString(pcszSerialNumber) || IsNullOrEmptyString(pcszRevokedReason) ||
            IsNullOrEmptyString(pcszRevokedDate) || IsNullOrEmptyString(pcszTimeValidFrom) ||
            IsNullOrEmptyString(pcszTimeValidTo) || !ppCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszSerialNumber, &(pCertData->pszSerialNumber));
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszIssuer))
    {
        dwError = LwCAAllocateStringA(pcszIssuer, &pCertData->pszIssuer);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pcszTimeValidFrom, &pCertData->pszTimeValidFrom);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszTimeValidTo, &pCertData->pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszRevokedReason, &pCertData->pszRevokedReason);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszRevokedDate, &pCertData->pszRevokedDate);
    BAIL_ON_LWCA_ERROR(dwError);

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

VOID
LwCADbFreeEncryptedKey(
    PLWCA_DB_ENCRYPTED_KEY pEncryptedKey
    )
{
    if (pEncryptedKey)
    {
        LWCA_SAFE_FREE_MEMORY(pEncryptedKey->pData);
        LWCA_SAFE_FREE_MEMORY(pEncryptedKey);
    }
}

VOID
LwCADbFreeCAData(
    PLWCA_DB_CA_DATA pCAData
    )
{
    if (pCAData)
    {
        LWCA_SAFE_FREE_STRINGA(pCAData->pszIssuer);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszSubject);
        LwCADbFreeCertificates(pCAData->pCertificates);
        LwCADbFreeEncryptedKey(pCAData->pEncryptedPrivateKey);
        LwCADbFreeEncryptedKey(pCAData->pEncryptedEncryptionKey);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszTimeValidFrom);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszTimeValidTo);
        LWCA_SAFE_FREE_MEMORY(pCAData);
    }
}

VOID
LwCADbFreeCertData(
    PLWCA_DB_CERT_DATA pCertData
    )
{
    if (pCertData != NULL)
    {
        LWCA_SAFE_FREE_STRINGA(pCertData->pszSerialNumber);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszIssuer);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszRevokedReason);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszRevokedDate);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszTimeValidFrom);
        LWCA_SAFE_FREE_STRINGA(pCertData->pszTimeValidTo);
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

VOID
LwCADbFreeCertificate(
    PSTR pCertificate
    )
{
    if (pCertificate)
    {
        LWCA_SAFE_FREE_STRINGA(pCertificate);
    }
}

VOID
LwCADbFreeCertificates(
    PLWCA_DB_CERTIFICATE_ARRAY pCertArray
    )
{
    DWORD iEntry = 0;

    if (pCertArray != NULL)
    {
        if (pCertArray->dwCount > 0 && pCertArray->ppCertificates)
        {
            for(; iEntry < pCertArray->dwCount; ++iEntry)
            {
                LwCADbFreeCertificate(pCertArray->ppCertificates[iEntry]);
            }
        }
        LWCA_SAFE_FREE_MEMORY(pCertArray->ppCertificates);
        LWCA_SAFE_FREE_MEMORY(pCertArray);
    }
}

VOID
LwCADbFreeFunctionTable(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{
    if (pFt)
    {
        LWCA_SAFE_FREE_MEMORY(pFt);
    }
}

VOID
LwCADbFreeCtx(
   VOID
   )
{
    if (gpDbCtx)
    {
        if(gpDbCtx->pFt)
        {
            if (gpDbCtx->pDbHandle)
            {
                gpDbCtx->pFt->pFnFreeHandle(gpDbCtx->pDbHandle);
            }
            LwCADbFreeFunctionTable(gpDbCtx->pFt);
        }
        if (gpDbCtx->pPluginHandle)
        {
            LwCAPluginDeinitialize(gpDbCtx->pPluginHandle);
        }
        LWCA_SAFE_FREE_MEMORY(gpDbCtx->pszPlugin);
        LWCA_SAFE_FREE_MEMORY(gpDbCtx);
    }
}
