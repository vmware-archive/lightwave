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

typedef struct _VMDIR_K8S_STATE
{
    PSTR    pszK8sOrgUnit;
    PSTR    pszK8sOrgUnitDN;
    PSTR    pszK8sGroupName;
    PSTR    pszK8sGroupNameDN;
    PSTR    pszMachineName;
    PSTR    pszMachineNameDN;
    PSTR    pszMachinePassword;
    PSTR    pszEtcdDN;

} VMDIR_K8S_STATE, *PVMDIR_K8S_STATE;

static
DWORD
_ACLOUContainer(
    PVMDIR_TEST_STATE   pState,
    PVMDIR_K8S_STATE    pK8s
    )
{
    DWORD   dwError = 0;
    PSTR    pszACLString = NULL;
    PSTR    pszGroupSid = NULL;
    PSTR    pszNewACLString = NULL;
    PSTR    ppszAttributeValues[] = { NULL, NULL };

    dwError = VmDirAllocateStringPrintf(
                &pK8s->pszK8sOrgUnitDN,
                "%s=%s,%s=%s,%s",
                ATTR_OU,
                pK8s->pszK8sOrgUnit,
                ATTR_OU,
                VMDIR_COMPUTERS_RDN_VAL,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // ACL container
    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pK8s->pszK8sOrgUnitDN,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                ATTR_ACL_STRING,
                &pszACLString);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pK8s->pszK8sGroupNameDN,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                ATTR_OBJECT_SID,
                &pszGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszNewACLString,
                "%s(A;CIOI;CCDCRPWP;;;%s)",
                pszACLString,
                pszGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszAttributeValues[0] = pszNewACLString;
    dwError = VmDirTestReplaceAttributeValues(
                pState->pLd,
                pK8s->pszK8sOrgUnitDN,
                ATTR_ACL_STRING,
                (PCSTR*)&ppszAttributeValues);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszACLString);
    VMDIR_SAFE_FREE_MEMORY(pszGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszNewACLString);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_CleanupTestK8sMachineActSelfService(
    PVMDIR_TEST_STATE   pState,
    PVMDIR_K8S_STATE    pK8s
    )
{
    ldap_delete_ext_s(pState->pLd, pK8s->pszEtcdDN, NULL, NULL);
    ldap_delete_ext_s(pState->pLd, pK8s->pszMachineNameDN, NULL, NULL);
    ldap_delete_ext_s(pState->pLd, pK8s->pszK8sOrgUnitDN, NULL, NULL);
    ldap_delete_ext_s(pState->pLd, pK8s->pszK8sGroupNameDN, NULL, NULL);

    VMDIR_SAFE_FREE_MEMORY(pK8s->pszEtcdDN);
    VMDIR_SAFE_FREE_MEMORY(pK8s->pszK8sGroupNameDN);
    VMDIR_SAFE_FREE_MEMORY(pK8s->pszK8sOrgUnitDN);
    VMDIR_SAFE_FREE_MEMORY(pK8s->pszMachineNameDN);
    VMDIR_SAFE_FREE_MEMORY(pK8s->pszMachinePassword);

    return 0;
}

static
DWORD
_InitializeTestK8sMachineActSelfService(
    PVMDIR_TEST_STATE   pState,
    PVMDIR_K8S_STATE    pK8s
    )
{
    DWORD   dwError = 0;
    PSTR    pszOutPassword = NULL;
    PCSTR   pszTempGroupContainer = "users";

    //pState->pfnCleanupCallback = _CleanupTestK8sMachineActSelfService;

    dwError = VmDirCreateComputerAccount(
                  pState->pszServerName,
                  pState->pszUserName,
                  pState->pszPassword,
                  pK8s->pszMachineName,
                  pK8s->pszK8sOrgUnit,
                  &pszOutPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateGroup(
                pState,
                pszTempGroupContainer,
                pK8s->pszK8sGroupName,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pK8s->pszMachineNameDN,
                "cn=%s,%s=%s,%s=%s,%s",
                pK8s->pszMachineName,
                ATTR_OU,
                pK8s->pszK8sOrgUnit,
                ATTR_OU,
                VMDIR_COMPUTERS_RDN_VAL,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pK8s->pszK8sGroupNameDN,
                "cn=%s,cn=%s,%s",
                pK8s->pszK8sGroupName,
                pszTempGroupContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestAddUserToGroupByDn(
                pState->pLd,
                pK8s->pszMachineNameDN,
                pK8s->pszK8sGroupNameDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    pK8s->pszMachinePassword = pszOutPassword;

cleanup:

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszOutPassword);
    goto cleanup;
}

static
DWORD
_TestK8sMachineActSelfService(
    PVMDIR_TEST_STATE   pState,
    PVMDIR_K8S_STATE    pK8s
    )
{
    DWORD   dwError = 0;
    PSTR    pszOutPassword = NULL;
    PSTR    pszEtcdCN = "ETCD-NODE";

    dwError = VmDirCreateComputerAccount(
                  pState->pszServerName,
                  pK8s->pszMachineName,
                  pK8s->pszMachinePassword,
                  "ETCD-NODE",
                  pK8s->pszK8sOrgUnit,
                  &pszOutPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pK8s->pszEtcdDN,
                "cn=%s,%s",
                pszEtcdCN,
                pK8s->pszK8sOrgUnitDN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszOutPassword);

    return dwError;

error:
    goto cleanup;
}

DWORD
TestK8sMachineActSelfService(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    VMDIR_K8S_STATE K8sState =
    {
        .pszK8sOrgUnit          = "K8sOrgUnit",
        .pszK8sOrgUnitDN        = NULL,
        .pszK8sGroupName        = "K8sGroup",
        .pszK8sGroupNameDN      = NULL,
        .pszMachineNameDN       = NULL,
        .pszMachineName         = "TestK8SMachine",
        .pszMachinePassword     = NULL,
        .pszEtcdDN              = NULL,
    };

    dwError = _InitializeTestK8sMachineActSelfService(pState, &K8sState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestK8sMachineActSelfService(pState, &K8sState);
    if (dwError == 0) // should check LDAP_INSUFFICIENT_ACCESS?
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACL_VIOLATION);
    }

    dwError = _ACLOUContainer(pState, &K8sState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestK8sMachineActSelfService(pState, &K8sState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _CleanupTestK8sMachineActSelfService(pState, &K8sState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}
