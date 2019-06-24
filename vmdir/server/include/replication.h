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
 * Module Name: Replication
 *
 * Filename: replication.h
 *
 * Abstract:
 *
 * replication api
 *
 */

#ifndef __REPLICATION_H__
#define __REPLICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

extern VMDIR_REPLICATION_AGREEMENT * gVmdirReplAgrs;

#define LOCAL_PARTNER_DIR           "partner"
#define VMDIR_MDB_DATA_FILE_NAME    "data.mdb"
#define VMDIR_SWAP_DB_SUFFIX        "swap"

///////////////////////////////////////////////////////////////////////////////
// replication library initialize / shutdown
///////////////////////////////////////////////////////////////////////////////
/*
 * Initialize replication library
 */
DWORD
VmDirReplicationLibInit(
    VOID
    );

VOID
VmDirInsertRAToCache(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

DWORD
VmDirReplAgrEntryToInMemory(
    PVDIR_ENTRY                     pEntry,
    PVMDIR_REPLICATION_AGREEMENT *  ppReplAgr);

DWORD
VmDirConstructReplAgr(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PCSTR                           pszReplURI,
    PCSTR                           pszLastLocalUsnProcessed,
    PCSTR                           pszReplAgrDN,
    PVMDIR_REPLICATION_AGREEMENT *  ppReplAgr);

VOID
VmDirFreeReplicationAgreement(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

VOID
VmDirRemoveDeletedRAsFromCache();

DWORD
VmDirGetReplCycleCounter(
    VOID
    );

/*
 * Replication metrics cache
 */
DWORD
VmDirReplMetricsCacheInit(
    VOID
    );

DWORD
VmDirReplMetricsCacheAdd(
    PVDIR_ENTRY pServerEntry
    );

DWORD
VmDirReplMetricsCacheFind(
    PCSTR                       pszHostname,
    PVMDIR_REPLICATION_METRICS* ppReplMetrics
    );

DWORD
VmDirReplMetricsCacheRemove(
    PCSTR   pszHostname
    );

VOID
VmDirReplMetricsCacheShutdown(
    VOID
    );

/*
 * Replication metrics util
 */
DWORD
VmDirReplMetricsPersistCountConflictPermanent(
    PVMDIR_REPLICATION_METRICS  pReplMetrics,
    DWORD                       dwCount
    );

DWORD
VmDirReplMetricsLoadCountConflictPermanent(
    PVMDIR_REPLICATION_METRICS  pReplMetrics
    );

/*
 * UTD vector cache
 */
DWORD
VmDirUTDVectorGlobalCacheInit(
    VOID
    );

DWORD
VmDirUTDVectorGlobalCacheReplace(
    PCSTR                   pszNewUTDVector
    );

DWORD
VmDirUTDVectorGlobalCacheToString(
    PSTR*                   ppszUTDVector
    );

DWORD
VmDirUTDVectorGlobalCacheLookup(
    PCSTR                   pszInvocationId,
    USN*                    pUsn
    );

VOID
VmDirFreeUTDVectorGlobalCache(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __REPLICATION_H__ */


