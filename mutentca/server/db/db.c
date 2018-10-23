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


static
DWORD
LwCADbValidateContext(
    )
{
    if (!gDbCtx.isInitialized)
    {
        return  LWCA_DB_NOT_INITIALIZED;
    }

    return LWCA_SUCCESS;
}

static
BOOLEAN
IsValidFunctionTable(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{
    return (pFt && pFt->pFnInit
            && pFt->pFnAddCA
            && pFt->pFnAddCertData
            && pFt->pFnCheckCA
            && pFt->pFnCheckCertData
            && pFt->pFnGetCACertificates
            && pFt->pFnGetCA
            && pFt->pFnGetCertData
            && pFt->pFnGetCACRLNumber
            && pFt->pFnGetParentCAId
            && pFt->pFnUpdateCA
            && pFt->pFnUpdateCAStatus
            && pFt->pFnUpdateCACRLNumber
            && pFt->pFnFreeCAData
            && pFt->pFnUpdateCertData
            && pFt->pFnFreeCertDataArray
            && pFt->pFnFreeCertArray
            && pFt->pFnFreeString
            && pFt->pFnFreeHandle
            );
}

static
DWORD
_LwCADbCopyCAData(
    PLWCA_DB_CA_DATA pCAData,
    PLWCA_DB_CA_DATA *ppCAData
    );

/*
 * To initialize db context, the Mutent CA config file must provide db plugin with absolute path
 * Example:
 * {
      "dbPlugin": "/usr/lib/libdbplugin.so",
      "dbPluginConfigPath": "/usr/lib/pluginconfig.json"
 * }
 */
DWORD
LwCADbInitCtx(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PSTR pszPlugin = NULL;
    PSTR pszPluginConfigPath = NULL;
    BOOLEAN bLocked = FALSE;

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gDbCtx.dbMutex, bLocked);

    if (gDbCtx.isInitialized)
    {
        dwError = LWCA_DB_ALREADY_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pConfig, FALSE, CONFIG_DB_PLUGIN_KEY_NAME, &pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_FUNCTION_TABLE), (PVOID*)&gDbCtx.pFt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszPlugin, &gDbCtx.pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitialize(pszPlugin, gDbCtx.pFt, &gDbCtx.pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pConfig, FALSE, CONFIG_DB_PLUGIN_PATH, &pszPluginConfigPath);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnInit(pszPluginConfigPath, &gDbCtx.pDbHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsValidFunctionTable(gDbCtx.pFt))
    {
        dwError = LWCA_DB_INVALID_PLUGIN;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    gDbCtx.isInitialized = TRUE;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);

    LWCA_SAFE_FREE_STRINGA(pszPlugin);
    LWCA_SAFE_FREE_STRINGA(pszPluginConfigPath);
    return dwError;

error:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    if (dwError != LWCA_DB_ALREADY_INITIALIZED)
    {
        LwCADbFreeCtx();
    }
    goto cleanup;
}

DWORD
LwCADbAddCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData,
    PCSTR                   pcszParentCAId
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnAddCA(gDbCtx.pDbHandle, pcszCAId, pCAData, pcszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbAddCertData(
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnAddCertData(gDbCtx.pDbHandle, pcszCAId, pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbCheckCA(
    PCSTR                   pcszCAId,
    PBOOLEAN                pbExists
    )
{
    DWORD dwError = 0;
    BOOLEAN bExists = FALSE;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pbExists)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnCheckCA(gDbCtx.pDbHandle, pcszCAId, &bExists);
    BAIL_ON_LWCA_ERROR(dwError);

    *pbExists = bExists;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbCheckCertData(
    PCSTR                   pcszCAId,
    PCSTR                   pcszSerialNumber,
    PBOOLEAN                pbExists
    )
{
    DWORD dwError = 0;
    BOOLEAN bExists = FALSE;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId)
        || IsNullOrEmptyString(pcszSerialNumber) || !pbExists)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnCheckCertData(
                                gDbCtx.pDbHandle,
                                pcszCAId,
                                pcszSerialNumber,
                                &bExists
                                );
    BAIL_ON_LWCA_ERROR(dwError);

    *pbExists = bExists;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    if (pbExists)
    {
        *pbExists = FALSE;
    }
    goto cleanup;
}

DWORD
LwCADbGetCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        *ppCAData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_DB_CA_DATA pTempCAData = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnGetCA(gDbCtx.pDbHandle, pcszCAId, &pTempCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCAData)
    {
        dwError = _LwCADbCopyCAData(pTempCAData, &pCAData);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCAData = pCAData;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCAData(pTempCAData);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
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
LwCADbGetCACertificates(
    PCSTR                      pcszCAId,
    PLWCA_CERTIFICATE_ARRAY    *ppCertArray
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    PLWCA_CERTIFICATE_ARRAY pTempCertArray = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppCertArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnGetCACertificates(gDbCtx.pDbHandle, pcszCAId, &pTempCertArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCertArray)
    {
        dwError = LwCACopyCertArray(pTempCertArray, &pCertArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertArray = pCertArray;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCertArray(pTempCertArray);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
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
LwCADbGetCertData(
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;
    PLWCA_DB_CERT_DATA_ARRAY pTempCertDataArray = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppCertDataArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnGetCertData(gDbCtx.pDbHandle, pcszCAId, &pTempCertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCertDataArray)
    {
        dwError = LwCADbCopyCertDataArray(pTempCertDataArray, &pCertDataArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertDataArray = pCertDataArray;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCertDataArray(pTempCertDataArray);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);

    return dwError;

error:
    LwCADbFreeCertDataArray(pCertDataArray);
    if (ppCertDataArray)
    {
        *ppCertDataArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbGetCACRLNumber(
    PCSTR   pcszCAId,
    PSTR    *ppszCRLNumber
    )
{
    DWORD dwError = 0;
    PSTR pszCRLNumber = NULL;
    PSTR pszTempCRLNumber = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppszCRLNumber)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnGetCACRLNumber(gDbCtx.pDbHandle, pcszCAId, &pszTempCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pszTempCRLNumber)
    {
        dwError = LwCAAllocateStringA(pszTempCRLNumber, &pszCRLNumber);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszCRLNumber = pszCRLNumber;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeString(pszTempCRLNumber);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    if (ppszCRLNumber)
    {
        *ppszCRLNumber = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbGetParentCAId(
    PCSTR   pcszCAId,
    PSTR    *ppszParentCAId
    )
{
    DWORD dwError = 0;
    PSTR pszParentCAId = NULL;
    PSTR pszTempParentCAId = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppszParentCAId)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnGetParentCAId(gDbCtx.pDbHandle, pcszCAId, &pszTempParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pszTempParentCAId)
    {
        dwError = LwCAAllocateStringA(pszTempParentCAId, &pszParentCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszParentCAId = pszParentCAId;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeString(pszTempParentCAId);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
    if (ppszParentCAId)
    {
        *ppszParentCAId = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbUpdateCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUpdateCA(gDbCtx.pDbHandle, pcszCAId, pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbUpdateCAStatus(
    PCSTR                pcszCAId,
    LWCA_CA_STATUS       status
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUpdateCAStatus(gDbCtx.pDbHandle, pcszCAId, status);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbUpdateCertData(
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUpdateCertData(gDbCtx.pDbHandle, pcszCAId, pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbUpdateCACRLNumber(
    PCSTR   pcszCAId,
    PCSTR   pcszCRLNumber
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || IsNullOrEmptyString(pcszCRLNumber))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUpdateCACRLNumber(gDbCtx.pDbHandle, pcszCAId, pcszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
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

static
DWORD
_LwCADbCopyCAData(
    PLWCA_DB_CA_DATA pCAData,
    PLWCA_DB_CA_DATA *ppCAData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pTempCAData = NULL;

    if (!pCAData || !ppCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbCreateCAData(
                    pCAData->pszSubjectName,
                    pCAData->pCertificates,
                    pCAData->pEncryptedPrivateKey,
                    pCAData->pszCRLNumber,
                    pCAData->status,
                    &pTempCAData
                );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCAData = pTempCAData;

cleanup:
    return dwError;

error:
    LwCADbFreeCAData(pTempCAData);
    if (ppCAData)
    {
        *ppCAData = NULL;
    }
    goto cleanup;
}
