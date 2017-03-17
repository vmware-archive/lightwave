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
VmDirSchemaAttrIdMapInit(
    PVDIR_SCHEMA_ATTR_ID_MAP*   ppAttrIdMap
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap = NULL;

    if (!ppAttrIdMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_ATTR_ID_MAP),
            (PVOID*)&pAttrIdMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pAttrIdMap->pStoredIds,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pAttrIdMap->pNewIds,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttrIdMap->usNextId = MAX_RESERVED_ATTR_ID_MAP + 1;

    *ppAttrIdMap = pAttrIdMap;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeSchemaAttrIdMap(pAttrIdMap);
    goto cleanup;
}

DWORD
VmDirSchemaAttrIdMapReadDB(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PVMDIR_STRING_LIST  pStringList = NULL;
    PSTR                pszBuf = NULL;
    PSTR                pszName = NULL;
    PSTR                pszId = NULL;
    USHORT              usId = 0;

    if (!pAttrIdMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    beCtx.pBE = VmDirBackendSelect(PERSISTED_DSE_ROOT_DN);

    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = beCtx.pBE->pfnBEDupKeyGetValues(
            &beCtx, ATTR_ID_MAP_KEY, &pStringList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    for (i = 0; i < pStringList->dwCount; i++)
    {
        dwError = VmDirAllocateStringA(pStringList->pStringList[i], &pszBuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszId = VmDirStringTokA(pszBuf, SCHEMA_ATTR_ID_MAP_SEP, &pszName);
        usId = (USHORT)VmDirStringToIA(pszId);

        if (VmDirSchemaAttrIdMapGetAttrId(pAttrIdMap, pszName, NULL) != 0)
        {
            dwError = VmDirAllocateStringA(pszName, &pszName);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = LwRtlHashMapInsert(pAttrIdMap->pStoredIds,
                    pszName, (PVOID)(uintptr_t)usId, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (usId >= pAttrIdMap->usNextId)
            {
                pAttrIdMap->usNextId = usId + 1;
            }
        }

        VMDIR_SAFE_FREE_MEMORY(pszBuf);
    }

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VmDirStringListFree(pStringList);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszBuf);
    goto cleanup;
}

DWORD
VmDirSchemaAttrIdMapUpdateDB(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumNewIds = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PSTR                pszMapStr = NULL;
    PVMDIR_STRING_LIST  pMapStrList = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    if (!pAttrIdMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwNumNewIds = LwRtlHashMapGetCount(pAttrIdMap->pNewIds);
    if (dwNumNewIds == 0)
    {   // No new Id
        goto cleanup;
    }

    dwError = VmDirStringListInitialize(&pMapStrList, dwNumNewIds);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pAttrIdMap->pNewIds, &iter, &pair))
    {
        dwError = VmDirAllocateStringPrintf(&pszMapStr, "%d%s%s",
                (USHORT)(uintptr_t)pair.pValue,
                SCHEMA_ATTR_ID_MAP_SEP,
                (PSTR)pair.pKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pMapStrList, pszMapStr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    beCtx.pBE = VmDirBackendSelect(PERSISTED_DSE_ROOT_DN);

    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR( dwError );
    bHasTxn = TRUE;

    dwError = beCtx.pBE->pfnBEDupKeySetValues(&beCtx, ATTR_ID_MAP_KEY, pMapStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pAttrIdMap->pNewIds, &iter, &pair))
    {
        dwError = LwRtlHashMapInsert(pAttrIdMap->pStoredIds,
                pair.pKey, pair.pValue, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapClear(pAttrIdMap->pNewIds, VmDirNoopHashMapPairFree, NULL);

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VmDirStringListFree(pMapStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;

}

DWORD
VmDirSchemaAttrIdMapGetAttrId(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PSTR                        pszAttrName,
    USHORT*                     pusAttrId
    )
{
    DWORD   dwError = 0;
    uintptr_t   pAttrId = 0;

    if (!pAttrIdMap || IsNullOrEmptyString(pszAttrName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlHashMapFindKey(pAttrIdMap->pStoredIds,
            (PVOID*)&pAttrId, pszAttrName);
    if (dwError)
    {
        dwError = LwRtlHashMapFindKey(pAttrIdMap->pNewIds,
                    (PVOID*)&pAttrId, pszAttrName);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pusAttrId)
    {
        *pusAttrId = (USHORT)pAttrId;
    }

error:
    return dwError;
}

DWORD
VmDirSchemaAttrIdMapAddNewAttr(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PSTR                        pszAttrName,
    USHORT                      usAttrId    // optional
    )
{
    DWORD   dwError = 0;
    PSTR    pszKey = 0;

    if (!pAttrIdMap || IsNullOrEmptyString(pszAttrName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirSchemaAttrIdMapGetAttrId(pAttrIdMap, pszAttrName, NULL) == 0)
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszAttrName, &pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (usAttrId)
    {
        dwError = LwRtlHashMapInsert(pAttrIdMap->pNewIds,
                pszKey, (PVOID)(uintptr_t)usAttrId, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (usAttrId >= pAttrIdMap->usNextId)
        {
            pAttrIdMap->usNextId = usAttrId + 1;
        }
    }
    else
    {
        dwError = LwRtlHashMapInsert(pAttrIdMap->pNewIds,
                pszKey, (PVOID)(uintptr_t)pAttrIdMap->usNextId, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttrIdMap->usNextId++;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszKey);
    goto cleanup;
}

static
VOID
_FreeAttrIdMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
}

VOID
VmDirSchemaAttrIdMapRemoveAllPending(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    )
{
    if (pAttrIdMap)
    {
        if (pAttrIdMap->pNewIds)
        {
            LwRtlHashMapClear(pAttrIdMap->pNewIds,
                    _FreeAttrIdMapPair, NULL);
        }
    }
}

VOID
VmDirFreeSchemaAttrIdMap(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap
    )
{
    if (pAttrIdMap)
    {
        if (pAttrIdMap->pStoredIds)
        {
            LwRtlHashMapClear(pAttrIdMap->pStoredIds,
                    _FreeAttrIdMapPair, NULL);
            LwRtlFreeHashMap(&pAttrIdMap->pStoredIds);
        }
        if (pAttrIdMap->pNewIds)
        {
            LwRtlHashMapClear(pAttrIdMap->pNewIds,
                    _FreeAttrIdMapPair, NULL);
            LwRtlFreeHashMap(&pAttrIdMap->pNewIds);
        }
        VMDIR_SAFE_FREE_MEMORY(pAttrIdMap);
    }
}
