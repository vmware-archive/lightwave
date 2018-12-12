/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
VmDirInitMaxCommittedUSN(
    PVDIR_BACKEND_INTERFACE   pBE
    )
{
    USN       tmpUSN = 0;
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;
    VDIR_BACKEND_CTX    beCtx = {0};

    if (!pBE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = pBE;

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    dwError = pBE->pfnBEGetNextUSN(&beCtx, &tmpUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmDirServerOpsGlobals.maxCommittedUSN = tmpUSN;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirUpdateMaxCommittedUSNInLock(
    USN   committedUSN
    )
{
    if (committedUSN > gVmDirServerOpsGlobals.maxCommittedUSN)
    {
        gVmDirServerOpsGlobals.maxCommittedUSN = committedUSN;
    }
}

USN
VmDirGetMaxCommittedUSN(
    VOID
    )
{
    USN   maxCommittedUSN = 0;
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    maxCommittedUSN = gVmDirServerOpsGlobals.maxCommittedUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    return maxCommittedUSN;
}

DWORD
VmDirEntryUpdateUsnChanged(
    PVDIR_ENTRY    pEntry,
    USN            localUSN
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pCurrAttr = NULL;

    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for (pCurrAttr = pEntry->attrs; pCurrAttr; pCurrAttr = pCurrAttr->next)
    {
        if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE) == 0)
        {
            dwError = VmDirAttributeUpdateUsnValue(pCurrAttr, localUSN);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

    if (pCurrAttr == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SUCH_ATTRIBUTE);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirEntryUpdateUsnCreated(
    PVDIR_ENTRY    pEntry,
    USN            localUSN
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pCurrAttr = NULL;

    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for (pCurrAttr = pEntry->attrs; pCurrAttr; pCurrAttr = pCurrAttr->next)
    {
        if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_USN_CREATED, FALSE) == 0)
        {
            dwError = VmDirAttributeUpdateUsnValue(pCurrAttr, localUSN);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirAttributeUpdateUsnValue(
    PVDIR_ATTRIBUTE    pAttr,
    USN                localUSN
    )
{
    DWORD     dwError = 0;
    char      pszLocalUsn[VMDIR_MAX_USN_STR_LEN] = {'\0'};
    size_t    localUsnStrlen = 0;

    if (!pAttr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringNPrintFA(
            pszLocalUsn,
            sizeof(pszLocalUsn),
            sizeof(pszLocalUsn) - 1,
            "%"PRId64,
            localUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    localUsnStrlen = VmDirStringLenA(pszLocalUsn);

    VmDirFreeBervalContent(&pAttr->vals[0]);

    dwError = VmDirAllocateAndCopyMemory(
            pszLocalUsn, localUsnStrlen, (PVOID*)&pAttr->vals[0].lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr->vals[0].lberbv.bv_len = localUsnStrlen;
    pAttr->vals[0].bOwnBvVal = TRUE;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
