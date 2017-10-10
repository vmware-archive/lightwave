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

DWORD _VmDirFindUserDN(
    LDAP *pLd,
    PCSTR pszUserUPN,
    PSTR *ppszUserDN
    )
{
    DWORD dwError = 0;
    LDAPMessage *searchRes = NULL;
    LDAPMessage *entry = NULL;
    PSTR pszSearchFilter = NULL;
    PSTR pszUserDN = NULL;
    PSTR pszDN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszSearchFilter,
                "(%s=%s)",
                ATTR_KRB_UPN,
                pszUserUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(pLd,
                                "",
                                LDAP_SCOPE_SUBTREE,
                                pszSearchFilter,
                                NULL,
                                TRUE,
                                NULL,
                                NULL,
                                NULL,
                                0,
                                &searchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, searchRes) != 1)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    entry = ldap_first_entry(pLd, searchRes);
    if (!entry)
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszDN = ldap_get_dn(pLd, entry);
    if (IsNullOrEmptyString(pszDN))
    {
        dwError = ERROR_INVALID_DN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszDN, &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUserDN = pszUserDN;
    pszUserDN = NULL;

cleanup:
    ldap_memfree(pszDN);
    ldap_msgfree(searchRes);
    VMDIR_SAFE_FREE_MEMORY(pszSearchFilter);
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    return dwError;

error:
    goto cleanup;
}

