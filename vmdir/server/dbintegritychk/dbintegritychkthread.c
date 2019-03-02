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

static
DWORD
_VmDirDBIntegrityCheckThreadFun(
    PVOID    pArg
    );

DWORD
VmDirDBIntegrityCheckCreateThread(
    VOID
    )
{
    DWORD                dwError = 0;
    PVDIR_THREAD_INFO    pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            pThrInfo->bJoinThr,
            _VmDirDBIntegrityCheckThreadFun,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

static
DWORD
_VmDirDBIntegrityCheckThreadFun(
    PVOID    pArg
    )
{
    DWORD                               dwError = 0;
    DWORD                               dwDBCnt = 0;
    PSTR                                pszDBName = NULL;
    VDIR_BACKEND_CTX                    beCtx = {0};
    PVMDIR_STRING_LIST                  pDBList = NULL;
    VMDIR_DB_INTEGRITY_CHECK_JOB_CMD    command = DB_INTEGRITY_CHECK_JOB_NONE;

    beCtx.pBE = VmDirBackendSelect(NULL);

    dwError = beCtx.pBE->pfnBEBackendGetAllDBNames(&pDBList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDBIntegrityCheckGetCommand(&command);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (command == DB_INTEGRITY_CHECK_SUBDB)
    {
        dwError = VmDirDBIntegrityCheckGetDBName(&pszDBName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirDBIntegrityCheckAllocateJobPerDB(1);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirDBIntegrityCheckPopulateJobPerDB(pszDBName);
        BAIL_ON_VMDIR_ERROR(dwError);

        //'0' first index - since there is only one DB to check.
        dwError = VmDirMDBSubDBIntegrityCheck(0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (command == DB_INTEGRITY_CHECK_ALL)
    {
        dwError = VmDirDBIntegrityCheckAllocateJobPerDB(pDBList->dwCount);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwDBCnt = 0; dwDBCnt < pDBList->dwCount; dwDBCnt++)
        {
            dwError = VmDirDBIntegrityCheckPopulateJobPerDB((PSTR)pDBList->pStringList[dwDBCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirMDBSubDBIntegrityCheck(dwDBCnt);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VmDirDBIntegrityCheckComplete(pDBList, DB_INTEGRITY_CHECK_JOB_COMPLETE);
    pDBList = NULL; //transfer ownership

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    VmDirStringListFree(pDBList);
    return dwError;

error:
    VmDirDBIntegrityCheckComplete(NULL, DB_INTEGRITY_CHECK_JOB_FAILED);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
