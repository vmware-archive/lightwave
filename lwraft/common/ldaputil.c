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
VmDirConvertUPNToDN(
     LDAP*      pLd,
     PCSTR      pszUPN,
     PSTR*      ppszOutDN
     )
{
    DWORD       dwError = 0;


    LDAPMessage*    pEntry = NULL;
    LDAPMessage*    pResult = NULL;
    PSTR            pszFilter = NULL;
    PSTR            pszEntryDN = NULL;
    PSTR            pszOutDN = NULL;
    int             iCount = 0;

    if ( !pLd || !pszUPN || !ppszOutDN )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(
                    &pszFilter, "%s=%s",
                    ATTR_KRB_UPN,
                    pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    NULL,
                    FALSE, /* get values      */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    iCount = ldap_count_entries(pLd, pResult);

    // should have either 0 or 1 result
    if (iCount > 1)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }
    else if (iCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);
    }

    if ( (pEntry = ldap_first_entry(pLd, pResult)) != NULL )
    {
        pszEntryDN = ldap_get_dn(pLd, pEntry);

        dwError = VmDirAllocateStringA( pszEntryDN, &pszOutDN );
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppszOutDN = pszOutDN;
        pszOutDN = NULL;
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_ENTRY);
    }

cleanup:

    ldap_memfree( pszEntryDN );
    ldap_msgfree( pResult );
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    VMDIR_SAFE_FREE_MEMORY(pszOutDN);

    return dwError;

error:
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    goto cleanup;
}
