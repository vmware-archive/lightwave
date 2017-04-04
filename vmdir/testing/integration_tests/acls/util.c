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


// TODO -- No one calls this.
DWORD
_GetObjectAcl(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectDn,
    PSTR *ppszAcl
    )
{
    DWORD dwError = 0;
    PSTR pszAcl = NULL;

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pState->pszBaseDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszAcl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAcl = pszAcl;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
_VdcSearchForEntryAndAttribute(
    LDAP *pLd,
    PCSTR pszBaseDN,
    PCSTR pszAttribute, // OPTIONAL
    PSTR *ppszValue // OUT OPTIONAL
    )
{
    DWORD dwError = 0;
    LDAPMessage* pResult = NULL;
    PCSTR ppszAttrs[] = { NULL, NULL };
    PSTR pszAttributeValue = NULL;
    BerValue **ppBerValues = NULL;

    if (pszAttribute != NULL)
    {
        ppszAttrs[0] = pszAttribute;
    }

    dwError = ldap_search_ext_s(
                pLd,
                pszBaseDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                TRUE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszAttribute != NULL)
    {
        if (ldap_count_entries(pLd, pResult) > 0)
        {
            LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

            for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
            {
                BerValue** ppBerValues = NULL;
                ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
                if (ppBerValues != NULL && ldap_count_values_len(ppBerValues) > 0)
                {
                    dwError = VmDirAllocateStringA(
                                ppBerValues[0][0].bv_val,
                                &pszAttributeValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                    break;
                }
            }
        }

        *ppszValue = pszAttributeValue;
    }

cleanup:
    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszAttributeValue);

    goto cleanup;
}

DWORD
VmDirTestAddAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    )
{
    DWORD dwError = 0;

    LDAPMod addition;
    LDAPMod *mods[2];

    addition.mod_op     = LDAP_MOD_ADD;
    addition.mod_type   = (PSTR) pszAttribute;
    addition.mod_values = (PSTR*) ppszAttributeValues;

    mods[0] = &addition;
    mods[1] = NULL;

    dwError = ldap_modify_ext_s(pLd, pszDN, mods, NULL, NULL);

    return dwError;
}

// TODO -- Get rid of this and just call VmDirTestGetAttributeValue
DWORD
_VdcGetObjectSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectDN,
    BYTE **ppbSecurityDescriptor,
    PDWORD pdwSDLength
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    BYTE *pbSecurityDescriptor = NULL;
    DWORD dwSDLength = 0;
    BOOLEAN bValidDescriptor = FALSE;
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszFilter,
                "%s=*",
                ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValue(
                pState->pLd,
                pszObjectDN,
                LDAP_SCOPE_BASE,
                pszFilter,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pbSecurityDescriptor,
                &dwSDLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSecurityDescriptor = (PSECURITY_DESCRIPTOR_RELATIVE)pbSecurityDescriptor;
    bValidDescriptor = RtlValidRelativeSecurityDescriptor(
                        pSecurityDescriptor,
                        dwSDLength,
                        OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION);
    TestAssertMsg(bValidDescriptor, "The object already has a bad SD?!?");

    *ppbSecurityDescriptor = pbSecurityDescriptor;
    *pdwSDLength = dwSDLength;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    return dwError;

error:
    goto cleanup;
}

DWORD
_GetBuiltinGroupSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszGroupCn,
    PSTR *ppszGroupSid
    )
{
    PSTR pszGroupDn = NULL;
    PSTR pszGroupSid = NULL;
    DWORD dwError = 0;

    dwError = VmDirAllocateStringPrintf(
                &pszGroupDn,
                "cn=%s,cn=Builtin,%s",
                pszGroupCn,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszGroupDn,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                "objectSid",
                &pszGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGroupSid = pszGroupSid;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszGroupDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
_VdcConnectionFromUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserCn,
    LDAP **ppLd
    )
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    PSTR pszUserUPN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszUserUPN,
                "%s@%s",
                pszUserCn,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pState->pszServerName,
                pszUserUPN,
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserUPN);
    return dwError;
error:
    goto cleanup;
}
