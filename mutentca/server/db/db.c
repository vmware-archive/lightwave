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
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gDbCtx.dbMutex, bLocked);

    if (gDbCtx.isInitialized)
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

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_FUNCTION_TABLE), (PVOID*)&gDbCtx.pFt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszPlugin, &gDbCtx.pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitialize(pszPlugin, gDbCtx.pFt, &gDbCtx.pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnInit(&gDbCtx.pDbHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    gDbCtx.isInitialized = TRUE;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);

    LWCA_SAFE_FREE_STRINGA(pszPlugin);
    return dwError;

error:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    LwCADbFreeCtx();
    goto cleanup;
}

DWORD
LwCADbCreateCAData(
    PCSTR                       pcszIssuer,
    PCSTR                       pcszSubject,
    PLWCA_CERTIFICATE_ARRAY     pCertificates,
    PLWCA_KEY                   pEncryptedPrivateKey,
    PLWCA_KEY                   pEncryptedEncryptionKey,
    PCSTR                       pcszTimeValidFrom,
    PCSTR                       pcszTimeValidTo,
    LWCA_CA_STATUS              status,
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

    dwError = LwCACopyCertArray(pCertificates, &pCAData->pCertificates);
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

    dwError = LwCACopyKey(pEncryptedPrivateKey, &pCAData->pEncryptedPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACopyKey(pEncryptedEncryptionKey, &pCAData->pEncryptedEncryptionKey);
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
    LWCA_CERT_STATUS        status,
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
LwCADbFreeCAData(
    PLWCA_DB_CA_DATA pCAData
    )
{
    if (pCAData)
    {
        LWCA_SAFE_FREE_STRINGA(pCAData->pszIssuer);
        LWCA_SAFE_FREE_STRINGA(pCAData->pszSubject);
        LwCAFreeCertificates(pCAData->pCertificates);
        LwCAFreeKey(pCAData->pEncryptedPrivateKey);
        LwCAFreeKey(pCAData->pEncryptedEncryptionKey);
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
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gDbCtx.dbMutex, bLocked);

    if (gDbCtx.pDbHandle)
    {
        gDbCtx.pFt->pFnFreeHandle(gDbCtx.pDbHandle);
    }
    if(gDbCtx.pFt)
    {
        LwCADbFreeFunctionTable(gDbCtx.pFt);
    }
    if (gDbCtx.pPluginHandle)
    {
        LwCAPluginDeinitialize(gDbCtx.pPluginHandle);
    }
    LWCA_SAFE_FREE_MEMORY(gDbCtx.pszPlugin);

    gDbCtx.isInitialized = FALSE;

    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
}
