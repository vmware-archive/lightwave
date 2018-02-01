/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

DWORD
VmDirRESTGetLightwaveOIDCSigningCertPEM(
    PCSTR   pszDCName,
    PCSTR   pszDomainName,
    PSTR*   ppszOIDCSigningCertPEM
    )
{
    DWORD   dwError = 0;
    DWORD   dwOIDCError = 0;
    PSTR    pszOIDCSigningCertPEM = NULL;
    POIDC_SERVER_METADATA   pOidcMetadata = NULL;

    if (IsNullOrEmptyString(pszDCName) ||
        IsNullOrEmptyString(pszDomainName) ||
        !ppszOIDCSigningCertPEM)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwOIDCError = OidcServerMetadataAcquire(
            &pOidcMetadata,
            pszDCName,
            VMDIR_REST_OIDC_PORT,
            pszDomainName,
            LIGHTWAVE_TLS_CA_PATH);
    dwError = VmDirOidcToVmdirError(dwOIDCError);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcServerMetadataGetSigningCertificatePEM(pOidcMetadata),
            &pszOIDCSigningCertPEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOIDCSigningCertPEM = pszOIDCSigningCertPEM;

cleanup:
    OidcServerMetadataDelete(pOidcMetadata);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) OIDC error (%d)",
            __FUNCTION__,
            dwError,
            dwOIDCError);

    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    goto cleanup;
}

DWORD
VmDirRESTGetLightwaveObjectSid(
    PCSTR   pszDCName,
    PCSTR   pszDomainName,
    PCSTR   pszDN,
    PSID*   ppObjectSid
    )
{
    DWORD   dwError = 0;
    DWORD   dwAFDError = 0;
    DWORD   dwPort = VMDIR_REST_LIGHTWAVE_LDAP_PORT;
    PSTR    pszAccount = NULL;
    PSTR    pszAccountUPN = NULL;
    PSTR    pszPassword = NULL;
    LDAP*   pLd = NULL;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppSidVals = NULL;
    PSID    pSid = NULL;

    PSTR    ppszAttrs[] = { ATTR_OBJECT_SID, NULL };

    if (IsNullOrEmptyString(pszDCName) ||
        IsNullOrEmptyString(pszDN) ||
        !ppObjectSid)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwAFDError = gpVdirVmAfdApi->pfnGetMachineAccountInfo(
            NULL, &pszAccount, &pszPassword);
    dwError = dwAFDError ? VMDIR_ERROR_AFD_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszAccountUPN, "%s@%s", pszAccount, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBindToPort(
            &pLd, pszDCName, dwPort, pszAccountUPN, pszPassword, gVmdirGlobals.dwLdapConnectTimeoutSec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
            pLd, pszDN, LDAP_SCOPE_BASE, NULL, ppszAttrs, TRUE,
            NULL, NULL, NULL, 0,
            &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    if (pEntry == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    ppSidVals = ldap_get_values_len(pLd, pEntry, ATTR_OBJECT_SID);
    if (ldap_count_values_len(ppSidVals) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    dwError = VmDirAllocateSidFromCString(ppSidVals[0]->bv_val, &pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppObjectSid = pSid;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAccount);
    VMDIR_SAFE_FREE_MEMORY(pszAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);
    VDIR_SAFE_LDAP_UNBIND_EXT_S(pLd);
    VDIR_SAFE_LDAP_MSGFREE(pResult);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppSidVals);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) AFD error (%d)",
            __FUNCTION__,
            dwError,
            dwAFDError);

    VMDIR_SAFE_FREE_MEMORY(pSid);
    goto cleanup;
}

DWORD
VmDirRESTGetLightwaveBuiltInAdminsGroupSid(
    PCSTR   pszDCName,
    PCSTR   pszDomainName,
    PSID*   ppBuiltInAdminsGroupSid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainDN = NULL;
    PSTR    pszBuiltInAdminsGroupDN = NULL;
    PSID    pSid = NULL;

    if (IsNullOrEmptyString(pszDCName) ||
        IsNullOrEmptyString(pszDomainName) ||
        !ppBuiltInAdminsGroupSid)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszBuiltInAdminsGroupDN,
            "cn=Administrators,cn=Builtin,%s",
            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetLightwaveObjectSid(
            pszDCName, pszDomainName, pszBuiltInAdminsGroupDN, &pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppBuiltInAdminsGroupSid = pSid;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInAdminsGroupDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pSid);
    goto cleanup;
}
