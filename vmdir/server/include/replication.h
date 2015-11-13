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

#ifdef __cplusplus
}
#endif

#endif /* __REPLICATION_H__ */


