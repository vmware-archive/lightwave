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

DWORD
VmDirLoadIndex(
    BOOLEAN bFirstboot
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;
    PVDIR_SCHEMA_AT_DESC*   ppATDescList = NULL;
    PVDIR_INDEX_CFG         pIndexCfg = NULL;

    if (bFirstboot)
    {
        // Firstboot should use only the default indices
        // Nothing to load
        goto cleanup;
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaAttrList(pSchemaCtx, &ppATDescList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Identify non-default indices by their searchFlags and open them
    for (i = 0; ppATDescList[i]; i++)
    {
        if ((ppATDescList[i]->dwSearchFlags & 1) &&
                !VmDirIndexIsDefault(ppATDescList[i]->pszName))
        {
            dwError = VmDirCustomIndexCfgInit(ppATDescList[i], &pIndexCfg);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirIndexOpen(pIndexCfg);
            BAIL_ON_VMDIR_ERROR(dwError);
            pIndexCfg = NULL;
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppATDescList);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}
