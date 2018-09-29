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
    return (pFt && pFt->pFnInit && pFt->pFnAddCA && pFt->pFnAddCertData && pFt->pFnGetCACertificates &&
        pFt->pFnGetCertData && pFt->pFnUpdateCA && pFt->pFnUpdateCAStatus && pFt->pFnUpdateCertData &&
        pFt->pFnFreeCertDataArray && pFt->pFnFreeCertArray);
}

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

    dwError = gDbCtx.pFt->pFnInit(&gDbCtx.pDbHandle);
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
    return dwError;

error:
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
    PCSTR                   pcszParentCA
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

    dwError = gDbCtx.pFt->pFnAddCA(gDbCtx.pDbHandle, pcszCAId, pCAData, pcszParentCA);
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
    if (!gDbCtx.isInitialized)
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
    if (!gDbCtx.isInitialized)
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
