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

static int
_VmDirCompareLogIdx(
    const void * logIdx1,
    const void * logIdx2);

/*
 * This is to create entry Id for Raft log entry, which is within the same
 * MDB transaction for its associated (external) LDAP Add
 * One MDB transaction only has one Raft log, which might include multiple
 * LDAP transaction (if user defined transaction feature to be implemented)
 */
ENTRYID
VmDirRaftLogEntryId(unsigned long long LogIndex)
{
    return LOG_ENTRY_EID_PREFIX | LogIndex;
}

//Create the initial persistent state
DWORD
_VmDirInitRaftPsStateInBE(
    PVDIR_BACKEND_INTERFACE pBE
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    PSTR pszLogEntryDn = NULL;

    PSTR ppContex[] = { ATTR_OBJECT_CLASS,  "vmwDirCfg",
                        ATTR_CN, RAFT_CONTEXT_CONTAINER_NAME,
                        NULL };

    assert(pBE);

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreateInBE(pBE, pSchemaCtx, ppContex, RAFT_CONTEXT_DN, RAFT_CONTEXT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    PSTR ppLogs[] = { ATTR_OBJECT_CLASS,  "vmwDirCfg",
                      ATTR_CN, RAFT_LOGS_CONTAINER_NAME,
                      NULL };
    dwError = VmDirSimpleEntryCreateInBE(pBE, pSchemaCtx, ppLogs, RAFT_LOGS_CONTAINER_DN, RAFT_LOGS_CONTAINER_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);


    PSTR ppPersisteState[] = { ATTR_OBJECT_CLASS, OC_CLASS_RAFT_PERSIST_STATE,
                               ATTR_CN, RAFT_PERSIST_STATE_NAME,
                               ATTR_RAFT_LAST_APPLIED, "0",
                               ATTR_RAFT_FIRST_LOGINDEX, "1",
                               ATTR_RAFT_TERM, "1",
                               ATTR_RAFT_VOTEDFOR_TERM, "0",
                               NULL };
    dwError = VmDirSimpleEntryCreateInBE(pBE, pSchemaCtx, ppPersisteState, RAFT_PERSIST_STATE_DN, RAFT_PERSIST_STATE_ENTRY_ID);
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
    /* if entry already exists, database already initialized */
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS)
    {
        dwError = 0;
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirInitRaftPsState: error %d", dwError);
    }
    goto cleanup;
}

/*
 * initialize main db
 * initialize log db
*/
DWORD
VmDirInitRaftPsState(
    )
{
    DWORD dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    /* init main db */
    pBE = VmDirBackendSelect(ALIAS_MAIN);

    dwError = _VmDirInitRaftPsStateInBE(pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* init log db */
    pBE = VmDirBackendSelect(ALIAS_LOG_CURRENT);

    dwError = _VmDirInitRaftPsStateInBE(pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLoadRaftState(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR  pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_RAFT_LOG logEntry = {0};
    ENTRYID maxEId = 0;
    PVDIR_BACKEND_INTERFACE pLogBE = NULL;

    pLogBE = VmDirBackendSelect(ALIAS_LOG_CURRENT);
    assert(pLogBE);

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

    /* Populate previous log db details if any */
    if (VmDirHasBackend(ALIAS_LOG_PREVIOUS))
    {
        gRaftState.bHasPrevLog = TRUE;
    }

    dwError = _VmDirFetchLogEntry(gRaftState.lastApplied, &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    gRaftState.commitIndex = logEntry.index;
    gRaftState.commitIndexTerm = logEntry.term;

    gRaftState.cmd = ExecNone;
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.

    dwError = pLogBE->pfnBEMaxEntryId(pLogBE, &maxEId);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirChgLogFree(&logEntry, FALSE);
    dwError = _VmDirFetchLogEntry(maxEId & (~LOG_ENTRY_EID_PREFIX), &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    gRaftState.lastLogIndex = logEntry.index;
    gRaftState.lastLogTerm = logEntry.term;

    gRaftState.initialized = TRUE;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VmDirChgLogFree(&logEntry, FALSE);

    if (dwError==0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirLoadRaftState: term %d commitIdx %llu lastLogIdx %llu lastLogTerm %d lastApplied %llu",
          gRaftState.currentTerm, gRaftState.commitIndex, gRaftState.lastLogIndex,
          gRaftState.lastLogTerm, gRaftState.lastApplied);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLoadRaftState: error %d; %s", dwError, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

DWORD
VmDirAddRaftEntry(PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    char logEntryDn[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    VDIR_ENTRY raftEntry = {0};
    char termStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char logIndexStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char objectGuidStr[VMDIR_GUID_STR_LEN];
    VDIR_BERVALUE bvIndexApplied = VDIR_BERVALUE_INIT;
    uuid_t guid;
    VDIR_BACKEND_CTX logBeCtx = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_OPERATION op = {0};
    PSTR pszLocalErrorMsg = NULL;
    BOOLEAN bHasTxn = FALSE;
    BOOLEAN bLock = FALSE;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "%s: log index %llu", __func__, pLogEntry->index);

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirSchemaCtxAcquire");

    dwError = VmDirInitStackOperation(&op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirInitStackOperation");

    dwError = VmDirStringPrintFA(logIndexStr, sizeof(logIndexStr), "%llu", pLogEntry->index);
    BAIL_ON_VMDIR_ERROR(dwError);

    bvIndexApplied.lberbv.bv_val  = logIndexStr;
    bvIndexApplied.lberbv.bv_len = VmDirStringLenA(logIndexStr);

    op.pBEIF = VmDirBackendSelect(RAFT_PERSIST_STATE_DN);
    dwError = VmDirAddModSingleAttributeReplace(&op, RAFT_PERSIST_STATE_DN,
                                                ATTR_RAFT_LAST_APPLIED, &bvIndexApplied);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logEntryDn, sizeof(logEntryDn), "%s=%llu,%s",
                ATTR_CN, pLogEntry->index, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(termStr, sizeof(termStr), "%d", pLogEntry->term);
    BAIL_ON_VMDIR_ERROR(dwError);

    op.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalModifyEntry(&op);
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

    dwError = VmDirGetParentDN(&raftEntry.dn, &raftEntry.pdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirUuidGenerate (&guid);
    VmDirUuidToStringLower(&guid, objectGuidStr, sizeof(objectGuidStr));

    dwError = VmDirAllocateStringA(objectGuidStr, &raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(&raftEntry, ATTR_OBJECT_GUID, raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddBervArrayAttribute(&raftEntry, ATTR_RAFT_LOG_ENTRIES, &pLogEntry->packRaftLog, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirComputeObjectSecurityDescriptor(NULL, &raftEntry, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    raftEntry.eId = VmDirRaftLogEntryId(pLogEntry->index);

    logBeCtx.pBE = VmDirBackendSelect(RAFT_LOGS_CONTAINER_DN);

    dwError = logBeCtx.pBE->pfnBETxnBegin(&logBeCtx, VDIR_BACKEND_TXN_WRITE, &bHasTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = logBeCtx.pBE->pfnBEEntryAdd(&logBeCtx, &raftEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bHasTxn)
    {
        dwError = logBeCtx.pBE->pfnBETxnCommit(&logBeCtx);
        bHasTxn = FALSE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "pfnBETxnCommit adding raftEntry");
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    gRaftState.lastLogIndex = pLogEntry->index;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

cleanup:
    VmDirFreeOperationContent(&op);
    VmDirFreeEntryContent(&raftEntry);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    if (bHasTxn)
    {
        logBeCtx.pBE->pfnBETxnAbort(&logBeCtx);
    }
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "%s: log(%llu, %d, %d) term %d lastLogIndex %llu commitIndex %llu %s error %d",
      __func__, pLogEntry->index, pLogEntry->term, pLogEntry->requestCode, gRaftState.currentTerm,
      gRaftState.lastLogIndex, gRaftState.commitIndex, VDIR_SAFE_STRING(pszLocalErrorMsg), dwError);
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
    VmDirChgLogFree(&logEntry, FALSE);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

//Remove all logs with index >= startLogIndex
DWORD
VmDirDeleteAllLogs(unsigned long long startLogIndex, BOOLEAN *pbFatalError)
{
    DWORD dwError = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    char filterStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    int i = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    UINT64 logIndex = 0;
    int logCnt = 0;
    unsigned long long *pLogIdxArray = NULL;

    *pbFatalError = FALSE;

    dwError = VmDirStringPrintFA(filterStr, sizeof(filterStr), "(%s>=%llu)",
                                 ATTR_RAFT_LOGINDEX, startLogIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFilterInternalSearch(RAFT_LOGS_CONTAINER_DN, LDAP_SCOPE_ONE, filterStr, 0, NULL, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    logCnt = entryArray.iSize;
    if (logCnt < 1)
    {
        goto cleanup;
    }

    dwError = VmDirAllocateMemory( sizeof(unsigned long long)*logCnt, (PVOID*)&pLogIdxArray );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < logCnt; i++)
    {
        pAttr = VmDirFindAttrByName(&(entryArray.pEntry[i]), ATTR_RAFT_LOGINDEX);
        if (!pAttr)
        {
            //This indicate the corruption of the log entry data.
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirDeleteAllLogs invalid log entry, logIdx %llu lastLogIndex %llu",
                            logIndex, gRaftState.lastLogIndex);
            *pbFatalError = TRUE;
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        logIndex = VmDirStringToLA(pAttr->vals[0].lberbv.bv_val, NULL, 10);
        pLogIdxArray[i] = logIndex;
    }

    //To delete logs in high to low order
    qsort(pLogIdxArray, logCnt, sizeof(unsigned long long), _VmDirCompareLogIdx);

    if (logCnt > 1)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirDeleteAllLogs deleting %d logs from %llu to %llu ...",
                       logCnt, pLogIdxArray[0], pLogIdxArray[logCnt-1]);
    }

    for (i=0; i<logCnt; i++)
    {
        if (pLogIdxArray[i] <= gRaftState.lastApplied)
        {
             /* This shouldn't occur. If it does, then there would be a bug, and the consistency might be
              * compromised. We should investigate the sequence of events on how to get here.
              */
             VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
               "VmDirDeleteAllLogs attempt to delete log already applied, logIdx %llu lastApplied %llu lastLogIndex %llu",
               pLogIdxArray[i], gRaftState.lastApplied, gRaftState.lastLogIndex);

             *pbFatalError = TRUE;
             dwError = LDAP_OPERATIONS_ERROR;
             BAIL_ON_VMDIR_ERROR(dwError);
        }
        dwError = _VmdirDeleteLog(pLogIdxArray[i], FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pLogIdxArray);
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

    ldapOp.pBEIF = VmDirBackendSelect(RAFT_LOGS_CONTAINER_DN);
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
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirFetchLogEntry entering with logIndex %llu", logIndex);

    dwError = VmDirStringPrintFA(filterStr, sizeof(filterStr), "(%s=%llu)", ATTR_RAFT_LOGINDEX, logIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendForLogIndex(logIndex);
    assert(pBE);

    dwError = VmDirFilterInternalSearchInBE(
                  pBE,
                  RAFT_LOGS_CONTAINER_DN,
                  LDAP_SCOPE_ONE,
                  filterStr,
                  0,
                  NULL,
                  &entryArray);
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
    VmDirChgLogFree(pLogEntry, FALSE);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s line %d", VDIR_SAFE_STRING(pszLocalErrorMsg), from);
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
    VmDirChgLogFree(&prevLogEntry, FALSE);
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

    writer = (unsigned char *) pLogEntry->packRaftLog.lberbv_val;
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

//Pack multiple log entries into pLogEntry
//Assume chglog in pChgLogs is not packed yet.
DWORD
VmDirPackLogEntries(PVDIR_RAFT_LOG pLogEntry, PDEQUE pChglogs)
{
    DWORD dwError = 0;
    PDEQUE_NODE pDeque = NULL;
    PVDIR_RAFT_LOG pChgLog = NULL;
    VDIR_RAFT_LOG chgLog = {0};
    int chglog_size = 0;
    int iSize = 0;
    unsigned long long logIndex = 0;
    int logTerm = 0;
    unsigned long long entryId;
    int requestCode = 0;
    unsigned char *writer = NULL;

    // Calculate total size of packing individual logs into one.
    for (pDeque = pChglogs->pHead; pDeque != NULL; pDeque = pDeque->pNext)
    {
        pChgLog = (PVDIR_RAFT_LOG)pDeque->pElement;
        chglog_size += RAFT_LOG_HEADER_LEN + pChgLog->chglog.lberbv_len + sizeof(UINT64);
        if (logIndex == 0)
        {
            logIndex = pChgLog->index;
            logTerm = pChgLog->term;
            requestCode = 1;
            entryId = 0;
        }
    }

    chglog_size += RAFT_LOG_HEADER_LEN;

    dwError = VmDirAllocateMemory(chglog_size, (PVOID*)&(chgLog.packRaftLog.lberbv_val));
    BAIL_ON_VMDIR_ERROR(dwError);

    writer = (unsigned char *) chgLog.packRaftLog.lberbv_val;
    _VmDirEncodeUINT64(&writer, logIndex);
    _VmDirEncodeUINT32(&writer, logTerm);
    _VmDirEncodeUINT64(&writer, entryId);
    _VmDirEncodeUINT32(&writer, requestCode);

    // Each individual change log is encoded as:
    // size of that changelog, followed by encoded change log as the orignal format.
    while(!dequeIsEmpty(pChglogs))
    {
        dequePopLeft(pChglogs, (PVOID*)&pChgLog);
        iSize = pChgLog->chglog.lberbv_len + RAFT_LOG_HEADER_LEN;
        _VmDirEncodeUINT64(&writer, iSize);
        _VmDirEncodeUINT64(&writer, pChgLog->index);
        _VmDirEncodeUINT32(&writer, pChgLog->term);
        _VmDirEncodeUINT64(&writer, pChgLog->entryId);
        _VmDirEncodeUINT32(&writer, pChgLog->requestCode);
        if (pChgLog->chglog.lberbv_len > 0)
        {
            memcpy(writer, pChgLog->chglog.lberbv_val, pChgLog->chglog.lberbv_len);
            writer += pChgLog->chglog.lberbv_len;
        }
        VmDirChgLogFree(pChgLog, TRUE);
    }

    pLogEntry->packRaftLog.lberbv_val = chgLog.packRaftLog.lberbv_val;
    pLogEntry->packRaftLog.lberbv_len = chglog_size;
    pLogEntry->packRaftLog.bOwnBvVal = TRUE;
    pLogEntry->index = logIndex;
    pLogEntry->term = logTerm;
    pLogEntry->entryId = entryId;
    pLogEntry->requestCode = requestCode;

error:
    return dwError;
}

DWORD
_VmDirUnpackLogEntry(PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    unsigned char *reader = (unsigned char *) pLogEntry->packRaftLog.lberbv_val;

    if (pLogEntry->packRaftLog.lberbv_val == NULL ||
        pLogEntry->packRaftLog.lberbv_len < RAFT_LOG_HEADER_LEN)
    {
         dwError = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    pLogEntry->index = _VmDirDecodeUINT64(&reader);
    pLogEntry->term = _VmDirDecodeUINT32(&reader);
    pLogEntry->entryId = _VmDirDecodeUINT64(&reader);
    pLogEntry->requestCode = _VmDirDecodeUINT32(&reader);
    if (pLogEntry->packRaftLog.lberbv_len > RAFT_LOG_HEADER_LEN)
    {
        pLogEntry->chglog.lberbv_len = pLogEntry->packRaftLog.lberbv_len - RAFT_LOG_HEADER_LEN;
        dwError = VmDirAllocateMemory(pLogEntry->chglog.lberbv_len, (PVOID*)&pLogEntry->chglog.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        memcpy(pLogEntry->chglog.lberbv_val, reader, pLogEntry->chglog.lberbv_len);
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

DWORD
VmDirUnpackLogEntries(PDEQUE pChglogs, PVDIR_RAFT_LOG pLogEntry)
{
    DWORD dwError = 0;
    VDIR_RAFT_LOG logEntry = {0};
    PVDIR_RAFT_LOG pRaftLog = NULL;
    int iSize = 0;

    unsigned char *reader = (unsigned char *) pLogEntry->packRaftLog.lberbv_val;
    unsigned char *end = (unsigned char *)(pLogEntry->packRaftLog.lberbv_val + pLogEntry->packRaftLog.lberbv_len);

    if (pLogEntry->packRaftLog.lberbv_val == NULL ||
        pLogEntry->packRaftLog.lberbv_len < RAFT_LOG_HEADER_LEN)
    {
         dwError = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    pLogEntry->index = _VmDirDecodeUINT64(&reader);
    pLogEntry->term = _VmDirDecodeUINT32(&reader);
    pLogEntry->entryId = _VmDirDecodeUINT64(&reader);
    pLogEntry->requestCode = _VmDirDecodeUINT32(&reader);
    while(reader < (unsigned char *)(end - RAFT_LOG_HEADER_LEN))
    {
       iSize = _VmDirDecodeUINT64(&reader);
       if ((reader + iSize) > end)
       {
           BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
       }
       logEntry.packRaftLog.lberbv_val = (PSTR) reader;
       logEntry.packRaftLog.lberbv_len = iSize;
       dwError = _VmDirUnpackLogEntry(&logEntry);
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirAllocateMemory(sizeof(VDIR_RAFT_LOG), (PVOID*)&pRaftLog);
       BAIL_ON_VMDIR_ERROR(dwError);

       *pRaftLog = logEntry;
       dequePush(pChglogs, pRaftLog);
       reader += iSize;
    }

    if (reader != end)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
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
VmDirChgLogFree(
    PVDIR_RAFT_LOG chgLog,
    BOOLEAN freeSelf
)
{
    if (chgLog == NULL)
    {
        return;
    }
    if (chgLog->beCtx.pBE && chgLog->beCtx.pBEPrivate)
    {
        chgLog->beCtx.pBE->pfnBETxnAbort(&chgLog->beCtx);
    }
    VmDirBackendCtxContentFree(&chgLog->beCtx);
    VmDirFreeBervalContent(&chgLog->packRaftLog);
    VmDirFreeBervalContent(&chgLog->chglog);
    memset(chgLog, 0, sizeof(VDIR_RAFT_LOG));
    if (freeSelf)
    {
        VMDIR_SAFE_FREE_MEMORY(chgLog);
    }
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
    VmDirChgLogFree(&logEntry, FALSE);
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

    gVmdirServerGlobals.bPromoted = TRUE;
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

/*
 * Sort log index array in high to lower order
 */
static int
_VmDirCompareLogIdx(
    const void * logIdx1,
    const void * logIdx2)
{
    if ( *(unsigned long long *)logIdx1 < *(unsigned long long*)logIdx2 )
    {
        return 1;
    }
    else if (*(unsigned long long *)logIdx1 == *(unsigned long long*)logIdx2 )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static
BOOLEAN
_VmDirIsEntryInCurrentLog(
    unsigned long long logIndex
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasLog = FALSE;
    char filterStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    VDIR_ENTRY_ARRAY entryArray = {0};

    dwError = VmDirStringPrintFA(
                  filterStr,
                  sizeof(filterStr),
                  "%llu",
                  logIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
                  RAFT_LOGS_CONTAINER_DN,
                  LDAP_SCOPE_ONE,
                  ATTR_RAFT_LOGINDEX,
                  filterStr,
                  &entryArray);
    if (entryArray.iSize == 1)
    {
        bHasLog = TRUE;
    }
error:
    VmDirFreeEntryArrayContent(&entryArray);
    return bHasLog;
}

/*
 * if there is a previous log, a log entry could be in the previous logdb.
 * this function takes this into consideration and returns the right db.
 * if there is a previous log and logIndex is not found in current db,
 * the previous log db is returned. Otherwise, current log db
 * is returned.
*/
PVDIR_BACKEND_INTERFACE
VmDirBackendForLogIndex(
    unsigned long long logIndex
    )
{
    PVDIR_BACKEND_INTERFACE pBE = VmDirBackendSelect(ALIAS_LOG_CURRENT);

    /*
     * simplify previous log handling by a quick look up
     * entry by index in current db. if does not exist,
     * return previous db.
    */
    if (gRaftState.bHasPrevLog)
    {
        if (!_VmDirIsEntryInCurrentLog(logIndex))
        {
            pBE = VmDirBackendSelect(ALIAS_LOG_PREVIOUS);
        }
    }
    return pBE;
}
