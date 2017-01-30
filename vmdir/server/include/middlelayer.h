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



#ifndef ML_INTERFACE_H_
#define ML_INTERFACE_H_

#include "vmdir.h"

// libmain.c

/*
 * Initialize middle-layer library
 */
DWORD
VmDirMiddleLayerLibInit(
    VOID
    );

/*
 * Shutdown middle-layer library
 */
void
VmDirMiddleLayerLibShutdown(
    void
    );

// add.c

int
VmDirInternalAddEntry(
    VDIR_OPERATION *   pOperation);

int
VmDirMLAdd(
   PVDIR_OPERATION pOperation
   );

// delete.c

int
VmDirInternalDeleteEntry(
    PVDIR_OPERATION    pOperation
    );

int
VmDirMLDelete(
    PVDIR_OPERATION    pOperation
    );

// modify.c

int
VmDirApplyModsToEntryStruct(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ModifyReq *         modReq,
    PVDIR_ENTRY         pEntry,
    PBOOLEAN            pbDnModified,
    PSTR*               ppszErrorMsg,
    BOOLEAN             bIsReplOp
    );

int
VmDirModifyEntryCoreLogic(
    VDIR_OPERATION *    pOperation, /* IN */
    ModifyReq *         modReq, /* IN */
    ENTRYID             entryId, /* IN */
    VDIR_ENTRY *        pEntry  /* OUT */
    );

int
VmDirGenerateModsNewMetaData(
    PVDIR_OPERATION          pOperation,
    PVDIR_MODIFICATION       pmods,
    USN                      entryId
    );

int
VmDirInternalModifyEntry(
    PVDIR_OPERATION pOperation
    );

int
VmDirMLModify(
    PVDIR_OPERATION pOperation
    );

int
VmDirNormalizeMods(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_MODIFICATION  pMods,
    PSTR*               ppszErrorMsg
    );

DWORD
VmDirInternalEntryAttributeReplace(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszNormDN,
    PCSTR               pszAttrName,
    PVDIR_BERVALUE      pBervAttrValue
    );

// bind.c
int
VmDirMLBind(
   PVDIR_OPERATION   pOperation
   );

int
VmDirInternalBindEntry(
    PVDIR_OPERATION  pOperation
    );

DWORD
VmDirMLSetupAnonymousAccessInfo(
    PVDIR_ACCESS_INFO pAccessInfo
    );

// dn.c

DWORD
VmDirGetParentDN(
    VDIR_BERVALUE * dn,
    VDIR_BERVALUE * pdn
    );

DWORD
VmDirGetRdn(
    VDIR_BERVALUE * dn,
    VDIR_BERVALUE * rdn
    );

DWORD
VmDirRdnToNameValue(
    VDIR_BERVALUE * rdn,
    PSTR *ppszName,
    PSTR *ppszValue
    );

DWORD
VmDirCatDN(
    PVDIR_BERVALUE pRdn,
    PVDIR_BERVALUE pDn,
    PVDIR_BERVALUE pResult
    );

// search.c

int
VmDirMLSearch(
    PVDIR_OPERATION pOperation
    );

int
VmDirInternalSearch(
    PVDIR_OPERATION pOperation
    );

DWORD
VmDirIsDirectMemberOf(
    PSTR                pszBindDN,
    UINT32              getAccessInfo,
    UINT32              *accessRoleBitmap,
    PBOOLEAN            pbIsMemberOf
    );

// plugin.c
/*
 * Initialize plugin
 */
DWORD
VmDirPluginInit(
    VOID
    );

VOID
VmDirPluginShutdown(
    VOID
    );

// password.c
/*
 * Initialize password scheme
 */
DWORD
VmDirPasswordSchemeInit(
    VOID
    );

VOID
VmDirPasswordSchemeFree(
    VOID
    );

// sasl.c
VOID
VmDirSASLSessionClose(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    );

VOID
VmDirSASLShutdown(
    VOID
    );

// krb.c
DWORD
VmDirKrbSimpleDNToRealm(
    PVDIR_BERVALUE  pBervDN,
    PSTR*           ppszRealm           // out: could be NULL
    );

DWORD
VmDirGenerateRandomPasswordByDefaultPolicy
(
    PSTR *ppRandPwd
    );

// util.c

VOID
VmDirAuditWriteOp(
    PVDIR_OPERATION  pOp,
    PCSTR            pszDN
    );

#endif /* ML_INTERFACE_H_ */
