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
VmDirTestGetUserSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserName,
    PCSTR pszUserContainer, // optional
    PSTR *ppszUserSid
    )
{
    DWORD dwError;
    PSTR pszUserSid = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszContainerDn = NULL;

    if (pszUserContainer != NULL)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszContainerDn,
                    "cn=%s,",
                    pszContainerDn);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,%scn=users,%s",
                pszUserName,
                pszUserContainer ? pszContainerDn : "",
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszUserDn,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                "objectSid",
                &pszUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppszUserSid = pszUserSid;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    VMDIR_SAFE_FREE_STRINGA(pszContainerDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirTestDeleteUserEx(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    BOOLEAN bUseLimitedAccount
    )
{
    DWORD dwError = 0;
    PSTR pszDN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer ? pszContainer : "Users",
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bUseLimitedAccount)
    {
        dwError = ldap_delete_ext_s(pState->pLdLimited, pszDN, NULL, NULL);
    }
    else
    {
        dwError = ldap_delete_ext_s(pState->pLd, pszDN, NULL, NULL);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirTestDeleteUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName
    )
{
    return VmDirTestDeleteUserEx(pState, pszContainer, pszUserName, FALSE);
}

DWORD
VmDirTestCreateUserWithLimitedAccount(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PCSTR pszAcl /* OPTIONAL */
    )
{
    DWORD dwError = 0;
    PCSTR valsAcl[] = {NULL, NULL};
    PCSTR valsCn[] = {pszUserName, NULL};
    PCSTR valssAMActName[] = {pszUserName, NULL};
    PCSTR valsClass[] = {OC_USER, OC_PERSON, OC_TOP, OC_ORGANIZATIONAL_PERSON, NULL};
    PCSTR valsPNE[] = {"TRUE", NULL};
    PCSTR valsPN[] = {NULL, NULL};
    PCSTR valsPass[] = {"Admin!23", NULL};
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, ATTR_PASSWORD_NEVER_EXPIRES, {(PSTR*)valsPNE}},
        {LDAP_MOD_ADD, ATTR_KRB_UPN, {(PSTR*)valsPN}},
        {LDAP_MOD_ADD, ATTR_USER_PASSWORD, {(PSTR*)valsPass}},
        {LDAP_MOD_ADD, ATTR_SN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_ACL_STRING, {(PSTR*)valsAcl}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], &mod[6], &mod[7], NULL};

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    valsPN[0] = pszUPN;
    if (pszAcl != NULL)
    {
        valsAcl[0] = pszAcl;
    }
    else
    {
        attrs[7] = NULL;
    }

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer ? pszContainer : "Users",
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pState->pLdLimited,
                pszDN,
                attrs,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestCreateUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PCSTR pszAcl /* OPTIONAL */
    )
{
    DWORD dwError = 0;
    PCSTR valsAcl[] = {NULL, NULL};
    PCSTR valsCn[] = {pszUserName, NULL};
    PCSTR valssAMActName[] = {pszUserName, NULL};
    PCSTR valsClass[] = {OC_USER, OC_PERSON, OC_TOP, OC_ORGANIZATIONAL_PERSON, NULL};
    PCSTR valsPNE[] = {"TRUE", NULL};
    PCSTR valsPN[] = {NULL, NULL};
    PCSTR valsPass[] = {"Admin!23", NULL};
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, ATTR_PASSWORD_NEVER_EXPIRES, {(PSTR*)valsPNE}},
        {LDAP_MOD_ADD, ATTR_KRB_UPN, {(PSTR*)valsPN}},
        {LDAP_MOD_ADD, ATTR_USER_PASSWORD, {(PSTR*)valsPass}},
        {LDAP_MOD_ADD, ATTR_SN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_ACL_STRING, {(PSTR*)valsAcl}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], &mod[6], &mod[7], NULL};

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    valsPN[0] = pszUPN;
    if (pszAcl != NULL)
    {
        valsAcl[0] = pszAcl;
    }
    else
    {
        attrs[7] = NULL;
    }

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer ? pszContainer : "Users",
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
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestCreateUserWithSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PBYTE pbSecurityDescriptor,
    DWORD dwLength
    )
{
    DWORD dwError = 0;
    PCSTR valsCn[] = {pszUserName, NULL};
    PCSTR valssAMActName[] = {pszUserName, NULL};
    PCSTR valsClass[] = {OC_USER, OC_PERSON, OC_TOP, OC_ORGANIZATIONAL_PERSON, NULL};
    PCSTR valsPNE[] = {"TRUE", NULL};
    PCSTR valsPN[] = {NULL, NULL};
    PCSTR valsPass[] = {"Admin!23", NULL};
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, ATTR_PASSWORD_NEVER_EXPIRES, {(PSTR*)valsPNE}},
        {LDAP_MOD_ADD, ATTR_KRB_UPN, {(PSTR*)valsPN}},
        {LDAP_MOD_ADD, ATTR_USER_PASSWORD, {(PSTR*)valsPass}},
        {LDAP_MOD_ADD, ATTR_SN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD | LDAP_MOD_BVALUES, ATTR_OBJECT_SECURITY_DESCRIPTOR, {0}},
    };
    BerValue *pbvSecurityDescriptors[2] = {NULL, NULL};
    BerValue bvSecurityDescriptor = {0};
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], &mod[6], &mod[7], NULL};

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    valsPN[0] = pszUPN;

    bvSecurityDescriptor.bv_len = dwLength;
    bvSecurityDescriptor.bv_val = (char*)pbSecurityDescriptor;
    pbvSecurityDescriptors[0] = &bvSecurityDescriptor;
    mod[7].mod_vals.modv_bvals = pbvSecurityDescriptors;

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer ? pszContainer : "Users",
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
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    return dwError;

error:
    goto cleanup;
}
