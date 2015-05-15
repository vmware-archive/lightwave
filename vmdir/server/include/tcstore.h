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
 * Module Name: Directory data store - tokyocabinet
 *
 * Filename: tcstore.h
 *
 * Abstract:
 *
 * data store api
 *
 */

#ifndef __TOKYOCABINET_H__
#define __TOKYOCABINET_H__

#ifdef __cplusplus
extern "C" {
#endif

PVDIR_BACKEND_INTERFACE
VmDirTCBEInterface (
    VOID);

DWORD
VmDirTCCurrentEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID*            pEId
    );

DWORD
VmDirTCSimpleEIdToEntry(
    ENTRYID         eId,
    PVDIR_ENTRY     pEntry
    );

DWORD
VmDirTCSimpleDNToEntry(
    PSTR            pszNormDN,
    PVDIR_ENTRY     pEntry
    );

DWORD
VmDirTCEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE lockType
    );

DWORD
VmDirTCDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE lockType
    );

DWORD
VmDirTCDNToEntryId(
    PVDIR_BACKEND_CTX           pBECtx,
    VDIR_BERVALUE*              pDn,
    ENTRYID*                    pEId
    );

DWORD
VmDirTCEntryAdd(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirTCEntryModify(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirTCEntryDelete(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirTCCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirTCCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             eId,
    PBOOLEAN            pIsLeafEntry
    );


DWORD
VmDirTCGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter
    );

DWORD
VmDirTCIndicesRebuild(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD                       dwNumIndices,
    DWORD                       dwStartEntryId,
    DWORD                       dwBatchSize
    );

DWORD
VmDirTCTxnBegin(
    PVDIR_BACKEND_CTX        pBECtx,
    VDIR_BACKEND_TXN_MODE    txnMode
    );

DWORD
VmDirTCTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    );

DWORD
VmDirTCTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    );

DWORD
VmDirTCBEInit(
    VOID
    );

DWORD
VmDirTCOpenDB(
    VOID
    );

DWORD
VmDirTCOpenIndex(
    VOID
    );

DWORD
VmDirTCShutdown(
    VOID
    );

DWORD
VmDirTCGetAttrMetaData(
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ATTRIBUTE         pAttr,
    ENTRYID                 entryId
    );

DWORD
VmDirTCGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE*  ppAttrMetaDataNode,
    int *                       pNumAttrMetaData
    );

DWORD
VmDirTCGetNextEntryID(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID*            pEID
    );

DWORD
VmDirTCGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               pUSN
    );


#ifdef __cplusplus
}
#endif

#endif /* __TOKYOCABINET_H__ */
