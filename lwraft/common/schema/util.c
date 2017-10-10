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
VmDirLdapSearchSubSchemaSubEntry(
    LDAP*           pLd,
    LDAPMessage**   ppResult,
    LDAPMessage**   ppEntry
    )
{
    static PSTR ppszSubSchemaSubEntryAttrs[] =
    {
        ATTR_ATTRIBUTETYPES,
        ATTR_OBJECTCLASSES,
        ATTR_DITCONTENTRULES,
        ATTR_LDAPSYNTAXES,
        NULL
    };

    DWORD           dwError = 0;
    PCSTR           pcszFilter = "(objectclass=*)";
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;

    if (!ppResult)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                pLd,
                SUB_SCHEMA_SUB_ENTRY_DN,
                LDAP_SCOPE_BASE,
                pcszFilter,
                ppszSubSchemaSubEntryAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);

    *ppResult = pResult;
    *ppEntry = pEntry;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    goto cleanup;
}
