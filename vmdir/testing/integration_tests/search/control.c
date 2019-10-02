/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
TestLdapParseSearchPlanControl(
    LDAPControl**           ppCtrls,
    PVDIR_SEARCH_EXEC_PATH  pExecPath
    )
{
    DWORD   dwError = 0;
    LDAPControl* pSearchPlanReplyCtrl = NULL;

    assert(ppCtrls);
    assert(pExecPath);

    pExecPath->searchAlgo = SEARCH_ALGO_UNKNOWN;

    pSearchPlanReplyCtrl = ldap_control_find(LDAP_SEARCH_PLAN_CONTROL, ppCtrls, NULL);
    if (pSearchPlanReplyCtrl)
    {
        dwError = VmDirParseSearchPlanControlContent(pSearchPlanReplyCtrl, pExecPath);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}
