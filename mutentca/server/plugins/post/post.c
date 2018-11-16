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
_LwCAAddRootCAToConfig(
    PLWCA_POST_HANDLE   pPostHandle,
    PCSTR               pcszCAId
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
_LwCADbPostPluginGetCertDataImpl(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszAttr,
    PCSTR               pcszFilter,
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
    PCSTR               pcszCAId,
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
_LwCAAddCertContainerInCA(
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

    pFt->pFnInit = &LwCADbPostPluginInitialize;
    pFt->pFnAddCA = &LwCADbPostPluginAddCA;
    pFt->pFnAddCertData = &LwCADbPostPluginAddCertData;
    pFt->pFnCheckCA = &LwCADbPostPluginCheckCA;
    pFt->pFnCheckCertData = &LwCADbPostPluginCheckCertData;
    pFt->pFnGetCACertificates = &LwCADbPostPluginGetCACertificates;
    pFt->pFnGetCA = &LwCADbPostPluginGetCA;
    pFt->pFnGetCertData = &LwCADbPostPluginGetCertData;
    pFt->pFnGetCACRLNumber = &LwCADbPostPluginGetCACRLNumber;
    pFt->pFnGetParentCAId = &LwCADbPostPluginGetParentCAId;
    pFt->pFnUpdateCA = &LwCADbPostPluginUpdateCA;
    pFt->pFnUpdateCAStatus = &LwCADbPostPluginUpdateCAStatus;
    pFt->pFnUpdateCACRLNumber = &LwCADbPostPluginUpdateCACRLNumber;
    pFt->pFnFreeCAData = &LwCADbPostPluginFreeCAData;
    pFt->pFnUpdateCertData = &LwCADbPostPluginUpdateCertData;
    pFt->pFnFreeCertDataArray = &LwCADbPostPluginFreeCertDataArray;
    pFt->pFnFreeCertArray = &LwCADbPostPluginFreeCertificates;
    pFt->pFnFreeString = &LwCADbPostPluginFreeString;
    pFt->pFnFreeHandle = &LwCADbPostPluginFreeHandle;

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
    LwCAJsonCleanupObject(pJson);
    return dwError;

error:
    LwCADbPostPluginFreeHandle((PLWCA_DB_HANDLE)pHandle);
    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PLWCA_DB_CA_DATA    pCAData,
    PCSTR               pcszParentCA
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    PSTR                pszReqBody = NULL;
    PSTR                pszResponse = NULL;
    PSTR                pszParentDN = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCAData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    if (IsNullOrEmptyString(pcszParentCA))
    {
        dwError = _LwCAAddRootCAToConfig(pPostHandle, pcszCAId);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateStringA(pPostHandle->pszDomain, &pszParentDN);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = _LwCADbPostGetCADN(pHandle, pcszParentCA, &pszParentDN);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASerializeCAToJSON(pcszCAId,
                                    pCAData,
                                    pcszParentCA,
                                    pszParentDN,
                                    &pszReqBody
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszReqBody,
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    // create certs container under the CA
    dwError = _LwCAAddCertContainerInCA(pPostHandle,
                                        pcszCAId,
                                        pcszParentCA,
                                        pszParentDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszParentDN);
    return dwError;

error:
    _LwCADbPostDeleteCA(pHandle, pcszCAId, pcszParentCA, pszParentDN);
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
    PSTR                pszConfigDN = NULL;
    PSTR                pszCertDN = NULL;
    PCSTR               pcszCertDNFormat = NULL;
    PSTR                pszCertAuthDN = NULL;
    PCSTR               pcszCertAuthDNFormat = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;

    if (!pHandle ||
        IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszParentDN)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    if (IsNullOrEmptyString(pcszParentCA))
    {
        // delete the configuration entry
        dwError = LwCAAllocateStringPrintfA(&pszConfigDN,
                                            LWCA_POST_CA_CONFIG_DN,
                                            pcszCAId
                                            );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCARestExecuteDelete(pPostHandle,
                                         pszConfigDN,
                                         &pszResponse,
                                         &statusCode
                                         );
        if (dwError == LWCA_HTTP_NOT_FOUND)
        {
            dwError = 0;
        }
        BAIL_ON_LWCA_ERROR(dwError);
        LWCA_SAFE_FREE_STRINGA(pszResponse);

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
    LWCA_SAFE_FREE_STRINGA(pszConfigDN);
    LWCA_SAFE_FREE_STRINGA(pszCertDN);
    LWCA_SAFE_FREE_STRINGA(pszCertAuthDN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return;
}

DWORD
LwCADbPostPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    PSTR                pszReqBody = NULL;
    PSTR                pszCADN = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !pCertData)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = LwCASerializeCertDataToJSON(pcszCAId,
                                          pCertData,
                                          pszCADN,
                                          &pszReqBody
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszReqBody,
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszCADN);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginCheckCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PBOOLEAN            pbExists
    )
{
    DWORD       dwError = 0;
    PSTR        pszResponse = NULL;
    PSTR        pszCNValue = NULL;

    if (IsNullOrEmptyString(pcszCAId) || !pbExists || !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_LDAP_CN,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetStringAttrFromResponse(pszResponse,
                                            LWCA_LDAP_CN,
                                            &pszCNValue
                                            );
    BAIL_ON_LWCA_ERROR(dwError);

    *pbExists = TRUE;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCNValue);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    if (pbExists)
    {
        *pbExists = FALSE;
    }
    if (dwError == LWCA_ERROR_ENTRY_NOT_FOUND)
    {
        // there was no error in the request. Entry just didn't exist
        dwError = 0;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginCheckCertData(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszSerialNumber,
    PBOOLEAN            pbExists
    )
{
    DWORD           dwError = 0;
    PSTR            pszResponse = NULL;
    PSTR            pszCNValue = NULL;
    PSTR            pszFilter = NULL;

    if (IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszSerialNumber) ||
        !pbExists ||
        !pHandle
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbPostCNFilterBuilder(pcszSerialNumber,
                                        LWCA_POST_CERT_OBJ_CLASS,
                                        &pszFilter
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCADbPostPluginGetCertDataImpl(pHandle,
                                               pcszCAId,
                                               LWCA_LDAP_CN,
                                               pszFilter,
                                               &pszResponse
                                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetStringAttrFromResponse(pszResponse,
                                            LWCA_LDAP_CN,
                                            &pszCNValue
                                            );
    BAIL_ON_LWCA_ERROR(dwError);

    *pbExists = TRUE;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszCNValue);
    LWCA_SAFE_FREE_STRINGA(pszFilter);
    return dwError;

error:
    if (pbExists)
    {
        *pbExists = FALSE;
    }
    if (dwError == LWCA_ERROR_ENTRY_NOT_FOUND)
    {
        dwError = 0;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE          pHandle,
    PCSTR                    pcszCAId,
    PLWCA_DB_CA_DATA         *ppCAData
    )
{
    DWORD               dwError = 0;
    PSTR                pszResponse = NULL;
    PLWCA_DB_CA_DATA    pCAData = NULL;

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
LwCADbPostPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_CERTIFICATE_ARRAY      *ppCertArray
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszResponse = NULL;
    PLWCA_STRING_ARRAY          pStrArray = NULL;
    PLWCA_CERTIFICATE_ARRAY     pCertArray = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppCertArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_POST_CA_CERTIFICATES,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetStringArrayAttrFromResponse(pszResponse,
                                                 LWCA_POST_CA_CERTIFICATES,
                                                 &pStrArray
                                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    if (pStrArray)
    {
        dwError = LwCAGetCertArrayFromEncodedStringArray(pStrArray, &pCertArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertArray = pCertArray;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LwCAFreeStringArray(pStrArray);
    return dwError;

error:
    LwCADbPostPluginFreeCertificates(pCertArray);
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }
    goto cleanup;

}

DWORD
LwCADbPostPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszResponse = NULL;
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppCertDataArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCertDataImpl(pHandle,
                                               pcszCAId,
                                               NULL,
                                               LWCA_POST_CERT_OBJ_FILTER,
                                               &pszResponse
                                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADeserializeJSONToCertData(pszResponse, &pCertDataArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertDataArray = pCertDataArray;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LwCADbPostPluginFreeCertDataArray(pCertDataArray);
    if (ppCertDataArray)
    {
        *ppCertDataArray = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginGetCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCRLNumber
    )
{
    DWORD           dwError = 0;
    PSTR            pszCRLNumber = NULL;
    PSTR            pszResponse = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppszCRLNumber)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_POST_CA_CRL_NUM,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetStringAttrFromResponse(pszResponse,
                                            LWCA_POST_CA_CRL_NUM,
                                            &pszCRLNumber
                                            );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCRLNumber = pszCRLNumber;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
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
LwCADbPostPluginGetParentCAId(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszParentCAId
    )
{
    DWORD               dwError = 0;
    PSTR                pszParentCAId = NULL;
    PSTR                pszResponse = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppszParentCAId)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_POST_CA_PARENT,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetStringAttrFromResponse(pszResponse,
                                            LWCA_POST_CA_PARENT,
                                            &pszParentCAId
                                            );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszParentCAId = pszParentCAId;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
    if (ppszParentCAId)
    {
        *ppszParentCAId = pszParentCAId;
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

    dwError = _LwCARestExecutePatch((PLWCA_POST_HANDLE)pHandle,
                                    pszDN,
                                    pszBody,
                                    &pszResponse,
                                    &statusCode
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszBody);
    return dwError;

error:
    goto cleanup;
}


DWORD
LwCADbPostPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_CA_STATUS          status
    )
{
    DWORD               dwError = 0;
    PLWCA_DB_CA_DATA    pCAData = NULL;

    if (!pHandle || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbCreateCAData(NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 status,
                                 &pCAData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbPostPluginUpdateCA(pHandle, pcszCAId, pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCADbPostPluginFreeCAData(pCAData);

    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    DWORD               dwError = 0;
    PSTR                pszResponse = NULL;
    PSTR                pszDN = NULL;
    PSTR                pszBody = NULL;
    long                statusCode = 0;

    if (IsNullOrEmptyString(pcszCAId) ||
        !pCertData ||
        IsNullOrEmptyString(pCertData->pszSerialNumber) ||
        !pHandle
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostGetCertDN(pHandle, pcszCAId, pCertData->pszSerialNumber, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateCertPatchRequestBody(pCertData, &pszBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePatch((PLWCA_POST_HANDLE)pHandle,
                                    pszDN,
                                    pszBody,
                                    &pszResponse,
                                    &statusCode
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszBody);

    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginUpdateCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszCRLNumber
    )
{
    DWORD               dwError = 0;
    PLWCA_DB_CA_DATA    pCAData = NULL;
    PSTR                pszResponse = NULL;
    int                 status = 0;

    if (!pHandle ||
        IsNullOrEmptyString(pcszCAId) ||
        IsNullOrEmptyString(pcszCRLNumber)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCADbPostPluginGetCAImpl(pHandle,
                                         pcszCAId,
                                         LWCA_POST_CA_STATUS,
                                         &pszResponse
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetIntAttrFromResponse(pszResponse,
                                         LWCA_POST_CA_STATUS,
                                         &status
                                         );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbCreateCAData(NULL,
                                 NULL,
                                 NULL,
                                 pcszCRLNumber,
                                 NULL,
                                 NULL,
                                 NULL,
                                 status,
                                 &pCAData
                                 );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbPostPluginUpdateCA(pHandle, pcszCAId, pCAData);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LwCADbPostPluginFreeCAData(pCAData);
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

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
    LwCADbFreeCertDataArray(pCertDataArray);
}

VOID
LwCADbPostPluginFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    )
{
    LwCAFreeCertificates(pCertArray);
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
        !ppszResultCond
        )
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

static
DWORD
_LwCAAddRootCAToConfig(
    PLWCA_POST_HANDLE   pPostHandle,
    PCSTR               pcszCAId
    )
{
    DWORD               dwError = 0;
    PSTR                pszReqBody = NULL;
    PSTR                pszResponse = NULL;
    PSTR                pszDN = NULL;
    long                statusCode = 0;

    if (!pPostHandle || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(&pszDN,
                                        LWCA_POST_CA_CONFIG_DN,
                                        pcszCAId,
                                        pPostHandle->pszDomain
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASerializeContainerToJSON(pszDN, pcszCAId, &pszReqBody);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszReqBody,
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszDN);
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
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
                               &pszResponse,
                               &statusCode
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _IsHttpResponseValid(pszResponse, statusCode, VMHTTP_METHOD_PATCH);
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
_LwCADbPostPluginGetCertDataImpl(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszAttr,
    PCSTR               pcszFilter,
    PSTR                *ppszResponse
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pPostHandle = NULL;
    PSTR                pszCADN = NULL;
    PSTR                pszResponse = NULL;
    long                statusCode = 0;

    if (!pHandle || IsNullOrEmptyString(pcszCAId) || !ppszResponse)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPostHandle = (PLWCA_POST_HANDLE)pHandle;

    dwError = _LwCADbPostGetCADN(pHandle, pcszCAId, &pszCADN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecuteGet(pPostHandle,
                                  pszCADN,
                                  LWCA_SAFE_STRING(pcszFilter),
                                  LWCA_SAFE_STRING(pcszAttr),
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszResponse = pszResponse;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCADN);
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
    PCSTR               pcszCAId,
    PCSTR               pcszSerialNumber,
    PSTR                *ppszDN
    )
{
    DWORD       dwError = 0;
    PSTR        pszDN = NULL;
    PSTR        pszResponse = NULL;
    PSTR        pszFilter = NULL;

    dwError = LwCADbPostCNFilterBuilder(pcszSerialNumber,
                                        LWCA_POST_CERT_OBJ_CLASS,
                                        &pszFilter
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCADbPostPluginGetCertDataImpl(pHandle,
                                               pcszCAId,
                                               LWCA_LDAP_CN,
                                               pszFilter,
                                               &pszResponse
                                               );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetDNFromResponse(pszResponse, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszFilter);
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
_LwCAAddCertContainerInCA(
    PLWCA_POST_HANDLE   pPostHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszParentCA,
    PCSTR               pcszDN
    )
{
    DWORD       dwError = 0;
    PSTR        pszReqBody = NULL;
    PSTR        pszResponse = NULL;
    PSTR        pszCertDN = NULL;
    PCSTR       pcszDnFormat = NULL;
    long        statusCode = 0;

    if (!pPostHandle || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pcszParentCA))
    {
        pcszDnFormat = LWCA_POST_ROOT_CERTS_DN;
    }
    else
    {
        pcszDnFormat = LWCA_POST_INTR_CERTS_DN;
    }

    dwError = LwCAAllocateStringPrintfA(&pszCertDN,
                                        pcszDnFormat,
                                        pcszCAId,
                                        pcszDN
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASerializeContainerToJSON(pszCertDN,
                                           LWCA_POST_CERTS_CN,
                                           &pszReqBody
                                           );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszReqBody,
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCertDN);
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
    goto cleanup;
}

