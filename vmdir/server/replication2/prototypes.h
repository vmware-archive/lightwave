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

// thread.c

DWORD
InitializeReplicationThread(
    void);

int
VmDirFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr);

DWORD
VmDirCacheKrb5Creds(
    PCSTR pszUPN,
    PCSTR pszPwd,
    PSTR  *ppszErrorMsg
    );

VOID
VmDirPopulateInvocationIdInReplAgr(
    VOID
    );

// replentry.c
int
ReplAddEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_SCHEMA_CTX*               ppOutSchemaCtx
    );

int
ReplDeleteEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    );

int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_SCHEMA_CTX*               ppOutSchemaCtx
    );

// metrics.c
DWORD
VmDirReplMetricsInit(
    PSTR                        pszHostname,
    PSTR                        pszSite,
    PVMDIR_REPLICATION_METRICS* ppReplMetrics
    );

VOID
VmDirFreeReplMetrics(
    PVMDIR_REPLICATION_METRICS  pReplMetrics
    );

// replqueue.c
DWORD
VmDirReplicationDecodeEntryForRetry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_ENTRY                     pEntry
    );

VOID
VmDirReplicationEncodeEntryForRetry(
    PVDIR_ENTRY                     pEntry,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    );

DWORD
VmDirReplicationPushFailedEntriesToQueue(
    PVMDIR_REPLICATION_CONTEXT      pContext,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    );

DWORD
VmDirReplicationDupPageEntry(
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVMDIR_REPLICATION_PAGE_ENTRY*  ppPageEntryDup
    );

VOID
VmDirReapplyFailedEntriesFromQueue(
    PVMDIR_REPLICATION_CONTEXT      pContext
    );

VOID
VmDirReplicationClearFailedEntriesFromQueue(
    PVMDIR_REPLICATION_CONTEXT      pContext
    );

VOID
VmDirReplicationFreePageEntry(
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    );

VOID
VmDirReplicationFreePageEntryContent(
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    );

VOID
VmDirShutdownDB(
    VOID
    );

int
VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

// replcookies.c
int
VmDirReplCookieUpdate(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    struct berval *                 syncDoneCtrlVal,
    VMDIR_REPLICATION_AGREEMENT *   replAgr
    );

// dbswap.c
DWORD
VmDirSwapDB(
    PCSTR   dbHomeDir
    );

DWORD
VmDirPrepareSwapDBInfo(
    PCSTR                   pszHostName,    // partner server object cn
    PVMDIR_SWAP_DB_INFO*    ppSwapDBInfo
    );

VOID
VmDirFreeSwapDBInfo(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    );

// dbcopy.c
int
VmDirCopyRemoteDB(
    PCSTR   pszHostname,
    PCSTR   dbHomeDir
    );

// metadata.c
DWORD
VmDirReplSetAttrNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PLW_HASHMAP*        ppMetaDataMap
    );

// conflictresolution.c
int
VmDirReplResolveConflicts(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PLW_HASHMAP         pMetaDataMap
    );

#ifdef __cplusplus
}
#endif

#endif // _CFG_PROTOTYPES_H_

