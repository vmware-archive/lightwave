/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
_LwCARestMapSignAlgoToEnum(
    PCSTR                       pcszRestSignAlgorithm,
    PLWCA_SIGNING_ALGORITHM     pSignAlgorithm
    );

DWORD
LwCARestGetCertValidityInput(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_CERT_VALIDITY*        ppCertValidity
    )
{
    DWORD                       dwError         = 0;
    time_t                      tStartTime      = 0;
    time_t                      tEndTime        = 0;
    PLWCA_CERT_VALIDITY         pCertValidity   = NULL;

    if (!pJsonBody || !ppCertValidity)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetTimeFromKey(pJsonBody, FALSE, LWCA_JSON_KEY_START_TIME, &tStartTime);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetTimeFromKey(pJsonBody, FALSE, LWCA_JSON_KEY_END_TIME, &tEndTime);
    BAIL_ON_LWCA_ERROR(dwError);

    if (tStartTime == 0 || tEndTime == 0 || (tEndTime - tStartTime) <= 0)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCACreateCertValidity(tStartTime, tEndTime, &pCertValidity);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertValidity = pCertValidity;

cleanup:
    return dwError;

error:
    if (dwError == LWCA_JSON_PARSE_ERROR)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
    }

    LwCAFreeCertValidity(pCertValidity);
    if (ppCertValidity)
    {
        *ppCertValidity = NULL;
    }

    goto cleanup;
}

DWORD
LwCARestGetIntCAInputSpec(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_REST_INT_CA_SPEC*     ppIntCASpec
    )
{
    DWORD                       dwError         = 0;
    PLWCA_STRING_ARRAY          pCountryList    = NULL;
    PLWCA_STRING_ARRAY          pStateList      = NULL;
    PLWCA_STRING_ARRAY          pLocalityList   = NULL;
    PLWCA_STRING_ARRAY          pOUList         = NULL;
    PSTR                        pszPolicy       = NULL;
    PLWCA_JSON_OBJECT           pJsonValue      = NULL;
    PLWCA_REST_INT_CA_SPEC      pIntCASpec      = NULL;

    if (!pJsonBody || !ppIntCASpec)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_REST_INT_CA_SPEC), (PVOID*)&pIntCASpec);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJsonBody, FALSE, LWCA_JSON_KEY_CA_ID, &pIntCASpec->pszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_PARENT_CA_ID, &pIntCASpec->pszParentCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringArrayFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_COUNTRY, &pCountryList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringArrayFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_STATE, &pStateList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringArrayFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_LOCALITY, &pLocalityList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringArrayFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_OU, &pOUList);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_POLICY, &pszPolicy);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateIntCARequest(pCountryList, pStateList, pLocalityList, pOUList, pszPolicy, &pIntCASpec->pIntCAReqData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_VALIDITY, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pJsonValue)
    {
        dwError = LwCARestGetCertValidityInput(pJsonValue, &pIntCASpec->pCertValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppIntCASpec = pIntCASpec;

cleanup:
    LwCAFreeStringArray(pCountryList);
    LwCAFreeStringArray(pStateList);
    LwCAFreeStringArray(pLocalityList);
    LwCAFreeStringArray(pOUList);
    LWCA_SAFE_FREE_STRINGA(pszPolicy);

    return dwError;

error:
    if (dwError == LWCA_JSON_PARSE_ERROR)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
    }

    LwCARestFreeIntCAInputSpec(pIntCASpec);
    if (ppIntCASpec)
    {
        *ppIntCASpec = NULL;
    }

    goto cleanup;
}

DWORD
LwCARestGetSignCertInputSpec(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_REST_SIGN_CERT_SPEC*  ppSignCertSpec
    )
{
    DWORD                       dwError         = 0;
    PSTR                        pszSignAlgo     = NULL;
    PLWCA_JSON_OBJECT           pJsonValue      = NULL;
    PLWCA_REST_SIGN_CERT_SPEC   pSignCertSpec   = NULL;

    if (!pJsonBody || !ppSignCertSpec)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_REST_SIGN_CERT_SPEC), (PVOID*)&pSignCertSpec);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJsonBody, FALSE, LWCA_JSON_KEY_CSR, &pSignCertSpec->pszCSR);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_VALIDITY, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pJsonValue)
    {
        dwError = LwCARestGetCertValidityInput(pJsonValue, &pSignCertSpec->pCertValidity);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pJsonBody, TRUE, LWCA_JSON_KEY_SIGN_ALGO, &pszSignAlgo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestMapSignAlgoToEnum(pszSignAlgo, &pSignCertSpec->signAlgorithm);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppSignCertSpec = pSignCertSpec;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszSignAlgo);

    return dwError;

error:
    if (dwError == LWCA_JSON_PARSE_ERROR)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
    }

    LwCARestFreeSignCertInputSpec(pSignCertSpec);
    if (ppSignCertSpec)
    {
        *ppSignCertSpec = NULL;
    }

    goto cleanup;
}

DWORD
LwCARestGetCertificateInput(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_CERTIFICATE*          ppCert
    )
{
    DWORD                       dwError = 0;
    PSTR                        pszCert = NULL;
    PLWCA_CERTIFICATE           pCert   = NULL;

    if (!pJsonBody || !ppCert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pJsonBody, FALSE, LWCA_JSON_KEY_CERT, &pszCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertificate(pszCert, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCert = pCert;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCert);

    return dwError;

error:
    if (dwError == LWCA_JSON_PARSE_ERROR)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
    }

    LwCAFreeCertificate(pCert);
    if (ppCert)
    {
        *ppCert = pCert;
    }

    goto cleanup;
}

VOID
LwCARestFreeIntCAInputSpec(
    PLWCA_REST_INT_CA_SPEC      pIntCASpec
    )
{
    if (pIntCASpec)
    {
        LWCA_SAFE_FREE_STRINGA(pIntCASpec->pszCAId);
        LWCA_SAFE_FREE_STRINGA(pIntCASpec->pszParentCAId);
        LwCAFreeIntCARequest(pIntCASpec->pIntCAReqData);
        LwCAFreeCertValidity(pIntCASpec->pCertValidity);
        LWCA_SAFE_FREE_MEMORY(pIntCASpec);
    }
}

VOID
LwCARestFreeSignCertInputSpec(
    PLWCA_REST_SIGN_CERT_SPEC   pSignCertSpec
    )
{
    if (pSignCertSpec)
    {
        LWCA_SAFE_FREE_STRINGA(pSignCertSpec->pszCSR);
        LwCAFreeCertValidity(pSignCertSpec->pCertValidity);
        LWCA_SAFE_FREE_MEMORY(pSignCertSpec);
    }
}

static
DWORD
_LwCARestMapSignAlgoToEnum(
    PCSTR                       pcszRestSignAlgorithm,
    PLWCA_SIGNING_ALGORITHM     pSignAlgorithm
    )
{
    DWORD                       dwError  = 0;
    LWCA_SIGNING_ALGORITHM      signAlgo = LWCA_SHA_256;

    if (!pSignAlgorithm)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pcszRestSignAlgorithm)
    {
        if (LwCAStringCompareA(pcszRestSignAlgorithm, LWCA_REST_SHA256_ALGO, TRUE) == 0)
        {
            signAlgo = LWCA_SHA_256;
        }
        else
        {
            dwError = LWCA_ERROR_INVALID_REQUEST;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

    *pSignAlgorithm = signAlgo;

cleanup:
    return dwError;

error:
    goto cleanup;
}
