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



#ifndef LH_H_
#define LH_H_

// add.c
int
VmDirParseBerToEntry(
    BerElement *ber,
    PVDIR_ENTRY e,
    ber_int_t *pErrCode,
    PSTR *ppszErrMsg
    );

int
VmDirParseEntry(
    VDIR_OPERATION * op
    );

int
VmDirPerformAdd(
   PVDIR_OPERATION pOperation
   );

void
VmDirFreeAddRequest(
   AddReq * ar,
   BOOLEAN  freeSelf
   );

DWORD
VmDirResetAddRequestEntry(
    PVDIR_OPERATION     pOp,
    PVDIR_ENTRY         pEntry
    );

// bind.c
int
VmDirPerformBind(
    PVDIR_OPERATION   pOperation
    );

void
VmDirFreeBindRequest(
    BindReq*     pBindReq,
    BOOLEAN      bFreeSelf
    );

// connection.c
DWORD
VmDirInitConnAcceptThread(
    void);

VOID
VmDirShutdownConnAcceptThread(
    VOID
    );

void
VmDirFreeAccessInfo(
    PVDIR_ACCESS_INFO pAccessInfo
    );

void
VmDirDeleteConnection(
    VDIR_CONNECTION **  conn
    );

// controls.c
void
DeleteControls(
   VDIR_LDAP_CONTROL ** controls);

int
ParseRequestControls(
   VDIR_OPERATION *    op,
   VDIR_LDAP_RESULT *   lr );

int
ParseSyncStateControlVal(
    BerValue *  controlValue,
    int *       entryState);

int
ParseAndFreeSyncStateControl(
    LDAPControl ***pCtrls,
    int *piEntryState
    );

int
WriteSyncDoneControl(
    VDIR_OPERATION *     op,
    BerElement *    ber
    );

int
WritePagedSearchDoneControl(
    VDIR_OPERATION *    op,
    BerElement *        ber
    );

int
WriteSyncStateControl(
   VDIR_OPERATION *   op,
   VDIR_ATTRIBUTE *   pAttr,
   BerElement *       ber,
   PSTR*              ppszErrorMsg
   );

int
WriteConsistencyWriteDoneControl(
    VDIR_OPERATION *       pOp,
    BerElement *           pBer
    );

// delete.c
int
VmDirPerformDelete(
   PVDIR_OPERATION pOperation
   );

// filter.c
DWORD
VmDirConcatTwoFilters(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR                pszAttrFilterName1,
    PSTR                pszAttrFilterVal1,
    PSTR                pszAttrFilterName2,
    PSTR                pszAttrFilterVal2,
    PVDIR_FILTER*       ppFilter
    );

int
AppendDNFilter(
	VDIR_OPERATION *    op);

VDIR_FILTER_COMPUTE_RESULT
CheckIfEntryPassesFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *   f);

void
DeleteFilter(
   VDIR_FILTER * f
   );

DWORD
FilterToStrFilter(
   PVDIR_FILTER f,
   PVDIR_BERVALUE strFilter
   );

DWORD
StrFilterToFilter(
    PCSTR pszString,
    PVDIR_FILTER *ppFilter
    );

int
ParseFilter(
   VDIR_OPERATION *   op,
   VDIR_FILTER **     filt,
   VDIR_LDAP_RESULT *       lr);

// openssl.c
DWORD
VmDirOpensslInit(
    VOID
    );

VOID
VmDirOpensslShutdown(
    VOID
    );

// operation.c
int
VmDirExternalOperationCreate(
    BerElement*       ber,
    ber_int_t         msgId,
    ber_tag_t         reqCode,
    PVDIR_CONNECTION  pConn,
    PVDIR_OPERATION*  ppOperation
    );

void
VmDirFreeOperation(
    PVDIR_OPERATION pOperation
    );

// result.c
void
VmDirSendLdapResult(
   VDIR_OPERATION *   op
   );

VOID
VmDirSendSASLBindResponse(
    PVDIR_OPERATION     pOperation
    );

// modify.c
int
VmDirPerformModify(
   PVDIR_OPERATION pOperation
   );

void
VmDirFreeModifyRequest(
   ModifyReq * mr,
   BOOLEAN     freeSelf
   );

// rename.c
int
VmDirPerformRename(
   PVDIR_OPERATION pOperation
   );

// search.c
int
VmDirPerformSearch(
    PVDIR_OPERATION   pOperation
    );

void
VmDirFreeSearchRequest(
   SearchReq * sr,
   BOOLEAN     freeSelf
   );

// unbind.c
int
VmDirPerformUnbind(
   PVDIR_OPERATION pOperation
   );

// delete.c

void
VmDirFreeDeleteRequest(
   DeleteReq * dr,
   BOOLEAN     freeSelf
   );

// opstatistic.c
DWORD
VmDirInitOPStatisticGlobals(
    VOID
    );

uint16_t
VmDirOPStatisticGetAvgTime(
    PVMDIR_OPERATION_STATISTIC   pStatistic
    );

uint64_t
VmDirOPStatisticGetCount(
    ber_tag_t opTag
    );

PSTR
VmDirOPStatistic(
    ber_tag_t      opTag
    );

PCSTR
VmDirGetOperationStringFromTag(
    ber_tag_t opTag);

#endif /* LH_H_ */
