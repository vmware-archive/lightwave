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

DWORD
VmDirLdapSchemaLoadRemoteSchema(
    PVDIR_LDAP_SCHEMA   pSchema,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    PCSTR   pcszAtFilter = "(objectClass=attributeSchema)";
    PCSTR   pcszOcFilter = "(objectClass=classSchema)";
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;

    if (!pSchema || !pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(pLd,
            SCHEMA_NAMING_CONTEXT_DN, LDAP_SCOPE_SUBTREE, pcszAtFilter,
            NULL, FALSE, NULL, NULL, NULL, 0, &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult); pEntry;
            pEntry = ldap_next_entry(pLd, pEntry))
    {
        dwError = VmDirLdapAtParseLDAPEntry(pLd, pEntry, &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        pAt = NULL;
    }

    ldap_msgfree(pResult);

    dwError = ldap_search_ext_s(pLd,
            SCHEMA_NAMING_CONTEXT_DN, LDAP_SCOPE_SUBTREE, pcszOcFilter,
            NULL, FALSE, NULL, NULL, NULL, 0, &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult); pEntry;
            pEntry = ldap_next_entry(pLd, pEntry))
    {
        dwError = VmDirLdapOcParseLDAPEntry(pLd, pEntry, &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrParseLDAPEntry(pLd, pEntry, &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pOc = NULL;

        if (pCr)
        {
            dwError = VmDirLdapSchemaAddCr(pSchema, pCr);
            BAIL_ON_VMDIR_ERROR(dwError);
            pCr = NULL;
        }
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    VmDirFreeLdapAt(pAt);
    VmDirFreeLdapOc(pOc);
    VmDirFreeLdapCr(pCr);
    goto cleanup;
}
