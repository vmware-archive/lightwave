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

DWORD
VmDirTestGetDomainSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszDomainDn,
    PSTR *ppszDomainSid
    )
{
    DWORD dwError;
    PSTR pszDomainSid = NULL;

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszDomainDn,
                LDAP_SCOPE_BASE,
                "(objectclass=dcObject)",
                "objectSid",
                &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppszDomainSid = pszDomainSid;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirTestCreateClass(
    PVMDIR_TEST_STATE pState,
    PCSTR pszClassName
    )
{
    DWORD dwError = 0;
    PCSTR valsCn[] = {pszClassName, NULL};
    PCSTR valsClass[] = {"classschema", NULL};
    PCSTR valsSubclass[] = {OC_TOP, NULL};
    PCSTR valsGovernsId[] = {"111.111.4.001", NULL};
    PCSTR valsCategory[] = {"1", NULL};
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, "governsid", {(PSTR*)valsGovernsId}},
        {LDAP_MOD_ADD, "objectclasscategory", {(PSTR*)valsCategory}},
        {LDAP_MOD_ADD, "subclassof", {(PSTR*)valsSubclass}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], NULL};

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=schemacontext",
                pszClassName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pState->pLd,
                pszDN,
                attrs,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestCreateObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszClassName,
    PCSTR pszObjectName
    )
{
    DWORD dwError = 0;
    PCSTR valsCn[] = {pszObjectName, NULL};
    PCSTR valsClass[] = {"classschema", NULL};
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], NULL};

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,cn=%s,%s",
                pszClassName,
                pszContainer,
                VmDirTestGetTestContainerCn(pState),
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pState->pLd,
                pszDN,
                attrs,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestCreateContainer(
    PVMDIR_TEST_STATE pState,
    PCSTR pszName
    )
{
    DWORD dwError = 0;
    PCSTR valsCn[] = {pszName, NULL};
    PCSTR valsClass[] = {"top", "container", NULL};
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], NULL};

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszName,
                VmDirTestGetTestContainerCn(pState),
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pState->pLd,
                pszDN,
                attrs,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestGetGuid(
    PSTR *ppszGuid
    )
{
    DWORD dwError = 0;
    PSTR pszGuid = NULL;
    uuid_t guid = {0};
    char szGuid[VMDIR_GUID_STR_LEN] = {0};

    VmDirUuidGenerate(&guid);
    VmDirUuidToStringLower(&guid, szGuid, VMDIR_ARRAY_SIZE(szGuid));

    dwError = VmDirAllocateStringA(szGuid, &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;

cleanup:
    return dwError;
error:
    goto cleanup;
}
