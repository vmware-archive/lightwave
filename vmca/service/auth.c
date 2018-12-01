/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
VMCAIsAdministrator(
    PCSTR       szAuthPrinc,
    PBOOLEAN    pbHasAdminPrivilege
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasAdminPrivilege = FALSE;

    if (IsNullOrEmptyString(szAuthPrinc) || !pbHasAdminPrivilege)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCALdapAccessCheck(szAuthPrinc, VMCA_ADMINISTRATORS);
    if (dwError == ERROR_ACCESS_DENIED)
    {
        dwError = 0;
        bHasAdminPrivilege = FALSE;
    }
    else if (dwError == 0)
    {
        bHasAdminPrivilege = TRUE;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *pbHasAdminPrivilege = bHasAdminPrivilege;

cleanup:
    return dwError;

error:
    if (pbHasAdminPrivilege)
    {
        *pbHasAdminPrivilege = FALSE;
    }
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
