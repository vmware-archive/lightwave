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

#define VMIT_INDEX                                                       \
{                                                                        \
    {                                                                    \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_VMWITCUSTOMERNUMBER),          \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bScopeEditable, TRUE),                            \
        VMDIR_SF_INIT(.bGlobalUniq, TRUE),                               \
        VMDIR_SF_INIT(.bIsNumeric, FALSE)                                \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.pszAttrName,  ATTR_VMWITUSERGUID),                \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bScopeEditable, TRUE),                            \
        VMDIR_SF_INIT(.bGlobalUniq, TRUE),                               \
        VMDIR_SF_INIT(.bIsNumeric, FALSE)                                \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.pszAttrName, ATTR_UID),                           \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bScopeEditable, TRUE),                            \
        VMDIR_SF_INIT(.bGlobalUniq, TRUE),                               \
        VMDIR_SF_INIT(.bIsNumeric, FALSE)                                \
    },                                                                   \
    {                                                                    \
        VMDIR_SF_INIT(.pszAttrName, NULL),                               \
        VMDIR_SF_INIT(.iTypes, INDEX_TYPE_EQUALITY),                     \
        VMDIR_SF_INIT(.bScopeEditable, FALSE),                           \
        VMDIR_SF_INIT(.bGlobalUniq, FALSE),                              \
        VMDIR_SF_INIT(.bIsNumeric, FALSE)                                \
    }                                                                    \
}

// VMIT support
DWORD
VmDirIndexLibInitVMIT(
    VOID
    )
{
    static VDIR_DEFAULT_INDEX_CFG vmitIdx[] = VMIT_INDEX;

    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    for (i = 0; vmitIdx[i].pszAttrName; i++)
    {
        dwError = VmDirVMITIndexCfgInit(&vmitIdx[i], &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pIndexCfg)
        {
            dwError = VmDirIndexOpen(pIndexCfg);
            BAIL_ON_VMDIR_ERROR(dwError);
            pIndexCfg = NULL;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

DWORD
VmDirVMITIndexCfgInit(
    PVDIR_DEFAULT_INDEX_CFG pDefIdxCfg,
    PVDIR_INDEX_CFG*        ppIndexCfg
    )
{
    DWORD   dwError = 0;
    PSTR    pszScope = NULL;
    BOOLEAN bRestore = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR            pszIdxStatus = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    if (!pDefIdxCfg || !ppIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIndexCfgCreate(pDefIdxCfg->pszAttrName, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    beCtx.pBE = VmDirBackendSelect(NULL);
    if (!beCtx.pBE->pfnBEIndexExist(pIndexCfg))
    {
        VmDirFreeIndexCfg(pIndexCfg);
        goto cleanup;
    }

    pIndexCfg->bDefaultIndex = TRUE;
    pIndexCfg->bScopeEditable = pDefIdxCfg->bScopeEditable;
    pIndexCfg->bIsNumeric = pDefIdxCfg->bIsNumeric;
    pIndexCfg->iTypes = pDefIdxCfg->iTypes;

    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirIndexCfgRestoreProgress(&beCtx, pIndexCfg, &bRestore);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bRestore && pDefIdxCfg->bGlobalUniq)
    {
        dwError = VmDirAllocateStringA(PERSISTED_DSE_ROOT_DN, &pszScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                pIndexCfg->pUniqScopes, pszScope, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszScope = NULL;
    }

    dwError = VmDirIndexCfgRecordProgress(&beCtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    *ppIndexCfg = pIndexCfg;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszScope);
    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}
