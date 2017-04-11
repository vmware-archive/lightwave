/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

//TODO: Should remove this function after it is included in vmdirclient.h
#ifdef __cplusplus
extern "C"
#endif


static
DWORD
DirCliCopyQueryResultAttributeString(
    LDAP*        pLotus,
    LDAPMessage* pSearchResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
);

static
DWORD
DirCliGetDefaultDomainName(
    LDAP* pLotus,
    PSTR* ppDomainName
    );

DWORD
VmDirSafeLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszHost,
    PCSTR       pszUPN,         // opt, if exists, will try SRP mech
    PCSTR       pszPassword     // opt, if exists, will try SRP mech
    );

static
BOOLEAN
_doLdapConnectRetry(
    DWORD dwError
    );

DWORD
DirCliLdapConnect(
    PCSTR  pszHostName,
    PCSTR  pszUser,
    PCSTR  pszDomain,
    PCSTR  pszPassword,
    LDAP** ppLd
    )
{
    DWORD dwError = 0;
    PSTR  pszUserDN = NULL;
    LDAP* pLd = NULL;
    PSTR pszUpn = NULL;
    int retryCount = 2;

    dwError = DirCliGetUserDN(pszUser, pszDomain, &pszUserDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pszUpn,
                "%s@%s",
                pszUser,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    while (retryCount > 0)
    {
        dwError = VmDirSafeLDAPBind(
                        &pLd,
                        pszHostName,
                        pszUpn,
                        pszPassword);
        if ((dwError != 0) && (_doLdapConnectRetry(dwError) == TRUE))
        {
            retryCount--;
            VmAfdSleep(1000);
        }
        else
        {
            break;
        }
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUserDN);

    return dwError;

error:

    *ppLd = NULL;

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    switch (dwError)
    {
        //Should not hit this case
        case VMDIR_SUCCESS:
            break;
        case ERROR_INVALID_PARAMETER:
        case VMDIR_ERROR_INVALID_PARAMETER:
            dwError = ERROR_INVALID_PARAMETER;
            break;
        case VMDIR_ERROR_SERVER_DOWN:
            dwError = ERROR_NO_LOGON_SERVERS;
            break;
        case VMDIR_ERROR_ENTRY_NOT_FOUND:
        case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
            dwError = ERROR_LOGON_FAILURE;
            break;
        default:
            dwError = ERROR_GENERIC_NOT_MAPPED;
            break;
    }
    goto cleanup;
}

DWORD
DirCliLdapCreateService(
    LDAP*         pLd,
    PCSTR         pszServiceName,
    PCSTR         pszDomain,
    PDIR_CLI_CERT pCert,
    PSTR*         ppszServiceDN
    )
{
    DWORD dwError = 0;
    LDAPMod mod_oc   = {0};
    LDAPMod mod_cn   = {0};
    LDAPMod mod_acct = {0};
    LDAPMod mod_cert = {0};
    LDAPMod mod_subject_dn = {0};
    LDAPMod *mods[] =
    {
        &mod_oc,
        &mod_cn,
        &mod_acct,
        &mod_subject_dn,
        &mod_cert,
        NULL
    };
    PSTR  vals_oc[] = {OBJECT_CLASS_SVC_PRINCIPAL, OBJECT_CLASS_USER, NULL};
    PSTR  vals_cn[] = {(PSTR)pszServiceName, NULL};
    PSTR  vals_account[] = {(PSTR)pszServiceName, NULL};
    PSTR  vals_subject_dn[] = {pCert->pszSubjectName, NULL};
    struct berval bercert = { 0 };
    struct berval *bervals_cert[] = {&bercert, NULL};
    PSTR  pszServiceDN = NULL;
    PBYTE pCertBytes = NULL;
    DWORD dwLength = 0;

    dwError = DirCliGetServiceDN(pszServiceName, pszDomain, &pszServiceDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliX509ToDER(pCert->pX509Cert, &pCertBytes, &dwLength);
    BAIL_ON_VMAFD_ERROR(dwError);

    mod_oc.mod_op   = LDAP_MOD_ADD;
    mod_oc.mod_type = ATTR_NAME_OBJECTCLASS;
    mod_oc.mod_vals.modv_strvals = vals_oc;

    mod_cn.mod_op   = LDAP_MOD_ADD;
    mod_cn.mod_type = ATTR_NAME_CN;
    mod_cn.mod_vals.modv_strvals = vals_cn;

    mod_acct.mod_op   = LDAP_MOD_ADD;
    mod_acct.mod_type = ATTR_NAME_ACCOUNT;
    mod_acct.mod_vals.modv_strvals = vals_account;

    mod_subject_dn.mod_op   = LDAP_MOD_ADD;
    mod_subject_dn.mod_type = ATTR_NAME_SUBJECT_DN;
    mod_subject_dn.mod_vals.modv_strvals = vals_subject_dn;

    bercert.bv_len = dwLength;
    bercert.bv_val = pCertBytes;

    mod_cert.mod_op   = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mod_cert.mod_type = ATTR_NAME_CERT;
    mod_cert.mod_vals.modv_bvals = bervals_cert;

    dwError = ldap_add_ext_s(pLd, pszServiceDN, mods, NULL, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszServiceDN = pszServiceDN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pCertBytes);

    return dwError;

error:

    *ppszServiceDN = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszServiceDN);

    goto cleanup;
}

DWORD
DirCliLdapUpdateService(
    LDAP*         pLd,
    PCSTR         pszServiceName,
    PCSTR         pszDomain,
    PDIR_CLI_CERT pCert
    )
{
    DWORD dwError = 0;
    PSTR  pszServiceDN = NULL;
    PSTR  pszSubjectDN = NULL;
    PBYTE pCertBytes = NULL;
    DWORD dwLength = 0;

    dwError = DirCliLdapFindService(
                    pLd,
                    pszServiceName,
                    pszDomain,
                    &pszSubjectDN,
                    &pszServiceDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliX509ToDER(pCert->pX509Cert, &pCertBytes, &dwLength);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!VmAfdStringCompareA(pszSubjectDN, pCert->pszSubjectName, FALSE))
    {
        LDAPMod mod_cert = {0};
        LDAPMod *mods[] =
        {
            &mod_cert,
            NULL
        };
        struct berval bercert = { 0 };
        struct berval *bervals_cert[] = {&bercert, NULL};

        bercert.bv_len = dwLength;
        bercert.bv_val = pCertBytes;

        mod_cert.mod_op   = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
        mod_cert.mod_type = ATTR_NAME_CERT;
        mod_cert.mod_vals.modv_bvals = bervals_cert;

        dwError = ldap_modify_ext_s(pLd, pszServiceDN, mods, NULL, NULL);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else /* subject DN in certificate changed */
    {
        LDAPMod mod_cert = {0};
        LDAPMod mod_subject_dn = {0};
        LDAPMod *mods[] =
        {
            &mod_subject_dn,
            &mod_cert,
            NULL
        };
        PSTR  vals_subject_dn[] = {pCert->pszSubjectName, NULL};
        struct berval bercert = { 0 };
        struct berval *bervals_cert[] = {&bercert, NULL};

        mod_subject_dn.mod_op   = LDAP_MOD_REPLACE;
        mod_subject_dn.mod_type = ATTR_NAME_SUBJECT_DN;
        mod_subject_dn.mod_vals.modv_strvals = vals_subject_dn;

        bercert.bv_len = dwLength;
        bercert.bv_val = pCertBytes;

        mod_cert.mod_op   = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
        mod_cert.mod_type = ATTR_NAME_CERT;
        mod_cert.mod_vals.modv_bvals = bervals_cert;

        dwError = ldap_modify_ext_s(pLd, pszServiceDN, mods, NULL, NULL);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSubjectDN);
    VMAFD_SAFE_FREE_MEMORY(pszServiceDN);
    VMAFD_SAFE_FREE_MEMORY(pCertBytes);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapFindService(
    LDAP* pLd,
    PCSTR pszServiceName,
    PCSTR pszDomain,
    PSTR* ppszSubjectDN,
    PSTR* ppszDN
    )
{
    DWORD dwError = 0;
    PSTR  ppszAttrs[] = {ATTR_NAME_SUBJECT_DN, NULL};
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR  pszFilter = NULL;
    PSTR  pszSearchBase = NULL;
    PSTR  pszSubjectDN = NULL;
    PSTR  pszDN = NULL;
    PSTR  pszObjectDN = NULL;
    DWORD dwCount = 0;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_SVC_PRINCIPAL,
                    ATTR_NAME_ACCOUNT,
                    pszServiceName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwCount = ldap_count_entries(pLd, pSearchRes);
    if (!dwCount)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwCount > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_NAME_SUBJECT_DN);

    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                        &pszSubjectDN,
                        "%s",
                        ppValues[0]->bv_val);
    BAIL_ON_VMAFD_ERROR(dwError);

    pszObjectDN = ldap_get_dn(pLd, pEntry);
    if (IsNullOrEmptyString(pszObjectDN))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszObjectDN, &pszDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSubjectDN = pszSubjectDN;
    *ppszDN = pszDN;

cleanup:

    if (pszObjectDN)
    {
        ldap_memfree(pszObjectDN);
    }
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree( pSearchRes );
    }

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:

    *ppszSubjectDN = NULL;
    *ppszDN = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszSubjectDN);
    VMAFD_SAFE_FREE_MEMORY(pszDN);

    goto cleanup;
}

DWORD
DirCliLdapBeginEnumServices(
    LDAP*  pLd,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_SVC_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_ACCOUNT, NULL};
    PDIR_CLI_ENUM_SVC_CONTEXT pContext = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_ENUM_SVC_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->pLd = pLd;
    pContext->dwMaxCount = (dwMaxCount <= 256 ? 256 : dwMaxCount);

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(%s=%s)",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_SVC_PRINCIPAL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pContext->pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->dwNumEntries = ldap_count_entries(
                                    pContext->pLd,
                                    pContext->pSearchRes);

    *ppContext = pContext;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        DirCliLdapEndEnumServices(pContext);
    }

    goto cleanup;
}

DWORD
DirCliLdapEnumServices(
    PDIR_CLI_ENUM_SVC_CONTEXT pContext,
    PSTR** pppszAccounts,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* ppszAccounts = NULL;
    DWORD dwCount = 0;
    DWORD idx = 0;
    struct berval** ppValues = NULL;

    if (!pContext->dwNumEntries ||
         (pContext->dwNumRead == pContext->dwNumEntries))
    {
        dwError = ERROR_NO_MORE_ITEMS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwCount = pContext->dwNumEntries - pContext->dwNumRead;
    if (dwCount > pContext->dwMaxCount)
    {
        dwCount = pContext->dwMaxCount;
    }

    dwError = VmAfdAllocateMemory(sizeof(PSTR)*dwCount, (PVOID*)&ppszAccounts);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; idx < dwCount; idx++)
    {
        if (!pContext->pEntry)
        {
            pContext->pEntry = ldap_first_entry(
                                    pContext->pLd,
                                    pContext->pSearchRes);
        }
        else
        {
            pContext->pEntry = ldap_next_entry(
                                    pContext->pLd,
                                    pContext->pEntry);
        }

        if (!pContext->pEntry)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(
                        pContext->pLd,
                        pContext->pEntry,
                        ATTR_NAME_ACCOUNT);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdAllocateStringPrintf(
                        &ppszAccounts[idx],
                        "%s",
                        ppValues[0]->bv_val);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->dwNumRead += dwCount;

    *pppszAccounts = ppszAccounts;
    *pdwCount = dwCount;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    return dwError;

error:

    *pppszAccounts = NULL;
    *pdwCount = 0;

    if (ppszAccounts)
    {
        VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
    }

    goto cleanup;
}

VOID
DirCliLdapEndEnumServices(
    PDIR_CLI_ENUM_SVC_CONTEXT pContext
    )
{
    if (pContext->pSearchRes)
    {
        ldap_msgfree(pContext->pSearchRes);
    }
    VmAfdFreeMemory(pContext);
}

DWORD
DirCliLdapBeginEnumMembers(
    LDAP*  pLd,
    PCSTR  pszGroup,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_GROUP_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_MEMBER, NULL};
    PDIR_CLI_ENUM_GROUP_CONTEXT pContext = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_ENUM_GROUP_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->pLd = pLd;
    pContext->dwMaxCount = (dwMaxCount <= 256 ? 256 : dwMaxCount);

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_GROUP,
                    ATTR_NAME_ACCOUNT,
                    pszGroup);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pContext->pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->dwNumEntries = ldap_count_entries(
                                    pContext->pLd,
                                    pContext->pSearchRes);
    if (pContext->dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_GROUP;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (pContext->dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->pEntry = ldap_first_entry(pLd, pContext->pSearchRes);
    if (!pContext->pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->ppValues = ldap_get_values_len(
                             pContext->pLd,
                             pContext->pEntry,
                             ATTR_NAME_MEMBER);
    if (pContext->ppValues)
    {
        pContext->dwNumValues = ldap_count_values_len(pContext->ppValues);
    }
    else
    {
        pContext->dwNumValues = 0;
    }

    *ppContext = pContext;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        DirCliLdapEndEnumMembers(pContext);
    }

    goto cleanup;
}

DWORD
DirCliLdapEnumMembers(
    PDIR_CLI_ENUM_GROUP_CONTEXT pContext,
    PSTR** pppszAccounts,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* ppszAccounts = NULL;
    DWORD dwCount = 0;
    DWORD idx = 0;

    if (!pContext->dwNumValues ||
         (pContext->dwNumRead == pContext->dwNumValues))
    {
        dwError = ERROR_NO_MORE_ITEMS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwCount = pContext->dwNumValues - pContext->dwNumRead;
    if (dwCount > pContext->dwMaxCount)
    {
        dwCount = pContext->dwMaxCount;
    }

    dwError = VmAfdAllocateMemory(sizeof(PSTR)*dwCount, (PVOID*)&ppszAccounts);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; idx < dwCount; idx++)
    {
        dwError = VmAfdAllocateStringPrintf(
                        &ppszAccounts[idx],
                        "%s",
                        pContext->ppValues[idx]->bv_val);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->dwNumRead += dwCount;

    *pppszAccounts = ppszAccounts;
    *pdwCount = dwCount;

cleanup:

    return dwError;

error:

    *pppszAccounts = NULL;
    *pdwCount = 0;

    if (ppszAccounts)
    {
        VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
    }

    goto cleanup;
}

VOID
DirCliLdapEndEnumMembers(
    PDIR_CLI_ENUM_GROUP_CONTEXT pContext
    )
{
    if (pContext->ppValues)
    {
        ldap_value_free_len(pContext->ppValues);
    }
    if (pContext->pSearchRes)
    {
        ldap_msgfree(pContext->pSearchRes);
    }

    VmAfdFreeMemory(pContext);
}

DWORD
DirCliLdapFindFingerPrintMatch(
    LDAP* pLd,
    PCSTR pszFingerPrint,
    PCSTR pszDomain,
    PSTR* ppszServiceName
    )
{
    DWORD dwError = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_ACCOUNT, ATTR_NAME_CERT, NULL};
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR  pszServiceName = NULL;
    PSTR  pszFingerPrintOther = NULL;
    struct berval** ppValues = NULL;
    X509* pX509Cert = NULL;
    BOOLEAN bMatched = FALSE;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(%s=%s)",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_SVC_PRINCIPAL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!ldap_count_entries(pLd, pSearchRes))
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (pEntry = ldap_first_entry(pLd, pSearchRes);
         pEntry && !bMatched;
         pEntry = ldap_next_entry(pLd, pEntry))
    {
        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(pLd, pEntry, ATTR_NAME_CERT);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pX509Cert)
        {
            X509_free(pX509Cert);
            pX509Cert = NULL;
        }

        dwError = DirCliDERToX509(
                        ppValues[0]->bv_val,
                        ppValues[0]->bv_len,
                        &pX509Cert);
        BAIL_ON_VMAFD_ERROR(dwError);

        VMAFD_SAFE_FREE_MEMORY(pszFingerPrintOther);

        dwError = VecsComputeCertFingerPrint(
                    pX509Cert,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pszFingerPrintOther);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!VmAfdStringCompareA(pszFingerPrint, pszFingerPrintOther, FALSE))
        {
            bMatched = TRUE;

            if (ppValues)
            {
                ldap_value_free_len(ppValues);
                ppValues = NULL;
            }

            ppValues = ldap_get_values_len(pLd, pEntry, ATTR_NAME_ACCOUNT);

            if (!ppValues || (ldap_count_values_len(ppValues) != 1))
            {
                dwError = ERROR_INVALID_STATE;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringPrintf(
                            &pszServiceName,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    if (!bMatched)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszServiceName = pszServiceName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszFingerPrintOther);
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    if (pX509Cert)
    {
        X509_free(pX509Cert);
    }

    return dwError;

error:

    *ppszServiceName = NULL;

    goto cleanup;
}

DWORD
DirCliLdapDeleteService(
    LDAP* pLd,
    PCSTR pszServiceName,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszServiceDN = NULL;

    dwError = DirCliGetServiceDN(pszServiceName, pszDomain, &pszServiceDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_delete_ext_s(
                    pLd,
                    pszServiceDN,
                    NULL,
                    NULL
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszServiceDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapAddServiceToFQDNGroup(
    LDAP* pLd,
    PCSTR pszServiceDN,
    PCSTR pszGroupDN
    )
{
    DWORD dwError = 0;
    LDAPMod mod = {0};
    LDAPMod *mods[] = { &mod, NULL };
    PSTR  vals[] = {(PSTR)pszServiceDN, NULL};

    mod.mod_op   = LDAP_MOD_ADD;
    mod.mod_type = ATTR_NAME_MEMBER;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(pLd, pszGroupDN, mods, NULL, NULL);
    if (dwError == LDAP_TYPE_OR_VALUE_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapAddServiceToGroup(
    LDAP* pLd,
    PCSTR pszServiceDN,
    PCSTR pszGroupName,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszGroupDN = NULL;

    dwError = DirCliLdapFindGroup(pLd, pszGroupName, pszDomain, &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapAddServiceToFQDNGroup(pLd, pszServiceDN, pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapFindGroup(
    LDAP* pLd,
    PCSTR pszGroup,
    PCSTR pszDomain,
    PSTR* ppszGroupDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszFilter = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR  pszDN = NULL;
    PSTR  pszGroupDN = NULL;
    DWORD dwNumEntries = 0;

    dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_GROUP,
                    ATTR_NAME_ACCOUNT,
                    pszGroup);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszDomainDN,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    NULL,
                    TRUE ,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_GROUP;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pszDN = ldap_get_dn(pLd, pEntry);
    if (IsNullOrEmptyString(pszDN))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszDN, &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszGroupDN = pszGroupDN;

cleanup:

    if (pszDN)
    {
        ldap_memfree(pszDN);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    *ppszGroupDN = NULL;

    goto cleanup;
}

DWORD
DirCliLdapFindUser(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PSTR* ppszUserDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszFilter = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR  pszDN = NULL;
    PSTR  pszUserDN = NULL;
    DWORD dwNumEntries = 0;

    dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_ACCOUNT,
                    pszAccount,
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_USER);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszDomainDN,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    NULL,
                    TRUE ,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pszDN = ldap_get_dn(pLd, pEntry);
    if (IsNullOrEmptyString(pszDN))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszDN, &pszUserDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszUserDN = pszUserDN;

cleanup:

    if (pszDN)
    {
        ldap_memfree(pszDN);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    *ppszUserDN = NULL;

    goto cleanup;
}

DWORD
DirCliLdapDeleteUser(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszUserDN = NULL;

    dwError = DirCliLdapFindUser(pLd, pszAccount, pszDomain, &pszUserDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_delete_ext_s(
                    pLd,
                    pszUserDN,
                    NULL,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszUserDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapGetUserAttr(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    USER_INFO_LEVEL userInfoLevel,
    PDIR_CLI_USER_INFO* ppUserInfo
    )
{
    DWORD dwError = 0;
    if (userInfoLevel == USER_INFO_LEVEL_DEFAULT)
    {
        dwError = DirCliLdapGetUserAttrLevelDefault(
                      pLd,
                      pszAccount,
                      pszDomain,
                      ppUserInfo);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (userInfoLevel == USER_INFO_LEVEL_ONE)
    {
        dwError = DirCliLdapGetUserAttrLevelOne(
                      pLd,
                      pszAccount,
                      pszDomain,
                      ppUserInfo);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (userInfoLevel == USER_INFO_LEVEL_TWO)
    {
        dwError = DirCliLdapGetUserAttrLevelTwo(
                      pLd,
                      pszAccount,
                      pszDomain,
                      ppUserInfo);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:
    return dwError;
}

DWORD
DirCliLdapGetUserAttrLevelDefault(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  pszAttr = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_UPN, NULL};
    PSTR* ppszAccount = NULL;
    PSTR* ppszUserUPN = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    BerElement* ber = NULL;
    struct berval** ppValues = NULL;
    PDIR_CLI_USER_INFO pUserInfo = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO),
                    (PVOID*)&pUserInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO_0),
                    (PVOID*)&(pUserInfo->info.pUserInfo_0));
    BAIL_ON_VMAFD_ERROR(dwError);

    ppszAccount = &(pUserInfo->info.pUserInfo_0->pszAccount);
    ppszUserUPN = &(pUserInfo->info.pUserInfo_0->pszUPN);
    pUserInfo->dwInfoLevel = USER_INFO_LEVEL_DEFAULT;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_USER,
                    ATTR_NAME_ACCOUNT,
                    pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    ppszAccount,
                    "%s",
                    pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (pszAttr = ldap_first_attribute(pLd, pEntry, &ber);
         pszAttr != NULL;
         pszAttr = ldap_next_attribute(pLd, pEntry, ber))
    {
        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(pLd, pEntry, pszAttr);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_UPN, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszUserUPN,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
          dwError = ERROR_INVALID_STATE;
          BAIL_ON_VMAFD_ERROR(dwError);
        }

    }

    *ppUserInfo = pUserInfo;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszAttr);

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    if (ber)
    {
        ber_free(ber, 0);
    }

    return dwError;

error:

    *ppUserInfo = NULL;

    if (pUserInfo)
    {
        DirCliFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

DWORD
DirCliLdapGetUserAttrLevelOne(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  pszAttr = NULL;
    PSTR  ppszAttrs[] = {
        ATTR_NAME_UPN,
        ATTR_NAME_GIVEN_NAME,
        ATTR_NAME_SN,
        NULL};
    PSTR* ppszAccount = NULL;
    PSTR* ppszUserUPN = NULL;
    PSTR* ppszUserFirstName = NULL;
    PSTR* ppszUserSN = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    BerElement* ber = NULL;
    struct berval** ppValues = NULL;
    PDIR_CLI_USER_INFO pUserInfo = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO),
                    (PVOID*)&pUserInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO_1),
                    (PVOID*)&(pUserInfo->info.pUserInfo_1));
    BAIL_ON_VMAFD_ERROR(dwError);

    ppszAccount = &(pUserInfo->info.pUserInfo_1->pszAccount);
    ppszUserUPN = &(pUserInfo->info.pUserInfo_1->pszUPN);
    ppszUserFirstName = &(pUserInfo->info.pUserInfo_1->pszFirstName);
    ppszUserSN = &(pUserInfo->info.pUserInfo_1->pszLastName);
    pUserInfo->dwInfoLevel = USER_INFO_LEVEL_ONE;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_USER,
                    ATTR_NAME_ACCOUNT,
                    pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    ppszAccount,
                    "%s",
                    pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (pszAttr = ldap_first_attribute(pLd, pEntry, &ber);
         pszAttr != NULL;
         pszAttr = ldap_next_attribute(pLd, pEntry, ber))
    {
        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(pLd, pEntry, pszAttr);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_UPN, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszUserUPN,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_GIVEN_NAME, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszUserFirstName,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_SN, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszUserSN,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
          dwError = ERROR_INVALID_STATE;
          BAIL_ON_VMAFD_ERROR(dwError);
        }

    }

    *ppUserInfo = pUserInfo;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszAttr);

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    if (ber)
    {
        ber_free(ber, 0);
    }

    return dwError;

error:

    *ppUserInfo = NULL;

    if (pUserInfo)
    {
        DirCliFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

DWORD
DirCliLdapGetUserAttrLevelTwo(
    LDAP* pLd,
    PCSTR pszAccount,
    PCSTR pszDomain,
    PDIR_CLI_USER_INFO* ppUserInfo
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  pszAttr = NULL;

    PSTR  ppszAttrs[] = {
        ATTR_NAME_UPN,
        ATTR_NAME_USER_ACC_CTRL,
        ATTR_NAME_PWD_NEVER_EXPIRE,
        ATTR_NAME_PWD_LAST_SET,
        NULL};
    PSTR* ppszAccount = NULL;
    PSTR* ppszUserUPN = NULL;
    PSTR* ppszPwdLastSet = NULL;
    PSTR  pszUserAccCtrl = NULL;
    PSTR  pszPwdExpTime = NULL;
    PSTR  pszPwdNeverExp = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    BerElement* ber = NULL;
    struct berval** ppValues = NULL;
    PDIR_CLI_USER_INFO pUserInfo = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO),
                    (PVOID*)&pUserInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_USER_INFO_2),
                    (PVOID*)&(pUserInfo->info.pUserInfo_2));
    BAIL_ON_VMAFD_ERROR(dwError);

    ppszAccount = &(pUserInfo->info.pUserInfo_2->pszAccount);
    ppszUserUPN = &(pUserInfo->info.pUserInfo_2->pszUPN);
    ppszPwdLastSet = &(pUserInfo->info.pUserInfo_2->pszPwdLastSet);
    pUserInfo->dwInfoLevel = USER_INFO_LEVEL_TWO;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(&(%s=%s)(%s=%s))",
                    ATTR_NAME_ACCOUNT,
                    pszAccount,
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_USER);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    ppszAccount,
                    "%s",
                    pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (pszAttr = ldap_first_attribute(pLd, pEntry, &ber);
         pszAttr != NULL;
         pszAttr = ldap_next_attribute(pLd, pEntry, ber))
    {
        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(pLd, pEntry, pszAttr);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_UPN, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszUserUPN,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_USER_ACC_CTRL, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            &pszUserAccCtrl,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(
                        pszAttr,
                        ATTR_NAME_PWD_NEVER_EXPIRE,
                        TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            &pszPwdNeverExp,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(pszAttr, ATTR_NAME_PWD_LAST_SET, TRUE))
        {
            dwError = VmAfdAllocateStringPrintf(
                            ppszPwdLastSet,
                            "%s",
                            ppValues[0]->bv_val);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
          dwError = ERROR_INVALID_STATE;
          BAIL_ON_VMAFD_ERROR(dwError);
        }

    }

    if (pszUserAccCtrl != NULL)
    {

        pUserInfo->info.pUserInfo_2->dwUserAccCtrl =
            VmAfdStringToLA((PCSTR)(pszUserAccCtrl), NULL, 10);
    }

    if (pszPwdNeverExp != NULL)
    {
        pUserInfo->info.pUserInfo_2->bIsPwdNeverExpired =
            !VmAfdStringCompareA(
                pszPwdNeverExp,
                LDAP_BOOLEAN_SYNTAX_TRUE_STR,
                TRUE);
    }

    dwError = DirCliUserGetUserPwdExpTimeDays(pLd, pszDomain, &pszPwdExpTime);
    BAIL_ON_VMAFD_ERROR(dwError);

    pUserInfo->info.pUserInfo_2->pszPwdExpTime = pszPwdExpTime;
    *ppUserInfo = pUserInfo;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);
    VMAFD_SAFE_FREE_MEMORY(pszAttr);
    VMAFD_SAFE_FREE_MEMORY(pszUserAccCtrl);
    VMAFD_SAFE_FREE_MEMORY(pszPwdNeverExp);

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    if (ber)
    {
        ber_free(ber, 0);
    }

    return dwError;

error:

    *ppUserInfo = NULL;

    if (pUserInfo)
    {
        DirCliFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

DWORD
DirCliUserGetUserPwdExpTimeDays(
    LDAP*  pLd,
    PCSTR  pszDomain,
    PSTR* ppszPwdExpTime
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszPolicyDN = NULL;
    PSTR  pszAttr = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_PASS_EXP_IN_DAY, NULL};
    PSTR  pszPwdExpTime = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    BerElement* ber = NULL;
    struct berval** ppValues = NULL;

    dwError = DirCliGetDomainDN(pszDomain, &pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszPolicyDN,
                    "cn=%s,%s",
                    ATTR_NAME_PWD_LOCKOUT_POLICY_CN,
                    pszSearchBase);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszPolicyDN,
                    LDAP_SCOPE_BASE,
                    NULL,
                    ppszAttrs,
                    FALSE,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchRes);
    if (dwNumEntries == 0)
    {
        dwError = ERROR_POLICY_OBJECT_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_NAME_PASS_EXP_IN_DAY);

    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszPwdExpTime,
                    "%s",
                    ppValues[0]->bv_val);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPwdExpTime = pszPwdExpTime;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszPolicyDN);
    VMAFD_SAFE_FREE_MEMORY(pszAttr);

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    if (ber)
    {
        ber_free(ber, 0);
    }

    return dwError;

error:

    *ppszPwdExpTime = NULL;

    goto cleanup;
}

DWORD
DirCliUserGetCurrentPwdExpTime(
    PDIR_CLI_USER_INFO pUserInfo,
    PSTR* ppszParsedPwdExpTime
    )
{
    DWORD dwError = 0;
    DWORD dwTimeLeftSecs = 0;
    DWORD dwTimeLeftMins = 0;
    DWORD dwTimeLeftHours = 0;
    DWORD dwTimeLeftDays = 0;
    PSTR pszTimeLeftSecs = NULL;
    PSTR pszTimeLeftMins = NULL;
    PSTR pszTimeLeftHours = NULL;
    PSTR pszTimeLeftDays = NULL;
    PSTR pszParsedPwdExpTime = NULL;
    int64_t iPwdLastSet = 0;
    int64_t iExpireInDay = 0;
    int64_t iExpireTimeLeft = 0;
    int64_t iExpireInSeconds = 0;
    time_t  tNow = 0;

    iPwdLastSet = VmAfdStringToLA(
                    (PCSTR)pUserInfo->info.pUserInfo_2->pszPwdLastSet,
                    NULL,
                    10);
    iExpireInDay = VmAfdStringToLA(
                    (PCSTR)pUserInfo->info.pUserInfo_2->pszPwdExpTime,
                    NULL,
                    10);

    iExpireInSeconds = iExpireInDay * SECS_IN_DAY;
    time(&tNow);

    if (iExpireInDay == 0)
    {
        dwError = VmAfdAllocateStringPrintf(
                    &pszParsedPwdExpTime,
                    "never");
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if ((tNow - iPwdLastSet) < iExpireInSeconds)
    {
        iExpireTimeLeft = iPwdLastSet + iExpireInSeconds - tNow;

        dwTimeLeftDays = iExpireTimeLeft / SECS_IN_DAY;
        iExpireTimeLeft %= SECS_IN_DAY;
        dwTimeLeftHours = iExpireTimeLeft / SECS_IN_HOUR;
        iExpireTimeLeft %= SECS_IN_HOUR;
        dwTimeLeftMins = iExpireTimeLeft / SECS_IN_MINUTE;
        iExpireTimeLeft %= SECS_IN_MINUTE;
        dwTimeLeftSecs = iExpireTimeLeft;

        dwError = VmAfdAllocateStringPrintf(
                    &pszTimeLeftSecs,
                    (dwTimeLeftSecs > 0) ? "%d second(s)":"",
                    dwTimeLeftSecs);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                    &pszTimeLeftMins,
                    (dwTimeLeftMins > 0) ? "%d minute(s) ":"",
                    dwTimeLeftMins);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                    &pszTimeLeftHours,
                    (dwTimeLeftHours > 0) ? "%d hour(s) ":"",
                    dwTimeLeftHours);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                    &pszTimeLeftDays,
                   (dwTimeLeftDays > 0) ?  "%d day(s) ":"",
                    dwTimeLeftDays);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                    &pszParsedPwdExpTime,
                    "%s%s%s%s",
                    pszTimeLeftDays,
                    pszTimeLeftHours,
                    pszTimeLeftMins,
                    pszTimeLeftSecs);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        struct tm tmptime = {0};
        time_t expiryTime = iPwdLastSet + iExpireInSeconds;
        CHAR timeBuf[128] = "";

#ifdef _WIN32
        localtime_s(&tmptime, &expiryTime);
#else
        localtime_r(&expiryTime, &tmptime);
#endif
        strftime(timeBuf, sizeof(timeBuf), "%Y%m%d%H%M%S", &tmptime);

        dwError = VmAfdAllocateStringPrintf(
                    &pszParsedPwdExpTime,
                    "password expired at %s",
                    &timeBuf[0]);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszParsedPwdExpTime = pszParsedPwdExpTime;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszTimeLeftSecs);
    VMAFD_SAFE_FREE_MEMORY(pszTimeLeftMins);
    VMAFD_SAFE_FREE_MEMORY(pszTimeLeftHours);
    VMAFD_SAFE_FREE_MEMORY(pszTimeLeftDays);

    return dwError;

error:

    *ppszParsedPwdExpTime = NULL;

    goto cleanup;

}


VOID
DirCliFreeUserInfo(
    PDIR_CLI_USER_INFO pUserInfo
    )
{
    if (pUserInfo->dwInfoLevel == USER_INFO_LEVEL_DEFAULT &&
        pUserInfo->info.pUserInfo_0 != NULL)
    {
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_0->pszAccount);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_0->pszUPN);
    }
    else if (pUserInfo->dwInfoLevel == USER_INFO_LEVEL_ONE &&
                pUserInfo->info.pUserInfo_1 != NULL)
    {
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_1->pszAccount);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_1->pszUPN);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_1->pszFirstName);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_1->pszLastName);
    }
    else if (pUserInfo->dwInfoLevel == USER_INFO_LEVEL_TWO &&
                pUserInfo->info.pUserInfo_1 != NULL)
    {
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_2->pszAccount);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_2->pszUPN);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_2->pszPwdLastSet);
        VMAFD_SAFE_FREE_MEMORY(pUserInfo->info.pUserInfo_2->pszPwdExpTime);
    }

    VmAfdFreeMemory(pUserInfo);
}

DWORD
DirCliLdapReplaceUserAttr(
    LDAP* pLd,
    PCSTR pszAccountDN,
    PCSTR pszAttrName,
    PCSTR pszAttrValue
    )
{
    DWORD dwError = 0;
    PSTR  vals[] = {(PSTR)pszAttrValue, NULL};
    LDAPMod mod = {0};
    LDAPMod *mods[] = { &mod, NULL };

    if (IsNullOrEmptyString(pszAccountDN) ||
        IsNullOrEmptyString(pszAttrName)  ||
        IsNullOrEmptyString(pszAttrValue))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    mod.mod_op   = LDAP_MOD_REPLACE;
    mod.mod_type = (PSTR)pszAttrName;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(
                    pLd,
                    pszAccountDN,
                    mods,
                    NULL,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error :

    goto cleanup;
}

DWORD
DirCliLdapUserModifyAttrPwdNeverExp(
    LDAP* pLd,
    PCSTR pszAccountDN,
    PCSTR pszDomain,
    PCSTR pszAttrValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    )
{
    DWORD dwError = 0;

    dwError = DirCliLdapCheckAttribute(
            pLd,
            pszAccountDN,
            ATTR_NAME_PWD_NEVER_EXPIRE,
            pszAttrValue,
            pAttrStatus
            );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (*pAttrStatus != ATTR_MATCH)
    {
        dwError = DirCliLdapReplaceUserAttr(
                pLd,
                pszAccountDN,
                ATTR_NAME_PWD_NEVER_EXPIRE,
                pszAttrValue);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;

error :

    goto cleanup;
}

DWORD
DirCliLdapChangePassword(
    LDAP* pLd,
    PCSTR pszUserDN,
    PCSTR pszPasswordCurrent,
    PCSTR pszPasswordNew
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod[2] = {{0}};
    LDAPMod*    mods[3] = {&mod[0], &mod[1], NULL};
    PSTR        vals_new[2] = {(PSTR)pszPasswordNew, NULL};
    PSTR        vals_old[2] = {(PSTR)pszPasswordCurrent, NULL};

    if (IsNullOrEmptyString(pszUserDN) ||
        IsNullOrEmptyString(pszPasswordCurrent) ||
        IsNullOrEmptyString(pszPasswordNew))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    mod[0].mod_op = LDAP_MOD_ADD;
    mod[0].mod_type = ATTR_USER_PASSWORD;
    mod[0].mod_vals.modv_strvals = vals_new;

    mod[1].mod_op = LDAP_MOD_DELETE;
    mod[1].mod_type = ATTR_USER_PASSWORD;
    mod[1].mod_vals.modv_strvals = vals_old;

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszUserDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
DirCliLdapResetPassword(
    LDAP* pLd,
    PCSTR pszUserDN,
    PCSTR pszNewPassword
    )
{
    DWORD    dwError = 0;
    LDAPMod  mod = {0};
    LDAPMod* mods[2] = {&mod, NULL};
    PSTR     vals[2] = {(PSTR)pszNewPassword, NULL};

    if (IsNullOrEmptyString(pszUserDN) ||
        IsNullOrEmptyString(pszNewPassword))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = ATTR_USER_PASSWORD;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(
                    pLd,
                    pszUserDN,
                    mods,
                    NULL,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;

}

VOID
DirCliLdapClose(
    LDAP* pLd
    )
{
    if (pLd)
    {
        ldap_unbind_ext(pLd, NULL, NULL);
    }
}

DWORD
DirCliGetUserDN(
    PCSTR pszUser,
    PCSTR pszDomain,
    PSTR* ppszUserDN
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszUserDN = NULL;

    dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszUserDN,
                    "CN=%s,CN=Users,%s",
                    pszUser,
                    pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszUserDN = pszUserDN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    *ppszUserDN = NULL;

    goto cleanup;
}

DWORD
DirCliGetServiceDN(
    PCSTR pszServiceName,
    PCSTR pszDomain,
    PSTR* ppszServiceDN
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszUserDN = NULL;

    dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszUserDN,
                    "CN=%s,CN=ServicePrincipals,%s",
                    pszServiceName,
                    pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszServiceDN = pszUserDN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    *ppszServiceDN = NULL;

    goto cleanup;
}

DWORD
DirCliGetOrgunitDN(
    PCSTR pszOrgunit,
    PCSTR pszDomain,
    PCSTR pszParentDN,
    PSTR* ppszOrgunitDN
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszOrgunitDN = NULL;
    PSTR pszLocalParentDN = NULL;

    if (!pszParentDN)
    {
        dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                        &pszLocalParentDN,
                        "OU=Computers,%s",
                        pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszOrgunitDN,
                    "OU=%s,%s",
                    pszOrgunit,
                    pszLocalParentDN ? pszLocalParentDN : pszParentDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszOrgunitDN = pszOrgunitDN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);
    VMAFD_SAFE_FREE_MEMORY(pszLocalParentDN);

    return dwError;

error:

    *ppszOrgunitDN = NULL;

    goto cleanup;
}

DWORD
DirCliGetGroupDN(
    PCSTR pszGroupName,
    PCSTR pszDomain,
    PCSTR pszParentDN,
    PSTR* ppszGroupDN
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszGroupDN = NULL;

    dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszParentDN == NULL)
    {
        dwError = VmAfdAllocateStringPrintf(
                        &pszGroupDN,
                        "CN=%s,%s",
                        pszGroupName,
                        pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdAllocateStringPrintf(
                        &pszGroupDN,
                        "CN=%s,%s,%s",
                        pszGroupName,
                        pszParentDN,
                        pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszGroupDN = pszGroupDN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);
    *ppszGroupDN = NULL;
    goto cleanup;
}

DWORD
DirCliGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    )
{
    DWORD dwError = 0;
    const char* pszDelim = ".";
    const char* pszPrefix = "DC=";
    size_t len_prefix = sizeof("DC=")-1;
    const char* pszCommaDelim = ",";
    size_t len_comma_delim = sizeof(",")-1;
    size_t len = 0;
    PSTR  pszDomainDN = NULL;
    PCSTR pszReadCursor = pszDomain;
    PSTR  pszWriteCursor = NULL;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (len > 0)
            {
                len += len_comma_delim;
            }
            len += len_prefix;
            len += len_name;
        }

        pszReadCursor += len_name;

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    if (!len)
    {
        dwError = ERROR_INVALID_DOMAINNAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(len+1, (PVOID*)&pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    pszReadCursor  = pszDomain;
    pszWriteCursor = pszDomainDN;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (pszWriteCursor > pszDomainDN)
            {
                memcpy(pszWriteCursor, pszCommaDelim, len_comma_delim);
                pszWriteCursor += len_comma_delim;
            }

            memcpy(pszWriteCursor, pszPrefix, len_prefix);
            pszWriteCursor += len_prefix;

            memcpy(pszWriteCursor, pszReadCursor, len_name);

            pszReadCursor += len_name;
            pszWriteCursor += len_name;
        }

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    *ppszDomainDN = pszDomainDN;

cleanup:

    return dwError;

error:

    *ppszDomainDN = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    goto cleanup;

}

DWORD
DirCliLdapAddServiceToBuiltinGroup(
    LDAP* pLd,
    PCSTR pszServiceDN,
    PCSTR pszGroupName,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszGroupDN = NULL;
    PCSTR pszParentDN = "CN=Builtin";

    dwError = DirCliGetGroupDN(
            pszGroupName,
            pszDomain,
            pszParentDN,
            &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapAddServiceToFQDNGroup(pLd, pszServiceDN, pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapCreateGroup(
    LDAP*         pLd,
    PCSTR         pszGroupName,
    PCSTR         pszDescription,
    PCSTR         pszDomain,
    PSTR*         ppszGroupDN
    )
{
    DWORD dwError = 0;
    LDAPMod mod_oc = {0};
    LDAPMod mod_cn = {0};
    LDAPMod mod_acct = {0};
    LDAPMod mod_desc = {0};
    LDAPMod *mods[] =
    {
        &mod_oc,
        &mod_cn,
        &mod_acct,
        &mod_desc,
        NULL
    };

    PCSTR pzLocalDescription = (pszDescription) ? pszDescription : pszGroupName;
    PSTR  vals_oc[] = {OBJECT_CLASS_GROUP, NULL};
    PSTR  vals_cn[] = {(PSTR)pszGroupName, NULL};
    PSTR  vals_account[] = {(PSTR)pszGroupName, NULL};
    PSTR  vals_description[] = {(PSTR)pzLocalDescription, NULL};

    PSTR  pszGroupDN = NULL;

    dwError = DirCliGetGroupDN(pszGroupName, pszDomain, NULL, &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    mod_oc.mod_op       = LDAP_MOD_ADD;
    mod_oc.mod_type     = ATTR_NAME_OBJECTCLASS;
    mod_oc.mod_vals.modv_strvals = vals_oc;

    mod_cn.mod_op       = LDAP_MOD_ADD;
    mod_cn.mod_type     = ATTR_NAME_CN;
    mod_cn.mod_vals.modv_strvals = vals_cn;

    mod_acct.mod_op     = LDAP_MOD_ADD;
    mod_acct.mod_type   = ATTR_NAME_ACCOUNT;
    mod_acct.mod_vals.modv_strvals = vals_account;

    mod_desc.mod_op     = LDAP_MOD_ADD;
    mod_desc.mod_type   = ATTR_NAME_DESCRIPTION;
    mod_desc.mod_vals.modv_strvals = vals_description;

    dwError = ldap_add_ext_s(pLd, pszGroupDN, mods, NULL, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszGroupDN = pszGroupDN;

cleanup:

    return dwError;

error:
    *ppszGroupDN = NULL;
    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);

    goto cleanup;
}

DWORD
DirCliLdapAddGroupMember(
    LDAP*         pLd,
    PCSTR         pszGroup,
    PCSTR         pszAccount,
    PCSTR         pszDomain
    )
{
    DWORD dwError = 0;
    PSTR pszGroupDN = NULL;
    PSTR pszAccountDN = NULL;

    dwError = DirCliLdapFindGroup(pLd, pszGroup, pszDomain, &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    // find user
    if ((dwError = DirCliLdapFindUser(pLd, pszAccount, pszDomain, &pszAccountDN)) == ERROR_NO_SUCH_USER)
    {
        // find group
        dwError = DirCliLdapFindGroup(pLd, pszAccount, pszDomain, &pszAccountDN);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapAddServiceToFQDNGroup(pLd, pszAccountDN, pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);
    VMAFD_SAFE_FREE_MEMORY(pszAccountDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapRemoveGroupMember(
    LDAP*         pLd,
    PCSTR         pszGroupName,
    PCSTR         pszAcctName,
    PCSTR         pszDomain
    )
{
    DWORD dwError = 0;
    LDAPMod mod = {0};
    LDAPMod *mods[] =
    {
        &mod,
        NULL
    };
    PSTR  vals[] = {(PSTR)pszAcctName, NULL};
    PSTR  pszGroupDN = NULL;

    dwError = DirCliGetGroupDN(pszGroupName, pszDomain, NULL, &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    mod.mod_op = LDAP_MOD_DELETE;
    mod.mod_type = ATTR_NAME_MEMBER;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(pLd, pszGroupDN, mods, NULL, NULL);
    if (dwError == LDAP_NO_SUCH_ATTRIBUTE)
    {
        dwError = 0;  // already not a member of the group
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliLdapCreateOrgunit(
    LDAP*         pLd,
    PCSTR         pszOrgunit,
    PCSTR         pszDomain,
    PCSTR         pszParentDN,
    PSTR*         ppszOrgunitDN
    )
{
    DWORD dwError = 0;
    LDAPMod mod_oc = {0};
    LDAPMod mod_ou = {0};
    LDAPMod *mods[] =
    {
        &mod_oc,
        &mod_ou,
        NULL
    };

    PSTR  vals_oc[] = {OBJECT_CLASS_ORGANIZATIONAL_UNIT, OBJECT_CLASS_TOP, NULL};
    PSTR  vals_ou[] = {(PSTR)pszOrgunit, NULL};

    PSTR  pszOrgunitDN = NULL;

    dwError = DirCliGetOrgunitDN(
                  pszOrgunit,
                  pszDomain,
                  pszParentDN,
                  &pszOrgunitDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    mod_oc.mod_op       = LDAP_MOD_ADD;
    mod_oc.mod_type     = ATTR_NAME_OBJECTCLASS;
    mod_oc.mod_vals.modv_strvals = vals_oc;

    mod_ou.mod_op     = LDAP_MOD_ADD;
    mod_ou.mod_type   = ATTR_NAME_OU;
    mod_ou.mod_vals.modv_strvals = vals_ou;

    dwError = ldap_add_ext_s(pLd, pszOrgunitDN, mods, NULL, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszOrgunitDN = pszOrgunitDN;

cleanup:

    return dwError;

error:
    *ppszOrgunitDN = NULL;
    VMAFD_SAFE_FREE_MEMORY(pszOrgunitDN);

    goto cleanup;
}

DWORD
DirCliLdapBeginEnumOrgunits(
    LDAP*  pLd,
    PCSTR  pszContainerDN,
    PCSTR  pszDomain,
    DWORD  dwMaxCount,
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PSTR  pszSearchBase = NULL;
    PSTR  pszFilter = NULL;
    PSTR  ppszAttrs[] = {ATTR_NAME_OU, NULL};
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pContext = NULL;
    PSTR  pszDomainDN = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(DIR_CLI_ENUM_ORGUNIT_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->pLd = pLd;
    pContext->dwMaxCount = (dwMaxCount <= 256 ? 256 : dwMaxCount);

    if (!pszContainerDN)
    {
        dwError = DirCliGetDomainDN(pszDomain, &pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(
                        &pszSearchBase,
                        "OU=Computers,%s",
                        pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszFilter,
                    "(%s=%s)",
                    ATTR_NAME_OBJECTCLASS,
                    OBJECT_CLASS_ORGANIZATIONAL_UNIT);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSearchBase ? pszSearchBase : pszContainerDN,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,             /* attrs only  */
                    NULL,              /* serverctrls */
                    NULL,              /* clientctrls */
                    NULL,              /* timeout */
                    0,
                    &pContext->pSearchRes);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->dwNumEntries = ldap_count_entries(
                                    pContext->pLd,
                                    pContext->pSearchRes);

    *ppContext = pContext;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);
    VMAFD_SAFE_FREE_MEMORY(pszSearchBase);
    VMAFD_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        DirCliLdapEndEnumOrgunits(pContext);
    }

    goto cleanup;
}

DWORD
DirCliLdapEnumOrgunits(
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pContext,
    PSTR** pppszOrgunits,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* ppszOrgunits = NULL;
    DWORD dwCount = 0;
    DWORD idx = 0;
    struct berval** ppValues = NULL;

    if (!pContext->dwNumEntries ||
         (pContext->dwNumRead == pContext->dwNumEntries))
    {
        dwError = ERROR_NO_MORE_ITEMS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwCount = pContext->dwNumEntries - pContext->dwNumRead;
    if (dwCount > pContext->dwMaxCount)
    {
        dwCount = pContext->dwMaxCount;
    }

    dwError = VmAfdAllocateMemory(sizeof(PSTR)*dwCount, (PVOID*)&ppszOrgunits);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; idx < dwCount; idx++)
    {
        if (!pContext->pEntry)
        {
            pContext->pEntry = ldap_first_entry(
                                    pContext->pLd,
                                    pContext->pSearchRes);
        }
        else
        {
            pContext->pEntry = ldap_next_entry(
                                    pContext->pLd,
                                    pContext->pEntry);
        }

        if (!pContext->pEntry)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (ppValues)
        {
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(
                        pContext->pLd,
                        pContext->pEntry,
                        ATTR_NAME_OU);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdAllocateStringPrintf(
                        &ppszOrgunits[idx],
                        "%s",
                        ppValues[0]->bv_val);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->dwNumRead += dwCount;

    *pppszOrgunits = ppszOrgunits;
    *pdwCount = dwCount;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    return dwError;

error:

    *pppszOrgunits = NULL;
    *pdwCount = 0;

    if (ppszOrgunits)
    {
        VmAfdFreeStringArrayCountA(ppszOrgunits, dwCount);
    }

    goto cleanup;
}

VOID
DirCliLdapEndEnumOrgunits(
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pContext
    )
{
    if (pContext->pSearchRes)
    {
        ldap_msgfree(pContext->pSearchRes);
    }
    VmAfdFreeMemory(pContext);
}

DWORD
DirCliLdapGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    )
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PCHAR pDcFilter = "(objectClass=*)";
    PCHAR pDcAttr[] = { pszAttribute, NULL };
    PSTR pAttribute = NULL;
    BerElement* pBer = NULL;
    BerValue** ppValue = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;

    dwError = ldap_search_ext_s(
                  pLotus,
                  "",
                  LDAP_SCOPE_BASE,
                  pDcFilter,
                  pDcAttr,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  0,
                  &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ldap_count_entries(pLotus, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pResults = ldap_first_entry(pLotus, pSearchResult);
    if (pResults == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pLotus,pResults,&pBer);
    if (pAttribute == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pLotus, pResults, pAttribute);
    if (ppValue == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(ppValue[0]->bv_val, ppAttrValue);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if ( ppValue != NULL)
    {
        ldap_value_free_len(ppValue);
    }
    if (pAttribute != NULL)
    {
        ldap_memfree(pAttribute);
    }
    if (pBer != NULL)
    {
        ber_free(pBer,0);
    }
    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    *ppAttrValue = NULL;

    goto cleanup;
}

DWORD
DirCliLdapCheckCAContainer(
    LDAP*    pLd,
    PCSTR    pszConfigurationDN,
    PBOOLEAN pbExists
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    LDAPMessage* pResult = NULL;
    PSTR pszFilter = "(CN=" CA_CONTAINER_NAME ")";

    dwError = ldap_search_ext_s(
                  pLd,
                  (PSTR)pszConfigurationDN,
                  LDAP_SCOPE_ONELEVEL,
                  pszFilter,
                  NULL,      /* attributes      */
                  TRUE,
                  NULL,      /* server controls */
                  NULL,      /* client controls */
                  NULL,      /* timeout         */
                  0,
                  &pResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pResult);
    if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pbExists = (dwNumEntries != 0);

cleanup:

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error :

    *pbExists = FALSE;

    if (dwError == LDAP_NO_SUCH_OBJECT)
    {
        dwError = 0;
    }

    goto cleanup;
}

DWORD
DirCliLdapCreateCAContainer(
    LDAP* pLd,
    PCSTR pszCAConainter
    )
{
    DWORD dwError = 0;
    PSTR ppszObjectClassValues[] =
                {
                    "container",
                    "top",
                    NULL
                };
    char*   modv_cn[] = { CA_CONTAINER_NAME, NULL };
    LDAPMod mod_object = {0};
    LDAPMod mod_cn = {0};
    LDAPMod *mods[] = { &mod_object, &mod_cn, NULL };

    mod_cn.mod_op = LDAP_MOD_ADD;
    mod_cn.mod_type = ATTR_NAME_CN;
    mod_cn.mod_values = modv_cn;

    mod_object.mod_op     = LDAP_MOD_ADD;
    mod_object.mod_type   = ATTR_NAME_OBJECTCLASS;
    mod_object.mod_values = ppszObjectClassValues;

    dwError = ldap_add_ext_s(
                  pLd,
                  (PSTR)pszCAConainter,
                  mods,
                  NULL,
                  NULL
                  );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error :

    goto cleanup;
}

static
DWORD
DirCliLdapFindObject(
    LDAP*    pLd,
    PCSTR    pszBaseDN,
    ber_int_t scope,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    PSTR     *ppszObjectDN
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumEntries = 0;
    PSTR    pszFilter = NULL;
    PSTR    pszObjectDN = NULL;
    LDAPMessage* pResult = NULL;
    LDAPMessage* pEntry = NULL;

    if (!ppszObjectDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszObjectDN = NULL;

    if (pszAttribute && pszValue)
    {
        VmAfdAllocateStringPrintf(
            &pszFilter,
            "(%s=%s)",
            pszAttribute,
            pszValue);
    }

    dwError = ldap_search_ext_s(
                  pLd,
                  (PSTR)pszBaseDN,
                  scope,
                  pszFilter,
                  NULL,      /* attributes      */
                  TRUE,
                  NULL,      /* server controls */
                  NULL,      /* client controls */
                  NULL,      /* timeout         */
                  0,
                  &pResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pResult);
    if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries == 0)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        pEntry = ldap_first_entry(pLd, pResult);
        if (!pEntry)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszObjectDN = ldap_get_dn(pLd, pEntry);
        if (IsNullOrEmptyString(pszObjectDN))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        *ppszObjectDN = pszObjectDN;
    }

cleanup:

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMAFD_SAFE_FREE_STRINGA(pszFilter);

    return dwError;

error :

    if (dwError == LDAP_NO_SUCH_OBJECT)
    {
        dwError = 0;
    }

    if (ppszObjectDN)
    {
        *ppszObjectDN = NULL;
    }

    goto cleanup;
}

DWORD
DirCliLdapCheckCAObject(
    LDAP*    pLd,
    PCSTR    pszCAContainerDN,
    PCSTR    pszCADN,
    PCSTR    pszCAIssuerDN,
    PSTR     *ppszObjectDN
    )
{
    DWORD dwError = 0;
    PSTR  pszObjectDN = NULL;

    if (!pszCADN && !pszCAIssuerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszCADN)
    {
        dwError = DirCliLdapFindObject(pLd, pszCADN, LDAP_SCOPE_BASE,
                NULL, NULL, &pszObjectDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = DirCliLdapFindObject(pLd, pszCAContainerDN,
                LDAP_SCOPE_ONELEVEL, ATTR_NAME_CA_CERTIFICATE_DN,
                pszCAIssuerDN, &pszObjectDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszObjectDN = pszObjectDN;

cleanup:

    return dwError;

error :

    if (ppszObjectDN)
    {
        *ppszObjectDN = NULL;
    }

    goto cleanup;
}

DWORD
DirCliLdapCreateCAObject(
    LDAP*   pLd,
    PCSTR   pszCACN,
    PCSTR   pszCADN
    )
{
    DWORD dwError = 0;
    PSTR ppszObjectClassValues[] =
                {
                    "vmwCertificationAuthority",
                    "pkiCA",
                    "top",
                    NULL
                };
    char*   modv_cn[] = { (PSTR)pszCACN, NULL };
    LDAPMod mod_object = {0};
    LDAPMod mod_cn = {0};
    LDAPMod *mods[] = { &mod_object, &mod_cn, NULL };

    mod_cn.mod_op = LDAP_MOD_ADD;
    mod_cn.mod_type = ATTR_NAME_CN;
    mod_cn.mod_values = modv_cn;

    mod_object.mod_op     = LDAP_MOD_ADD;
    mod_object.mod_type   = ATTR_NAME_OBJECTCLASS;
    mod_object.mod_values = ppszObjectClassValues;

    dwError = ldap_add_ext_s(
                  pLd,
                  (PSTR)pszCADN,
                  mods,
                  NULL,
                  NULL
                  );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error :

    goto cleanup;
}

DWORD
DirCliLdapDeleteCAObject(
    LDAP*    pLd,
    PCSTR    pszCADN
    )
{
    DWORD dwError = 0;

    dwError = ldap_delete_ext_s(
                    pLd,
                    pszCADN,
                    NULL,
                    NULL
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error :

    goto cleanup;
}

DWORD
DirCliLdapCheckAttribute(
    LDAP*    pLd,
    PCSTR    pszObjectDN,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    )
{
    DWORD dwError = 0;
    PCHAR pszFilter = "(objectClass=*)";
    PSTR  pszRetrievedValue = NULL;
    DWORD dwNumEntries = 0;
    LDAPMessage* pSearchResult = NULL;
    PCHAR ppszAttr[] = { (PSTR)pszAttribute, NULL };

    dwError = ldap_search_ext_s(
                  pLd,
                  (PSTR)pszObjectDN,
                  LDAP_SCOPE_BASE,
                  pszFilter,
                  ppszAttr,      /* attributes      */
                  FALSE,
                  NULL,      /* server controls */
                  NULL,      /* client controls */
                  NULL,      /* timeout         */
                  0,
                  &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pSearchResult);
    if (dwNumEntries == 0)
    {
        // Caller should make sure that the ObjectDN passed in does exist
        // by calling DirCliLdapCheckCAObject first.
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwNumEntries != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliCopyQueryResultAttributeString(pLd, pSearchResult,
        pszAttribute, TRUE, &pszRetrievedValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszRetrievedValue)
    {
        *pAttrStatus = ATTR_NOT_FOUND;
    }
    else if (VmAfdStringCompareA(pszValue, pszRetrievedValue, FALSE))
    {
        *pAttrStatus = ATTR_DIFFER;
    }
    else
    {
        *pAttrStatus = ATTR_MATCH;
    }

cleanup:

    if (pSearchResult)
    {
        ldap_msgfree(pSearchResult);
    }
    VMAFD_SAFE_FREE_MEMORY(pszRetrievedValue);

    return dwError;

error :

    *pAttrStatus = ATTR_NOT_FOUND;

    if (dwError == LDAP_NO_SUCH_OBJECT)
    {
        dwError = 0;
    }

    goto cleanup;
}

DWORD
DirCliLdapUpdateAttribute(
    LDAP*   pLd,
    PCSTR   pszObjectDN,
    PSTR    pszAttribute,
    PSTR    pszValue,
    BOOL    bAdd
)
{
    DWORD dwError = 0;
    LDAPMod mod_cert = {0};
    LDAPMod *mods[] = { &mod_cert, NULL};
    struct berval bercert = { 0 };
    struct berval *bervals[] = {&bercert, NULL};

    bercert.bv_len = (ULONG) strlen(pszValue);
    bercert.bv_val = pszValue;

    mod_cert.mod_op = (bAdd ? LDAP_MOD_ADD : LDAP_MOD_REPLACE)
                        | LDAP_MOD_BVALUES;
    mod_cert.mod_type = pszAttribute;
    mod_cert.mod_vals.modv_bvals = bervals;

    dwError = ldap_modify_ext_s(
                  pLd,
                  pszObjectDN,
                  mods,
                  NULL,
                  NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}

DWORD
DirCliLdapUpdateCRL(
    LDAP*   pLd,
    PCSTR   pszCADN,
    PSTR    pszCrl,
    BOOL    bAdd
)
{
    DWORD   dwError = 0;
    PCHAR   pszAttrName = ATTR_NAME_CA_CRL;
    LDAPMod mod_cert = {0};
    LDAPMod *mods[] = { &mod_cert, NULL};
    struct berval bercert = { 0 };
    struct berval *bervals[] = {&bercert, NULL};

    bercert.bv_len = (ULONG) strlen(pszCrl);
    bercert.bv_val = pszCrl;

    mod_cert.mod_op = (bAdd ? LDAP_MOD_ADD : LDAP_MOD_REPLACE)
                        | LDAP_MOD_BVALUES;
    mod_cert.mod_type = pszAttrName;
    mod_cert.mod_vals.modv_bvals = bervals;

    dwError = ldap_modify_ext_s(
                  pLd,
                  pszCADN,
                  mods,
                  NULL,
                  NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}

DWORD
DirCliGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    )
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PCHAR pDcFilter = "(objectClass=*)";
    PCHAR pDcAttr[] = { pszAttribute, NULL };
    PSTR pAttribute = NULL;
    BerElement* pBer = NULL;
    BerValue** ppValue = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;

    dwError = ldap_search_ext_s(
                  pLotus,
                  "",
                  LDAP_SCOPE_BASE,
                  pDcFilter,
                  pDcAttr,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  0,
                  &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ldap_count_entries(pLotus, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pResults = ldap_first_entry(pLotus, pSearchResult);
    if (pResults == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pLotus,pResults,&pBer);
    if (pAttribute == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pLotus, pResults, pAttribute);
    if (ppValue == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(ppValue[0]->bv_val, ppAttrValue);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if ( ppValue != NULL)
    {
        ldap_value_free_len(ppValue);
    }
    if (pAttribute != NULL)
    {
        ldap_memfree(pAttribute);
    }
    if (pBer != NULL)
    {
        ber_free(pBer,0);
    }
    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    *ppAttrValue = NULL;

    goto cleanup;
}

DWORD
DirCliQueryCACerts(
    LDAP* pLotus,
    PCSTR pszCACN,
    BOOL  bDetail,
    PVMAFD_CA_CERT_ARRAY* ppCACertificates
    )
{
    DWORD dwError = 0;
    PSTR pszClassFilter = "(objectClass=vmwCertificationAuthority)";
    PSTR pszFilter      = NULL;
    PSTR pszCACNFilter  = NULL;
    PSTR pszAttrEntryDN = ATTR_NAME_ENTRY_DN;
    PSTR pszAttrCADN    = ATTR_NAME_CA_CERTIFICATE_DN;
    PSTR pszAttrCert    = ATTR_NAME_CA_CERTIFICATE;
    PSTR pszAttrCrl     = ATTR_NAME_CA_CRL;
    PSTR pszSearchBaseDN= NULL;
    PSTR pszDomainName  = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pCAResult = NULL;
    PVMAFD_CA_CERT_ARRAY pCertArray = NULL;

    PCHAR attrs[] = {
        pszAttrEntryDN,
        pszAttrCADN,
        pszAttrCert,
        pszAttrCrl,
        NULL};
    struct berval** ppValues = NULL;
    int nCertCount = 0;
    int certIndex = 0;

    dwError = DirCliGetDefaultDomainName(
                 pLotus,
                 &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                 &pszSearchBaseDN,
                 "cn=Configuration,%s",
                 pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszCACN)
    {
        dwError = VmAfdAllocateStringPrintf(
                &pszCACNFilter, "(&(CN=%s)%s)",
                pszCACN, pszClassFilter);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszFilter = pszCACNFilter;
    }
    else
    {
        pszFilter = pszClassFilter;
    }

    dwError = ldap_search_ext_s(
                 pLotus,
                 pszSearchBaseDN,
                 LDAP_SCOPE_SUBTREE,
                 pszFilter,
                 attrs,
                 0, /* get values and attrs */
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    nCertCount = ldap_count_entries(pLotus, pSearchResult);
    if (nCertCount == 0)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CA_CERT_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCertArray->dwCount = nCertCount;

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CA_CERT) * nCertCount,
                    (PVOID*)&pCertArray->pCACerts);
    BAIL_ON_VMAFD_ERROR(dwError);

    for ( pCAResult = ldap_first_entry(pLotus, pSearchResult);
          pCAResult != NULL;
          pCAResult = ldap_next_entry(pLotus, pCAResult), ++certIndex)
    {
        // The following assumes there's only one certificate for each CA
        // object. In the future if the whole chain is store, we will
        // update accordingly.

        // Copy CN
        dwError = DirCliCopyQueryResultAttributeString(
                pLotus, pCAResult, pszAttrEntryDN, FALSE,
                (PSTR*)&pCertArray->pCACerts[certIndex].pCN);
        BAIL_ON_VMAFD_ERROR(dwError);

        // Copy subject DN
        dwError = DirCliCopyQueryResultAttributeString(
                pLotus, pCAResult, pszAttrCADN, FALSE,
                (PSTR*)&pCertArray->pCACerts[certIndex].pSubjectDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (bDetail)
        {
            // Copy certificate
            dwError = DirCliCopyQueryResultAttributeString(
                    pLotus, pCAResult, pszAttrCert, FALSE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pCert);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = DirCliCopyQueryResultAttributeString(
                    pLotus, pCAResult, pszAttrCrl, TRUE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pCrl);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *ppCACertificates = pCertArray;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszCACNFilter);

    if ( ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    if ( pCertArray )
    {
        VecsFreeCACertArray(pCertArray);
    }
    if (ppCACertificates)
    {
        *ppCACertificates = NULL;
    }

    goto cleanup;
}

static
DWORD
DirCliCopyQueryResultAttributeString(
    LDAP*        pLotus,
    LDAPMessage* pSearchResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
)
{
    DWORD   dwError = 0;
    struct berval** ppValues = NULL;
    PSTR   pszOut = NULL;

    ppValues = ldap_get_values_len(
                                pLotus,
                                pSearchResult,
                                pszAttribute);
    if (ppValues && ppValues[0])
    {
        dwError = VmAfdAllocateMemory(
                        sizeof(CHAR) * ppValues[0]->bv_len + 1,
                        (PVOID*)&pszOut);
        BAIL_ON_VMAFD_ERROR(dwError);
        memcpy(
            (PVOID) pszOut,
            (PVOID) ppValues[0]->bv_val,
            (size_t) ppValues[0]->bv_len);
    }
    else if (!bOptional)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszOut = pszOut;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pszOut);
    if (ppszOut)
    {
        *ppszOut = NULL;
    }
    goto cleanup;
}

static
DWORD
DirCliGetDefaultDomainName(
    LDAP* pLotus,
    PSTR* ppDomainName
    )
{
    DWORD dwError = 0;
    PCHAR pszDomainNameAttr = "rootdomainnamingcontext";
    PSTR pszDomainName = NULL;

    if (!pLotus || !ppDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliGetDSERootAttribute(
                    pLotus,
                    pszDomainNameAttr,
                    &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDomainName = pszDomainName;

cleanup:

    return dwError;

error :

    if (ppDomainName)
    {
        *ppDomainName = NULL;
    }

    goto cleanup;
}

static
BOOLEAN
_doLdapConnectRetry(
    DWORD dwError)
{
    BOOLEAN doRetry = FALSE;
    switch (dwError)
    {
        case ERROR_INVALID_PARAMETER:
        case VMDIR_ERROR_INVALID_PARAMETER:
        case VMDIR_ERROR_ENTRY_NOT_FOUND:
        case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
            doRetry = FALSE;
            break;
        case VMDIR_ERROR_SERVER_DOWN:
        default:
            doRetry = TRUE;
            break;
    }
    return doRetry;
}
