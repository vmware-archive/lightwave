/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#ifndef VMDIR_COMMON_REPLSTATE_H_
#define VMDIR_COMMON_REPLSTATE_H_

typedef struct _VDIR_SCHEMA_REPL_STATE
{
    PSTR    pszHostName;
    PSTR    pszDomainName;
    BOOLEAN bCheckInitiated;
    BOOLEAN bCheckSucceeded;
    BOOLEAN bTreeInSync;
    BOOLEAN bBlobInSync;
    DWORD   dwAttrMissingInTree;
    DWORD   dwAttrMismatchInTree;
    DWORD   dwClassMissingInTree;
    DWORD   dwClassMismatchInTree;
    DWORD   dwAttrMissingInBlob;
    DWORD   dwAttrMismatchInBlob;
    DWORD   dwClassMissingInBlob;
    DWORD   dwClassMismatchInBlob;

} VDIR_SCHEMA_REPL_STATE, *PVDIR_SCHEMA_REPL_STATE;

// schemareplstate.c
DWORD
VmDirSchemaReplStateCreate(
    PSTR                        pszHostName,
    PSTR                        pszDomainName,
    PVDIR_SCHEMA_REPL_STATE*    ppReplState
    );

DWORD
VmDirSchemaReplStateCheck(
    PVDIR_SCHEMA_REPL_STATE pReplState,
    PVDIR_LDAP_SCHEMA       pTargetSchema
    );

DWORD
VmDirSchemaReplStateParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_SCHEMA_REPL_STATE*    ppReplState
    );

DWORD
VmDirSchemaReplStateLog(
    PVDIR_SCHEMA_REPL_STATE pReplState
    );

VOID
VmDirFreeSchemaReplState(
    PVDIR_SCHEMA_REPL_STATE pReplStatus
    );

#endif /* VMDIR_COMMON_REPLSTATE_H_ */
