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
