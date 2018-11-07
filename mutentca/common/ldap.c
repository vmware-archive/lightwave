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
LwCABuildUPNToDNURI(
    PVM_HTTP_CLIENT         pHTTPClient,
    PCSTR                   pszDomainName,
    PCSTR                   pcszUPN,
    PSTR                    *ppszUPNToDNURI
    );

/*
 * This function converts a domain name to an LDAP Base DN.
 *
 * E.g.
 *      - input1:   lightwave.example.com
 *      - output1:  dc=lightwave,dc=example,dc=com
 *
 *      - input2:   lightwave-domain
 *      - output2:  dc=lightwave-domain
 */
DWORD
LwCADNSNameToDCDN(
    PCSTR       pcszDNSName,
    PSTR        *ppszDN
    )
{
    DWORD       dwError = 0;
    DWORD       dwCursor = 0;
    size_t      szRDNLen = 0;
    PSTR        pszTempDNSName = NULL;
    PSTR        pszStrTok = NULL;
    PSTR        pszLabel = NULL;
    PSTR        pszRDN = NULL;
    PSTR        pszTempDN = NULL;
    PSTR        pszDN = NULL;

    if (IsNullOrEmptyString(pcszDNSName) || !ppszDN)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAAllocateMemory(LWCA_LDAP_DC_DN_MAXLENGTH, (PVOID *)&pszTempDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszDNSName, &pszTempDNSName);
    BAIL_ON_LWCA_ERROR(dwError);

    pszLabel = LwCAStringTokA(pszTempDNSName, ".", &pszStrTok);

    dwError = LwCAAllocateStringPrintfA(&pszRDN, "dc=%s", pszLabel);
    BAIL_ON_LWCA_ERROR(dwError);

    szRDNLen = LwCAStringLenA(pszRDN);

    dwError = LwCACopyMemory(
                    (PVOID)(pszTempDN + dwCursor),
                    szRDNLen,
                    pszRDN,
                    szRDNLen);
    BAIL_ON_LWCA_ERROR(dwError);
    dwCursor += szRDNLen;

    LWCA_SAFE_FREE_STRINGA(pszRDN);
    pszRDN = NULL;
    szRDNLen = 0;

    while ((pszLabel = LwCAStringTokA(NULL, ".", &pszStrTok)) && dwCursor < LWCA_LDAP_DC_DN_MAXLENGTH)
    {
        dwError = LwCAAllocateStringPrintfA(&pszRDN, ",dc=%s", pszLabel);
        BAIL_ON_LWCA_ERROR(dwError);

        szRDNLen = LwCAStringLenA(pszRDN);

        dwError = LwCACopyMemory(
                        (PVOID)(pszTempDN + dwCursor),
                        szRDNLen,
                        pszRDN,
                        szRDNLen);
        BAIL_ON_LWCA_ERROR(dwError);
        dwCursor += szRDNLen;

        LWCA_SAFE_FREE_STRINGA(pszRDN);
        pszRDN = NULL;
        szRDNLen = 0;
    }
    if (dwCursor >= LWCA_LDAP_DC_DN_MAXLENGTH)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_BUFFER_OVERFLOW);
    }
    pszTempDN[dwCursor] = '\0';

    dwError = LwCAAllocateStringA(pszTempDN, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;


cleanup:

    LWCA_SAFE_FREE_STRINGA(pszTempDNSName);
    LWCA_SAFE_FREE_STRINGA(pszRDN);
    LWCA_SAFE_FREE_STRINGA(pszTempDN);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszDN);
    if (ppszDN)
    {
        *ppszDN = NULL;
    }

    goto cleanup;
}

DWORD
LwCAIsValidDN(
    PCSTR       pcszDN,
    PBOOLEAN    pbIsValid
    )
{
    DWORD dwError = 0;
    LDAPDN dn = NULL;

    if (IsNullOrEmptyString(pcszDN) || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = ldap_str2dn(pcszDN, &dn, LDAP_DN_FORMAT_LDAPV3);
    if (dwError == LDAP_SUCCESS && dn != NULL)
    {
        *pbIsValid = TRUE;
    }
    else
    {
        *pbIsValid = FALSE;
    }

error:
    if (dn)
    {
        ldap_dnfree(dn);
    }
    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }
    return dwError;
}

/*
 * convert DN to a list of RDN.
 *
 * say dc=lwraft,dc=local
 * if bNotypes == false, {"dc=vmdir", "dc=local"} is returned;
 * otherwise {"vmdir", "local"} is returned.
 */
DWORD
LwCADNToRDNArray(
    PCSTR               pcszDN,
    BOOLEAN             bNotypes,
    PLWCA_STRING_ARRAY* ppRDNStrArray
    )
{
    DWORD               dwError = 0;
    DWORD               dwCount = 0;
    PLWCA_STRING_ARRAY  pStrArray = NULL;
    PSTR*               ppszRDN = NULL;
    PSTR*               ppszTmp = NULL;
    DWORD               iEntry = 0;

    if (IsNullOrEmptyString(pcszDN) || !ppRDNStrArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (bNotypes)
    {
        ppszRDN = ldap_explode_dn(pcszDN, 1);
    }
    else
    {
        ppszRDN = ldap_explode_dn(pcszDN, 0);
    }

    if (!ppszRDN)
    {
        dwError = LWCA_ERROR_INVALID_DN;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    for (ppszTmp = ppszRDN; *ppszTmp; ppszTmp++)
    {
        dwCount++;
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwCount, (PVOID*)&pStrArray->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    pStrArray->dwCount = dwCount;

    for (ppszTmp = ppszRDN; *ppszTmp; ppszTmp++)
    {
        dwError = LwCAAllocateStringA(*ppszTmp, &pStrArray->ppData[iEntry++]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppRDNStrArray = pStrArray;

cleanup:
    if (ppszRDN)
    {
        ldap_value_free(ppszRDN);
    }

    return dwError;

error:
    LwCAFreeStringArray(pStrArray);
    if (ppRDNStrArray)
    {
        *ppRDNStrArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCAUPNToDN(
    PCSTR                   pcszUPN,
    PSTR                    *ppszDN
    )
{
    DWORD                   dwError = 0;
    long                    httpReqStatusCode = 0;
    PVM_HTTP_CLIENT         pHTTPClient = NULL;
    PLWCA_JSON_OBJECT       pJson = NULL;
    PLWCA_JSON_OBJECT       pJsonResult = NULL;
    PLWCA_JSON_OBJECT       pJsonResultElement = NULL;
    PSTR                    pszDomainName = NULL;
    PSTR                    pszDCName = NULL;
    PSTR                    pszSvcAcctKey = NULL;
    PSTR                    pszSvcAcctCert = NULL;
    PSTR                    pszJWT = NULL;
    PSTR                    pszHTTPReqTime = NULL;
    PSTR                    pszHTTPReqURI = NULL;
    PSTR                    pszHTTPReqURL = NULL;
    PSTR                    pszHTTPReqSignature = NULL;
    PCSTR                   pcszHTTPResponse = NULL;
    PSTR                    pszDN = NULL;

    if (IsNullOrEmptyString(pcszUPN) || !ppszDN)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAGetDomainName(&pszDomainName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetDCName(&pszDCName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetAccessToken(pszDCName, pszDomainName, LWCA_OIDC_VMDIR_SCOPE, &pszJWT);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientInit(&pHTTPClient, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCABuildUPNToDNURI(
                        pHTTPClient,
                        pszDomainName,
                        pcszUPN,
                        &pszHTTPReqURI);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetCurrentTime(&pszHTTPReqTime);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetVecsMutentCACert(&pszSvcAcctCert, &pszSvcAcctKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientSignRequest(
                        VMHTTP_METHOD_GET,
                        pszHTTPReqURI,
                        "",
                        pszSvcAcctKey,
                        pszHTTPReqTime,
                        &pszHTTPReqSignature);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientSetupHOTK(pHTTPClient, pszJWT, pszHTTPReqSignature, pszHTTPReqTime, "");
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmFormatUrl(
                    "https",
                    pszDCName,
                    LWCA_VMDIR_REST_LDAP_PORT,
                    pszHTTPReqURI,
                    NULL,
                    &pszHTTPReqURL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientPerform(pHTTPClient, VMHTTP_METHOD_GET, pszHTTPReqURL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetStatusCode(pHTTPClient, &httpReqStatusCode);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHTTPClient, &pcszHTTPResponse);
    BAIL_ON_LWCA_ERROR(dwError);

    if (httpReqStatusCode != LWCA_HTTP_RESP_OK)
    {
        LWCA_LOG_ERROR(
                "[%s:%d] Failed to get DN of UPN (%s). HTTP Status Code (%ld) Response (%s)",
                __FUNCTION__,
                __LINE__,
                pcszUPN,
                httpReqStatusCode,
                LWCA_SAFE_STRING(pcszHTTPResponse));
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_ENTRY_NOT_FOUND);
    }

    dwError = LwCAJsonLoadObjectFromString(pcszHTTPResponse, &pJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_VMDIR_RESP_RESULT, &pJsonResult);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonArrayGetBorrowedRef(pJsonResult, 0, &pJsonResultElement);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJsonResultElement, TRUE, LWCA_LDAP_ATTR_DN, &pszDN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDN = pszDN;


cleanup:

    LWCA_SAFE_FREE_STRINGA(pszDomainName);
    LWCA_SAFE_FREE_STRINGA(pszDCName);
    LWCA_SECURE_SAFE_FREE_MEMORY(pszSvcAcctKey, LwCAStringLenA(pszSvcAcctKey));
    LWCA_SECURE_SAFE_FREE_MEMORY(pszSvcAcctCert, LwCAStringLenA(pszSvcAcctCert));
    LWCA_SAFE_FREE_STRINGA(pszJWT);
    LWCA_SAFE_FREE_STRINGA(pszHTTPReqTime);
    LWCA_SAFE_FREE_STRINGA(pszHTTPReqURI);
    LWCA_SAFE_FREE_STRINGA(pszHTTPReqURL);
    LWCA_SAFE_FREE_STRINGA(pszHTTPReqSignature);
    LWCA_SAFE_JSON_DECREF(pJson);
    VmHttpClientFreeHandle(pHTTPClient);

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
LwCABuildUPNToDNURI(
    PVM_HTTP_CLIENT         pHTTPClient,
    PCSTR                   pcszDomainName,
    PCSTR                   pcszUPN,
    PSTR                    *ppszUPNToDNURI
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszDCDN = NULL;
    PSTR                    pszUPNFilter = NULL;
    PSTR                    pszURIParamDN = NULL;
    PSTR                    pszURIParamFilter = NULL;
    PSTR                    pszUPNToDNURI = NULL;

    dwError = LwCADNSNameToDCDN(pcszDomainName, &pszDCDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpUrlEncodeString(pHTTPClient, pszDCDN, &pszURIParamDN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(&pszUPNFilter, LWCA_LDAP_FILTER_UPN_FMT, pcszUPN);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmHttpUrlEncodeString(pHTTPClient, pszUPNFilter, &pszURIParamFilter);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                        &pszUPNToDNURI,
                        "%s?%s=%s&%s=%s&%s=%s&%s=%s",
                        LWCA_VMDIR_REST_LDAP_URI_PREFIX,
                        LWCA_LDAP_ATTR_DN,
                        pszURIParamDN,
                        LWCA_LDAP_SCOPE,
                        LWCA_LDAP_SCOPE_SUB,
                        LWCA_LDAP_FILTER,
                        pszURIParamFilter,
                        LWCA_LDAP_ATTRS,
                        LWCA_LDAP_ATTR_DN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszUPNToDNURI = pszUPNToDNURI;


cleanup:

    LWCA_SAFE_FREE_STRINGA(pszDCDN);
    LWCA_SAFE_FREE_STRINGA(pszUPNFilter);
    LWCA_SAFE_FREE_STRINGA(pszURIParamDN);
    LWCA_SAFE_FREE_STRINGA(pszURIParamFilter);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszUPNToDNURI);
    if (ppszUPNToDNURI)
    {
        *ppszUPNToDNURI = NULL;
    }

    goto cleanup;
}
