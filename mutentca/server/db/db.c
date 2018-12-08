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
_LwCAIsValidFunctionTable(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{
    return (pFt &&
            pFt->pFnInit &&
            pFt->pFnAddCA &&
            pFt->pFnGetCA &&
            pFt->pFnUpdateCA &&
            pFt->pFnFreeCAData &&
            pFt->pFnLockCA &&
            pFt->pFnUnlockCA &&
            pFt->pFnAddCACert &&
            pFt->pFnGetCACerts &&
            pFt->pFnGetCACert &&
            pFt->pFnUpdateCACert &&
            pFt->pFnFreeCACertData &&
            pFt->pFnFreeCACertDataArray &&
            pFt->pFnLockCACert &&
            pFt->pFnUnlockCACert &&
            pFt->pFnAddCert &&
            pFt->pFnGetCerts &&
            pFt->pFnGetCert &&
            pFt->pFnGetRevokedCerts &&
            pFt->pFnUpdateCert &&
            pFt->pFnFreeCertData &&
            pFt->pFnFreeCertDataArray &&
            pFt->pFnLockCert &&
            pFt->pFnUnlockCert &&
            pFt->pFnFreeString &&
            pFt->pFnFreeHandle);
}

static
DWORD
_LwCADbGetAllCACertsHelper(
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    );

static
DWORD
_LwCADbGetCACertHelper(
    PCSTR                           pcszCAId,
    PCSTR                           pcszSKI,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    );

static
DWORD
_LwCADbGetActiveCACertHelper(
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    );

static
DWORD
_LwCADbGetAllCertsHelper(
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    );

static
DWORD
_LwCADbGetCertHelper(
    PCSTR                       pcszCAId,
    PCSTR                       pcszSerialNumber,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    );

static
DWORD
_LwCADbGetRevokedCertsHelper(
    PCSTR                       pcszCAId,
    PCSTR                       pcszAKI,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
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

    if (!_LwCAIsValidFunctionTable(gDbCtx.pFt))
    {
        dwError = LWCA_DB_INVALID_PLUGIN;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOG_INFO("Loaded %s DB plugin", gDbCtx.pFt->pFnGetVersion());

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
    PCSTR                       pcszCAId,
    PLWCA_DB_CA_DATA            pCAData,
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCAData || !pCARootCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnAddCA(gDbCtx.pDbHandle, pcszCAId, pCAData, pCARootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
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
        dwError = LwCADbCopyCAData(pTempCAData, &pCAData);
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
LwCADbLockCA(
    PCSTR   pcszCAId,
    PSTR    *ppszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszUuid = NULL;
    PSTR    pszTmpUuid = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !ppszUuid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnLockCA(gDbCtx.pDbHandle, pcszCAId, &pszTmpUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszTmpUuid, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeString(pszTmpUuid);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    if (ppszUuid)
    {
        *ppszUuid = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbUnlockCA(
    PCSTR   pcszCAId,
    PCSTR   pcszUuid
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || IsNullOrEmptyString(pcszUuid))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUnlockCA(gDbCtx.pDbHandle, pcszCAId, pcszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbAddCACert(
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    )
{
    DWORD                       dwError = 0;
    BOOLEAN                     bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCACert)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnAddCACert(gDbCtx.pDbHandle, pcszCAId, pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbGetCACerts(
    LWCA_DB_GET_CERTS_FLAGS         certsToGet,
    PCSTR                           pcszCAId,
    PCSTR                           pcszSKI,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    )
{
    DWORD                           dwError = 0;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACerts = NULL;

    if (!ppCACerts)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    switch (certsToGet)
    {
    case LWCA_DB_GET_ALL_CA_CERTS:

        if (IsNullOrEmptyString(pcszCAId))
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
        }

        dwError = _LwCADbGetAllCACertsHelper(pcszCAId, &pCACerts);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    case LWCA_DB_GET_ACTIVE_CA_CERT:

        if (IsNullOrEmptyString(pcszCAId))
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
        }

        dwError = _LwCADbGetActiveCACertHelper(pcszCAId, &pCACerts);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    case LWCA_DB_GET_CERT_VIA_SKI:

        dwError = _LwCADbGetCACertHelper(pcszCAId, pcszSKI, &pCACerts);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    default:

        LWCA_LOG_ERROR(
                "[%s,%d] Unsupported LWCA_DB_GET_CERTS_FLAGS presented (%d)",
                __FUNCTION__,
                __LINE__,
                certsToGet);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    *ppCACerts = pCACerts;


cleanup:
    return dwError;

error:
    LwCADbFreeRootCertDataArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }

    goto cleanup;
}

DWORD
LwCADbUpdateCACert(
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    )
{
    DWORD                       dwError = 0;
    BOOLEAN                     bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || !pCACert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUpdateCACert(gDbCtx.pDbHandle, pcszCAId, pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbLockCACert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PSTR    *ppszUuid
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;
    PSTR    pszTempUuid = NULL;
    PSTR    pszUuid = NULL;

    if (IsNullOrEmptyString(pcszCAId) || IsNullOrEmptyString(pcszSerialNumber) ||
        !ppszUuid)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnLockCACert(gDbCtx.pDbHandle, pcszCAId, pcszSerialNumber, &pszTempUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszTempUuid, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeString(pszTempUuid);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    if (ppszUuid)
    {
        *ppszUuid = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbUnlockCACert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PCSTR   pcszUuid
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) || IsNullOrEmptyString(pcszSerialNumber) ||
        IsNullOrEmptyString(pcszUuid))
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUnlockCACert(gDbCtx.pDbHandle, pcszCAId, pcszSerialNumber, pcszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbAddCert(
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

    dwError = gDbCtx.pFt->pFnAddCert(gDbCtx.pDbHandle, pcszCAId, pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbGetCerts(
    LWCA_DB_GET_CERTS_FLAGS     certsToGet,
    PCSTR                       pcszCAId,
    PCSTR                       pcszSerialNumber,
    PCSTR                       pcszAKI,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;

    if (!ppCertDataArray)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    switch (certsToGet)
    {
    case LWCA_DB_GET_ALL_CERTS:

        dwError = _LwCADbGetAllCertsHelper(pcszCAId, &pCertDataArray);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    case LWCA_DB_GET_CERT_VIA_SERIAL:

        dwError = _LwCADbGetCertHelper(pcszCAId, pcszSerialNumber, &pCertDataArray);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    case LWCA_DB_GET_REVOKED_CERTS:

        dwError = _LwCADbGetRevokedCertsHelper(pcszCAId, pcszAKI, &pCertDataArray);
        BAIL_ON_LWCA_ERROR(dwError);

        break;

    default:

        LWCA_LOG_ERROR(
                "[%s,%d] Unsupported LWCA_DB_GET_CERTS_FLAGS presented (%d)",
                __FUNCTION__,
                __LINE__,
                certsToGet);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    *ppCertDataArray = pCertDataArray;

cleanup:

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
LwCADbUpdateCert(
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

    dwError = gDbCtx.pFt->pFnUpdateCert(gDbCtx.pDbHandle, pcszCAId, pCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbLockCert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PSTR    *ppszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszUuid = NULL;
    PSTR    pszTmpUuid = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) ||
        !ppszUuid
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnLockCert(gDbCtx.pDbHandle,
                                      pcszCAId,
                                      pcszSerialNumber,
                                      &pszTmpUuid
                                      );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszTmpUuid, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeString(pszTmpUuid);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    if (ppszUuid)
    {
        *ppszUuid = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbUnlockCert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PCSTR   pcszUuid
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) ||
        IsNullOrEmptyString(pcszUuid)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = LwCADbValidateContext();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gDbCtx.pFt->pFnUnlockCert(gDbCtx.pDbHandle,
                                        pcszCAId,
                                        pcszSerialNumber,
                                        pcszUuid
                                        );
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
_LwCADbGetAllCACertsHelper(
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pTempCACerts = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetCACerts(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            &pTempCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCACerts)
    {
        dwError = LwCADbCopyRootCertDataArray(pTempCACerts, &pCACerts);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCACerts = pCACerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCACertDataArray(pTempCACerts);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeRootCertDataArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbGetCACertHelper(
    PCSTR                           pcszCAId,
    PCSTR                           pcszSKI,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_ROOT_CERT_DATA         pTempCACert = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetCACert(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            pcszSKI,
                            &pTempCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCACert)
    {
        dwError = LwCAAllocateMemory(sizeof(LWCA_DB_ROOT_CERT_DATA_ARRAY), (PVOID *)&pCACerts);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateMemory(sizeof(PLWCA_DB_ROOT_CERT_DATA), (PVOID *)&pCACerts->ppRootCertData);
        BAIL_ON_LWCA_ERROR(dwError);

        pCACerts->dwCount = 1;

        dwError = LwCADbCopyRootCertData(pTempCACert, &pCACerts->ppRootCertData[0]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCACerts = pCACerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCACertData(pTempCACert);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeRootCertDataArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbGetActiveCACertHelper(
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_CA_DATA                pTempCAData = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetCA(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            &pTempCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCADbGetCACertHelper(pcszCAId, pTempCAData->pszActiveCertSKI, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCAData(pTempCAData);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeRootCertDataArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbGetAllCertsHelper(
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_CERT_DATA_ARRAY        pTempCerts = NULL;
    PLWCA_DB_CERT_DATA_ARRAY        pCerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetCerts(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            &pTempCerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCerts)
    {
        dwError = LwCADbCopyCertDataArray(pTempCerts, &pCerts);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCerts = pCerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCertDataArray(pTempCerts);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeCertDataArray(pCerts);
    if (ppCerts)
    {
        *ppCerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbGetCertHelper(
    PCSTR                       pcszCAId,
    PCSTR                       pcszSerialNumber,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_CERT_DATA              pTempCert = NULL;
    PLWCA_DB_CERT_DATA_ARRAY        pCerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetCert(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            pcszSerialNumber,
                            &pTempCert);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCert)
    {
        dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA_ARRAY), (PVOID *)&pCerts);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateMemory(sizeof(PLWCA_DB_CERT_DATA), (PVOID *)&pCerts->ppCertData);
        BAIL_ON_LWCA_ERROR(dwError);

        pCerts->dwCount = 1;

        dwError = LwCADbCopyCertData(pTempCert, &pCerts->ppCertData[0]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCerts = pCerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCertData(pTempCert);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeCertDataArray(pCerts);
    if (ppCerts)
    {
        *ppCerts = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbGetRevokedCertsHelper(
    PCSTR                       pcszCAId,
    PCSTR                       pcszAKI,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    PLWCA_DB_CERT_DATA_ARRAY        pTempCerts = NULL;
    PLWCA_DB_CERT_DATA_ARRAY        pCerts = NULL;

    LWCA_LOCK_MUTEX_SHARED(&gDbCtx.dbMutex, bLocked);

    dwError = gDbCtx.pFt->pFnGetRevokedCerts(
                            gDbCtx.pDbHandle,
                            pcszCAId,
                            pcszAKI,
                            &pTempCerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTempCerts)
    {
        dwError = LwCADbCopyCertDataArray(pTempCerts, &pCerts);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCerts = pCerts;

cleanup:
    if (gDbCtx.isInitialized)
    {
        gDbCtx.pFt->pFnFreeCertDataArray(pTempCerts);
    }
    LWCA_LOCK_MUTEX_UNLOCK(&gDbCtx.dbMutex, bLocked);
    return dwError;

error:
    LwCADbFreeCertDataArray(pCerts);
    if (ppCerts)
    {
        *ppCerts = NULL;
    }
    goto cleanup;
}
