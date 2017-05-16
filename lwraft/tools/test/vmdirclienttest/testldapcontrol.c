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

#define TEST_CONDWRITE_VALUE    "testCondWriteControl description value"
#define TEST_CONDWRITE_VALUE_1  "testCondWriteControl description value 1"

DWORD
TestVmDirCondWriteControl(
    PVMDIRCLIENT_TEST_CONTEXT    pCtx
    )
{
    DWORD   dwError = 0;
    LDAPControl condWriteCtrl={0};
    LDAPMod     mod = {0};
    LDAPMod*    mods[2] = {&mod, NULL};
    PSTR        vals[2] = {TEST_CONDWRITE_VALUE, NULL};
    LDAPControl* srvCtrl[2] = { &condWriteCtrl, NULL};
    PSTR        pszDomainDN = NULL;

    if (!pCtx->pLd)
    {
        printf("\n ERROR, please use option 0 to setup server information first\n");
        goto cleanup;
    }

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = "description";
    mod.mod_vals.modv_strvals = vals;

    dwError = VmDirFQDNToDN(pCtx->pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_modify_ext_s(
                            pCtx->pLd,
                            pszDomainDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateCondWriteCtrlContent("description=" TEST_CONDWRITE_VALUE_1, &condWriteCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_modify_ext_s(
                            pCtx->pLd,
                            pszDomainDN,
                            mods,
                            srvCtrl,
                            NULL);
    if (dwError != VMDIR_LDAP_ERROR_PRE_CONDITION)
    {
        printf("\n VMDIR_LDAP_ERROR_PRE_CONDITION/9300 error code expected. Error code %d observed\n", dwError);
    }

    VmDirFreeCtrlContent(&condWriteCtrl);

    dwError = VmDirCreateCondWriteCtrlContent("description=" TEST_CONDWRITE_VALUE, &condWriteCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_modify_ext_s(
                            pCtx->pLd,
                            pszDomainDN,
                            mods,
                            srvCtrl,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    printf("\n %s failed, error code (%d) \n", __FUNCTION__, dwError);
    goto cleanup;
}
