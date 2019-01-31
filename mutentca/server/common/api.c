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

static
DWORD
_LwCAGetCARestEndpoint(
    PCSTR   pcszCAId,
    PSTR    *ppszUri
    );

DWORD
LwCACreateIntCARequest(
    PLWCA_STRING_ARRAY      pCountryList,
    PLWCA_STRING_ARRAY      pStateList,
    PLWCA_STRING_ARRAY      pLocalityList,
    PLWCA_STRING_ARRAY      pOUList,
    PCSTR                   pcszPolicy,
    PLWCA_INT_CA_REQ_DATA   *ppIntCARequest
    )
{
    DWORD dwError = 0;
    PLWCA_INT_CA_REQ_DATA pIntCARequest = NULL;

    if (!ppIntCARequest)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_INT_CA_REQ_DATA), (PVOID*)&pIntCARequest);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCountryList)
    {
        dwError = LwCACopyStringArray(pCountryList, &pIntCARequest->pCountryList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pStateList)
    {
        dwError = LwCACopyStringArray(pStateList, &pIntCARequest->pStateList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pLocalityList)
    {
        dwError = LwCACopyStringArray(pLocalityList, &pIntCARequest->pLocalityList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pOUList)
    {
        dwError = LwCACopyStringArray(pOUList, &pIntCARequest->pOUList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pcszPolicy)
    {
        dwError = LwCAAllocateStringA(pcszPolicy, &pIntCARequest->pszPolicy);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppIntCARequest = pIntCARequest;

cleanup:
    return dwError;

error:
    LwCAFreeIntCARequest(pIntCARequest);
    if (ppIntCARequest)
    {
        *ppIntCARequest = NULL;
    }

    goto cleanup;
}

DWORD
LwCACreateCertValidity(
    time_t                  tmNotBefore,
    time_t                  tmNotAfter,
    PLWCA_CERT_VALIDITY     *ppCertValidity
    )
{
    DWORD dwError = 0;
    PLWCA_CERT_VALIDITY pCertValidity = NULL;

    if (!tmNotBefore || !tmNotAfter || !ppCertValidity)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_CERT_VALIDITY), (PVOID*)&pCertValidity);
    BAIL_ON_LWCA_ERROR(dwError);

    pCertValidity->tmNotBefore = tmNotBefore;
    pCertValidity->tmNotAfter = tmNotAfter;

    *ppCertValidity = pCertValidity;

cleanup:
    return dwError;

error:
    LwCAFreeCertValidity(pCertValidity);
    if (ppCertValidity)
    {
        *ppCertValidity = NULL;
    }

    goto cleanup;
}

VOID
LwCAFreeIntCARequest(
    PLWCA_INT_CA_REQ_DATA   pIntCAReqData
    )
{
    if (pIntCAReqData)
    {
        LwCAFreeStringArray(pIntCAReqData->pCountryList);
        LwCAFreeStringArray(pIntCAReqData->pStateList);
        LwCAFreeStringArray(pIntCAReqData->pLocalityList);
        LwCAFreeStringArray(pIntCAReqData->pOUList);
        LWCA_SAFE_FREE_STRINGA(pIntCAReqData->pszPolicy);
        LWCA_SAFE_FREE_MEMORY(pIntCAReqData);
    }
}

VOID
LwCAFreeCertValidity(
    PLWCA_CERT_VALIDITY pCertValidity
    )
{
    LWCA_SAFE_FREE_MEMORY(pCertValidity);
}

VOID
LwCAFreeCrl(
    PLWCA_CRL pCrl
    )
{
    LWCA_SAFE_FREE_STRINGA(pCrl);
}

DWORD
LwCAGetCAEndpoint(
    PCSTR   pcszCAId,
    PSTR    *ppszUri
    )
{
    DWORD dwError = 0;
    PSTR pszUri = NULL;

    if (!ppszUri || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

#ifdef REST_ENABLED
    dwError = _LwCAGetCARestEndpoint(pcszCAId, &pszUri);
    BAIL_ON_LWCA_ERROR(dwError);
#endif

    *ppszUri = pszUri;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUri);
    if (ppszUri)
    {
        *ppszUri = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetRootCAId(
    PSTR *ppszRootCAId
    )
{
    DWORD dwError = 0;
    PSTR pszRootCAId = NULL;
    BOOLEAN bLocked = FALSE;

    if (!ppszRootCAId)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX(bLocked, &gSrvCtx.mutex);

    if (IsNullOrEmptyString(gSrvCtx.pszRootCAId))
    {
        dwError = LWCA_INIT_CA_FAILED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(gSrvCtx.pszRootCAId, &pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszRootCAId = pszRootCAId;

cleanup:
    LWCA_UNLOCK_MUTEX(bLocked, &gSrvCtx.mutex);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
    if (ppszRootCAId)
    {
        *ppszRootCAId = NULL;
    }
    goto cleanup;
}

DWORD
LwCASrvInitCtx(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pCAConfig = NULL;
    BOOLEAN bLocked = FALSE;
    PSTR pszHost = NULL;
    PSTR pszName = NULL;

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX(bLocked, &gSrvCtx.mutex);

    /* get the ca endpoint */
    dwError = LwCAJsonGetStringFromKey(
                  pConfig,
                  TRUE,
                  LWCA_HOST_CONFIG_KEY,
                  &pszHost);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(
                  pConfig,
                  FALSE,
                  LWCA_CA_CONFIG_KEY,
                  &pCAConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(
                  pCAConfig,
                  FALSE,
                  LWCA_CA_NAME_KEY,
                  &pszName);
    BAIL_ON_LWCA_ERROR(dwError);

    if (IsNullOrEmptyString(pszHost))
    {
        dwError = LwCAGetCanonicalHostName(LOCALHOST, &pszHost);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    gSrvCtx.pszRootCAId = pszName;
    gSrvCtx.pszHost = pszHost;

cleanup:
    LWCA_UNLOCK_MUTEX(bLocked, &gSrvCtx.mutex);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszHost);
    LWCA_SAFE_FREE_STRINGA(pszName);
    goto cleanup;
}

VOID
LwCASrvFreeCtx(
   VOID
   )
{
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX(bLocked, &gSrvCtx.mutex);

    LWCA_SAFE_FREE_MEMORY(gSrvCtx.pszHost);
    LWCA_SAFE_FREE_MEMORY(gSrvCtx.pszRootCAId);

    LWCA_UNLOCK_MUTEX(bLocked, &gSrvCtx.mutex);
}

DWORD
LwCASrvGetHost(
    PSTR *ppszHost
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    PSTR pszHost = NULL;

    if (!ppszHost)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX(bLocked, &gSrvCtx.mutex);

    if (IsNullOrEmptyString(gSrvCtx.pszHost))
    {
        dwError = LWCA_INIT_CA_FAILED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(gSrvCtx.pszHost, &pszHost);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszHost = pszHost;

cleanup:
    LWCA_UNLOCK_MUTEX(bLocked, &gSrvCtx.mutex);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszHost);
    if (ppszHost)
    {
        *ppszHost = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetCARestEndpoint(
    PCSTR   pcszCAId,
    PSTR    *ppszUri
    )
{
    DWORD dwError = 0;
    PSTR pszUri = NULL;
    PSTR pszHost = NULL;
    PSTR pszRootCAId = NULL;
    PSTR pszTempUri = NULL;

    if (!ppszUri || IsNullOrEmptyString(pcszCAId))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetRootCAId(&pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASrvGetHost(&pszHost);
    BAIL_ON_LWCA_ERROR(dwError);

    if (LwCAStringCompareA(pcszCAId, pszRootCAId, false) == 0)
    {
        dwError = LwCAAllocateStringPrintfA(&pszUri, HTTPS_URI_FORMAT, pszHost, LWCA_HTTPS_PORT_NUM, LWCA_REST_ENDPOINT_ROOTCA);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAAllocateStringPrintfA(&pszTempUri, HTTPS_URI_FORMAT, pszHost, LWCA_HTTPS_PORT_NUM, LWCA_REST_ENDPOINT_INTERMEDIATECA);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateStringPrintfA(&pszUri, pszTempUri, pcszCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszUri = pszUri;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszHost);
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
    LWCA_SAFE_FREE_STRINGA(pszTempUri);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUri);
    if (ppszUri)
    {
        *ppszUri = NULL;
    }
    goto cleanup;
}
