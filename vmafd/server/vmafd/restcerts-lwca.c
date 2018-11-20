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
_VmAfdLwCAGetHttpResultCADetails(
    PCSTR                   pszResult,
    PVMAFD_CA_CERT_ARRAY    *ppCACerts
    );

static
DWORD
_VmAfdLwCAJsonResultTopLevelCADetailsObjectCB(
    PVOID                   pUserData,
    PCSTR                   pszKey,
    PVM_JSON_RESULT_VALUE   pValue
    );

static
DWORD
_VmAfdLwCAJsonResultCADetailsArrayCB(
    PVOID                   pUserData,
    size_t                  nSize,
    size_t                  nIndex,
    PVM_JSON_POSITION       pPosition
    );

static
DWORD
_VmAfdLwCAJsonResultCADetailsElementCB(
    PVOID                   pUserData,
    PCSTR                   pszKey,
    PVM_JSON_RESULT_VALUE   pValue
    );


DWORD
VmAfdRestGetCACertsLwCA(
    PCSTR                   pcszLwCAServer,
    PCSTR                   pcszLwCAId,
    BOOLEAN                 bDetail,
    PVM_HTTP_CLIENT         pHTTPClient,
    PVMAFD_CA_CERT_ARRAY    *ppCACerts
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszURL = NULL;
    PSTR                    pszURI = NULL;
    PSTR                    pszQueryParams = NULL;
    PCSTR                   pszResult = NULL;
    PVMAFD_CA_CERT_ARRAY    pCACerts = NULL;

    if (IsNullOrEmptyString(pcszLwCAServer) || !pHTTPClient || !ppCACerts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                        &pszQueryParams,
                        "?detail=%s",
                        bDetail ? "true" : "false");
    BAIL_ON_VMAFD_ERROR(dwError);

    /* Make REST URL */
    if (IsNullOrEmptyString(pcszLwCAId))
    {
        /* If no CA Id is provided, then obtain Root CA certificates */

        dwError = VmFormatUrl(
                      "https",
                      pcszLwCAServer,
                      LWCA_REST_API_HTTPS_PORT,
                      LWCA_REST_API_BASE"/"LWCA_REST_API_GET_ROOT_CERT,
                      pszQueryParams,
                      &pszURL);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        /* If CA Id is provided, then obtain CA certificate from it */

        dwError = VmAfdAllocateStringPrintf(
                    &pszURI,
                    "%s/"LWCA_REST_API_GET_SUBCA_CERT,
                    LWCA_REST_API_BASE,
                    pcszLwCAId);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmFormatUrl(
                      "https",
                      pcszLwCAServer,
                      LWCA_REST_API_HTTPS_PORT,
                      pszURI,
                      pszQueryParams,
                      &pszURL);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmHttpClientPerform(pHTTPClient, VMHTTP_METHOD_GET, pszURL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHTTPClient, &pszResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdLwCAGetHttpResultCADetails(pszResult, &pCACerts);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCACerts = pCACerts;


cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszURL);
    VMAFD_SAFE_FREE_STRINGA(pszURI);
    VMAFD_SAFE_FREE_STRINGA(pszQueryParams);

    return dwError;

error:

    VecsFreeCACertArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }

    goto cleanup;
}


static
DWORD
_VmAfdLwCAGetHttpResultCADetails(
    PCSTR                   pszResult,
    PVMAFD_CA_CERT_ARRAY    *ppCACerts
    )
{
    DWORD                   dwError = 0;
    PVM_JSON_RESULT         pJsonResult = NULL;
    PVM_JSON_POSITION       pPosition = NULL;
    PVMAFD_CA_CERT_ARRAY    pCACerts = NULL;

    dwError = VmJsonResultInit(&pJsonResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmJsonResultLoadString(pszResult, pJsonResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pJsonResult, &pPosition);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                  sizeof(VMAFD_CA_CERT_ARRAY),
                  (PVOID *)&pCACerts);
    BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * Root position is an object which holds an array that looks like this:
     *     {"caDetails":[{"cert":"...", "crl":"..."}, {"cert":"...","crl":"..." }]}
     * This iterate call is on the top level object, caDetails.
     */
    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  pCACerts,
                 _VmAfdLwCAJsonResultTopLevelCADetailsObjectCB);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:

    VmJsonResultFreeHandle(pJsonResult);

    return dwError;

error:

    VecsFreeCACertArray(pCACerts);
    if (ppCACerts)
    {
        *ppCACerts = NULL;
    }
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
_VmAfdLwCAJsonResultTopLevelCADetailsObjectCB(
    PVOID                   pUserData,
    PCSTR                   pszKey,
    PVM_JSON_RESULT_VALUE   pValue
    )
{
    DWORD                   dwError = 0;
    PVMAFD_CA_CERT_ARRAY    pCACerts = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCACerts = (PVMAFD_CA_CERT_ARRAY)pUserData;

    if (VmAfdStringCompareA("caDetails", pszKey, TRUE) != 0)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "Error parsing certs json. Could not find key caDetails. Found %s instead.",
                 pszKey);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* expect an array objects containing cert and crl */
    if (pValue->nType != JSON_RESULT_ARRAY)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "Error parsing certs json. Expected array for key \"caDetails\". Found type: %d",
                 pValue->nType);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmJsonResultIterateArrayAt(
                  pValue->value.pArray,
                  pCACerts,
                  _VmAfdLwCAJsonResultCADetailsArrayCB);
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
_VmAfdLwCAJsonResultCADetailsArrayCB(
    PVOID                   pUserData,
    size_t                  nSize,
    size_t                  nIndex,
    PVM_JSON_POSITION       pPosition
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bCACertArrayAlloced = FALSE;
    PVMAFD_CA_CERT_ARRAY    pCertsArray = NULL;

    if (!pUserData || !nSize || !pPosition)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCertsArray = (PVMAFD_CA_CERT_ARRAY)pUserData;

    if (!pCertsArray->pCACerts)
    {
        bCACertArrayAlloced = TRUE;

        dwError = VmAfdAllocateMemory(
                        sizeof(VMAFD_CA_CERT) * nSize,
                        (PVOID *)&pCertsArray->pCACerts);
        BAIL_ON_VMAFD_ERROR(dwError);

        pCertsArray->dwCount = nSize;
    }

    if (pCertsArray->dwCount != nSize || nIndex >= pCertsArray->dwCount)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "caDetails json parse callback array bounds error. array size: %ld, requested size: %ld, index: %ld",
                 pCertsArray->dwCount,
                 nSize,
                 nIndex);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmJsonResultIterateObjectAt(
                    pPosition,
                    &pCertsArray->pCACerts[nIndex],
                    _VmAfdLwCAJsonResultCADetailsElementCB);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (bCACertArrayAlloced)
    {
        // only free pCertsArray if we are allocating it for the first time.
        VecsFreeCACertArray(pCertsArray);
    }

    goto cleanup;
}

static
DWORD
_VmAfdLwCAJsonResultCADetailsElementCB(
    PVOID                   pUserData,
    PCSTR                   pszKey,
    PVM_JSON_RESULT_VALUE   pValue
    )
{
    DWORD                   dwError = 0;
    PVMAFD_CA_CERT          pCACert = NULL;
    PSTR                    *ppszDst = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCACert = (PVMAFD_CA_CERT)pUserData;

    if (!VmAfdStringCompareA("cert", pszKey, TRUE))
    {
        ppszDst = (PSTR *)&pCACert->pCert;
    }
    else if (!VmAfdStringCompareA("crl", pszKey, TRUE))
    {
        ppszDst = (PSTR *)&pCACert->pCrl;
    }
    else
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "caDetails json parse error: invalid key provided. found \"%s\" expected either \"cert\" or \"crl\"",
                 pszKey);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pValue->nType != JSON_RESULT_STRING)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "caDetails element key (%s) has invalid value type. found type (%d) expected type (%d)",
                 pszKey,
                 pValue->nType,
                 JSON_RESULT_STRING);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pValue->value.pszValue, ppszDst);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:


    if (ppszDst)
    {
        VMAFD_SAFE_FREE_MEMORY(*ppszDst);
    }

    goto cleanup;
}
