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

static
DWORD
DirCliReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    );

static
DWORD
DirCliParsePrincipal(
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    );

static
DWORD
DirCliParsePrincipalEx(
    PVMAF_CFG_CONNECTION pConnection,
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    );

static
DWORD
DirCliInitLdapConnection(
    PCSTR pszLogin,
    PCSTR pszPassword,
    LDAP** ppLd
    );

static
DWORD
DirCliBytesToHexString(
    PUCHAR  pData,
    int     length,
    PSTR*   ppszHexString
    );

static
DWORD
DirCliGenerateCACNForLdap(
    X509*   pCertificate,
    PSTR*   ppszCACN
    );

static
DWORD
DirCliKeyIdToHexString(
    ASN1_OCTET_STRING*  pIn,
    PSTR*               ppszOut
);

static
DWORD
DirCliGetCrlAuthKeyIdHexString(
    X509_CRL*   pCrl,
    PSTR*       ppszAid
    );

static
DWORD
DirCliGetDCName(
    PSTR*      ppszDCName
    );

static
VOID
DirCliPrintDCInfo(
    PVMDIR_DC_INFO* ppDC,
    DWORD dwNumDC
    );

static
VOID
DirCliPrintComputers(
    PSTR* ppszComputers,
    DWORD dwNumComputers
    );

DWORD
DirCliCreateServiceA(
    PCSTR pszServiceName,
    PCSTR pszCertPath,
    PCSTR pszSsoGroups,
    BOOL  bTrustedUserGroup,
    SSO_ADMIN_ROLE ssoAdminRole,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PDIR_CLI_CERT pCert = NULL;
    PSTR  pszServiceDN = NULL;
    PSTR  pszServiceNameOther = NULL;
    PCSTR pszGroupDelimiter = ",";
    PCSTR pszGroupName = NULL;
    PSTR  pszTokenContext = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszServiceName) ||
        IsNullOrEmptyString(pszCertPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliCreateCert(pszCertPath, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapFindFingerPrintMatch(
                    pLd,
                    pCert->pszFingerPrint,
                    pszDomain,
                    &pszServiceNameOther);
    if (dwError == ERROR_SUCCESS)
    {
        fprintf(
            stdout,
            "Error: Attempt to reuse certificate currently used by service [%s].\n",
            pszServiceNameOther);

        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwError == ERROR_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapCreateService(
                    pLd,
                    pszServiceName,
                    pszDomain,
                    pCert,
                    &pszServiceDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapAddServiceToGroup(
                    pLd,
                    pszServiceDN,
                    SOLUTION_USERS_GROUP_NAME,
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszSsoGroups)
    {
        pszGroupName = VmAfdStringTokA(
                            (PSTR)pszSsoGroups,
                            pszGroupDelimiter,
                            &pszTokenContext);
        while(pszGroupName != NULL)
        {
            dwError = DirCliLdapAddServiceToGroup(
                            pLd,
                            pszServiceDN,
                            pszGroupName,
                            pszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);

            pszGroupName = VmAfdStringTokA(
                                NULL,
                                pszGroupDelimiter,
                                &pszTokenContext);
        }
    }

    if (bTrustedUserGroup)
    {
        dwError = DirCliLdapAddServiceToGroup(
                        pLd,
                        pszServiceDN,
                        TRUSTED_USERS_GROUP_NAME,
                        pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ssoAdminRole == SSO_ROLE_ADMINISTRATOR)
    {
        dwError = DirCliLdapAddServiceToBuiltinGroup(
                        pLd,
                        pszServiceDN,
                        BUILTIN_ADMINISTRATORS_GROUP_NAME,
                        pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (ssoAdminRole == SSO_ROLE_USER)
    {
        dwError = DirCliLdapAddServiceToBuiltinGroup(
                        pLd,
                        pszServiceDN,
                        BUILTIN_USERS_GROUP_NAME,
                        pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszServiceDN);
    VMAFD_SAFE_FREE_MEMORY(pszServiceNameOther);
    if (pCert)
    {
        DirCliFreeCert(pCert);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliUpdateServiceA(
    PCSTR pszServiceName,
    PCSTR pszCertPath,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszServiceNameOther = NULL;
    PDIR_CLI_CERT pCert = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszServiceName) ||
        IsNullOrEmptyString(pszCertPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliCreateCert(pszCertPath, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapFindFingerPrintMatch(
                    pLd,
                    pCert->pszFingerPrint,
                    pszDomain,
                    &pszServiceNameOther);
    if (dwError == ERROR_SUCCESS)
    {
        fprintf(
            stdout,
            "Error: The same certificate cannot be used by multiple services [%s].\n",
            pszServiceNameOther);

        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (dwError == ERROR_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapUpdateService(
                    pLd,
                    pszServiceName,
                    pszDomain,
                    pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszServiceNameOther);
    if (pCert)
    {
        DirCliFreeCert(pCert);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliListServiceA(
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    LDAP* pLd = NULL;
    PDIR_CLI_ENUM_SVC_CONTEXT pEnumContext = NULL;
    PSTR* ppszAccounts = NULL;
    DWORD dwCount = 0;
    DWORD iAcct = 0;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapBeginEnumServices(pLd, pszDomain, 256, &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        DWORD idx = 0;

        if (ppszAccounts)
        {
            VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
            ppszAccounts = NULL;
        }

        dwError = DirCliLdapEnumServices(pEnumContext, &ppszAccounts, &dwCount);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; idx < dwCount; idx++)
        {
            fprintf(stdout, "%d. %s\n", ++iAcct, ppszAccounts[idx]);
        }

    } while (dwCount > 0);

cleanup:

    if (pEnumContext)
    {
        DirCliLdapEndEnumServices(pEnumContext);
    }
    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    if (ppszAccounts)
    {
        VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
    }
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliDeleteServiceA(
    PCSTR pszServiceName,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszServiceName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapDeleteService(
                    pLd,
                    pszServiceName,
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliCreateGroupA(
    PCSTR pszGroupName,
    PCSTR pszDescription,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszGroupDN = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszGroupName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapCreateGroup(
                    pLd,
                    pszGroupName,
                    pszDescription,
                    pszDomain,
                    &pszGroupDN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszGroupDN);

    goto cleanup;
}

DWORD
DirCliAddGroupMemberA(
    PCSTR pszGroupName,
    PCSTR pszAcctName,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszGroupName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapAddGroupMember(
                    pLd,
                    pszGroupName,
                    pszAcctName,
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(
        stdout,
        "Account [%s] added to group [%s]\n",
        VMAFD_SAFE_STRING(pszAcctName),
        VMAFD_SAFE_STRING(pszGroupName));

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    return dwError;

error:

    fprintf(
        stdout,
        "Failed to add Account [%s] to group [%s] (%d)\n",
        VMAFD_SAFE_STRING(pszAcctName),
        VMAFD_SAFE_STRING(pszGroupName),
        dwError);

    goto cleanup;
}

DWORD
DirCliRemoveGroupMemberA(
    PCSTR pszGroupName,
    PCSTR pszAcctName,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszGroupName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapRemoveGroupMember(
                    pLd,
                    pszGroupName,
                    pszAcctName,
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(
        stdout,
        "Account [%s] removed from group [%s]\n",
        VMAFD_SAFE_STRING(pszAcctName),
        VMAFD_SAFE_STRING(pszGroupName));

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    fprintf(
        stdout,
        "Failed to remove Account [%s] from group [%s] (%d)\n",
        VMAFD_SAFE_STRING(pszAcctName),
        VMAFD_SAFE_STRING(pszGroupName),
        dwError);

    goto cleanup;
}

DWORD
DirCliListGroupA(
    PCSTR pszGroup,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    LDAP* pLd = NULL;
    PDIR_CLI_ENUM_GROUP_CONTEXT pEnumContext = NULL;
    PSTR* ppszAccounts = NULL;
    DWORD dwCount = 0;
    DWORD dwMembers = 0;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapBeginEnumMembers(
                    pLd,
                    pszGroup,
                    pszDomain,
                    256,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        DWORD idx = 0;

        if (ppszAccounts)
        {
            VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
            ppszAccounts = NULL;
        }

        dwError = DirCliLdapEnumMembers(pEnumContext, &ppszAccounts, &dwCount);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; idx < dwCount; idx++)
        {
            fprintf(stdout, "%s\n", ppszAccounts[idx]);
            dwMembers++;
        }

    } while (dwCount > 0);

    if (dwMembers == 0)
    {
        fprintf(stderr, "group [%s] has no members\n", pszGroup);
    }

cleanup:

    if (pEnumContext)
    {
        DirCliLdapEndEnumMembers(pEnumContext);
    }
    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    if (ppszAccounts)
    {
        VmAfdFreeStringArrayCountA(ppszAccounts, dwCount);
    }
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliKeyIdToHexString(
    ASN1_OCTET_STRING*  pIn,
    PSTR*               ppszOut)
{
    DWORD dwError = ERROR_SUCCESS;
    char* pszOut = NULL;

    if (!pIn || !ppszOut)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliBytesToHexString((PUCHAR)pIn->data, pIn->length, &pszOut);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszOut = pszOut;

error:
    return dwError;
}

DWORD
DirCliPublishCertA(
    PCSTR pszCertFile,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword,
    BOOL  bPublishChain
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszCertificate = NULL;
    PSTR pszConfigurationDN = NULL;
    PSTR pszCAContainerDN = NULL;
    PSTR pszCADN = NULL;
    PSTR pszCACN = NULL;
    PSTR pszCADNTemp = NULL;
    PSTR pszCAIssuerDN = NULL;
    PSTR pszFoundCADN = NULL;
    X509_NAME*  pCertName = NULL;
    STACK_OF(X509) *skX509certs = NULL;
    DWORD dwNumCerts = 0;
    DWORD dwIndex = 0;
    X509*       pCert = NULL;
    LDAP*       pLd = NULL;
    ATTR_SEARCH_RESULT attrSearchResult = ATTR_NOT_FOUND;

    dwError = DirCliInitLdapConnection(pszLogin, pszPassword, &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMFiletoX509Stack(pszCertFile, &skX509certs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumCerts = sk_X509_num(skX509certs);

    if (!dwNumCerts)
    {
        fprintf(
                stderr,
                "The file [%s] contains no certificates\n",
                pszCertFile
               );
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (dwNumCerts > 1 && !bPublishChain)
    {
        fprintf(
                stderr,
                "The file [%s] contains more than 1 certificate\n"
                "If you want to publish a chain of certificates, use "
                "the command \"trustedcert publish\" with the --chain flag.\n",
                pszCertFile
               );
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; dwIndex < dwNumCerts; dwIndex++)
    {

        VMAFD_SAFE_FREE_MEMORY(pszCACN);
        VMAFD_SAFE_FREE_MEMORY(pszCADN);
        VMAFD_SAFE_FREE_MEMORY(pszFoundCADN);
        VMAFD_SAFE_FREE_MEMORY(pszCAContainerDN);
        VMAFD_SAFE_FREE_MEMORY(pszConfigurationDN);
        VMAFD_SAFE_FREE_MEMORY(pszCAIssuerDN);
        VMAFD_SAFE_FREE_MEMORY(pszCertificate);

        pCert = sk_X509_value(skX509certs, dwIndex);
        if (!pCert)
        {
            dwError = ERROR_INVALID_DATA;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pCertName = X509_get_subject_name(pCert);
        if (!pCertName)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = DirCliGetX509Name(pCertName, XN_FLAG_COMPAT, &pszCAIssuerDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!pszCAIssuerDN)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = DirCliGenerateCACNForLdap(pCert, &pszCACN);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (IsNullOrEmptyString(pszCACN))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VecsCertToPEM(pCert, &pszCertificate);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = DirCliLdapGetDSERootAttribute(
            pLd,
            "configurationNamingContext",
            &pszConfigurationDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAVsnprintf(
            &pszCAContainerDN,
            "CN=%s,%s",
            CA_CONTAINER_NAME,
            pszConfigurationDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAVsnprintf(
            &pszCADN,
            "CN=%s,%s",
            pszCACN,
            pszCAContainerDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = DirCliLdapCheckCAObject(pLd, pszCAContainerDN,
            pszCADN, pszCAIssuerDN, &pszFoundCADN);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!pszFoundCADN)
        {
            dwError = DirCliLdapCreateCAObject(pLd, pszCACN, pszCADN);
            BAIL_ON_VMAFD_ERROR(dwError);
            pszCADNTemp = pszCADN;
        }
        else
        {
            continue;
        }

        dwError = DirCliLdapCheckAttribute(pLd, pszCADNTemp,
            ATTR_NAME_CA_CERTIFICATE, pszCertificate, &attrSearchResult);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (attrSearchResult != ATTR_MATCH)
        {
            dwError = DirCliLdapUpdateAttribute(pLd, pszCADNTemp,
                ATTR_NAME_CA_CERTIFICATE, pszCertificate,
                (attrSearchResult == ATTR_NOT_FOUND));
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = DirCliLdapCheckAttribute(pLd, pszCADNTemp,
            ATTR_NAME_CA_CERTIFICATE_DN, pszCAIssuerDN, &attrSearchResult);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (attrSearchResult != ATTR_MATCH)
        {
            dwError = DirCliLdapUpdateAttribute(pLd, pszCADNTemp,
                ATTR_NAME_CA_CERTIFICATE_DN, pszCAIssuerDN,
                (attrSearchResult == ATTR_NOT_FOUND));
            BAIL_ON_VMAFD_ERROR(dwError);
        }

    }

    if (pszCrlFile)
    {
        dwError = DirCliPublishCrlA(pLd, pszCrlFile, pszLogin, pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCACN);
    VMAFD_SAFE_FREE_MEMORY(pszCADN);
    VMAFD_SAFE_FREE_MEMORY(pszFoundCADN);
    VMAFD_SAFE_FREE_MEMORY(pszCAContainerDN);
    VMAFD_SAFE_FREE_MEMORY(pszConfigurationDN);
    VMAFD_SAFE_FREE_MEMORY(pszCAIssuerDN);
    VMAFD_SAFE_FREE_MEMORY(pszCertificate);

    if (pCert)
    {
        pCert = NULL;
    }

    if (skX509certs)
    {
        sk_X509_pop_free(skX509certs, X509_free);
    }

    if (pLd)
    {
        DirCliLdapClose(pLd);
        pLd = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliPublishCrlA(
    LDAP* pLdap,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR pszCrl = NULL;
    PSTR pszConfigurationDN = NULL;
    PSTR pszCAContainerDN = NULL;
    PSTR pszCADN = NULL;
    PSTR pszCrlAuthorityKeyId = NULL;
    PSTR pszCADNTemp = NULL;
    PSTR pszCAIssuerDN = NULL;
    PSTR pszFoundCADN = NULL;
    LDAP*           pLd = NULL;
    LDAP*           pNewLd = NULL;
    X509_CRL*       pCrl = NULL;
    X509_NAME*      pIssuer = NULL;
    ATTR_SEARCH_RESULT attrSearchResult = ATTR_NOT_FOUND;

    if (pLdap)
    {
        pLd = pLdap;
    }
    else
    {
        dwError = DirCliInitLdapConnection(pszLogin, pszPassword, &pNewLd);
        BAIL_ON_VMAFD_ERROR(dwError);
        pLd = pNewLd;
    }

    dwError = DirCliLdapGetDSERootAttribute(
            pLd,
            "configurationNamingContext",
            &pszConfigurationDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
            &pszCAContainerDN,
            "CN=%s,%s",
            CA_CONTAINER_NAME,
            pszConfigurationDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMFiletoX509Crl(pszCrlFile, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Check if the CRL has auth key id.
    // From RFC 3280 4.2.1.1:
    //    The authority key identifier extension provides a means of
    //   identifying the public key corresponding to the private key used to
    //   sign a certificate.  This extension is used where an issuer has
    //   multiple signing keys (either due to multiple concurrent key pairs or
    //   due to changeover).
    dwError = DirCliGetCrlAuthKeyIdHexString(pCrl, &pszCrlAuthorityKeyId);
    if (dwError == ERROR_SUCCESS)
    {
        if (IsNullOrEmptyString(pszCrlAuthorityKeyId))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdAllocateStringAVsnprintf(
                &pszCADN,
                "CN=%s,%s",
                pszCrlAuthorityKeyId,
                pszCAContainerDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pIssuer = X509_CRL_get_issuer(pCrl); // Don't free
    dwError = DirCliGetX509Name(pIssuer, XN_FLAG_COMPAT, &pszCAIssuerDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszCAIssuerDN == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliLdapCheckCAObject(pLd, pszCAContainerDN, pszCADN,
            pszCAIssuerDN, &pszFoundCADN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszFoundCADN)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        pszCADNTemp = pszFoundCADN;
    }

    dwError = VecsX509CRLToPEM(pCrl, &pszCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapCheckAttribute(pLd, pszCADNTemp, ATTR_NAME_CA_CRL,
                                        pszCrl, &attrSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (attrSearchResult != ATTR_MATCH)
    {
        // If there's an existing CRL but doesn't match the new one byte-by-byte
        // it's going to be replaced.
        dwError = DirCliLdapUpdateCRL(pLd, pszCADNTemp, pszCrl,
                    (attrSearchResult == ATTR_NOT_FOUND));
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCADN);
    VMAFD_SAFE_FREE_MEMORY(pszCAContainerDN);
    VMAFD_SAFE_FREE_MEMORY(pszConfigurationDN);
    VMAFD_SAFE_FREE_MEMORY(pszCrlAuthorityKeyId);
    VMAFD_SAFE_FREE_MEMORY(pszCAIssuerDN);
    VMAFD_SAFE_FREE_MEMORY(pszFoundCADN);
    VMAFD_SAFE_FREE_MEMORY(pszCrl);

    if (pCrl)
    {
        X509_CRL_free(pCrl);
        pCrl = NULL;
    }

    if (pNewLd)
    {
        DirCliLdapClose(pNewLd);
        pNewLd = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliUnpublishCertA(
    PCSTR pszCertFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszCertificate = NULL;
    PSTR pszConfigurationDN = NULL;
    PSTR pszCAContainerDN = NULL;
    PSTR pszCADN = NULL;
    PSTR pszCACN = NULL;
    PSTR pszCAIssuerDN = NULL;
    PSTR pszFoundCADN = NULL;
    X509_NAME*  pCertName = NULL;
    X509*       pCert = NULL;
    STACK_OF(X509) *skX509certs = NULL;
    DWORD dwNumCerts = 0;
    LDAP*       pLd = NULL;

    if (!pszCertFile)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliInitLdapConnection(pszLogin, pszPassword, &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMFiletoX509Stack(pszCertFile, &skX509certs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwNumCerts = sk_X509_num(skX509certs);
    if (dwNumCerts != 1)
    {
        fprintf(
                stderr,
                "The file [%s] contains more than 1 certificate\n",
                pszCertFile
               );
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pCert = sk_X509_value(skX509certs, 0);

    pCertName = X509_get_subject_name(pCert);
    if (!pCertName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliGetX509Name(pCertName, XN_FLAG_COMPAT, &pszCAIssuerDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszCAIssuerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliGenerateCACNForLdap(pCert, &pszCACN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pszCACN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliCertToPEM(pCert, &pszCertificate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapGetDSERootAttribute(
            pLd,
            "configurationNamingContext",
            &pszConfigurationDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
            &pszCAContainerDN,
            "CN=%s,%s",
            CA_CONTAINER_NAME,
            pszConfigurationDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
            &pszCADN,
            "CN=%s,%s",
            pszCACN,
            pszCAContainerDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapCheckCAObject(pLd, pszCAContainerDN,
            pszCADN, pszCAIssuerDN, &pszFoundCADN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszFoundCADN)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliLdapDeleteCAObject(pLd, pszFoundCADN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCACN);
    VMAFD_SAFE_FREE_MEMORY(pszCADN);
    VMAFD_SAFE_FREE_MEMORY(pszFoundCADN);
    VMAFD_SAFE_FREE_MEMORY(pszCAContainerDN);
    VMAFD_SAFE_FREE_MEMORY(pszConfigurationDN);
    VMAFD_SAFE_FREE_MEMORY(pszCAIssuerDN);
    VMAFD_SAFE_FREE_MEMORY(pszCertificate);

    if (skX509certs)
    {
        sk_X509_pop_free(skX509certs, X509_free);
    }

    if (pLd)
    {
        DirCliLdapClose(pLd);
        pLd = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliGetCertificationAuthoritiesA(
    PCSTR pszCACN,
    PCSTR pszCertFile,
    PCSTR pszCrlFile,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD       dwError = ERROR_SUCCESS;
    LDAP*       pLd = NULL;
    PVMAFD_CA_CERT_ARRAY pCACertArray = NULL;

    if (!pszCACN || (!pszCertFile && !pszCrlFile))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliInitLdapConnection(pszLogin, pszPassword, &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliQueryCACerts(pLd, pszCACN, TRUE, &pCACertArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pCACertArray || pCACertArray->dwCount == 0)
    {
        dwError = ERROR_INVALID_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (pCACertArray->dwCount > 1)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSaveCACertificateAndCrlToFile(
            &pCACertArray->pCACerts[0], pszCertFile, pszCrlFile);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VecsFreeCACertArray(pCACertArray);

    if (pLd)
    {
        DirCliLdapClose(pLd);
        pLd = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliListCertificationAuthoritiesA(
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD       dwError = ERROR_SUCCESS;
    LDAP*       pLd = NULL;
    PVMAFD_CA_CERT_ARRAY pCACertArray = NULL;

    dwError = DirCliInitLdapConnection(pszLogin, pszPassword, &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliQueryCACerts(pLd, NULL, TRUE, &pCACertArray);
    if (dwError == ERROR_NOT_FOUND)
    {
        // It's ok to have an empty store.
        dwError = ERROR_SUCCESS;
        goto cleanup;
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdPrintCACertificates(pCACertArray);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VecsFreeCACertArray(pCACertArray);

    if (pLd)
    {
        DirCliLdapClose(pLd);
        pLd = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliListNodesA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    )
{
    DWORD     dwError = 0;
    PSTR      pszDomain = NULL;
    PSTR      pszPassword1 = NULL;
    PSTR      pszUser = NULL;
    PSTR      pszDCName = NULL;
    PCSTR     pszDCNameLocal = pszHostName;
    PCSTR     pszPasswordLocal = pszPassword;
    PCSTR     pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PVMDIR_DC_INFO* ppDC = NULL;
    DWORD     dwNumDC = 0;
    PSTR*     ppszComputers = NULL;
    DWORD     dwNumComputers = 0;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain,
                         NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    if(!pszHostName)
    {
        dwError = DirCliGetDCName(&pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszDCNameLocal = pszDCName;
    }

    dwError = VmDirGetDCInfo(
                    pszDCNameLocal,
                    pszUser,
                    pszPasswordLocal,
                    &ppDC,
                    &dwNumDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    DirCliPrintDCInfo(ppDC, dwNumDC);

    dwError = VmDirGetComputers( pszDCNameLocal,
                    pszUser,
                    pszPasswordLocal,
                    &ppszComputers,
                    &dwNumComputers);
    BAIL_ON_VMAFD_ERROR(dwError);

    DirCliPrintComputers(ppszComputers, dwNumComputers);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    VmDirFreeDCInfoArray(ppDC, dwNumDC);
    VmDirFreeStringArray(ppszComputers, dwNumComputers);

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliGeneratePassword(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszLogin1 = NULL;
    PSTR  pszDC = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PBYTE pPasswordNew = NULL;
    PSTR  pszPasswordNew = NULL;
    DWORD dwLength = 0;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                    &pszLogin1,
                    "%s@%s",
                    pszUser,
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirGeneratePassword(
                   pszDC,
                   pszLogin1,
                   pszPasswordLocal,
                   &pPasswordNew,
                   &dwLength);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(dwLength+1, (PVOID*)&pszPasswordNew);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszPasswordNew, pPasswordNew, dwLength);

    *ppszPassword = pszPasswordNew;

cleanup:

    if (pPasswordNew)
    {
        VmDirFreeMemory(pPasswordNew);
    }
    VMAFD_SAFE_FREE_STRINGA(pszDC);
    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszLogin1);

    return dwError;

error:

    *ppszPassword = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszPasswordNew);

    goto cleanup;
}

DWORD
DirCliChangePassword(
    PCSTR pszAccount,
    PCSTR pszPassword,
    PCSTR pszPasswordNew
    )
{
    DWORD dwError = 0;
    PCSTR pszPassword_cur_local = pszPassword;
    PCSTR pszPassword_new_local = pszPasswordNew;
    PSTR  pszPassword_cur = NULL;
    PSTR  pszPassword_new = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszUserDN = NULL;
    PSTR  pszUPN = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszAccount))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszAccount, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(
                        pszUser,
                        pszDomain,
                        "Current password",
                        &pszPassword_cur);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPassword_cur_local = pszPassword_cur;
    }

    if (!pszPasswordNew)
    {
        dwError = DirCliReadPassword(
                        pszUser,
                        pszDomain,
                        "New password",
                        &pszPassword_new);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPassword_new_local = pszPassword_new;
    }

    dwError = VmAfdAllocateStringPrintf(&pszUPN, "%s@%s", pszUser, pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliInitLdapConnection(
                    pszUPN,
                    pszPassword_cur_local,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapFindUser(
                    pLd,
                    pszUser,
                    pszDomain,
                    &pszUserDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapChangePassword(
                    pLd,
                    pszUserDN,
                    pszPassword_cur_local,
                    pszPassword_new_local);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszUserDN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword_cur);
    VMAFD_SAFE_FREE_MEMORY(pszPassword_new);
    VMAFD_SAFE_FREE_MEMORY(pszUPN);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliResetPassword(
    PCSTR pszAccount,
    PCSTR pszPasswordNew,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PCSTR pszPasswordLocal = pszPassword;
    PCSTR pszPasswordNewLocal = pszPasswordNew;
    PSTR  pszPassword1 = NULL;
    PSTR  pszPasswordNew1 = NULL;
    PSTR  pszAdminUser = NULL;
    PSTR  pszAdminDomain = NULL;
    PSTR  pszAdminUPN = NULL;
    PSTR  pszCandidateUser = NULL;
    PSTR  pszCandidateDomain = NULL;
    PSTR  pszUserDN = NULL;
    PSTR  pszPrompt = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszAccount))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(
                    pszLoginLocal,
                    &pszAdminUser,
                    &pszAdminDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliParsePrincipal(
                    pszAccount,
                    &pszCandidateUser,
                    &pszCandidateDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszAdminDomain &&
        pszCandidateDomain &&
        VmAfdStringCompareA(pszAdminDomain, pszCandidateDomain, FALSE))
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(
                        pszAdminUser,
                        pszAdminDomain,
                        NULL,
                        &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    if (!pszPasswordNew)
    {
        dwError = VmAfdAllocateStringPrintf(
                        &pszPrompt,
                        "Enter new password for [%s@%s]",
                        pszCandidateUser,
                        pszAdminDomain);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = DirCliReadPassword(
                        pszCandidateUser,
                        pszAdminDomain,   /* locate user in Admin's domain */
                        pszPrompt,
                        &pszPasswordNew1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordNewLocal = pszPasswordNew1;
    }

    dwError = VmAfdAllocateStringPrintf(
                        &pszAdminUPN,
                        "%s@%s",
                        pszAdminUser,
                        pszAdminDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliInitLdapConnection(
                    pszAdminUPN,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapFindUser(
                    pLd,
                    pszCandidateUser,
                    pszAdminDomain,
                    &pszUserDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapResetPassword(pLd, pszUserDN, pszPasswordNewLocal);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszUserDN);
    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPrompt);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszPasswordNew1);
    VMAFD_SAFE_FREE_MEMORY(pszAdminUser);
    VMAFD_SAFE_FREE_MEMORY(pszAdminDomain);
    VMAFD_SAFE_FREE_MEMORY(pszCandidateUser);
    VMAFD_SAFE_FREE_MEMORY(pszCandidateDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliCreateUserA(
    PCSTR pszAcctName,
    PCSTR pszFirstname,
    PCSTR pszLastname,
    PCSTR pszUserPassword,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PCSTR pszUserPasswordLocal = pszUserPassword;
    PSTR  pszUserPassword1 = NULL;
    PSTR  pszDC = NULL;
    DWORD dwFlags = 0;
    PVOID pReserved = NULL;
    PVMDIR_SERVER_CONTEXT pVmDirCtx = NULL;
    VMDIR_USER_CREATE_PARAMS_A createparams = {0};

    if (IsNullOrEmptyString(pszAcctName))
    {
        fprintf(stderr, "Error: User account name was not provided\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pszFirstname))
    {
        fprintf(stderr, "Error: User's first name was not provided\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (IsNullOrEmptyString(pszLastname))
    {
        fprintf(stderr, "Error: User's last name was not provided\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    if (!pszUserPassword)
    {
        dwError = DirCliReadPassword(
                        pszAcctName,
                        pszDomain,
                        NULL,
                        &pszUserPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszUserPasswordLocal = pszUserPassword1;
    }

    dwError = DirCliGetDCName(&pszDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirOpenServerA(
                pszDC,
                pszUser,
                pszDomain,
                pszPasswordLocal,
                dwFlags,
                pReserved,
                &pVmDirCtx);
    BAIL_ON_VMAFD_ERROR(dwError);

    createparams.pszName      = NULL;
    createparams.pszAccount   = (PSTR)pszAcctName;
    createparams.pszUPN       = NULL;
    createparams.pszFirstname = (PSTR)pszFirstname;
    createparams.pszLastname  = (PSTR)pszLastname;
    createparams.pszPassword  = (PSTR)pszUserPasswordLocal;

    dwError = VmDirCreateUserA(pVmDirCtx, &createparams);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pVmDirCtx)
    {
        VmDirCloseServer(pVmDirCtx);
    }

    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszPassword1);
    VMAFD_SAFE_FREE_STRINGA(pszUserPassword1);
    VMAFD_SAFE_FREE_STRINGA(pszDC);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliDeleteUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszAccount))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliInitLdapConnection(
                    pszLoginLocal,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapDeleteUser(pLd, pszAccount, pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszPassword1);

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliFindByNameUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword,
    USER_INFO_LEVEL userInfoLevel
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszPassword1 = NULL;
    PSTR  pszPasswordExpTime = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    LDAP* pLd = NULL;
    PDIR_CLI_USER_INFO pUserInfo = NULL;

    if (IsNullOrEmptyString(pszAccount))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliInitLdapConnection(
                    pszLoginLocal,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapGetUserAttr(
                pLd,
                pszAccount,
                pszDomain,
                userInfoLevel,
                &pUserInfo
                );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (userInfoLevel == USER_INFO_LEVEL_DEFAULT)
    {
        fprintf(stdout, "Account: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_0->pszAccount));
        fprintf(stdout, "UPN: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_0->pszUPN));
    }
    else if (userInfoLevel == USER_INFO_LEVEL_ONE)
    {
        fprintf(stdout, "Account: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_1->pszAccount));
        fprintf(stdout, "UPN: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_1->pszUPN));
        fprintf(stdout, "First Name: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_1->pszFirstName));
        fprintf(stdout, "Last Name: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_1->pszLastName));
    }
    else if (userInfoLevel == USER_INFO_LEVEL_TWO)
    {
        fprintf(stdout, "Account: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_2->pszAccount));
        fprintf(stdout, "UPN: %s\n",
            VMAFD_SAFE_STRING(pUserInfo->info.pUserInfo_2->pszUPN));

        if (DirCliIsFlagSet(pUserInfo->info.pUserInfo_2->dwUserAccCtrl,
                USER_ACC_CTRL_DISABLE_FLAG))
        {
            fprintf(stdout, "Account disabled: %s\n",
                LDAP_BOOLEAN_SYNTAX_TRUE_STR);
        }
        else
        {
            fprintf(stdout, "Account disabled: %s\n",
                LDAP_BOOLEAN_SYNTAX_FALSE_STR);
        }

        if (DirCliIsFlagSet(pUserInfo->info.pUserInfo_2->dwUserAccCtrl,
                    USER_ACC_CTRL_LOCKOUT_FLAG))
        {
            fprintf(stdout, "Account locked: %s\n",
                LDAP_BOOLEAN_SYNTAX_TRUE_STR);
        }
        else
        {
            fprintf(stdout, "Account locked: %s\n",
                LDAP_BOOLEAN_SYNTAX_FALSE_STR);;
        }

        if (pUserInfo->info.pUserInfo_2->bIsPwdNeverExpired)
        {
            fprintf(stdout, "Password never expires: %s\n",
                LDAP_BOOLEAN_SYNTAX_TRUE_STR);
        }
        else
        {
            fprintf(stdout, "Password never expires: %s\n",
                LDAP_BOOLEAN_SYNTAX_FALSE_STR);
        }

        if (DirCliIsFlagSet(pUserInfo->info.pUserInfo_2->dwUserAccCtrl,
                USER_ACC_CTRL_PASSWORD_EXPIRE_FLAG))
        {
            fprintf(stdout, "Password expired: %s\n",
                LDAP_BOOLEAN_SYNTAX_TRUE_STR);
        }
        else
        {
            fprintf(stdout, "Password expired: %s\n",
                LDAP_BOOLEAN_SYNTAX_FALSE_STR);
        }

        if (pUserInfo->info.pUserInfo_2->pszPwdLastSet &&
            pUserInfo->info.pUserInfo_2->pszPwdExpTime &&
            pUserInfo->info.pUserInfo_2->bIsPwdNeverExpired == FALSE )
        {
            dwError = DirCliUserGetCurrentPwdExpTime(
                    pUserInfo,
                    &pszPasswordExpTime);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout, "Password expiry: %s\n",
                VMAFD_SAFE_STRING(pszPasswordExpTime));
        }
        else
        {
            fprintf(stdout, "Password expiry: N/A\n");
        }

    }

cleanup:

    if (pUserInfo)
    {
        DirCliFreeUserInfo(pUserInfo);
    }
    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszPassword1);
    VMAFD_SAFE_FREE_STRINGA(pszPasswordExpTime);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliModifyAttributeUserA(
    PCSTR pszAccount,
    PCSTR pszLogin,
    PCSTR pszPassword,
    USER_MODIFY_OPT userModifyOpt
    )
{
    DWORD dwError = 0;
    PSTR pszAccountDN = NULL;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    LDAP* pLd = NULL;
    ATTR_SEARCH_RESULT attrStatus = ATTR_NOT_FOUND;

    if (IsNullOrEmptyString(pszAccount))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (userModifyOpt == USER_MODIFY_NO_OPT)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliInitLdapConnection(
                    pszLoginLocal,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapFindUser(pLd, pszAccount, pszDomain, &pszAccountDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (userModifyOpt & USER_MODIFY_PWD_NEVER_EXP)
    {
        dwError = DirCliLdapUserModifyAttrPwdNeverExp(
                    pLd,
                    pszAccountDN,
                    pszDomain,
                    LDAP_BOOLEAN_SYNTAX_TRUE_STR,
                    &attrStatus);

        if (attrStatus == ATTR_MATCH)
        {
            fprintf(
                stdout,
                "Password is currently set to never expire for [%s].\n",
                VMAFD_SAFE_STRING(pszAccount));
        }
        else
        {
            fprintf(
                stdout,
                "Password set to never expire for [%s].\n",
                VMAFD_SAFE_STRING(pszAccount));
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (userModifyOpt & USER_MODIFY_PWD_EXPIRE)
    {
        dwError = DirCliLdapUserModifyAttrPwdNeverExp(
                    pLd,
                    pszAccountDN,
                    pszDomain,
                    LDAP_BOOLEAN_SYNTAX_FALSE_STR,
                    &attrStatus);

        if (attrStatus == ATTR_MATCH)
        {
            fprintf(
                stdout,
                "Password is currently set to expire for [%s].\n",
                VMAFD_SAFE_STRING(pszAccount));
        }
        else
        {
            fprintf(
                stdout,
                "Password set to expire for [%s].\n",
                VMAFD_SAFE_STRING(pszAccount));
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }


cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszPassword1);
    VMAFD_SAFE_FREE_STRINGA(pszAccountDN);

    return dwError;

error:

    goto cleanup;
}

#ifndef _WIN32

static
DWORD
DirCliReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    struct termios orig, nonecho;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;

    memset(szPassword, 0, sizeof(szPassword));

    if (IsNullOrEmptyString(pszPrompt))
    {
        fprintf(stdout, "Enter password for %s@%s: ", pszUser, pszDomain);
    }
    else
    {
        fprintf(stdout, "%s:", pszPrompt);
    }
    fflush(stdout);

    tcgetattr(0, &orig); // get current settings
    memcpy(&nonecho, &orig, sizeof(struct termios)); // copy settings
    nonecho.c_lflag &= ~(ECHO); // don't echo password characters
    tcsetattr(0, TCSANOW, &nonecho); // set current settings to not echo

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword); iChar++)
    {
        ssize_t nRead = 0;
        CHAR ch;

        if ((nRead = read(STDIN_FILENO, &ch, 1)) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (nRead == 0 || ch == '\n')
        {
            fprintf(stdout, "\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(szPassword))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &orig);

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}

#else

static
DWORD
DirCliReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;
    BOOLEAN bConsole = FALSE;

    memset(szPassword, 0, sizeof(szPassword));

    if (IsNullOrEmptyString(pszPrompt))
    {
        fprintf(stdout, "Enter password for %s@%s: ", pszUser, pszDomain);
    }
    else
    {
        fprintf(stdout, "%s:", pszPrompt);
    }
    fflush(stdout);

    bConsole = _isatty(0); // Is stdin console?

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword); iChar++)
    {
        CHAR ch = bConsole ? _getch() : getchar();

        if (ch == EOF || ch == '\r' || ch == '\n')
        {
            fprintf(stdout, "\r\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(szPassword))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}

#endif

static
DWORD
VmAfdGetDomainNameFromRegistryA(
    PVMAF_CFG_CONNECTION pConnection,
    PSTR *ppszDomain
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pSubKey = NULL;
    PSTR pszDomain = NULL;

    dwError = VmAfConfigOpenRootKey(
                pConnection,
                "HKEY_LOCAL_MACHINE",
                0,
                KEY_READ,
                &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                pConnection,
                pRootKey,
                VMAFD_CONFIG_PARAMETER_KEY_PATH,
                0,
                KEY_READ,
                &pSubKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                pSubKey,
                NULL,
                VMAFD_REG_KEY_DOMAIN_NAME,
                &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDomain = pszDomain;
    pszDomain = NULL;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszDomain);

    if (pSubKey)
    {
        VmAfConfigCloseKey(pSubKey);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
DirCliParsePrincipalEx(
    PVMAF_CFG_CONNECTION pConnection,
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    )
{
    DWORD dwError = 0;
    PCSTR pszCursor = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;

    if (IsNullOrEmptyString(pszLogin) || *pszLogin == '@'
            || !ppszUser || !ppszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!(pszCursor = strchr(pszLogin, '@')) || !pszCursor++)
    {
        if (pConnection == NULL)
        {
            dwError = VmAfdGetDomainNameA(NULL, &pszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = VmAfdGetDomainNameFromRegistryA(pConnection, &pszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdAllocateStringA(pszLogin, &pszUser);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        size_t len = pszCursor-pszLogin-1;
        int i = 0;

        dwError = VmAfdAllocateStringA(pszCursor, &pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateMemory(len+1, (PVOID*)&pszUser);
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; i < len; i++)
        {
            pszUser[i] = pszLogin[i];
        }
    }

    *ppszUser = pszUser;
    *ppszDomain = pszDomain;

cleanup:

    return dwError;

error:

    *ppszUser = NULL;
    *ppszDomain = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}

static
DWORD
DirCliParsePrincipal(
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    )
{
    return DirCliParsePrincipalEx(NULL, pszLogin, ppszUser, ppszDomain);
}

static
DWORD
DirCliInitLdapConnection(
    PCSTR pszLogin,
    PCSTR pszPassword,
    LDAP** ppLd
    )
{
    DWORD   dwError = 0;
    PCSTR   pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR    pszPassword1 = NULL;
    PCSTR   pszPasswordLocal = pszPassword;
    PSTR    pszUser = NULL;
    PSTR    pszDomain = NULL;
    PSTR    pszDCName = NULL;
    LDAP    *pLd = NULL;

    if (!ppLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
            pszDCName,
            pszUser,
            pszDomain,
            pszPasswordLocal,
            &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppLd = pLd;

error:
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;
}

static
DWORD
DirCliBytesToHexString(
    PUCHAR  pData,
    int     length,
    PSTR*   ppszHexString
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszOut = NULL;
    int     i = 0;

    if (!pData || !ppszHexString || length <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(length * 2 + 1, (PVOID*)&pszOut);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; i < length; ++i)
    {
        sprintf(pszOut + i * 2, "%02X", pData[i]);
    }
    pszOut[length * 2] = '\0';

    *ppszHexString = pszOut;

error:
    return dwError;
}

static
DWORD
DirCliGenerateCACNForLdap(
    X509*   pCertificate,
    PSTR*   ppszCACN
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    int     length = 0;
    UCHAR*  pEncodedKey = NULL;
    UCHAR*  pKey = NULL;
    UCHAR   md[SHA_DIGEST_LENGTH];
    EVP_PKEY*       pPubKey = NULL;
    PSTR            pszCACN = NULL;
    ASN1_OCTET_STRING* pSid = NULL;

    if (!pCertificate || !ppszCACN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pSid = X509_get_ext_d2i(pCertificate, NID_subject_key_identifier, NULL, NULL);
    if (pSid)
    {
        dwError = DirCliKeyIdToHexString(pSid, &pszCACN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszCACN))
    {
        pPubKey = X509_get_pubkey(pCertificate);
        length = i2d_PUBKEY(pPubKey, NULL);
        dwError = VmAfdAllocateMemory(length, (PVOID*)&pEncodedKey);
        BAIL_ON_VMAFD_ERROR(dwError);

        pKey = pEncodedKey;
        length = i2d_PUBKEY(pPubKey, &pKey);
        SHA1(pEncodedKey, length, md);

        dwError = DirCliBytesToHexString(md, SHA_DIGEST_LENGTH, &pszCACN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszCACN = pszCACN;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pEncodedKey);
    if (pPubKey)
    {
        EVP_PKEY_free(pPubKey);
    }
    if (pSid)
    {
        ASN1_OCTET_STRING_free(pSid);
    }

    return dwError;

error:
    if (ppszCACN)
    {
        *ppszCACN = NULL;
    }
    goto cleanup;
}

static
DWORD
DirCliGetCrlAuthKeyIdHexString(
    X509_CRL*   pCrl,
    PSTR*       ppszAid
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszAid = NULL;
    AUTHORITY_KEYID *pId = NULL;

    if (!pCrl && !ppszAid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (X509_CRL_get_ext_by_NID(pCrl, NID_authority_key_identifier, -1) == -1)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pId = X509_CRL_get_ext_d2i(pCrl,
        NID_authority_key_identifier, NULL, NULL);
    if (!pId)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliKeyIdToHexString(pId->keyid, &pszAid);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAid = pszAid;

cleanup:
    if (pId)
    {
        AUTHORITY_KEYID_free(pId);
    }
    return dwError;
error:
    if (ppszAid)
    {
        *ppszAid = NULL;
    }

    goto cleanup;
}

static
DWORD
DirCliGetDCName(
    PSTR*      ppszDCName
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_A pDCInfo = NULL;
    PVMAFD_SERVER pServer = NULL;
    PSTR pszDCName = NULL;

    if (!ppszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdOpenServerA(NULL, NULL, NULL, &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcGetDCNameA(
                        pServer,
                        NULL,
                        NULL,
                        NULL,
                        0,
                        &pDCInfo
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringA(
                            pDCInfo->pszDCName,
                            &pszDCName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDCName = pszDCName;

cleanup:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
    if (pDCInfo)
    {
        CdcFreeDomainControllerInfoA(pDCInfo);
    }
    return dwError;
error:

    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    goto cleanup;
}

DWORD
DirCliSetState(
    PCSTR      pszHostName,
    PCSTR      pszLogin,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    DWORD      dwState
    )
{
    PSTR       pszPassword1 = NULL;
    PCSTR      pszPasswordLocal = pszPassword;
    DWORD      dwError = 0;
    PSTR       pszUser = NULL;
    PCSTR      pszDomainLocal = pszDomainName;
    PCSTR      pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR       pszDomain = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    if (dwState <= VMDIRD_STATE_UNDEFINED ||
        dwState > VMDIRD_STATE_STANDALONE)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(!pszDomainName)
    {
        pszDomainLocal = pszDomain;
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomainLocal,
                                     NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirOpenServerA(
                        pszHostName,
                        pszLoginLocal,
                        pszDomainLocal,
                        pszPasswordLocal,
                        0,
                        NULL,
                        &hServer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirSetState(hServer, dwState );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    if (hServer)
    {
        VmDirCloseServer(hServer);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliGetState(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword,
    PCSTR     pszDomainName,
    PDWORD    pdwState
    )
{
    DWORD     dwError = 0;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    PCSTR     pszDomainLocal = pszDomainName;
    UINT32    vmdirState = 0;
    PSTR      pszUser = NULL;
    PSTR      pszPassword1 = NULL;
    PCSTR     pszPasswordLocal = pszPassword;
    PCSTR     pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR      pszDomain = NULL;

    if (pdwState == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(!pszDomainName)
    {
        pszDomainLocal = pszDomain;
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomainLocal,
                                     NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirOpenServerA(
                        pszHostName,
                        pszLoginLocal,
                        pszDomainLocal,
                        pszPasswordLocal,
                        0,
                        NULL,
                        &hServer
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirGetState(hServer, &vmdirState );
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwState = vmdirState;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    if (hServer)
    {
        VmDirCloseServer(hServer);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliGetFuncLvl(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword,
    PCSTR     pszDomainName,
    PDWORD    pdwFuncLvl
    )
{
    DWORD     dwError = 0;
    DWORD     dwFuncLvl = 0;
    PSTR      pszDomain = NULL;
    PSTR      pszPassword1 = NULL;
    PSTR      pszUser = NULL;
    PSTR      pszDCName = NULL;
    PCSTR     pszDCNameLocal = pszHostName;
    PCSTR     pszDomainLocal = pszDomainName;
    PCSTR     pszPasswordLocal = pszPassword;
    PCSTR     pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;

    if ( !pdwFuncLvl)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(!pszDomainName)
    {
        pszDomainLocal = pszDomain;
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomainLocal,
                                     NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    if(!pszHostName)
    {
        dwError = DirCliGetDCName(&pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszDCNameLocal = pszDCName;
    }

    dwError = VmDirGetDomainFunctionalLevel( pszDCNameLocal,
                                     pszUser,
                                     pszPasswordLocal,
                                     pszDomainLocal,
                                     &dwFuncLvl );
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwFuncLvl = dwFuncLvl;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliSetFuncLvl(
    PCSTR      pszHostName,
    PCSTR      pszLogin,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PDWORD     pdwFuncLvl
    )
{

    DWORD      dwError = 0;
    DWORD      dwFuncLvl = 1;
    PCSTR      pszDomainLocal = pszDomainName;
    PSTR       pszDomain = NULL;
    PSTR       pszPassword1 = NULL;
    PSTR       pszUser = NULL;
    PSTR       pszDCName = NULL;
    PCSTR      pszDCNameLocal = pszHostName;
    PCSTR      pszPasswordLocal = pszPassword;
    PCSTR      pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    BOOLEAN    bUseDefault = FALSE;

    if ( !pdwFuncLvl )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (*pdwFuncLvl == 0)
    {
        bUseDefault = TRUE;
    }
    else
    {
        dwFuncLvl = *pdwFuncLvl;
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszDomainName)
    {
        pszDomainLocal = pszDomain;
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomainLocal,
                                     NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    if (!pszHostName)
    {
        dwError = DirCliGetDCName(&pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszDCNameLocal = pszDCName;
    }

    dwError = VmDirSetDomainFunctionalLevel(
                pszDCNameLocal,
                pszUser,
                pszPasswordLocal,
                pszDomainLocal,
                &dwFuncLvl,
                bUseDefault);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bUseDefault)
    {
        *pdwFuncLvl = dwFuncLvl;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliGetDCNodesVersion(
    PCSTR      pszHostName,
    PCSTR      pszLogin,
    PCSTR      pszPassword,
    PCSTR      pszDomainName,
    PVMDIR_DC_VERSION_INFO *ppDCVerInfo
    )
{
    DWORD      dwError = 0;
    PSTR       pszDomain = NULL;
    PSTR       pszPassword1 = NULL;
    PSTR       pszUser = NULL;
    PSTR       pszDCName = NULL;
    PCSTR      pszDCNameLocal = pszHostName;
    PCSTR      pszDomainLocal = pszDomainName;
    PCSTR      pszPasswordLocal = pszPassword;
    PCSTR      pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PVMDIR_DC_VERSION_INFO pDCVerInfo = NULL;

    if ( !ppDCVerInfo )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(!pszDomainName)
    {
        pszDomainLocal = pszDomain;
    }

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomainLocal,
                                     NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    if(!pszHostName)
    {
        dwError = DirCliGetDCName(&pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszDCNameLocal = pszDCName;
    }

    dwError =VmDirGetDCNodesVersion (
                pszDCNameLocal,
                pszUser,
                pszPasswordLocal,
                pszDomainLocal,
                &pDCVerInfo);

    if (dwError == VMDIR_ERROR_INCOMPLETE_MAX_DFL)
    {
        dwError = 0;
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDCVerInfo = pDCVerInfo;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    return dwError;

error:
    VmDirFreeDCVersionInfo(pDCVerInfo);

    goto cleanup;
}

static
VOID
DirCliPrintDCInfo(
    PVMDIR_DC_INFO* ppDC,
    DWORD dwNumDC
    )
{
    DWORD idx = 0;
    DWORD idxPartner = 0;

    for (; idx < dwNumDC; ++idx)
    {
        fprintf(
            stdout,
            "Node: %s\n"
            "Type: PSC\n"
            "Site: %s\n",
            ppDC[idx]->pszHostName,
            ppDC[idx]->pszSiteName
            );
        if (ppDC[idx]->dwPartnerCount > 0)
        {
            for (idxPartner=0; idxPartner < ppDC[idx]->dwPartnerCount; ++idxPartner)
            {
                fprintf(
                    stdout,
                    "Partner #%d: %s\n",
                    idxPartner + 1,
                    ppDC[idx]->ppPartners[idxPartner]
                    );
            }
        }
        fprintf(stdout, "\n");
    }
}

static
VOID
DirCliPrintComputers(
    PSTR* ppszComputers,
    DWORD dwNumComputers
    )
{
    DWORD idx = 0;

    for (; idx < dwNumComputers; ++idx)
    {
        fprintf(
            stdout,
            "Node: %s\n"
            "Type: Management\n",
            ppszComputers[idx]
            );
        fprintf(stdout, "\n");
    }
}

static
DWORD
DirCliGetMachineAccount(
    PVMAF_CFG_CONNECTION pConnection,
    PSTR *ppszMachineAccount
    )
{
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pSubKey = NULL;
    DWORD dwError = 0;
    PSTR pszMachineAccount = NULL;

    dwError = VmAfConfigOpenRootKey(
                pConnection,
                "HKEY_LOCAL_MACHINE",
                0,
                KEY_READ,
                &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                pConnection,
                pRootKey,
                VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                0,
                KEY_READ,
                &pSubKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                pSubKey,
                NULL,
                VMAFD_REG_KEY_DC_ACCOUNT,
                &pszMachineAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszMachineAccount = pszMachineAccount;
    pszMachineAccount = NULL;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszMachineAccount);

    if (pSubKey)
    {
        VmAfConfigCloseKey(pSubKey);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;
error:
    goto cleanup;
}

static
DWORD
DirCliIsManagementNode(
    PVMAF_CFG_CONNECTION pConnection,
    BOOLEAN *pbManagementNode
    )
{
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pSubKey = NULL;
    DWORD dwError = 0;
    PSTR pszDCAccountDN = NULL;

    dwError = VmAfConfigOpenRootKey(
                pConnection,
                "HKEY_LOCAL_MACHINE",
                0,
                KEY_READ,
                &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                pConnection,
                pRootKey,
                VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                0,
                KEY_READ,
                &pSubKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                pSubKey,
                NULL,
                VMAFD_REG_KEY_DC_ACCOUNT_DN,
                &pszDCAccountDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (VmAfdCaselessStrStrA(pszDCAccountDN, "ou=Domain Controllers"))
    {
        *pbManagementNode = FALSE;
    }
    else
    {
        *pbManagementNode = TRUE;
    }

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszDCAccountDN);

    if (pSubKey)
    {
        VmAfConfigCloseKey(pSubKey);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliGetDCNameFromRegistry(
    PVMAF_CFG_CONNECTION pConnection,
    PCSTR pszSuppliedServerName, /* OPTIONAL */
    PSTR *ppszServerName
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pSubKey = NULL;

    if (pszSuppliedServerName != NULL)
    {
        dwError = VmAfdAllocateStringA(pszSuppliedServerName, &pszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    VMAFD_CONFIG_PARAMETER_KEY_PATH,
                    0,
                    KEY_READ,
                    &pSubKey);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfConfigReadStringValue(
                    pSubKey,
                    NULL,
                    VMAFD_REG_KEY_DC_NAME,
                    &pszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszServerName = pszServerName;
    pszServerName = NULL;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszServerName);

    if (pSubKey)
    {
        VmAfConfigCloseKey(pSubKey);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;
error:
    goto cleanup;
}

static
DWORD
DirCliUpdateLocalPassword(
    PVMAF_CFG_CONNECTION pConnection,
    PCSTR pszPassword
    )
{
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pSubKey = NULL;
    DWORD dwError = 0;
    PSTR pszOldPassword = NULL;

    dwError = VmAfConfigOpenRootKey(
                pConnection,
                "HKEY_LOCAL_MACHINE",
                0,
                KEY_READ,
                &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                pConnection,
                pRootKey,
                VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                0,
                KEY_SET_VALUE | KEY_READ,
                &pSubKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * There might not be an existing password so ignore any errors.
     */
    (VOID)VmAfConfigReadStringValue(
                pSubKey,
                NULL,
                VMAFD_REG_KEY_DC_PASSWORD,
                &pszOldPassword);

    dwError = VmAfConfigSetValue(
                pSubKey,
                VMAFD_REG_KEY_DC_PASSWORD,
                REG_SZ,
                (PBYTE)pszPassword,
                (DWORD)strlen(pszPassword) + 1);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                pSubKey,
                VMAFD_REG_KEY_DC_OLD_PASSWORD,
                REG_SZ,
                (PBYTE)pszOldPassword,
                (DWORD)strlen(pszOldPassword) + 1);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszOldPassword);
    if (pSubKey)
    {
        VmAfConfigCloseKey(pSubKey);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliResetMachineAccountCredentials(
    PCSTR pszServer,
    PCSTR pszUser,
    PCSTR pszPassword
    )
{
    PSTR pszLocalHostName = NULL;
    DWORD dwError = 0;

    dwError = VmAfdGetHostName(&pszLocalHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirResetMachineActCred(
                pszLocalHostName,
                pszServer,
                pszUser,
                pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszLocalHostName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliSetManagedPassword(
    PCSTR pszServerName,
    PCSTR pszLoginUPN,
    PCSTR pszPassword,
    PCSTR pszDomain,
    PCSTR pszMachineAccount,
    PCSTR pszMachinePassword
    )
{
    DWORD dwError = 0;
    PSTR pszMachineUPN = NULL;

    dwError = VmAfdAllocateStringPrintf(
                &pszMachineUPN,
                "%s@%s",
                pszMachineAccount,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirSetPassword(
                pszServerName,
                pszLoginUPN,
                pszPassword,
                pszMachineUPN,
                pszMachinePassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszMachineUPN);
    return dwError;
error:
    goto cleanup;
}

DWORD
DirCliMachineAccountReset(
    PCSTR pszSuppliedServerName, /* OPTIONAL */
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    BOOLEAN bManagementNode = FALSE;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PSTR pszMachineAccount = NULL;
    PSTR pszServerName = NULL;
    PSTR pszLoginUPN = NULL;
    PSTR pszUser = NULL;
    PSTR pszDomain = NULL;
    PBYTE pBytePassword = NULL;
    DWORD dwPasswordSize = 0;

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliIsManagementNode(pConnection, &bManagementNode);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliGetDCNameFromRegistry(
                pConnection,
                pszSuppliedServerName,
                &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliGetMachineAccount(pConnection, &pszMachineAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * We accept a login ID or UPN so we need to do some parsing /
     * re-constructing.
     */
    dwError = DirCliParsePrincipalEx(
                pConnection,
                pszLogin,
                &pszUser,
                &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pszLoginUPN,
                "%s@%s",
                pszUser,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bManagementNode)
    {
        dwError = VmDirGeneratePassword(
                    pszServerName,
                    pszLoginUPN,
                    pszPassword,
                    &pBytePassword,
                    &dwPasswordSize);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = DirCliSetManagedPassword(
                    pszServerName,
                    pszLoginUPN,
                    pszPassword,
                    pszDomain,
                    pszMachineAccount,
                    (PSTR)pBytePassword);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = DirCliUpdateLocalPassword(pConnection, (PSTR)pBytePassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = DirCliResetMachineAccountCredentials(
                    pszServerName,
                    pszUser,
                    pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    //
    // Tell the user we did what was asked.
    //
    printf("Password for machine account reset.\n");

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszMachineAccount);
    VMAFD_SAFE_FREE_STRINGA(pszUser);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszLoginUPN);

    //
    // NB -- This should be VMDIR_SAFE_FREE_STRINGA but that's unavailable.
    // As things currently stand both routines are the same but this is a bit
    // fragile.
    //
    VMAFD_SAFE_FREE_STRINGA(pBytePassword);

    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
DirCliCreateTenant(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszDomainName,
    PCSTR pszNewUserName,
    PCSTR pszNewUserPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAdminUPN = NULL;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
                &pszAdminUPN,
                "%s@%s",
                pszUser,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirCreateTenant(
                pszAdminUPN,
                pszPasswordLocal,
                pszDomainName,
                pszNewUserName,
                pszNewUserPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Tenant domain %s successfully created\n", pszDomainName);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliDeleteTenant(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszDomainName
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAdminUPN = NULL;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
                &pszAdminUPN,
                "%s@%s",
                pszUser,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirDeleteTenant(pszAdminUPN, pszPasswordLocal, pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Tenant domain %s successfully deleted\n", pszDomainName);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliEnumerateTenants(
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAdminUPN = NULL;
    DWORD idx = 0;
    DWORD dwTenantCount = 0;
    PSTR *ppszTenantDomains = NULL;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(
                &pszAdminUPN,
                "%s@%s",
                pszUser,
                pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirEnumerateTenants(
                pszAdminUPN,
                pszPasswordLocal,
                &ppszTenantDomains,
                &dwTenantCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (idx = 0; idx < dwTenantCount; ++idx)
    {
        fprintf(stdout, "Tenant: %s\n", ppszTenantDomains[idx]);
    }
    VmDirFreeStringArray(ppszTenantDomains, dwTenantCount);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliCreateOrgunit(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszOrgunit,
    PCSTR pszParentDN
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAdminUPN = NULL;
    PSTR  pszOrgunitDN = NULL;
    LDAP* pLd = NULL;

    if (IsNullOrEmptyString(pszOrgunit))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapCreateOrgunit(
                pLd,
                pszOrgunit,
                pszDomain,
                pszParentDN,
                &pszOrgunitDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Orgunit %s successfully created\n", pszOrgunit);

cleanup:

    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    VMAFD_SAFE_FREE_MEMORY(pszOrgunitDN);
    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliEnumerateOrgunits(
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszContainerDN
    )
{
    DWORD dwError = 0;
    PCSTR pszLoginLocal = pszLogin ? pszLogin : DIR_LOGIN_DEFAULT;
    PSTR  pszPassword1 = NULL;
    PCSTR pszPasswordLocal = pszPassword;
    PSTR  pszDCName = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAdminUPN = NULL;
    LDAP* pLd = NULL;
    PDIR_CLI_ENUM_ORGUNIT_CONTEXT pEnumContext = NULL;
    PSTR* ppszOrgunits = NULL;
    DWORD dwOrgunitCount = 0;
    DWORD dwCount = 0;

    dwError = DirCliParsePrincipal(pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = DirCliReadPassword(pszUser, pszDomain, NULL, &pszPassword1);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPasswordLocal = pszPassword1;
    }

    dwError = DirCliGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapConnect(
                    pszDCName,
                    pszUser,
                    pszDomain,
                    pszPasswordLocal,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliLdapBeginEnumOrgunits(
                    pLd,
                    pszContainerDN,
                    pszDomain,
                    256,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        DWORD idx = 0;

        if (ppszOrgunits)
        {
            VmAfdFreeStringArrayCountA(ppszOrgunits, dwCount);
            ppszOrgunits = NULL;
        }

        dwError = DirCliLdapEnumOrgunits(pEnumContext, &ppszOrgunits, &dwCount);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; idx < dwCount; idx++)
        {
            fprintf(stdout, "%s\n", ppszOrgunits[idx]);
            dwOrgunitCount++;
        }

    } while (dwCount > 0);

    if (dwOrgunitCount == 0)
    {
        fprintf(stderr, "Container has no organization units\n");
    }

cleanup:

    if (pEnumContext)
    {
        DirCliLdapEndEnumOrgunits(pEnumContext);
    }
    if (pLd)
    {
        DirCliLdapClose(pLd);
    }
    if (ppszOrgunits)
    {
        VmAfdFreeStringArrayCountA(ppszOrgunits, dwCount);
    }
    VMAFD_SAFE_FREE_MEMORY(pszAdminUPN);
    VMAFD_SAFE_FREE_MEMORY(pszPassword1);
    VMAFD_SAFE_FREE_MEMORY(pszUser);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:

    goto cleanup;
}
