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
VmDirIntegrityReportCreate(
    PVMDIR_INTEGRITY_REPORT*    ppReport
    )
{
    DWORD   dwError = 0;
    PVMDIR_INTEGRITY_REPORT pReport = NULL;

    if (!ppReport)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_INTEGRITY_REPORT), (PVOID*)&pReport);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pReport->pMismatchMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pReport->pMissingMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReport = pReport;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeIntegrityReport(pReport);
    goto cleanup;
}

DWORD
VmDirIntegrityReportSetPartner(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszPartner
    )
{
    DWORD   dwError = 0;

    if (!pReport || IsNullOrEmptyString(pszPartner))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_SAFE_FREE_MEMORY(pReport->pszPartner);
    dwError = VmDirAllocateStringA(pszPartner, &pReport->pszPartner);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirIntegrityReportAddMismatch(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszDn
    )
{
    DWORD   dwError = 0;
    PSTR    pszData = NULL;

    if (!pReport || IsNullOrEmptyString(pszDn))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszDn, &pszData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pReport->pMismatchMap, pszData, (PVOID)pszData, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReport->dwMismatchCnt++;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VMDIR_SAFE_FREE_MEMORY(pszData);
    goto cleanup;
}

DWORD
VmDirIntegrityReportAddMissing(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszDn
    )
{
    DWORD   dwError = 0;
    PSTR    pszData = NULL;

    if (!pReport || IsNullOrEmptyString(pszDn))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszDn, &pszData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pReport->pMissingMap, pszData, (PVOID)pszData, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReport->dwMissingCnt++;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VMDIR_SAFE_FREE_MEMORY(pszData);
    goto cleanup;
}

DWORD
VmDirIntegrityReportWriteToFile(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszFilePath
    )
{
    DWORD   dwError = 0;
    FILE*   fp = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pReport || IsNullOrEmptyString(pszFilePath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    fp = fopen(pszFilePath, "w+");
    if (!fp)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "open file %s failed %d", pszFilePath, errno);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_FILE_OPEN);
    }

    while (LwRtlHashMapIterate(pReport->pMismatchMap, &iter, &pair))
    {
        if (fprintf(fp, "I %s\n", (PSTR)pair.pKey) < 0)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }
    }

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pReport->pMissingMap, &iter, &pair))
    {
        if (fprintf(fp, "M %s\n", (PSTR)pair.pKey) < 0)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }
    }

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirIntegrityReportLoadFile(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszFilePath
    )
{
    DWORD   dwError = 0;
    FILE*   fp = NULL;
    PCSTR   pszFileName = NULL;
    PSTR    pszPartner = NULL;
    PSTR    pszTime = NULL;
    CHAR    lineBuf[VMDIR_MAX_DN_LEN + 3] = {0};

    if (!pReport || IsNullOrEmptyString(pszFilePath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszFileName = VmDirStringStrA(pszFilePath, "Integrity_");
    if (IsNullOrEmptyString(pszFileName))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "not a integrity report file %s", pszFilePath);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // note: this only works for POSIX
    if (sscanf(pszFileName, "Integrity_%ms %ms", &pszPartner, &pszTime) != 2)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "unable to get partner name from %s", pszFilePath);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirIntegrityReportSetPartner(pReport, pszPartner);
    BAIL_ON_VMDIR_ERROR(dwError);

    fp = fopen(pszFilePath, "r");
    if (!fp)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "open file %s failed %d", pszFilePath, errno);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_FILE_OPEN);
    }

    while (fgets(lineBuf, sizeof(lineBuf), fp))
    {
        // replace '\n' with '\0';
        size_t len = VmDirStringLenA(lineBuf) - 1;
        lineBuf[len] = '\0';

        if (lineBuf[0] == 'I')
        {
            // skip first two chars "I " for DN
            dwError = VmDirIntegrityReportAddMismatch(pReport, lineBuf + 2);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (lineBuf[0] == 'M')
        {
            // skip first two chars "M " for DN
            dwError = VmDirIntegrityReportAddMissing(pReport, lineBuf + 2);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // unknown type
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "failed loading entry (%s)", lineBuf);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPartner);
    VMDIR_SAFE_FREE_MEMORY(pszTime);
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirIntegrityReportRemoveNonOverlaps(
    PVMDIR_INTEGRITY_REPORT pReport,    // this gets modified
    PVMDIR_INTEGRITY_REPORT pUpdate
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pNewMismatchMap = NULL;
    PLW_HASHMAP pNewMissingMap = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pReport || !pUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pNewMismatchMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pNewMissingMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pReport->pMismatchMap, &iter, &pair))
    {
        PSTR pszDn = (PSTR)pair.pKey;
        if (LwRtlHashMapFindKey(pUpdate->pMismatchMap, NULL, pszDn) == 0)
        {
            dwError = LwRtlHashMapInsert(pNewMismatchMap, pszDn, (PVOID)pszDn, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pReport->pMissingMap, &iter, &pair))
    {
        PSTR pszDn = (PSTR)pair.pKey;
        if (LwRtlHashMapFindKey(pUpdate->pMissingMap, NULL, pszDn) == 0)
        {
            dwError = LwRtlHashMapInsert(pNewMissingMap, pszDn, (PVOID)pszDn, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    LwRtlHashMapClear(pReport->pMismatchMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pReport->pMismatchMap);
    pReport->pMismatchMap = pNewMismatchMap;
    pReport->dwMismatchCnt = LwRtlHashMapGetCount(pNewMismatchMap);

    LwRtlHashMapClear(pReport->pMissingMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pReport->pMissingMap);
    pReport->pMissingMap = pNewMissingMap;
    pReport->dwMissingCnt = LwRtlHashMapGetCount(pNewMissingMap);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);

    if (pNewMismatchMap)
    {
        LwRtlHashMapClear(pNewMismatchMap, VmDirNoopHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pNewMismatchMap);
    }
    if (pNewMissingMap)
    {
        LwRtlHashMapClear(pNewMissingMap, VmDirNoopHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pNewMissingMap);
    }
    goto cleanup;
}

VOID
VmDirFreeIntegrityReport(
    PVMDIR_INTEGRITY_REPORT pReport
    )
{
    if (pReport)
    {
        if (pReport->pMismatchMap)
        {
            LwRtlHashMapClear(pReport->pMismatchMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
            LwRtlFreeHashMap(&pReport->pMismatchMap);
        }
        if (pReport->pMissingMap)
        {
            LwRtlHashMapClear(pReport->pMissingMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
            LwRtlFreeHashMap(&pReport->pMissingMap);
        }
        VMDIR_SAFE_FREE_MEMORY(pReport->pszPartner);
        VMDIR_SAFE_FREE_MEMORY(pReport);
    }
}

VOID
VmDirFreeIntegrityReportList(
    PVDIR_LINKED_LIST   pReports
    )
{
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVMDIR_INTEGRITY_REPORT pReport = NULL;

    if (pReports)
    {
        pNode = pReports->pHead;
        while (pNode)
        {
            pReport = (PVMDIR_INTEGRITY_REPORT)pNode->pElement;
            VmDirFreeIntegrityReport(pReport);
            pNode = pNode->pNext;
        }
        VmDirFreeLinkedList(pReports);
    }
}
