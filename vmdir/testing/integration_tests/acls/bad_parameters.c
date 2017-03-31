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
SpecifyingAclStringAndSecurityDescriptorShouldFail(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserName,
    PCSTR pszAcl
    )
{
    DWORD dwError = 0;
    BYTE *pbSecurityDescriptor = NULL;
    DWORD dwLength = 0;
    PCSTR valsAcl[] = {NULL, NULL};
    PCSTR valsCn[] = {pszUserName, NULL};
    PCSTR valssAMActName[] = {pszUserName, NULL};
    PCSTR valsClass[] = {OC_USER, OC_PERSON, OC_TOP, OC_ORGANIZATIONAL_PERSON, NULL};
    PCSTR valsPNE[] = {"TRUE", NULL};
    PCSTR valsPN[] = {NULL, NULL};
    PCSTR valsPass[] = {"Admin!23", NULL};
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    struct berval bvSecurityDescriptor = {0};
    struct berval *bvSecurityDescriptorValues[2] = {NULL, NULL};
    LDAPMod mod[9]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, ATTR_PASSWORD_NEVER_EXPIRES, {(PSTR*)valsPNE}},
        {LDAP_MOD_ADD, ATTR_KRB_UPN, {(PSTR*)valsPN}},
        {LDAP_MOD_ADD, ATTR_USER_PASSWORD, {(PSTR*)valsPass}},
        {LDAP_MOD_ADD, ATTR_SN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_ACL_STRING, {(PSTR*)valsAcl}},
        {0,NULL,{NULL}}
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], &mod[6], &mod[7], &mod[8], NULL};

    dwError = _VdcGetObjectSecurityDescriptor(
                pState,
                pState->pszBaseDN,
                &pbSecurityDescriptor,
                &dwLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    bvSecurityDescriptor.bv_val = pbSecurityDescriptor;
    bvSecurityDescriptor.bv_len = dwLength;
    bvSecurityDescriptorValues[0] = &bvSecurityDescriptor;
    mod[8].mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    mod[8].mod_type = ATTR_OBJECT_SECURITY_DESCRIPTOR;
    mod[8].mod_bvalues = bvSecurityDescriptorValues;

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    valsPN[0] = pszUPN;
    valsAcl[0] = pszAcl;

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                "Users",
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pState->pLd,
                pszDN,
                attrs,
                NULL,
                NULL);
    TestAssertEquals(dwError, LDAP_CONSTRAINT_VIOLATION);
    printf("Add of %s returned %d\n", pszDN, dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    return dwError;

error:
    goto cleanup;
}

DWORD
TestBadParameters(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszDomainSid = NULL;
    PSTR pszSecurityDescriptor = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszSecurityDescriptor,
                "O:%s-500G:BAD:P(A;;RCRP;;;%s-500)",
                pszDomainSid,
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SpecifyingAclStringAndSecurityDescriptorShouldFail(
                pState,
                pszUserName,
                pszSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    VMDIR_SAFE_FREE_STRINGA(pszSecurityDescriptor);
    return dwError;

error:
    goto cleanup;
}
