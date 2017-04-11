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
 * Filename: replentry.c
 */

#include "includes.h"

static DWORD _VmDirGetLastIndex(UINT64 *index, UINT32 *term);

/*
 * This is to create entry Id for Raft log entry, which is within the same
 * MDB transaction for its associated (external) LDAP Add
 * One MDB transaction only has one Raft log, which might include multiple
 * LDAP transaction (if user defined transaction feature to be implemented)
 */
static
ENTRYID
VmDirRaftLogEntryId(unsigned long long LogIndex)
{
    return LOG_ENTRY_EID_PREFIX | LogIndex;
}

BOOLEAN
_VmDirRaftPeerIsReady(PCSTR pPeerHostName)
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;
    BOOLEAN bReady = FALSE;

    for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext )
    {
        if (pPeerProxy->isDeleted) // skip deleted peer
        {
            continue;
        }
        if (VmDirStringCompareA(pPeerProxy->raftPeerHostname, pPeerHostName, FALSE)==0)
        {
            break;
        }
    }

    if (pPeerProxy && (pPeerProxy->proxy_state == RPC_IDLE || pPeerProxy->proxy_state == RPC_BUSY))
    {
        bReady = TRUE;
    }

    return bReady;
}

//Create the initial persistent state
DWORD
VmDirInitRaftPsState(
VOID
)
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    PSTR pszLogEntryDn = NULL;

    PSTR ppContex[] = { ATTR_OBJECT_CLASS,  "vmwDirCfg",
                        ATTR_CN, RAFT_CONTEXT_CONTAINER_NAME,
                        NULL };

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppContex, RAFT_CONTEXT_DN, RAFT_CONTEXT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    PSTR ppLogs[] = { ATTR_OBJECT_CLASS,  "vmwDirCfg",
                      ATTR_CN, RAFT_LOGS_CONTAINER_NAME,
                      NULL };
    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppLogs, RAFT_LOGS_CONTAINER_DN, RAFT_LOGS_CONTAINER_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);


    PSTR ppPersisteState[] = { ATTR_OBJECT_CLASS, OC_CLASS_RAFT_PERSIST_STATE,
                               ATTR_CN, RAFT_PERSIST_STATE_NAME,
                               ATTR_RAFT_LAST_APPLIED, "0",
                               ATTR_RAFT_FIRST_LOGINDEX, "1",
                               ATTR_RAFT_TERM, "1",
                               ATTR_RAFT_VOTEDFOR_TERM, "0",
                               NULL };
    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppPersisteState, RAFT_PERSIST_STATE_DN, RAFT_PERSIST_STATE_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLogEntryDn);
    if (dwError==0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirInitRaftPsState: completed; currentTerm %d cluster size %d commitIndex %llu lastApplied %llu",
                   gRaftState.currentTerm, gRaftState.clusterSize, gRaftState.commitIndex, gRaftState.lastApplied);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirInitRaftPsState: error %d", dwError);
    goto cleanup;
}

DWORD
_VmDirLoadRaftState(
    VOID
)
{
    DWORD dwError = 0;
    PSTR  pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_RAFT_LOG logEntry = {0};

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    //the follow gRaftState values are loaded from the backend.
    dwError = VmDirSimpleEqualFilterInternalSearch(RAFT_PERSIST_STATE_DN, LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS, OC_CLASS_RAFT_PERSIST_STATE, &entryArray);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "internalSearch; dn %s", RAFT_PERSIST_STATE_DN);

    if (entryArray.iSize == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "no entry found; dn %s", RAFT_PERSIST_STATE_DN);
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_TERM, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "cannot find attr %s", ATTR_RAFT_TERM);
    }
    gRaftState.currentTerm = VmDirStringToIA((PCSTR)pAttr->vals[0].lberbv.bv_val);

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_LAST_APPLIED, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "cannot find attr %s", ATTR_RAFT_LAST_APPLIED);
    }
    gRaftState.lastApplied = VmDirStringToLA(pAttr->vals[0].lberbv.bv_val, NULL, 10);

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_FIRST_LOGINDEX, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "cannot find attr %s", ATTR_RAFT_FIRST_LOGINDEX);
    }
    gRaftState.firstLogIndex = VmDirStringToLA(pAttr->vals[0].lberbv.bv_val, NULL, 10);

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_VOTEDFOR_TERM, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "cannot find attr %s", ATTR_RAFT_VOTEDFOR_TERM);
    }
    gRaftState.votedForTerm = VmDirStringToIA((PCSTR)pAttr->vals[0].lberbv.bv_val);

    if (gRaftState.votedForTerm > 0)
    {
        pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_VOTEDFOR, entryArray.pEntry);
        if (pAttr == NULL)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "cannot find attr %s", ATTR_RAFT_VOTEDFOR);
        } else
        {
            dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s", pAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError)
        }
    }

    dwError = _VmDirFetchLogEntry(gRaftState.lastApplied, &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    gRaftState.commitIndex = logEntry.index;
    gRaftState.commitIndexTerm = logEntry.term;

    gRaftState.cmd = ExecNone;
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.

    dwError = _VmDirGetLastIndex(&gRaftState.lastLogIndex, &gRaftState.lastLogTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    gRaftState.initialized = TRUE;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    _VmDirChgLogFree(&logEntry);

    if (dwError==0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirLoadRaftState: completed; currentTerm %d commitIndex %llu lastApplied %llu",
                   gRaftState.currentTerm, gRaftState.commitIndex, gRaftState.lastApplied);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLoadRaftState: error %d; %s", dwError, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

DWORD
_VmDirUpdateRaftPsState(
    int term,
    BOOLEAN updateVotedForTerm,
    UINT32 votedForTerm,
    PVDIR_BERVALUE pVotedFor,
    UINT64 lastApplied,
    UINT64 firstLog
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    CHAR pszTerm[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    CHAR pszLastApplied[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    CHAR pszFirstLog[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    VDIR_BERVALUE berTerm = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE berLastApplied = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE berFirstLog = VDIR_BERVALUE_INIT;
    VDIR_OPERATION ldapOp = {0};
    PSTR pszLocalErrorMsg = NULL;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirInitStackOperation");

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    if (term > 0)
    {
        dwError = VmDirStringPrintFA(pszTerm , sizeof(pszTerm), "%d", term );
        BAIL_ON_VMDIR_ERROR(dwError);

        berTerm.lberbv.bv_val = pszTerm;
        berTerm.lberbv.bv_len = VmDirStringLenA(pszTerm);

        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_TERM, &berTerm);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace term");
    }

    if (updateVotedForTerm)
    {
        dwError = VmDirStringPrintFA(pszTerm , sizeof(pszTerm), "%d", votedForTerm );
        BAIL_ON_VMDIR_ERROR(dwError);

        berTerm.lberbv.bv_val = pszTerm;
        berTerm.lberbv.bv_len = VmDirStringLenA(pszTerm);

        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_VOTEDFOR_TERM, &berTerm);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace updateVotedForTerm");
    }

    if (pVotedFor && pVotedFor->lberbv.bv_len > 0)
    {
        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_VOTEDFOR, pVotedFor);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace pVotedFor");
    }

    if (lastApplied > 0)
    {
        dwError = VmDirStringPrintFA(pszLastApplied , sizeof(pszLastApplied), "%llu", lastApplied);
        BAIL_ON_VMDIR_ERROR(dwError);

        berLastApplied.lberbv.bv_val = pszLastApplied;
        berLastApplied.lberbv.bv_len = VmDirStringLenA(pszLastApplied);

        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_LAST_APPLIED, &berLastApplied);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace lastApplied");
    }

    if (firstLog > 0)
    {
        dwError = VmDirStringPrintFA(pszFirstLog , sizeof(pszFirstLog), "%llu", firstLog);
        BAIL_ON_VMDIR_ERROR(dwError);

        berFirstLog.lberbv.bv_val = pszFirstLog;
        berFirstLog.lberbv.bv_len = VmDirStringLenA(pszFirstLog);

        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_FIRST_LOGINDEX, &berFirstLog);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace firstLogIndex");
    }

    ldapOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirInternalModifyEntry");

cleanup:
    VmDirFreeOperationContent(&ldapOp);

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    assert(dwError==0); //Raft cannot garantee safety if persist state cannot be updated.

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirUpdateRaftPsState: %s error %d term %d PsState.currentTerm %d; server role %d",
                    VDIR_SAFE_STRING(pszLocalErrorMsg), dwError, term, gRaftState.currentTerm, gRaftState.role);
    goto cleanup;
}

DWORD
VmDirAddRaftEntry(PVDIR_SCHEMA_CTX pSchemaCtx, PVDIR_RAFT_LOG pLogEntry, PVDIR_OPERATION pOp)
{
    DWORD dwError = 0;
    char logEntryDn[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    VDIR_ENTRY raftEntry = {0};
    VDIR_OPERATION modOp = {0};
    char termStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char logIndexStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char objectGuidStr[VMDIR_GUID_STR_LEN];
    VDIR_BERVALUE bvIndexApplied = VDIR_BERVALUE_INIT;
    uuid_t guid;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirAddRaftEntry: log index %llu", pLogEntry->index);

    dwError = VmDirStringPrintFA(logEntryDn, sizeof(logEntryDn), "%s=%llu,%s",
                ATTR_CN, pLogEntry->index, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(termStr, sizeof(termStr), "%d", pLogEntry->term);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logIndexStr, sizeof(logIndexStr), "%llu", pLogEntry->index);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppLogEntry[] = {ATTR_DN, logEntryDn,
                             ATTR_CN, logIndexStr,
                             ATTR_OBJECT_CLASS,  OC_CLASS_RAFT_LOG_ENTRY,
                             ATTR_RAFT_LOGINDEX, logIndexStr,
                             ATTR_RAFT_TERM, termStr,
                             NULL };
        dwError = AttrListToEntry(pSchemaCtx, logEntryDn, ppLogEntry, &raftEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetParentDN(&raftEntry.dn, &raftEntry.pdn );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirUuidGenerate (&guid);
    VmDirUuidToStringLower(&guid, objectGuidStr, sizeof(objectGuidStr));

    dwError = VmDirAllocateStringA(objectGuidStr, &raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(&raftEntry, ATTR_OBJECT_GUID, raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddBervArrayAttribute(&raftEntry, ATTR_RAFT_LOG_ENTRIES, &pLogEntry->packRaftLog, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    raftEntry.eId = VmDirRaftLogEntryId(pLogEntry->index);
    dwError = pOp->pBEIF->pfnBEEntryAdd( pOp->pBECtx, &raftEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCloneStackOperation(pOp, &modOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    bvIndexApplied.lberbv.bv_val  = logIndexStr;
    bvIndexApplied.lberbv.bv_len = VmDirStringLenA(logIndexStr);

    dwError = VmDirAddModSingleAttributeReplace(&modOp, RAFT_PERSIST_STATE_DN,
                                                ATTR_RAFT_LAST_APPLIED, &bvIndexApplied);
    BAIL_ON_VMDIR_ERROR(dwError);

    modOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalModifyEntry(&modOp);

    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (modOp.pBECtx)
    {
        modOp.pBECtx->pBEPrivate = NULL; //Make sure that calls commit/abort only once.
    }
    VmDirFreeOperationContent(&modOp);
    VmDirFreeEntryContent(&raftEntry);
    return dwError;

error:
    goto cleanup;
}

DWORD
_VmDirLogLookup(
    unsigned long long logIndex,
    UINT32 logTerm,
    BOOLEAN *pbLogFound,
    BOOLEAN *pbTermMatch
)
{
    DWORD dwError = 0;
    PSTR  pszLocalErrorMsg = NULL;
    VDIR_RAFT_LOG logEntry = {0};
    BOOLEAN bLogFound = FALSE;
    BOOLEAN bTermMatch = FALSE;

    dwError = _VmDirFetchLogEntry(logIndex, &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (logEntry.index > 0)
    {
        bLogFound = TRUE;
        if (logEntry.term == logTerm)
        {
            bTermMatch = TRUE;
        }
    }

    *pbLogFound = bLogFound;
    *pbTermMatch = bTermMatch;

cleanup:
    _VmDirChgLogFree(&logEntry);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

//Remove all logs with index >= startLogIndex
DWORD
_VmDirDeleteAllLogs(unsigned long long startLogIndex)
{
    DWORD dwError = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    char filterStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    int i = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    UINT64 logIndex = 0;

    dwError = VmDirStringPrintFA(filterStr, sizeof(filterStr), "(%s>=%llu)",
                                 ATTR_RAFT_LOGINDEX, startLogIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFilterInternalSearch(RAFT_LOGS_CONTAINER_DN, LDAP_SCOPE_ONE, filterStr, 0, NULL, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < entryArray.iSize; i++)
    {
        pAttr = VmDirFindAttrByName(&(entryArray.pEntry[i]), ATTR_RAFT_LOGINDEX);
        if (!pAttr)
        {
            //This indicate the corruption of the log entry data.
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirDeleteAllLogs invalid log entry, logIdx %llu", logIndex);
            assert(0);
        }

        logIndex = VmDirStringToLA(pAttr->vals[0].lberbv.bv_val, NULL, 10);
        if (logIndex <= gRaftState.lastApplied)
        {
             /* This shouldn't occur. If it does, then there would be a bug, and the consistency might be 
              * compromised. We should investigate the sequence of events on how to get here.
              */
             VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirDeleteAllLogs attempt to delete log already applied, logIdx %llu", logIndex);
             assert(0);
        }

        dwError = _VmdirDeleteLog(entryArray.pEntry[i].dn.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    goto cleanup;
}

DWORD
_VmDirPersistLog(PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    char logEntryDn[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    char termStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char logIndexStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    PSTR  pszLocalErrorMsg = NULL;
    VDIR_OPERATION ldapOp = {0};

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_ADD, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    dwError = VmDirStringPrintFA(logEntryDn, sizeof(logEntryDn), "%s=%llu,%s",
                ATTR_CN, pLogEntry->index, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.reqDn.lberbv.bv_val = logEntryDn;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(logEntryDn);

    dwError = VmDirStringPrintFA(termStr, sizeof(termStr), "%d", pLogEntry->term);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logIndexStr, sizeof(logIndexStr), "%llu", pLogEntry->index);
    BAIL_ON_VMDIR_ERROR(dwError);

    PSTR ppLogEntry[] = { ATTR_CN, logIndexStr,
                          ATTR_OBJECT_CLASS,  OC_CLASS_RAFT_LOG_ENTRY,
                          ATTR_RAFT_LOGINDEX, logIndexStr,
                          ATTR_RAFT_TERM, termStr,
                          NULL };

    dwError = AttrListToEntry(pSchemaCtx, logEntryDn, ppLogEntry, ldapOp.request.addReq.pEntry);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "_VmDirPersistLog: AttrListToEntry failed %s error %d",
                                 logEntryDn, dwError);

    ldapOp.request.addReq.pEntry->eId = VmDirRaftLogEntryId(pLogEntry->index);
    dwError = VmDirEntryAddBervArrayAttribute(ldapOp.request.addReq.pEntry, ATTR_RAFT_LOG_ENTRIES, &pLogEntry->packRaftLog, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalAddEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "_VmDirPersistLog: VmDirInternalAddEntry failed %s error %d",
                                 logEntryDn, dwError);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirPersistLog: succeeded %s", logEntryDn);
cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VmDirFreeOperationContent(&ldapOp);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

DWORD
_VmDirFetchLogEntry(unsigned long long logIndex, PVDIR_RAFT_LOG pLogEntry, int from)
{
    DWORD dwError = 0;
    PSTR  pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    char filterStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirFetchLogEntry entering with logIndex %llu", logIndex);
    dwError = VmDirStringPrintFA(filterStr, sizeof(filterStr), "(%s=%llu)", ATTR_RAFT_LOGINDEX, logIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFilterInternalSearch(RAFT_LOGS_CONTAINER_DN, LDAP_SCOPE_ONE, filterStr, 0, NULL, &entryArray);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
          "_VmDirFetchLogEntry: internalSearch error %d filter %s dn", dwError, filterStr, RAFT_LOGS_CONTAINER_DN);

    if (entryArray.iSize != 1)
    {
       goto cleanup;
    }

    pAttr = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_RAFT_LOG_ENTRIES);
    if (pAttr==NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
          "_VmDirFetchLogEntry: missing attr %s in with logIndex %llu error %d",
          ATTR_RAFT_LOG_ENTRIES, logIndex,  dwError);
    }

    pLogEntry->packRaftLog.lberbv_len = pAttr->vals[0].lberbv.bv_len;
    dwError = VmDirAllocateAndCopyMemory(pAttr->vals[0].lberbv.bv_val, pAttr->vals[0].lberbv.bv_len,
                                         (PVOID*)&(pLogEntry->packRaftLog.lberbv_val));
    BAIL_ON_VMDIR_ERROR(dwError);
    pLogEntry->packRaftLog.bOwnBvVal = TRUE;

    dwError = _VmDirUnpackLogEntry(pLogEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirFetchLogEntry: fetched log entry with logIndex %llu line %d", logIndex, from);

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    _VmDirChgLogFree(pLogEntry);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s line %d", VDIR_SAFE_STRING(pszLocalErrorMsg), from);
    goto cleanup;
}

DWORD
_VmdirDeleteLog(PSTR pDn)
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_OPERATION ldapOp = {0};
    DeleteReq *dr = NULL;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_DELETE, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    ldapOp.reqDn.lberbv.bv_val = pDn;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(pDn);

    dr = &(ldapOp.request.deleteReq);

    dwError = VmDirAllocateBerValueAVsnprintf(&(dr->dn), "%s", ldapOp.reqDn.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalDeleteEntry(&ldapOp);
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR( dwError );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmdirDeleteLog deleted log %s ...", pDn);

cleanup:
    VmDirFreeOperationContent(&ldapOp);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmdirDeleteLog: entry %s error %d", pDn,  dwError);
    goto cleanup;
}

DWORD
_VmDirGetPrevLogArgs(unsigned long long *pPrevIndex, UINT32 *pPrevTerm, UINT64 startIndex, int line)
{
    DWORD dwError = 0;
    UINT64 index = 0;
    VDIR_RAFT_LOG prevLogEntry = {0};
    UINT64 prevIndex = 0;
    UINT64 firstLogIndex = gRaftState.firstLogIndex;
    UINT32 prevTerm = 0;

    for (index=startIndex; index >= firstLogIndex; index--)
    {
        dwError = _VmDirFetchLogEntry(index, &prevLogEntry, line);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (prevLogEntry.index > 0)
        {
            prevIndex = prevLogEntry.index;
            prevTerm = prevLogEntry.term;
            break;
        }
    }
    *pPrevIndex = prevIndex;
    *pPrevTerm = prevTerm;

cleanup:
    _VmDirChgLogFree(&prevLogEntry);
    return dwError;
error:
    goto cleanup;
}

DWORD
_VmDirGetNextLog(UINT64 startIndex, UINT64 endIndex, PVDIR_RAFT_LOG pNextLogEntry, int line)
{
    DWORD dwError = 0;
    UINT64 index = 0;

    for (index=startIndex; pNextLogEntry->index==0 && index <= startIndex; index++)
    {
        dwError = _VmDirFetchLogEntry(index, pNextLogEntry, line);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

VOID
_VmDirClearProxyLogReplicatedInLock()
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;

    for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
    {
        if (pPeerProxy->isDeleted)
        {
            continue;
        }
        pPeerProxy->bLogReplicated = FALSE;
    }
}

DWORD
_VmDirGetAppendEntriesConsensusCountInLock()
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;
    int consensusCnt = 1;

    for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
    {
        if (pPeerProxy->isDeleted)
        {
            continue;
        }
        if (pPeerProxy->bLogReplicated)
        {
            consensusCnt++;
        }
    }
    return consensusCnt;
}

DWORD
_VmDirPeersConnectedInLock()
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;
    DWORD peersConnected = 0;

    for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
    {
        if (pPeerProxy->isDeleted || pPeerProxy->proxy_state == RPC_DISCONN ||
            pPeerProxy->proxy_state == PENDING_ADD)
        {
            continue;
        }
        peersConnected++;
    }

    return peersConnected;
}

DWORD
_VmDirPeersIdleInLock()
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;
    DWORD peersIdle = 0;

    for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
    {
        if (pPeerProxy->isDeleted)
        {
           continue;
        }
        if (pPeerProxy->proxy_state == RPC_IDLE)
        {
            peersIdle++;
        }
    }

    return peersIdle;
}

DWORD
_VmDirPackLogEntry(PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    int iSize = RAFT_LOG_HEADER_LEN + pLogEntry->chglog.lberbv_len;
    unsigned char *writer = NULL;

    dwError = VmDirAllocateMemory(iSize, (PVOID*)&(pLogEntry->packRaftLog.lberbv_val));
    BAIL_ON_VMDIR_ERROR(dwError);
    pLogEntry->packRaftLog.bOwnBvVal = TRUE;

    pLogEntry->packRaftLog.lberbv_len = iSize;

    writer = pLogEntry->packRaftLog.lberbv_val;
    _VmDirEncodeUINT64(&writer, pLogEntry->index);
    _VmDirEncodeUINT32(&writer, pLogEntry->term);
    _VmDirEncodeUINT64(&writer, pLogEntry->entryId);
    _VmDirEncodeUINT32(&writer, pLogEntry->requestCode);
    if (pLogEntry->chglog.lberbv_len > 0)
    {
        memcpy(writer, pLogEntry->chglog.lberbv_val, pLogEntry->chglog.lberbv_len);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
_VmDirUnpackLogEntry(PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    unsigned char *writer = pLogEntry->packRaftLog.lberbv_val;

    if (pLogEntry->packRaftLog.lberbv_val == NULL ||
        pLogEntry->packRaftLog.lberbv_len < RAFT_LOG_HEADER_LEN)
    {
         dwError = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    pLogEntry->index = _VmDirDecodeUINT64(&writer);
    pLogEntry->term = _VmDirDecodeUINT32(&writer);
    pLogEntry->entryId = _VmDirDecodeUINT64(&writer);
    pLogEntry->requestCode = _VmDirDecodeUINT32(&writer);
    if (pLogEntry->packRaftLog.lberbv_len > RAFT_LOG_HEADER_LEN)
    {
        pLogEntry->chglog.lberbv_len = pLogEntry->packRaftLog.lberbv_len - RAFT_LOG_HEADER_LEN;
        dwError = VmDirAllocateMemory(pLogEntry->chglog.lberbv_len, (PVOID*)&pLogEntry->chglog.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        memcpy(pLogEntry->chglog.lberbv_val, writer, pLogEntry->chglog.lberbv_len);
        pLogEntry->chglog.bOwnBvVal = TRUE;
    } else
    {
       pLogEntry->chglog.lberbv_val = NULL;
       pLogEntry->chglog.lberbv_len = 0;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

VOID
_VmDirEncodeUINT32(
    unsigned char ** ppbuf,
    UINT32 value)
{
    int i = 0;

    for (i=3; i>=0; i--)
    {
      **ppbuf = value >> (i<<3);
      (*ppbuf)++;
    }
}

UINT32
_VmDirDecodeUINT32(
    unsigned char ** ppbuf
)
{
    UINT32 value = 0;
    UINT32 j = 0;
    int i = 0;

    for (i=3; i>=0; i--)
    {
      j = **ppbuf;
      value |= (j << (i<<3));
      (*ppbuf)++;
    }
    return value;
}

VOID
_VmDirEncodeUINT64(
    unsigned char ** ppbuf,
    UINT64 value)
{
    int i = 0;
    for (i=7; i>=0; i--)
    {
      **ppbuf = value >> (i<<3);
      (*ppbuf)++;
    }
}

UINT64
_VmDirDecodeUINT64(
    unsigned char ** ppbuf
)
{
    UINT64 value = 0;
    UINT64 j = 0;
    int i = 0;

    for (i=7; i>=0; i--)
    {
      j = **ppbuf;
      value |= (j << (i<<3));
      (*ppbuf)++;
    }
    return value;
}

VOID
_VmDirChgLogFree(
    PVDIR_RAFT_LOG chgLog
)
{
    if (chgLog == NULL)
    {
        return;
    }
    VmDirFreeBervalContent(&chgLog->packRaftLog);
    VmDirFreeBervalContent(&chgLog->chglog);
    memset(chgLog, 0, sizeof(VDIR_RAFT_LOG));
}

static
DWORD
_VmDirGetLastIndex(UINT64 *index, UINT32 *term)
{
    DWORD dwError = 0;
    int i = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_ATTRIBUTE pAttr = NULL;
    char filterStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    UINT64 lastLogIndex = 0;
    UINT64 curLogIndex = 0;
    VDIR_RAFT_LOG logEntry = {0};

    dwError = VmDirStringPrintFA(filterStr, sizeof(filterStr), "(%s>=%llu)",
                                 ATTR_RAFT_LOGINDEX, gRaftState.lastApplied);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFilterInternalSearch(RAFT_LOGS_CONTAINER_DN, LDAP_SCOPE_ONE, filterStr, 0, NULL, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < entryArray.iSize; i++)
    {
        pAttr = VmDirFindAttrByName(&(entryArray.pEntry[i]), ATTR_RAFT_LOGINDEX);
        curLogIndex = VmDirStringToLA(pAttr->vals[0].lberbv.bv_val, NULL, 10);
        if (curLogIndex > lastLogIndex)
        {
            lastLogIndex = curLogIndex;
        }
    }

    if (lastLogIndex == 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirFetchLogEntry(lastLogIndex, &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    *index = lastLogIndex;
    *term = logEntry.term;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    _VmDirChgLogFree(&logEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirGetLastIndex: error %d", dwError);
    goto cleanup;
}

DWORD
_VmDirGetLogTerm(UINT64 index, UINT32 *term)
{
     DWORD dwError = 0;
     VDIR_RAFT_LOG logEntry = {0};

     dwError = _VmDirFetchLogEntry(index, &logEntry, __LINE__);
     BAIL_ON_VMDIR_ERROR(dwError);

     *term = logEntry.term;

cleanup:
    _VmDirChgLogFree(&logEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirGetLogTerm error %d", dwError);
    goto cleanup;
}

DWORD
_VmDirRaftLoadGlobals(PSTR *ppszLocalErrorMsg)
{
    DWORD dwError = 0;
    PSTR pszDCAccountDn = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PSTR pszDomainName = NULL;
    PSTR pzaName = NULL;
    PSTR pszHostname = NULL;
    VDIR_BERVALUE dcContainerDNrdn = VDIR_BERVALUE_INIT;
    PSTR pszLocalErrorMsg = NULL;

    dwError = VmDirRegReadDCAccountDn(&pszDCAccountDn);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirRegReadDCAccountDn failed");

    dwError = VmDirSimpleEqualFilterInternalSearch(pszDCAccountDn, LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS, OC_COMPUTER, &entryArray);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
           "VmDirSimpleEqualFilterInternalSearch failed on DN %s", pszDCAccountDn);

    if (entryArray.iSize != 1)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
           "VmDirSimpleEqualFilterInternalSearch failed to get exactly one entry on %s", pszDCAccountDn);
    }

    dwError = VmDirAllocateBerValueAVsnprintf(&gVmdirServerGlobals.dcAccountDN, "%s", pszDCAccountDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirNormalizeDNWrapper(&gVmdirServerGlobals.dcAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDomainDNToName(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRdn(&gVmdirServerGlobals.dcAccountDN, &dcContainerDNrdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&dcContainerDNrdn, &pzaName, &pszHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateBerValueAVsnprintf(&gVmdirServerGlobals.dcAccountUPN, "%s@%s", pszHostname, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.hostname, "%s", pszHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirRaftLoadGlobals: successfully loaded instance specific globals");

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeBervalContent(&dcContainerDNrdn);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccountDn);
    VMDIR_SAFE_FREE_STRINGA(pzaName);
    VMDIR_SAFE_FREE_STRINGA(pszHostname);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    return dwError;

error:
    *ppszLocalErrorMsg = pszLocalErrorMsg;
    goto cleanup;
}
