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



/*
 * Module Name: Directory indexer
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _CFG_PROTOTYPES_H_
#define _CFG_PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// indexcfg.c
DWORD
VmDirIndexCfgCreate(
    PCSTR               pszAttrName,
    PVDIR_INDEX_CFG*    ppIndexCfg
    );

DWORD
VmDirDefaultIndexCfgInit(
    PVDIR_DEFAULT_INDEX_CFG pDefIdxCfg,
    PVDIR_INDEX_CFG*        ppIndexCfg
    );

DWORD
VmDirIndexCfgCopy(
    PVDIR_INDEX_CFG     pIndexCfg,
    PVDIR_INDEX_CFG*    ppIndexCfgCpy
    );

DWORD
VmDirIndexCfgAddUniqueScopeMod(
    PVDIR_INDEX_CFG pIndexCfg,
    PCSTR           pszUniqScope
    );

DWORD
VmDirIndexCfgDeleteUniqueScopeMod(
    PVDIR_INDEX_CFG pIndexCfg,
    PCSTR           pszUniqScope
    );

DWORD
VmDirIndexCfgValidateUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    );

DWORD
VmDirIndexCfgApplyUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    );

DWORD
VmDirIndexCfgRevertBadUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    );

DWORD
VmDirIndexCfgStatusStringfy(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatus
    );

VOID
VmDirIndexCfgClear(
    PVDIR_INDEX_CFG pIndexCfg
    );

VOID
VmDirFreeIndexCfgMapPair(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    );

// indexingtask.c
DWORD
VmDirIndexingTaskInit(
    PVDIR_INDEXING_TASK*    ppTask
    );

DWORD
VmDirIndexingTaskCompute(
    PVDIR_INDEXING_TASK*    ppTask
    );

DWORD
VmDirIndexingTaskPopulateIndices(
    PVDIR_INDEXING_TASK pTask
    );

DWORD
VmDirIndexingTaskValidateScopes(
    PVDIR_INDEXING_TASK pTask
    );

DWORD
VmDirIndexingTaskDeleteIndices(
    PVDIR_INDEXING_TASK pTask
    );

DWORD
VmDirIndexingTaskRecordProgress(
    PVDIR_INDEXING_TASK pTask,
    PVDIR_INDEX_UPD     pIndexUpd
    );

BOOLEAN
VmDirIndexingTaskIsNoop(
    PVDIR_INDEXING_TASK pTask
    );

VOID
VmDirFreeIndexingTask(
    PVDIR_INDEXING_TASK pTask
    );

// indexingthr.c
DWORD
InitializeIndexingThread(
    VOID
    );

DWORD
VmDirIndexingThreadFun(
    PVOID   pArg
    );

// indexupd.c
DWORD
VmDirIndexUpdInit(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_UPD*    ppIndexUpd
    );

DWORD
VmDirIndexUpdCopy(
    PVDIR_INDEX_UPD     pSrcIdxUpd,
    PVDIR_INDEX_UPD     pTgtIdxUpd
    );

DWORD
VmDirIndexUpdApply(
    PVDIR_INDEX_UPD     pIndexUpd
    );

VOID
VmDirIndexUpdFree(
    PVDIR_INDEX_UPD     pIndexUpd
    );

// progress.c
DWORD
VmDirIndexCfgRecordProgress(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_CFG     pIndexCfg
    );

DWORD
VmDirIndexCfgRestoreProgress(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_CFG     pIndexCfg,
    PBOOLEAN            pbRestore
    );

// vmit.c
DWORD
VmDirIndexLibInitVMIT(
    VOID
    );

DWORD
VmDirVMITIndexCfgInit(
    PVDIR_DEFAULT_INDEX_CFG pDefIdxCfg,
    PVDIR_INDEX_CFG*        ppIndexCfg
    );

#ifdef __cplusplus
}
#endif

#endif // _CFG_PROTOTYPES_H_

