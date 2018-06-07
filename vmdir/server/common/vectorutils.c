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
VmDirVectorToStr(
    PLW_HASHMAP         pMap,
    PFN_VEC_PAIR_TO_STR PairToStr,
    PSTR*               ppOutStr
    )
{
    PSTR    pszTempVectorStr = NULL;
    PSTR    pszVectorStr = NULL;
    DWORD   dwError = 0;
    BOOLEAN bFirst = TRUE;
    LW_HASHMAP_ITER  iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR  pair = {NULL, NULL};
    PVMDIR_STRING_LIST   pStrList = NULL;

    if (!pMap || !PairToStr || !ppOutStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringListInitialize(&pStrList, 32);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pMap, &iter, &pair))
    {
        if (pair.pKey && pair.pValue)
        {
            dwError = PairToStr(pair, bFirst, &pszTempVectorStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            bFirst = FALSE;

            dwError = VmDirStringListAdd(pStrList, pszTempVectorStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszTempVectorStr = NULL;
        }
    }

    dwError = VmDirTokenListToString(pStrList, "", &pszVectorStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppOutStr = pszVectorStr;
    pszVectorStr = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszVectorStr);
    VMDIR_SAFE_FREE_MEMORY(pszTempVectorStr);
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * String format <A:B,C:D,..>
 * Vector (hash map) will have key: A value B, Key: C value: D
 */
DWORD
VmDirStrtoVector(
    PCSTR               pszVector,
    PFN_VEC_STR_TO_PAIR StrToPair,
    PLW_HASHMAP         pMap
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    PSTR    pszKey = NULL;
    PSTR    pszVal = NULL;
    LW_HASHMAP_PAIR      pair = {NULL, NULL};
    PVMDIR_STRING_LIST   pStrList = NULL;

    if (!pMap || !StrToPair || !pszVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringToTokenList(pszVector, ",", &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCount = 0; dwCount < pStrList->dwCount; dwCount++)
    {
        if (pStrList->pStringList[dwCount][0] != '\0')
        {
            pszKey = VmDirStringTokA((PSTR)pStrList->pStringList[dwCount], ":", &pszVal);

            if (pszKey && pszVal)
            {
                dwError = StrToPair(pszKey, pszVal, &pair);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = LwRtlHashMapInsert(
                        pMap,
                        pair.pKey,
                        pair.pValue,
                        NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_VECTOR_STR);
            }
        }
    }

cleanup:
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
