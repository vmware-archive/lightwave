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
VmDirRESTGetStrParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PSTR*                   ppszVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !ppszVal)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? VMDIR_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pszVal, ppszVal);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
VmDirRESTGetIntParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    int*                    piVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !piVal)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? VMDIR_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        *piVal = VmDirStringToIA(pszVal);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
VmDirRESTGetBoolParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    BOOLEAN*                pbVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !pbVal)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? VMDIR_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA(pszVal, "true", FALSE) == 0)
    {
        *pbVal = TRUE;
    }
    else if (VmDirStringCompareA(pszVal, "false", FALSE) == 0)
    {
        *pbVal = FALSE;
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
VmDirRESTGetStrListParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PVMDIR_STRING_LIST*     ppValList,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !ppValList)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? VMDIR_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirStringToTokenList(pszVal, ",", ppValList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
VmDirRESTGetLdapSearchParams(
    PVDIR_REST_OPERATION    pRestOp,
    int*                    piScope,
    PVDIR_FILTER*           ppFilter,
    PVDIR_BERVALUE*         ppbvAttrs,
    PVDIR_LDAP_CONTROL*     ppPagedResultsCtrl
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszScope = NULL;
    PSTR    pszFilter = NULL;
    PVMDIR_STRING_LIST  pAttrs = NULL;
    int     iPageSize = 0;
    PSTR    pszPagedResultsCookie = NULL;
    int                 scope = LDAP_SCOPE_BASE;
    PVDIR_FILTER        pFilter = NULL;
    PVDIR_BERVALUE      pbvAttrs = NULL;
    PVDIR_LDAP_CONTROL  pPagedResultsCtrl = NULL;

    if (!pRestOp || !piScope || !ppFilter || !ppbvAttrs || !ppPagedResultsCtrl)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRESTGetStrParam(pRestOp, "scope", &pszScope, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetStrParam(pRestOp, "filter", &pszFilter, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetStrListParam(pRestOp, "attrs", &pAttrs, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetIntParam(pRestOp, "page_size", &iPageSize, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetStrParam(
            pRestOp, "paged_results_cookie", &pszPagedResultsCookie, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    // scope
    if (!IsNullOrEmptyString(pszScope))
    {
        if (VmDirStringCompareA("base", pszScope, FALSE) == 0)
        {
            scope = LDAP_SCOPE_BASE;
        }
        else if (VmDirStringCompareA("one", pszScope, FALSE) == 0 ||
                 VmDirStringCompareA("onelevel", pszScope, FALSE) == 0)
        {
            scope = LDAP_SCOPE_ONELEVEL;
        }
        else if (VmDirStringCompareA("sub", pszScope, FALSE) == 0 ||
                 VmDirStringCompareA("subtree", pszScope, FALSE) == 0)
        {
            scope = LDAP_SCOPE_SUBTREE;
        }
        else
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    // filter
    if (IsNullOrEmptyString(pszFilter))
    {
        dwError = VmDirAllocateStringA("(objectclass=*)", &pszFilter);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = StrFilterToFilter(pszFilter, &pFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    // attrs
    if (pAttrs)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BERVALUE) * (pAttrs->dwCount + 1),
                (PVOID*)&pbvAttrs);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pAttrs->dwCount; i++)
        {
            dwError = VmDirStringToBervalContent(
                    pAttrs->pStringList[i], &pbvAttrs[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    // page results control
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_CONTROL), (PVOID*)&pPagedResultsCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!IsNullOrEmptyString(pszPagedResultsCookie))
    {
        dwError = VmDirStringNCpyA(
                pPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                VMDIR_PS_COOKIE_LEN,
                pszPagedResultsCookie,
                VMDIR_PS_COOKIE_LEN - 1);
        BAIL_ON_VMDIR_ERROR(dwError);

        pPagedResultsCtrl->value.pagedResultCtrlVal.pageSize = (DWORD)iPageSize;
    }
    else if (iPageSize)
    {
        pPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
        pPagedResultsCtrl->value.pagedResultCtrlVal.pageSize = (DWORD)iPageSize;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pPagedResultsCtrl);
    }

    *piScope = scope;
    *ppFilter = pFilter;
    *ppbvAttrs = pbvAttrs;
    *ppPagedResultsCtrl = pPagedResultsCtrl;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszScope);
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    VMDIR_SAFE_FREE_MEMORY(pszPagedResultsCookie);
    VmDirStringListFree(pAttrs);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    DeleteFilter(pFilter);
    VMDIR_SAFE_FREE_MEMORY(pbvAttrs);
    VMDIR_SAFE_FREE_MEMORY(pPagedResultsCtrl);
    goto cleanup;
}

DWORD
VmDirRESTRenameParamKey(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszOldKey,
    PCSTR                   pszNewKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszNewKeyCp = NULL;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (IsNullOrEmptyString(pszOldKey) || IsNullOrEmptyString(pszNewKey))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapRemove(pRestOp->pParamMap, (PVOID)pszOldKey, &pair) == 0)
    {
        VMDIR_SAFE_FREE_MEMORY(pair.pKey);

        dwError = VmDirAllocateStringA(pszNewKey, &pszNewKeyCp);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                pRestOp->pParamMap, pszNewKeyCp, pair.pValue, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszNewKeyCp);
    goto cleanup;
}
