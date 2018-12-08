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
_LwCADbRefreshAccessToken(
    PLWCA_POST_HANDLE pHandle
    );

static
DWORD
_LwCADbGetAccessToken(
    PLWCA_POST_HANDLE   pHandle,
    PSTR                *ppszAccessToken
    );

static
DWORD
_LwCABuildHttpClient(
    PLWCA_POST_HANDLE   pHandle,
    PVM_HTTP_CLIENT     pClient,
    PCSTR               pcszSignature,
    PCSTR               pcszReqTime,
    PCSTR               pcszReqBody
    );

static
DWORD
_LwCAUriBuilder(
    VM_HTTP_METHOD      httpMethod,
    PCSTR               pcszDN,
    PCSTR               pcszScope,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PVM_HTTP_CLIENT     pClient,
    PSTR                *ppszUri
    );

static
DWORD
_LwCAGetCerts(
    PSTR    *ppszCert,
    PSTR    *ppszKey
    );

static
DWORD
_IsPutResponseValid(
    PSTR    pszResponse,
    long    statusCode
    );

static
DWORD
_IsHttpResponseValid(
    PSTR            pszResponse,
    long            statusCode,
    VM_HTTP_METHOD  httpMethod
    );

static
DWORD
_LwCARestExecuteDelete(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

static
DWORD
_LwCADbPostPluginGetCAImpl(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszAttrs,
    PSTR                *ppszResponse
    );

static
DWORD
_LwCADbPostGetCADN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszDN
    );

static
DWORD
_LwCAGetDNFromResponse(
    PCSTR   pcszResponse,
    PSTR    *ppszDN
    );

static
DWORD
_LwCADbPostGetCertDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszSerialNumber,
    PSTR                *ppszDN
    );

static
DWORD
_LwCARestExecute(
    PLWCA_POST_HANDLE   pHandle,
    VM_HTTP_METHOD      httpMethod,
    PCSTR               pcszDN,
    PCSTR               pcszScope,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PCSTR               pcszReqBody,
    PCSTR               pcszIfMatch,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

static
DWORD
_LwCARestExecutePut(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pszReqBody,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

static
DWORD
_LwCARestExecuteGet(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

static
DWORD
_LwCARestExecutePatch(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszReqBody,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

static
DWORD
_LwCAAddContainersInCA(
    PLWCA_POST_HANDLE   pPostHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszParentCA,
    PCSTR               pcszDN
    );

static
VOID
_LwCADbPostDeleteCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszParentCA,
    PCSTR               pcszParentDN
    );

static
DWORD
_LwCADbPostGetCACertsDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pszCAId,
    PSTR                *ppszCACertsDN
    );

static
DWORD
_LwCADbPostGetCARootCertsDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCARootCertsDN
    );

DWORD
LwCAPluginLoad(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{
    DWORD dwError = 0;

    if (!pFt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pFt->pFnInit                    = &LwCADbPostPluginInitialize;
    pFt->pFnGetVersion              = &LwCADbPostPluginGetVersion;

    pFt->pFnAddCA                   = &LwCADbPostPluginAddCA;
    pFt->pFnGetCA                   = &LwCADbPostPluginGetCA;
    pFt->pFnUpdateCA                = &LwCADbPostPluginUpdateCA;
    pFt->pFnFreeCAData              = &LwCADbPostPluginFreeCAData;
    pFt->pFnLockCA                  = &LwCADbPostPluginLockCA;
    pFt->pFnUnlockCA                = &LwCADbPostPluginUnlockCA;

    pFt->pFnAddCACert               = &LwCADbPostPluginAddCACert;
    pFt->pFnGetCACerts              = &LwCADbPostPluginGetCACerts;
    pFt->pFnGetCACert               = &LwCADbPostPluginGetCACert;
    pFt->pFnUpdateCACert            = &LwCADbPostPluginUpdateCACert;
    pFt->pFnFreeCACertData          = &LwCADbPostPluginFreeRootCertData;
    pFt->pFnFreeCACertDataArray     = &LwCADbPostPluginFreeRootCertDataArray;
    pFt->pFnLockCACert              = &LwCADbPostPluginLockCACert;
    pFt->pFnUnlockCACert            = &LwCADbPostPluginUnlockCACert;

    pFt->pFnAddCert                 = &LwCADbPostPluginAddCertData;
    pFt->pFnGetCerts                = &LwCADbPostPluginGetCerts;
    pFt->pFnGetCert                 = &LwCADbPostPluginGetCert;
    pFt->pFnGetRevokedCerts         = &LwCADbPostPluginGetRevokedCerts;
    pFt->pFnUpdateCert              = &LwCADbPostPluginUpdateCertData;
    pFt->pFnFreeCertData            = &LwCADbPostPluginFreeCertData;
    pFt->pFnFreeCertDataArray       = &LwCADbPostPluginFreeCertDataArray;
    pFt->pFnLockCert                = &LwCADbPostPluginLockCert;
    pFt->pFnUnlockCert              = &LwCADbPostPluginUnlockCert;

    pFt->pFnFreeString              = &LwCADbPostPluginFreeString;
    pFt->pFnFreeHandle              = &LwCADbPostPluginFreeHandle;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginInitialize(
    PCSTR               pcszConfigPath,
    PLWCA_DB_HANDLE     *ppHandle
    )
{
    DWORD               dwError = 0;
    PSTR                pszDomain = NULL;
    PLWCA_POST_HANDLE   pHandle = NULL;
    PLWCA_JSON_OBJECT   pJson = NULL;

    if (IsNullOrEmptyString(pcszConfigPath) || !ppHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_POST_HANDLE), (PVOID *)&pHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = pthread_mutex_init(&(pHandle->accessTokenMutex), NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonLoadObjectFromFile(pcszConfigPath, &pJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_LW_SERVER,
                                       &(pHandle->pszLwServer)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_POST_SERVER,
                                       &(pHandle->pszPostServer)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_DOMAIN,
                                       &pszDomain
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADNSNameToDCDN(pszDomain, &(pHandle->pszDomain));
    BAIL_ON_LWCA_ERROR(dwError);

    *ppHandle = (PLWCA_DB_HANDLE)pHandle;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDomain);
    LWCA_SAFE_JSON_DECREF(pJson);
    return dwError;

error:
    LwCADbPostPluginFreeHandle((PLWCA_DB_HANDLE)pHandle);
    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    goto cleanup;
}

PCSTR
LwCADbPostPluginGetVersion(
    VOID
    )
{
    return LWCA_DB_POST_PLUGIN_NAME" v" \
           LWCA_DB_POST_PLUGIN_VERSION_MAJOR"." \
           LWCA_DB_POST_PLUGIN_VERSION_MINOR"." \
           LWCA_DB_POST_PLUGIN_VERSION_RELEASE;
}

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CA_DATA            pCAData,
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    PSTR                pszCAReqBody = NULL;
    PSTR                pszCACertReqBody = NULL;
    PSTR                pszResponse = NULL;
    PSTR                pszParentDN = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCAData || !pCARootCertData)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    if (IsNullOrEmptyString(pCAData->pszParentCAId))
    {
        dwError = LwCAAllocateStringA(pPostHandle->pszDomain, &pszParentDN);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = _LwCADbPostGetCADN(pHandle, pCAData->pszParentCAId, &pszParentDN);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // Add CA to POST
    dwError = LwCASerializeCAToJSON(pcszCAId, pCAData, pszParentDN, &pszCAReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle, pszCAReqBody, &pszResponse, &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    // Create trusted roots and certs containers under the CA
    dwError = _LwCAAddContainersInCA(pPostHandle, pcszCAId, pCAData->pszParentCAId, pszParentDN);
    BAIL_ON_LWCA_ERROR(dwError);

    // Add trusted root cert under CA's trusted-root container
    dwError = LwCADbPostPluginAddCACert(pHandle, pcszCAId, pCARootCertData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCAReqBody);
    LWCA_SAFE_FREE_STRINGA(pszCACertReqBody);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszParentDN);
    return dwError;

error:
    _LwCADbPostDeleteCA(pHandle, pcszCAId, pCAData->pszParentCAId, pszParentDN);
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        *ppCAData
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszResponse = NULL;
    PLWCA_DB_CA_DATA        pCAData = NULL;

    if (IsNullOrEmptyString(pcszCAId) || !ppCAData || !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle, pcszCAId, NULL, &pszResponse);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToCA(pszResponse, &pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCAData = pCAData;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
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
LwCADbPostPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    )
{
    DWORD               dwError = 0;
    PSTR                pszResponse = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszBody = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCAPatchRequestBody(pCAData, &pszBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePatch(
                        (PLWCA_POST_HANDLE)pHandle,
                        pszDN,
                        pszBody,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszBody);
    return dwError;

error:
    goto cleanup;
}

VOID
LwCADbPostPluginFreeCAData(
    PLWCA_DB_CA_DATA  pCAData
    )
{
    LwCADbFreeCAData(pCAData);
}

DWORD
LwCADbPostPluginLockCA(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PSTR            *ppszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszUuid = NULL;

    if (IsNullOrEmptyString(pcszCAId) || !pHandle || !ppszUuid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCALockDN((PLWCA_POST_HANDLE)pHandle, pszDN, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
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
LwCADbPostPluginUnlockCA(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszUuid) ||
        !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAUnlockDN((PLWCA_POST_HANDLE)pHandle, pszDN, pcszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginAddCACert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    )
{
    DWORD                       dwError = 0;
    PLWCA_POST_HANDLE           pPostHandle = NULL;
    PSTR                        pszReqBody = NULL;
    PSTR                        pszCADN = NULL;
    PSTR                        pszDN = NULL;
    PSTR                        pszResponse = NULL;
    long                        statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCACert)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszDN,
                        LWCA_POST_CA_ROOT_CERT_DN,
                        pCACert->pRootCertData->pszSerialNumber,
                        pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCASerializeRootCertDataToJSON(
                        pCACert,
                        pszDN,
                        &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(
                        pPostHandle,
                        pszReqBody,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszCADN);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCACerts(
    PLWCA_DB_HANDLE                 pHandle,
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    )
{
    DWORD                           dwError = 0;
    PSTR                            pszDN = NULL;
    PSTR                            pszFilter = NULL;
    PSTR                            pszResponse = NULL;
    long                            statusCode = 0;
    PLWCA_POST_HANDLE               pPostHandle = NULL;
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pCACerts = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppCACerts)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = _LwCADbPostGetCARootCertsDN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszFilter,
                        LWCA_POST_ROOT_CERT_CA_CERTS_FILTER,
                        pcszCAId,
                        LWCA_CERT_STATUS_ACTIVE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pszDN,
                        pszFilter,
                        NULL,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToRootCertDataArray(pszResponse, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
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
LwCADbPostPluginGetCACert(
    PLWCA_DB_HANDLE                 pHandle,
    PCSTR                           pcszCAId,
    PCSTR                           pcszSKI,
    PLWCA_DB_ROOT_CERT_DATA         *ppCACert
    )
{
    DWORD                           dwError = 0;
    PSTR                            pszFilter = NULL;
    PSTR                            pszResponse = NULL;
    long                            statusCode = 0;
    PLWCA_POST_HANDLE               pPostHandle = NULL;
    PLWCA_DB_ROOT_CERT_DATA         pCACert = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszSKI) || !ppCACert)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCAAllocateStringPrintfA(
                        &pszFilter,
                        LWCA_POST_CERT_SKI_FILTER,
                        pcszSKI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pPostHandle->pszDomain,
                        pszFilter,
                        NULL,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToRootCertData(pszResponse, &pCACert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCACert = pCACert;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LwCADbFreeRootCertData(pCACert);
    if (ppCACert)
    {
        *ppCACert = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginUpdateCACert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszResponse = NULL;
    PSTR                        pszDN = NULL;
    PSTR                        pszBody = NULL;
    long                        statusCode = 0;

    if (IsNullOrEmptyString(pcszCAId) || !pCACert || !pHandle)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = _LwCADbPostGetCertDN(
                            pHandle,
                            pCACert->pRootCertData->pszSerialNumber,
                            &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateRootCertPatchRequestBody(pCACert, &pszBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePatch(
                            (PLWCA_POST_HANDLE)pHandle,
                            pszDN,
                            pszBody,
                            &pszResponse,
                            &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszBody);

    return dwError;

error:
    goto cleanup;
}

VOID
LwCADbPostPluginFreeRootCertData(
    PLWCA_DB_ROOT_CERT_DATA         pRootCertData
    )
{
    LwCADbFreeRootCertData(pRootCertData);
}

VOID
LwCADbPostPluginFreeRootCertDataArray(
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pRootCertDataArray
    )
{
    LwCADbFreeRootCertDataArray(pRootCertDataArray);
}

DWORD
LwCADbPostPluginLockCACert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PSTR            *ppszUuid
    )
{
    DWORD           dwError = 0;
    PSTR            pszUuid = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) || !ppszUuid)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCADbPostPluginLockCert(pHandle, pcszCAId, pcszSerialNumber, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;


cleanup:
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
LwCADbPostPluginUnlockCACert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PCSTR           pcszUuid
    )
{
    DWORD           dwError = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) || IsNullOrEmptyString(pcszUuid))
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCADbPostPluginUnlockCert(pHandle, pcszCAId, pcszSerialNumber, pcszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD                   dwError = 0;
    PLWCA_POST_HANDLE       pPostHandle = NULL;
    PSTR                    pszReqBody = NULL;
    PSTR                    pszCADN = NULL;
    PSTR                    pszDN = NULL;
    PSTR                    pszResponse = NULL;
    long                    statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszDN,
                        LWCA_POST_CERT_DN,
                        pCertData->pszSerialNumber,
                        pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCASerializeCertDataToJSON(
                            pCertData,
                            pszDN,
                            &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(
                            pPostHandle,
                            pszReqBody,
                            &pszResponse,
                            &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszCADN);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCerts(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszDN = NULL;
    PSTR                        pszFilter = NULL;
    PSTR                        pszResponse = NULL;
    long                        statusCode = 0;
    PLWCA_POST_HANDLE           pPostHandle = NULL;
    PLWCA_DB_CERT_DATA_ARRAY    pCerts = NULL;


    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppCerts)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = _LwCADbPostGetCACertsDN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pszDN,
                        pszFilter,
                        NULL,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToCertDataArray(pszResponse, &pCerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCerts = pCerts;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LwCADbPostPluginFreeCertDataArray(pCerts);
    if (ppCerts)
    {
        *ppCerts = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PCSTR                       pcszSerialNumber,
    PLWCA_DB_CERT_DATA          *ppCert
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszFilter = NULL;
    PSTR                        pszResponse = NULL;
    long                        statusCode = 0;
    PLWCA_POST_HANDLE           pPostHandle = NULL;
    PLWCA_DB_CERT_DATA          pCert = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszSerialNumber) || !ppCert)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCAAllocateStringPrintfA(
                        &pszFilter,
                        LWCA_POST_CERT_SERIAL_FILTER,
                        pcszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pPostHandle->pszDomain,
                        pszFilter,
                        NULL,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToCertData(pszResponse, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCert = pCert;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LwCADbPostPluginFreeCertData(pCert);
    if (ppCert)
    {
        *ppCert = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginGetRevokedCerts(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PCSTR                       pcszAKI,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszDN = NULL;
    PSTR                        pszFilter = NULL;
    PSTR                        pszResponse = NULL;
    long                        statusCode = 0;
    PLWCA_POST_HANDLE           pPostHandle = NULL;
    PLWCA_DB_CERT_DATA_ARRAY    pCerts = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszAKI) || !ppCerts)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszFilter,
                        LWCA_POST_REVOKED_CERTS_FILTER,
                        pcszAKI,
                        LWCA_CERT_STATUS_INACTIVE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pszDN,
                        pszFilter,
                        NULL,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToCertDataArray(pszResponse, &pCerts);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCerts = pCerts;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LwCADbPostPluginFreeCertDataArray(pCerts);
    if (ppCerts)
    {
        *ppCerts = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszResponse = NULL;
    PSTR                    pszDN = NULL;
    PSTR                    pszBody = NULL;
    long                    statusCode = 0;

    if (IsNullOrEmptyString(pcszCAId) ||
        !pCertData ||
        IsNullOrEmptyString(pCertData->pszSerialNumber) ||
        !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCertDN(
                            pHandle,
                            pCertData->pszSerialNumber,
                            &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCertPatchRequestBody(pCertData, &pszBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePatch(
                            (PLWCA_POST_HANDLE)pHandle,
                            pszDN,
                            pszBody,
                            &pszResponse,
                            &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszBody);

    return dwError;

error:
    goto cleanup;
}

VOID
LwCADbPostPluginFreeCertData(
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    LwCADbFreeCertData(pCertData);
}

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
    LwCADbFreeCertDataArray(pCertDataArray);
}

DWORD
LwCADbPostPluginLockCert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PSTR            *ppszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszUuid = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) ||
        !pHandle ||
        !ppszUuid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCertDN(pHandle, pcszSerialNumber, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCALockDN((PLWCA_POST_HANDLE)pHandle, pszDN, &pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUuid = pszUuid;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
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
LwCADbPostPluginUnlockCert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PCSTR           pcszUuid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) ||
        IsNullOrEmptyString(pcszUuid) ||
        !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCertDN(pHandle, pcszSerialNumber, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAUnlockDN((PLWCA_POST_HANDLE)pHandle, pszDN, pcszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

VOID
LwCADbPostPluginFreeString(
    PSTR    pszString
    )
{
    LWCA_SAFE_FREE_STRINGA(pszString);
}

VOID
LwCADbPostPluginFreeHandle(
    PLWCA_DB_HANDLE     pHandle
    )
{
    PLWCA_POST_HANDLE pPostHandle = NULL;

    if (pHandle)
    {
        pPostHandle = (PLWCA_POST_HANDLE)pHandle;

        OidcAccessTokenDelete(pPostHandle->pOidcToken);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszAccessToken);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszLwServer);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszPostServer);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszDomain);

        LWCA_SAFE_FREE_MEMORY(pPostHandle);
    }

}

DWORD
LwCADbPostCNFilterBuilder(
    PCSTR   pcszContainer,
    PCSTR   pcszObjClass,
    PSTR    *ppszResultCond
    )
{
    DWORD   dwError = 0;
    PSTR    pszResultCond = NULL;

    if (IsNullOrEmptyString(pcszContainer) ||
        IsNullOrEmptyString(pcszObjClass) ||
        !ppszResultCond)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(&pszResultCond,
                                        LWCA_POST_CN_FILTER,
                                        pcszContainer,
                                        pcszObjClass
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResultCond = pszResultCond;

cleanup:
    return dwError;

error:
    if (ppszResultCond)
    {
        *ppszResultCond = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszResultCond);
    goto cleanup;
}

DWORD
LwCARestExecutePatchImpl(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszReqBody,
    PCSTR               pcszIfMatch,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD   dwError = 0;
    PSTR    pszResponse = NULL;
    long    statusCode = 0;

    if (!pHandle ||
        IsNullOrEmptyString(pcszDN) ||
        !ppszResponse ||
        !pStatusCode
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCARestExecute(pHandle,
                               VMHTTP_METHOD_PATCH,
                               pcszDN,
                               NULL,
                               NULL,
                               NULL,
                               pcszReqBody,
                               pcszIfMatch,
                               &pszResponse,
                               &statusCode
    );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;
    *pStatusCode = statusCode;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (!ppszResponse)
    {
        *ppszResponse = NULL;
    }
    if (!pStatusCode)
    {
        *pStatusCode = 0;
    }
    goto cleanup;
}


/*
 * This is a best effort cleanup, hence we will not bail on error after placing
 * REST request. We add three entries to POST while creating a CA, the creation
 * of these entries should be atomic to the caller. Hence, we delete all the
 * entries regardless of their existence on POST.
 */
static
VOID
_LwCADbPostDeleteCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszParentCA,
    PCSTR               pcszParentDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszCertDN = NULL;
    PCSTR               pcszCertDNFormat = NULL;
    PSTR                pszCertAuthDN = NULL;
    PCSTR               pcszCertAuthDNFormat = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;

    if (!pHandle ||
        IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszParentDN))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    if (IsNullOrEmptyString(pcszParentCA))
    {
        pcszCertDNFormat = LWCA_POST_ROOT_CERTS_DN;
        pcszCertAuthDNFormat = LWCA_POST_ROOT_CA_DN_ENTRY;
    }
    else
    {
        pcszCertDNFormat = LWCA_POST_INTR_CERTS_DN;
        pcszCertAuthDNFormat = LWCA_POST_INTERMEDIATE_CA_DN_ENTRY;
    }

    // delete the cert container under CA
    dwError = LwCAAllocateStringPrintfA(&pszCertDN,
                                        pcszCertDNFormat,
                                        pcszCAId,
                                        pcszParentDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteDelete(pPostHandle,
                                     pszCertDN,
                                     &pszResponse,
                                     &statusCode
                                     );
    if (dwError == LWCA_HTTP_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_LWCA_ERROR(dwError);
    LWCA_SAFE_FREE_STRINGA(pszResponse);

    // delete the CA itself
    dwError = LwCAAllocateStringPrintfA(&pszCertAuthDN,
                                        pcszCertAuthDNFormat,
                                        pcszCAId,
                                        pcszParentDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteDelete(pPostHandle,
                                     pszCertAuthDN,
                                     &pszResponse,
                                     &statusCode
                                     );
    if (dwError == LWCA_HTTP_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_LWCA_ERROR(dwError);

error:
    LWCA_SAFE_FREE_STRINGA(pszCertDN);
    LWCA_SAFE_FREE_STRINGA(pszCertAuthDN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return;
}

static
DWORD
_LwCADbRefreshAccessToken(
    PLWCA_POST_HANDLE pHandle
    )
{
    DWORD               dwError = 0;
    PSTR                pszAccessToken = NULL;
    POIDC_ACCESS_TOKEN  pOidcToken = NULL;
    PSTR                pszDomainFormatted = NULL;
    PSTR                pszTmp = NULL;
    PLWCA_STRING_ARRAY  pRDNStrArray = NULL;
    BOOLEAN             bNoTypes = TRUE;
    DWORD               i = 0;

    if (!pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pHandle->pOidcToken)
    {
        OidcAccessTokenDelete(pHandle->pOidcToken);
        pHandle->pOidcToken = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pHandle->pszAccessToken);

    dwError = LwCADNToRDNArray(pHandle->pszDomain,
                               bNoTypes,
                               &pRDNStrArray
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    if (pRDNStrArray->dwCount > 0)
    {
       dwError = LwCAAllocateStringA(pRDNStrArray->ppData[0],
                                     &pszDomainFormatted
                                     );
       BAIL_ON_LWCA_ERROR(dwError);

        for (i = 1; i < pRDNStrArray->dwCount; ++i)
        {
            pszTmp = pszDomainFormatted;
            pszDomainFormatted = NULL;

            dwError = LwCAAllocateStringPrintfA(&pszDomainFormatted,
                                                "%s.%s",
                                                pszTmp,
                                                pRDNStrArray->ppData[i]
                                                );
            BAIL_ON_LWCA_ERROR(dwError);
            LWCA_SAFE_FREE_STRINGA(pszTmp);
        }
    }

    dwError = LwCAGetAccessToken(pHandle->pszLwServer,
                                 pszDomainFormatted,
                                 LWCA_POST_OIDC_SCOPE,
                                 &pszAccessToken
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = OidcAccessTokenParse(&pOidcToken, pszAccessToken);
    BAIL_ON_LWCA_ERROR(dwError);

    pHandle->pOidcToken = pOidcToken;
    pHandle->pszAccessToken = pszAccessToken;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDomainFormatted);
    LWCA_SAFE_FREE_STRINGA(pszTmp);
    LwCAFreeStringArray(pRDNStrArray);
    return dwError;

error:
    if (pHandle)
    {
        LWCA_SAFE_FREE_STRINGA(pHandle->pszAccessToken);
        OidcAccessTokenDelete(pHandle->pOidcToken);
        pHandle->pOidcToken = NULL;
    }
    OidcAccessTokenDelete(pOidcToken);
    LWCA_SAFE_FREE_STRINGA(pszAccessToken);

    goto cleanup;
}

static
DWORD
_LwCADbGetAccessToken(
    PLWCA_POST_HANDLE   pHandle,
    PSTR                *ppszAccessToken
    )
{
    DWORD       dwError = 0;
    bool        bLock = false;
    SSO_LONG    expirationTime = 0;
    LONG        currentTime = 0;
    PSTR        pszAccessToken = NULL;

    if (!pHandle || !ppszAccessToken)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX(bLock, &pHandle->accessTokenMutex);
    if (!pHandle->pOidcToken)
    {
        dwError = _LwCADbRefreshAccessToken(pHandle);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    expirationTime = OidcAccessTokenGetExpirationTime(pHandle->pOidcToken);
    currentTime = (LONG)time(NULL);

    if (expirationTime <= (currentTime + LWCA_EXPIRATION_BUFFER_TIME))
    {
        dwError = _LwCADbRefreshAccessToken(pHandle);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pHandle->pszAccessToken, &pszAccessToken);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszAccessToken = pszAccessToken;

cleanup:
    if (pHandle)
    {
        LWCA_UNLOCK_MUTEX(bLock, &pHandle->accessTokenMutex);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszAccessToken);
    if (ppszAccessToken)
    {
        *ppszAccessToken = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCABuildHttpClient(
    PLWCA_POST_HANDLE   pHandle,
    PVM_HTTP_CLIENT     pClient,
    PCSTR               pcszSignature,
    PCSTR               pcszReqTime,
    PCSTR               pcszReqBody
    )
{
    DWORD           dwError = 0;
    PSTR            pszAccessToken = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszSignature) || !pClient)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbGetAccessToken(pHandle, &pszAccessToken);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientSetupHOTK(pClient,
                                    pszAccessToken,
                                    pcszSignature,
                                    pcszReqTime,
                                    pcszReqBody
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszAccessToken);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAUrlEncodeString(
    PVM_HTTP_CLIENT pClient,
    PCSTR           pcszParam,
    PCSTR           pcszAttrType,
    PCSTR           pcszCurrURI,
    BOOLEAN         paramSet,
    PSTR            *ppszNewURI
    )
{
    DWORD   dwError = 0;
    PSTR    pszEscapedStr = NULL;
    PSTR    pszNewURI = NULL;

    if (!pClient ||
        IsNullOrEmptyString(pcszParam) ||
        IsNullOrEmptyString(pcszAttrType) ||
        !ppszNewURI
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = VmHttpUrlEncodeString(pClient, pcszParam, &pszEscapedStr);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszNewURI,
                                        "%s%s%s=%s",
                                        LWCA_SAFE_STRING(pcszCurrURI),
                                        LWCA_PARAM_DELIM(paramSet),
                                        pcszAttrType,
                                        pszEscapedStr
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszNewURI = pszNewURI;

cleanup:
    LWCA_SAFE_FREE_CURL_MEMORY(pszEscapedStr);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszNewURI);
    if (ppszNewURI)
    {
        *ppszNewURI = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAUriBuilder(
    VM_HTTP_METHOD      httpMethod,
    PCSTR               pcszDN,
    PCSTR               pcszScope,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PVM_HTTP_CLIENT     pClient,
    PSTR                *ppszUri
    )
{
    DWORD   dwError = 0;
    PSTR    pszUri = NULL;
    PSTR    pszTmp = NULL;
    BOOLEAN paramSet = FALSE;

    if (!pClient || !ppszUri)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(LWCA_POST_URI_PREFIX, &pszUri);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszDN) &&
        (httpMethod == VMHTTP_METHOD_GET ||
         httpMethod == VMHTTP_METHOD_DELETE ||
         httpMethod == VMHTTP_METHOD_PATCH)
        )
    {
        pszTmp = pszUri;
        pszUri = NULL;

        dwError = _LwCAUrlEncodeString(pClient,
                                       pcszDN,
                                       LWCA_LDAP_DN,
                                       pszTmp,
                                       paramSet,
                                       &pszUri
                                       );
        BAIL_ON_LWCA_ERROR(dwError);

        paramSet = TRUE;
        LWCA_SAFE_FREE_STRINGA(pszTmp);
    }

    if (!IsNullOrEmptyString(pcszScope) && httpMethod == VMHTTP_METHOD_GET)
    {
        pszTmp = pszUri;
        pszUri = NULL;

        dwError = _LwCAUrlEncodeString(pClient,
                                       pcszScope,
                                       LWCA_LDAP_SCOPE,
                                       pszTmp,
                                       paramSet,
                                       &pszUri
                                       );
        BAIL_ON_LWCA_ERROR(dwError);

        paramSet = TRUE;
        LWCA_SAFE_FREE_STRINGA(pszTmp);
    }

    if (!IsNullOrEmptyString(pcszFilter) && httpMethod == VMHTTP_METHOD_GET)
    {
        pszTmp = pszUri;
        pszUri = NULL;

        dwError = _LwCAUrlEncodeString(pClient,
                                       pcszFilter,
                                       LWCA_LDAP_FILTER,
                                       pszTmp,
                                       paramSet,
                                       &pszUri
                                       );
        BAIL_ON_LWCA_ERROR(dwError);

        paramSet = TRUE;
        LWCA_SAFE_FREE_STRINGA(pszTmp);
    }
    if (!IsNullOrEmptyString(pcszAttrs) && httpMethod == VMHTTP_METHOD_GET)
    {
        pszTmp = pszUri;
        pszUri = NULL;

        dwError = _LwCAUrlEncodeString(pClient,
                                       pcszAttrs,
                                       LWCA_LDAP_ATTRS,
                                       pszTmp,
                                       paramSet,
                                       &pszUri
                                       );
        BAIL_ON_LWCA_ERROR(dwError);
    }

   *ppszUri = pszUri;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszTmp);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUri);
    if (ppszUri)
    {
       *ppszUri = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAGetCerts(
    PSTR    *ppszCert,
    PSTR    *ppszKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;

    dwError = LwCAGetVecsMutentCACert(&pszCert, &pszKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCert = pszCert;
    *ppszKey = pszKey;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCert);
    LWCA_SAFE_FREE_STRINGA(pszKey);
    goto cleanup;
}

/*
 * GET requests are strict. dwError is set if statusCode is not equal to 200.
 */
static
DWORD
_LwCARestExecuteGet(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD   dwError = 0;
    PSTR    pszResponse = NULL;
    long    statusCode = 0;

    if (!pHandle || !ppszResponse || !pStatusCode)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCARestExecute(pHandle,
                               VMHTTP_METHOD_GET,
                               pcszDN,
                               LWCA_LDAP_SCOPE_SUB,
                               pcszFilter,
                               pcszAttrs,
                               NULL,
                               NULL,
                               &pszResponse,
                               &statusCode
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _IsHttpResponseValid(pszResponse, statusCode, VMHTTP_METHOD_GET);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;
    *pStatusCode = statusCode;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    if (pStatusCode)
    {
        *pStatusCode = 0;
    }
    goto cleanup;
}

static
DWORD
_IsHttpResponseValid(
    PSTR            pszResponse,
    long            statusCode,
    VM_HTTP_METHOD  httpMethod
    )
{
    DWORD       dwError = 0;
    PCSTR       pcszMethod = NULL;

    if (statusCode == LWCA_HTTP_NOT_FOUND)
    {
        dwError = LWCA_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (statusCode != LWCA_HTTP_OK)
    {
        dwError = VmHttpGetRequestMethodInString(httpMethod, &pcszMethod);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_LOG_ERROR("%s:%d - Method: %s, Status Code: %ld, Response: %s",
                       __FUNCTION__,
                       __LINE__,
                       pcszMethod,
                       statusCode,
                       pszResponse
                       );
        switch (httpMethod)
        {
            case VMHTTP_METHOD_GET:
                dwError = LWCA_LDAP_GET_FAILED;
                break;

            case VMHTTP_METHOD_PATCH:
                dwError = LWCA_LDAP_PATCH_FAILED;
                break;

            case VMHTTP_METHOD_DELETE:
                dwError = LWCA_LDAP_DELETE_FAILED;
                break;

            default:
                dwError = LWCA_LDAP_UNKNOWN_OP;
                break;
        }
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCARestExecuteDelete(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD   dwError = 0;
    PSTR    pszResponse = NULL;
    long    statusCode = 0;

    if (!pHandle ||
        !ppszResponse ||
        !pStatusCode ||
        IsNullOrEmptyString(pcszDN)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCARestExecute(pHandle,
                               VMHTTP_METHOD_DELETE,
                               pcszDN,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               &pszResponse,
                               &statusCode
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _IsHttpResponseValid(pszResponse,
                                   statusCode,
                                   VMHTTP_METHOD_DELETE
                                   );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCARestExecutePut(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pszReqBody,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD           dwError = 0;
    PSTR            pszResponse = NULL;
    long            statusCode = 0;

    if (!pHandle ||
        !ppszResponse ||
        !pStatusCode ||
        IsNullOrEmptyString(pszReqBody)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCARestExecute(pHandle,
                               VMHTTP_METHOD_PUT,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               pszReqBody,
                               NULL,
                               &pszResponse,
                               &statusCode
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _IsPutResponseValid(pszResponse, statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;
    *pStatusCode = statusCode;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    if (pStatusCode)
    {
        *pStatusCode = 0;
    }
    goto cleanup;

}

static
DWORD
_LwCARestExecute(
    PLWCA_POST_HANDLE   pHandle,
    VM_HTTP_METHOD      httpMethod,
    PCSTR               pcszDN,
    PCSTR               pcszScope,
    PCSTR               pcszFilter,
    PCSTR               pcszAttrs,
    PCSTR               pcszReqBody,
    PCSTR               pcszIfMatch,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD               dwError = 0;
    PSTR                pszReqUri = NULL;
    PSTR                pszReqTime = NULL;
    PSTR                pszCert = NULL;
    PSTR                pszKey = NULL;
    PSTR                pszSignature = NULL;
    PSTR                pszUrl = NULL;
    PCSTR               pcszSafeReqBody = NULL;
    PVM_HTTP_CLIENT     pClient = NULL;
    PCSTR               pcszResult = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;

    if (!pHandle || !pStatusCode || !ppszResponse)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pcszSafeReqBody = LWCA_SAFE_STRING(pcszReqBody);

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAUriBuilder(httpMethod,
                              pcszDN,
                              pcszScope,
                              pcszFilter,
                              pcszAttrs,
                              pClient,
                              &pszReqUri
                              );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetCurrentTime(&pszReqTime);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetCerts(&pszCert, &pszKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientSignRequest(httpMethod,
                                      pszReqUri,
                                      pcszSafeReqBody,
                                      pszKey,
                                      pszReqTime,
                                      &pszSignature
                                      );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCABuildHttpClient(pHandle,
                                   pClient,
                                   pszSignature,
                                   pszReqTime,
                                   pcszSafeReqBody
                                   );
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszIfMatch))
    {
        dwError = VmHttpClientSetHeader(pClient,
                                        LWCA_POST_REST_IF_MATCH,
                                        pcszIfMatch
                                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = VmFormatUrl(LWCA_POST_REST_HTTPS,
                          pHandle->pszPostServer,
                          LWCA_POST_REST_PORT,
                          pszReqUri,
                          NULL,
                          &pszUrl
                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientPerform(pClient, httpMethod, pszUrl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetStatusCode(pClient, &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetResult(pClient, &pcszResult);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszResult, &pszResponse);
    BAIL_ON_LWCA_ERROR(dwError);

    *pStatusCode = statusCode;
    *ppszResponse = pszResponse;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqUri);
    LWCA_SAFE_FREE_STRINGA(pszReqTime);
    LWCA_SAFE_FREE_STRINGA(pszCert);
    LWCA_SAFE_FREE_STRINGA(pszKey);
    LWCA_SAFE_FREE_STRINGA(pszSignature);
    LWCA_SAFE_FREE_STRINGA(pszUrl);
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    if (pStatusCode)
    {
        *pStatusCode = 0;
    }
    goto cleanup;
}

static
DWORD
_IsPutResponseValid(
    PSTR    pszResponse,
    long    statusCode
    )
{
    DWORD               dwError = 0;

    if (statusCode != LWCA_HTTP_OK && statusCode != LWCA_HTTP_CONFLICT)
    {
        LWCA_LOG_ERROR("%s:%d - Status Code: %ld, Response: %s",
                       __FUNCTION__,
                       __LINE__,
                       statusCode,
                       pszResponse
                       );
        dwError = LWCA_LDAP_ADD_FAILED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCARestExecutePatch(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszReqBody,
    PSTR                *ppszResponse,
    long                *pStatusCode
    )
{
    DWORD   dwError = 0;
    PSTR    pszResponse = NULL;
    long    statusCode = 0;
    PSTR    pszUuid = NULL;

    if (!pHandle ||
        IsNullOrEmptyString(pcszDN) ||
        !ppszResponse ||
        !pStatusCode
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestExecutePatchImpl(pHandle,
                                       pcszDN,
                                       pcszReqBody,
                                       NULL,
                                       &pszResponse,
                                       &statusCode
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _IsHttpResponseValid(pszResponse, statusCode, VMHTTP_METHOD_PATCH);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;
    *pStatusCode = statusCode;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    if (pStatusCode)
    {
        *pStatusCode = 0;
    }
    goto cleanup;
}

static
DWORD
_LwCADbPostPluginGetCAImpl(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszAttrs,
    PSTR                *ppszResponse
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszFilter = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppszResponse)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

   dwError = LwCADbPostCNFilterBuilder(pcszCAId,
                                       LWCA_POST_CA_OBJ_CLASS,
                                       &pszFilter
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszDN,
                                        LWCA_POST_CA_DN,
                                        pPostHandle->pszDomain
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(pPostHandle,
                                  pszDN,
                                  pszFilter,
                                  LWCA_SAFE_STRING(pcszAttrs),
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    if (ppszResponse)
    {
        *ppszResponse = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbPostGetCADN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszDN = NULL;
    PSTR                pszResponse = NULL;

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_LDAP_CN,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetDNFromResponse(pszResponse, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    if (ppszDN)
    {
        *ppszDN = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetDNFromResponse(
    PCSTR   pcszResponse,
    PSTR    *ppszDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszDN = NULL;
    PLWCA_JSON_OBJECT   pJson = NULL;
    PLWCA_JSON_OBJECT   pResult = NULL;
    PLWCA_JSON_OBJECT   pResultElem = NULL;

    dwError = LwCAJsonLoadObjectFromString(pcszResponse, &pJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, LWCA_RESP_RESULT, &pResult);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetBorrowedRef(pResult, 0, &pResultElem);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pResultElem, FALSE, LWCA_LDAP_DN, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;

cleanup:
    LWCA_SAFE_JSON_DECREF(pJson);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    if (ppszDN)
    {
        *ppszDN = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbPostGetCertDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszSerialNumber,
    PSTR                *ppszDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszResponse = NULL;
    PSTR                pszFilter = NULL;
    PSTR                pszSearchBaseDN = NULL;
    PSTR                pszDN = NULL;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    long                statusCode = 0;

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCAAllocateStringPrintfA(
                        &pszFilter,
                        LWCA_POST_CERT_SERIAL_FILTER,
                        pcszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszSearchBaseDN,
                        LWCA_POST_CA_DN,
                        pPostHandle->pszDomain);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(
                        pPostHandle,
                        pszSearchBaseDN,
                        LWCA_SAFE_STRING(pszFilter),
                        LWCA_POST_DN,
                        &pszResponse,
                        &statusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetDNFromResponse(pszResponse, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    LWCA_SAFE_FREE_STRINGA(pszSearchBaseDN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    if (ppszDN)
    {
        *ppszDN = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAAddContainersInCA(
    PLWCA_POST_HANDLE   pPostHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszParentCA,
    PCSTR               pcszDN
    )
{
    DWORD       dwError = 0;
    PSTR        pszRootsReqBody = NULL;
    PSTR        pszCertsReqBody = NULL;
    PSTR        pszRootsDN = NULL;
    PSTR        pszCertsDN = NULL;
    PSTR        pszRootsResponse = NULL;
    PSTR        pszCertsResponse = NULL;
    PCSTR       pcszRootsDnFormat = NULL;
    PCSTR       pcszCertsDnFormat = NULL;
    long        statusCode = 0;

    if (!pPostHandle || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pcszParentCA))
    {
        pcszRootsDnFormat = LWCA_POST_ROOT_ROOTS_DN;
        pcszCertsDnFormat = LWCA_POST_ROOT_CERTS_DN;
    }
    else
    {
        pcszRootsDnFormat = LWCA_POST_INTR_ROOTS_DN;
        pcszCertsDnFormat = LWCA_POST_INTR_CERTS_DN;
    }

    dwError = LwCAAllocateStringPrintfA(&pszRootsDN,
                                        pcszRootsDnFormat,
                                        pcszCAId,
                                        pcszDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszCertsDN,
                                        pcszCertsDnFormat,
                                        pcszCAId,
                                        pcszDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASerializeContainerToJSON(pszRootsDN,
                                           LWCA_POST_ROOTS_CN,
                                           &pszRootsReqBody
                                           );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASerializeContainerToJSON(pszCertsDN,
                                           LWCA_POST_CERTS_CN,
                                           &pszCertsReqBody
                                           );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszRootsReqBody,
                                  &pszRootsResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszCertsReqBody,
                                  &pszCertsResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszRootsDN);
    LWCA_SAFE_FREE_STRINGA(pszCertsDN);
    LWCA_SAFE_FREE_STRINGA(pszRootsReqBody);
    LWCA_SAFE_FREE_STRINGA(pszCertsReqBody);
    LWCA_SAFE_FREE_STRINGA(pszRootsResponse);
    LWCA_SAFE_FREE_STRINGA(pszCertsResponse);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCADbPostGetCACertsDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCACertsDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszCADN = NULL;
    PSTR                pszCACertsDN = NULL;

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszCACertsDN,
                        LWCA_POST_CERTS_DN,
                        pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCACertsDN = pszCACertsDN;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCADN);

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCACertsDN);
    if (ppszCACertsDN)
    {
        *ppszCACertsDN = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCADbPostGetCARootCertsDN(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCARootCertsDN
    )
{
    DWORD               dwError = 0;
    PSTR                pszCADN = NULL;
    PSTR                pszCARootCertsDN = NULL;

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszCARootCertsDN,
                        LWCA_POST_ROOT_CERTS_DN,
                        pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCARootCertsDN = pszCARootCertsDN;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCADN);

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCARootCertsDN);
    if (ppszCARootCertsDN)
    {
        *ppszCARootCertsDN = NULL;
    }
    goto cleanup;
}
