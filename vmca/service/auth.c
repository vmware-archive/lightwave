/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#define VMCA_ADMINISTRATORS_GROUP "cn=CAAdmins,cn=Builtin"

static DWORD
VMCAOpenLocalLdapServer(
    PVMCA_LDAP_CONTEXT* pLd
    );

static BOOL
VMCAIsMemberOf(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    );

static VOID
VMCAFreeMemberShips(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VMCALdapAccessCheck(
    PCSTR szAuthPrinc,
    VMCA_USER_TYPE userType
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_LDAP_CONTEXT pLd = NULL;
    PSTR* ppszMemberships = NULL;
    PSTR szGroupName = NULL;
    PSTR pszDomainName = NULL;
    DWORD dwMemberships = 0;

    if (IsNullOrEmptyString(szAuthPrinc) ||
        (userType != VMCA_ADMINISTRATORS))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAOpenLocalLdapServer(&pLd);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetDefaultDomainName2(
                    pLd,
                    &pszDomainName
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCALdapGetMemberships(
                    pLd,
                    szAuthPrinc,
                    &ppszMemberships,
                    &dwMemberships
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                &szGroupName,
                "%s,%s",
                VMCA_ADMINISTRATORS_GROUP,
                pszDomainName);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("Checking upn: %s against CA admin group: %s ", szGroupName, szAuthPrinc);

    if (!VMCAIsMemberOf(
                    ppszMemberships,
                    dwMemberships,
                    szGroupName
                    ))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(szGroupName);

    if (ppszMemberships)
    {
        VMCAFreeMemberShips(ppszMemberships, dwMemberships);
        ppszMemberships = NULL;
    }

    if (pLd)
    {
        VMCALdapClose(pLd);
        pLd = NULL;
    }

    return dwError;
error:
    goto cleanup;
}

DWORD
VMCAOpenLocalLdapServer(
    PVMCA_LDAP_CONTEXT* pLd
    )
{
    DWORD dwError = 0;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDN = NULL;
    PSTR pszUPN = NULL;
    PVMCA_LDAP_CONTEXT pContext = NULL;

    if (pLd == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCASrvGetMachineAccountInfoA(
                &pszAccount,
                &pszDN,
                &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                &pszUPN,
                "%s@%s",
                pszAccount,
                pszDN);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCALdapConnect(
                    "localhost",
                    0, /* use default port */
                    pszUPN,
                    pszPassword,
                    &pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    *pLd = pContext;
cleanup:
    VMCA_SAFE_FREE_STRINGA(pszUPN);
    VMCA_SAFE_FREE_STRINGA(pszDN);
    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszPassword);
    return dwError;

error:
    goto cleanup;
}

BOOL
VMCAIsMemberOf(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    )
{
    BOOL bRetVal = FALSE;
    int nCmpRet = 0;
    DWORD i = 0;

    for(i = 0; i < dwMemberships; ++i)
    {
        VMCA_LOG_INFO("Checking user's group: %s against CA admin group: %s ", ppszMemberships[i], pszGroupName);
        nCmpRet = VMCAStringCompareA(ppszMemberships[i], pszGroupName, FALSE);
        if (nCmpRet == 0)
        {
            bRetVal = TRUE;
            break;
        }
    }

    return bRetVal;
}

VOID
VMCAFreeMemberShips(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    )
{
    DWORD i = 0;
    for(i = 0; i < dwMemberships; ++i)
    {
        VMCA_SAFE_FREE_STRINGA(ppszMemberships[i]);
    }

    VMCA_SAFE_FREE_MEMORY(ppszMemberships);
}
