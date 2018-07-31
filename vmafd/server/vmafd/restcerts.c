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


/*
{
  "certs":
  [
    {
      "cn":"CN=9EBEB7F388AE01DEE28FBD36D1236F412AB0EC42,CN=Certificate-Authorities,cn=Configuration,dc=lightwave,dc=local",
      "subjectdn":"CN=CA,DC=lightwave,DC=local, C=US, O=server.lightwave.local"
      "cert":"cert data",
      "crl":"crl data"
    }
  ]
}
*/

static
DWORD
_VmAfdJsonElementGetString(
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue,
    PSTR *ppszValue
    )
{
    DWORD dwError = 0;
    PSTR pszValue = NULL;

    if (pValue->nType == JSON_RESULT_STRING)
    {
        dwError = VmAfdAllocateStringA(
                      pValue->value.pszValue,
                      &pszValue);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "certs json parse error. key = %s", pszKey);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszValue);
    goto cleanup;
}

static
DWORD
_VmAfdJsonResultCertArrayElementCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVMAFD_CA_CERT pCACert = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCACert = (PVMAFD_CA_CERT)pUserData;

    if (!VmAfdStringCompareA("cn", pszKey, TRUE))
    {
        dwError = _VmAfdJsonElementGetString(pszKey, pValue, (PSTR *)&pCACert->pCN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (!VmAfdStringCompareA("subjectdn", pszKey, TRUE))
    {
        dwError = _VmAfdJsonElementGetString(pszKey, pValue, (PSTR *)&pCACert->pSubjectDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (!VmAfdStringCompareA("cert", pszKey, TRUE))
    {
        dwError = _VmAfdJsonElementGetString(pszKey, pValue, (PSTR *)&pCACert->pCert);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (!VmAfdStringCompareA("crl", pszKey, TRUE))
    {
        dwError = _VmAfdJsonElementGetString(pszKey, pValue, (PSTR *)&pCACert->pCrl);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmAfdJsonResultCertsArrayCB(
    PVOID pUserData,
    size_t nSize,
    size_t nIndex,
    PVM_JSON_POSITION pPosition
    )
{
    DWORD dwError = 0;
    PVMAFD_CA_CERT_ARRAY pCertsArray = NULL;

    if (!pUserData || !pPosition || !nSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCertsArray = (PVMAFD_CA_CERT_ARRAY)pUserData;

    if (!pCertsArray->pCACerts)
    {
        dwError = VmAfdAllocateMemory(
                      sizeof(VMAFD_CA_CERT) * nSize,
                      (PVOID *)&pCertsArray->pCACerts);
        BAIL_ON_VMAFD_ERROR(dwError);

        pCertsArray->dwCount = nSize;
    }

    if (pCertsArray->dwCount != nSize || nIndex >= pCertsArray->dwCount)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "certs json parse callback array bounds error. array size: %ld, requested size: %ld, index: %ld",
                 pCertsArray->dwCount, nSize, nIndex);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  &pCertsArray->pCACerts[nIndex],
                  _VmAfdJsonResultCertArrayElementCB);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}


static
DWORD
_VmAfdJsonResultTopLevelCertObjectCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVMAFD_CA_CERT_ARRAY pCACerts = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCACerts = (PVMAFD_CA_CERT_ARRAY)pUserData;

    /* top level key must be "certs" */
    if (VmStringCompareA("certs", pszKey, TRUE) != 0)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "Error parsing certs json. Top level key: %s. Expected certs",
                 pszKey);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* expect an array of certs as value */
    if (pValue->nType != JSON_RESULT_ARRAY)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "Error parsing certs json. Expected array. Found type: %d",
                 pValue->nType);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmJsonResultIterateArrayAt(
                  pValue->value.pArray,
                  pCACerts,
                  _VmAfdJsonResultCertsArrayCB);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmAfdGetHttpResultCACerts(
    PCSTR pszResult,
    PVMAFD_CA_CERT_ARRAY *ppCACerts
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pJsonResult = NULL;
    PVM_JSON_POSITION pPosition = NULL;
    PVMAFD_CA_CERT_ARRAY pCACerts = NULL;

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
     * root position is an object which holds an array
     * like {"certs":[{...}, {...}]}.
     * this iterate call is on the top level certs object.
    */
    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  pCACerts,
                 _VmAfdJsonResultTopLevelCertObjectCB);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    VmJsonResultFreeHandle(pJsonResult);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmAfdRestGetCACerts(
    PCSTR   pszServer,
    PCSTR   pszDomain,
    PCSTR   pszUser,
    PCSTR   pszPass,
    BOOLEAN bDetail,
    PVMAFD_CA_CERT_ARRAY *ppCACerts
    )
{
    DWORD dwError = 0;
    PSTR pszToken = NULL;
    PSTR pszUrl = NULL;
    PVM_HTTP_CLIENT pHttpClient = NULL;
    PSTR pszParamString = NULL;
    PCSTR pszResult = NULL;
    PVMAFD_CA_CERT_ARRAY pCACerts = NULL;
    PWSTR pwszCAPath = NULL;
    PSTR pszCAPath = NULL;

    if (IsNullOrEmptyString(pszServer) ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszUser) ||
        IsNullOrEmptyString(pszPass) ||
        !ppCACerts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* acquire token */
    dwError = VmAfdAcquireTokenForVmDirREST(
                  pszServer,
                  pszDomain,
                  pszUser,
                  pszPass,
                  pszCAPath,
                  &pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                  &pszParamString,
                  "?detail=%s",
                  bDetail?"true":"false");
    BAIL_ON_VMAFD_ERROR(dwError);

    /* make rest url */
    dwError = VmFormatUrl(
                  "https",
                  pszServer,
                  VMDIR_REST_API_HTTPS_PORT,
                  VMDIR_REST_API_BASE"/"VMDIR_REST_API_GET_CERTS_CMD,
                  pszParamString,
                  &pszUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientInit(&pHttpClient, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientSetToken(pHttpClient, pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientPerform(pHttpClient, VMHTTP_METHOD_GET, pszUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHttpClient, &pszResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdGetHttpResultCACerts(pszResult, &pCACerts);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCACerts = pCACerts;

cleanup:
    VmHttpClientFreeHandle(pHttpClient);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_STRINGA(pszParamString);
    VMAFD_SAFE_FREE_STRINGA(pszUrl);
    VMAFD_SAFE_FREE_STRINGA(pszToken);
    return dwError;

error:
    VecsFreeCACertArray(pCACerts);
    VmAfdLog(VMAFD_DEBUG_ANY, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}
