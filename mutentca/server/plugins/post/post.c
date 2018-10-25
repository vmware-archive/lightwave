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
_LwCARestExecute(
    PLWCA_POST_HANDLE   pHandle,
    VM_HTTP_METHOD      httpMethod,
    PCSTR               pcszDN,
    PCSTR               pcszScope,
    PCSTR               pcszFilter,
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
                                       &(pHandle->pszDomain)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppHandle = (PLWCA_DB_HANDLE)pHandle;

cleanup:
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
    }

    dwError = LwCASerializeCAToJSON(pcszCAId,
                                    pCAData,
                                    pcszParentCA,
                                    pPostHandle->pszDomain,
                                    &pszReqBody
                                    );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestExecutePut(pPostHandle,
                                  pszReqBody,
                                  &pszResponse,
                                  &statusCode
                                  );
    BAIL_ON_LWCA_ERROR(dwError);
    /* TODO -   the _LwCAAddRootCAToConfig step must be undone in case of a
     *          failure. This will be added when delete CA is implemented.
     */

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    LWCA_SAFE_FREE_STRINGA(pszResponse);
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
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginCheckCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PBOOLEAN            pbExists
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginCheckCertData(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszSerialNumber,
    PBOOLEAN            pbExists
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE          pHandle,
    PCSTR                    pcszCAId,
    PLWCA_DB_CA_DATA         *ppCAData
)
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_CERTIFICATE_ARRAY      *ppCertArray
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCRLNumber
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetParentCAId(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszParentCAId
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    )
{
    return LWCA_NOT_IMPLEMENTED;
}


DWORD
LwCADbPostPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_CA_STATUS          status
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszCRLNumber
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

VOID
LwCADbPostPluginFreeCAData(
    PLWCA_DB_CA_DATA  pCAData
    )
{
}

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
}

VOID
LwCADbPostPluginFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    )
{
}

VOID
LwCADbPostPluginFreeString(
    PSTR    pszString
    )
{
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
        (httpMethod == VMHTTP_METHOD_GET || httpMethod == VMHTTP_METHOD_DELETE)
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
    long                statusCode = 0;

    if (!pPostHandle || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCASerializeConfigCAToJSON(pcszCAId,
                                          pPostHandle->pszDomain,
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
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    return dwError;

error:
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
