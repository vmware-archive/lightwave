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
 * Module Name: Directory main
 *
 * Filename: plugin.c
 *
 * Abstract:
 *
 * ldap operation plugin
 *
 */

#include "includes.h"

/*
 * plugin function is called in the order defined in tables below.
 */

#define VDIR_NOT_INTERNAL_OPERATIONS    \
    VDIR_OPERATION_TYPE_EXTERNAL        \
    | VDIR_OPERATION_TYPE_REPL

#define VDIR_NOT_REPL_OPERATIONS        \
    VDIR_OPERATION_TYPE_EXTERNAL        \
    | VDIR_OPERATION_TYPE_INTERNAL

#define VDIR_ALL_OPERATIONS             \
    VDIR_OPERATION_TYPE_EXTERNAL        \
    | VDIR_OPERATION_TYPE_INTERNAL      \
    | VDIR_OPERATION_TYPE_REPL

// function parameter pEntry is NULL in this case!!
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_PRE_MODAPPLY_MODIFY_PLUGIN_INITIALIZER                 \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDIrPluginPasswordPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginReplaceOpAttrsPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDIrPluginHandleFSPsPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginHandleStructureOCPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginMapAclStringAttributePreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginIndexEntryPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_OPERATION_TYPE_EXTERNAL),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginVerifyAclAccess),       \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE: order of fields MUST stay in sync with struct definition...
// Following plugin will be call for normal and replication routes.
// This plugin sees entry with modification applied
#define VDIR_PRE_MODIFY_PLUGIN_INITIALIZER                          \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaLibUpdatePreModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginLockoutPolicyEntryIntegrityCheck), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginGroupTypePreModify),     \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginIndexEntryPreModify),    \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginDflValidatePreModify),  \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_POST_MODIFY_COMMIT_PLUGIN_INITIALIZER                  \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginDflUpdatePostModifyCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, FALSE),                            \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirpluginPasswordPostModifyCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, FALSE),                            \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaLibUpdatePostModifyCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginLockoutCachePostModifyCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_OPERATION_TYPE_REPL),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginDCAccountPostModifyCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE1: order of fields MUST stay in sync with struct definition...
// NOTE2: generate SID (_VmDirPluginGenerateSidPreAdd) after generating GUID in _VmDirPluginAddOpAttrsPreAdd plugin
#define VDIR_PRE_ADD_PLUGIN_INITIALIZER                             \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginGenericPreAdd),         \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginPasswordHashPreAdd),    \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginAddOpAttrsPreAdd),      \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginGenerateSidPreAdd),     \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginCreateFSPsPreAdd),      \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginLockoutPolicyEntryIntegrityCheck), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginGroupTypePreAdd),        \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaEntryPreAdd),     \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaLibUpdatePreAdd), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginIndexEntryPreAdd),       \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE 1: order of fields MUST stay in sync with struct definition...
// NOTE 2: VmDirPluginReplAgrPostAddCommit() plugin is called only when Add commit has succeeded.
#define VDIR_POST_ADD_COMMIT_PLUGIN_INITIALIZER                     \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginReplAgrPostAddCommit),   \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginServerEntryPostAddCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, FALSE),                            \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaLibUpdatePostAddCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_INTERNAL_OPERATIONS),         \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginIndexEntryPostAdd),      \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// function parameter pEntry is NULL in this case!!
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_PRE_MODAPPLY_DELETE_PLUGIN_INITIALIZER                 \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginGroupMemberPreModApplyDelete), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginReplaceOpAttrsPreModApplyModify), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSetDeletedObjAttrsPreModApplyDelete), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_POST_DELETE_COMMIT_PLUGIN_INITIALIZER                  \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginReplAgrPostDeleteCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_ALL_OPERATIONS),                  \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginServerEntryPostDeleteCommit), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

static
DWORD
_VmDirConstructFSPDN(
    PVDIR_BERVALUE   pEntryDn,
    PVDIR_BERVALUE   pSpecialDn,
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PSTR *           ppszFSPDN);

static
DWORD
_VmDirHandleFSPDNs(
    PVDIR_BERVALUE   pEntryDn,
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          createFSPs);

static
DWORD
_VmDirPluginLockoutPolicyEntryIntegrityCheck(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginSchemaLibUpdatePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginDflValidatePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginSchemaLibUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginLockoutCachePostModifyCommit(
        PVDIR_OPERATION  pOperation,
        PVDIR_ENTRY      pEntry,
        DWORD            dwPriorResult
        );

static
DWORD
_VmDirPluginDflUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirpluginPasswordPostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginGenericPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginPasswordHashPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginGenerateSidPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginAddOpAttrsPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginCreateFSPsPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginSchemaEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginSchemaLibUpdatePreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginSchemaLibUpdatePostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDIrPluginPasswordPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginVerifyAclAccess(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    );

static
DWORD
_VmDirPluginReplaceOpAttrsPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDIrPluginHandleFSPsPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    );

static
DWORD
_VmDirPluginHandleStructureOCPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    );

static
DWORD
_VmDirPluginMapAclStringAttributePreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    );

static
DWORD
_VmDirPluginSetDeletedObjAttrsPreModApplyDelete(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginInit(
    PVDIR_OP_PLUGIN_INFO*   ppHead,
    PVDIR_OP_PLUGIN_INFO    pTbl,
    int                     iSize);

static
VOID
_VmDirFreePlugins(
    PVDIR_OP_PLUGIN_INFO   pHead
    );

static
DWORD
_VmDirOperationPlugin(
    PVDIR_OP_PLUGIN_INFO    pInfo,
    PVDIR_OPERATION         pOperation,
    PVDIR_ENTRY             pEntry,
    DWORD                   dwPriorResult);

/*
 * static initialize gVmdirPluginGlobals
 */
DWORD
VmDirPluginInit(
    VOID)
{
    DWORD       dwError = 0;

    VDIR_OP_PLUGIN_INFO initPreModApplyDeleteTbl[] = VDIR_PRE_MODAPPLY_DELETE_PLUGIN_INITIALIZER;
    VDIR_OP_PLUGIN_INFO initPreModApplyModifyTbl[] = VDIR_PRE_MODAPPLY_MODIFY_PLUGIN_INITIALIZER;
    VDIR_OP_PLUGIN_INFO initPreModifyTbl[] = VDIR_PRE_MODIFY_PLUGIN_INITIALIZER;
    VDIR_OP_PLUGIN_INFO initPostModifyCommitTbl[] = VDIR_POST_MODIFY_COMMIT_PLUGIN_INITIALIZER;

    VDIR_OP_PLUGIN_INFO initPreAddTbl[] = VDIR_PRE_ADD_PLUGIN_INITIALIZER;
    VDIR_OP_PLUGIN_INFO initPostAddCommitTbl[] = VDIR_POST_ADD_COMMIT_PLUGIN_INITIALIZER;
    VDIR_OP_PLUGIN_INFO initPostDeleteCommitTbl[] = VDIR_POST_DELETE_COMMIT_PLUGIN_INITIALIZER;

    dwError = _VmDirPluginInit(
                &gVmdirPluginGlobals.pPreModApplyDeletePluginInfo,
                &(initPreModApplyDeleteTbl[0]),
                sizeof(initPreModApplyDeleteTbl)/sizeof(initPreModApplyDeleteTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
            &gVmdirPluginGlobals.pPreModApplyModifyPluginInfo,
            &(initPreModApplyModifyTbl[0]),
            sizeof(initPreModApplyModifyTbl)/sizeof(initPreModApplyModifyTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
            &gVmdirPluginGlobals.pPreModifyPluginInfo,
            &(initPreModifyTbl[0]),
            sizeof(initPreModifyTbl)/sizeof(initPreModifyTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
            &gVmdirPluginGlobals.pPostModifyCommitPluginInfo,
            &(initPostModifyCommitTbl[0]),
            sizeof(initPostModifyCommitTbl)/sizeof(initPostModifyCommitTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
            &gVmdirPluginGlobals.pPreAddPluginInfo,
            &(initPreAddTbl[0]),
            sizeof(initPreAddTbl)/sizeof(initPreAddTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
                &gVmdirPluginGlobals.pPostAddCommitPluginInfo,
                &(initPostAddCommitTbl[0]),
                sizeof(initPostAddCommitTbl)/sizeof(initPostAddCommitTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPluginInit(
                &gVmdirPluginGlobals.pPostDeleteCommitPluginInfo,
                &(initPostDeleteCommitTbl[0]),
                sizeof(initPostDeleteCommitTbl)/sizeof(initPostDeleteCommitTbl[0]));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VOID
VmDirPluginShutdown(
    VOID
    )
{
    _VmDirFreePlugins(gVmdirPluginGlobals.pPreAddPluginInfo);
    _VmDirFreePlugins(gVmdirPluginGlobals.pPostAddCommitPluginInfo);

    _VmDirFreePlugins(gVmdirPluginGlobals.pPreModApplyModifyPluginInfo);
    _VmDirFreePlugins(gVmdirPluginGlobals.pPreModifyPluginInfo);
    _VmDirFreePlugins(gVmdirPluginGlobals.pPostModifyCommitPluginInfo);

    _VmDirFreePlugins(gVmdirPluginGlobals.pPreModApplyDeletePluginInfo);
    _VmDirFreePlugins(gVmdirPluginGlobals.pPostDeleteCommitPluginInfo);
}

DWORD
VmDirExecutePreAddPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // entry been manipulated
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPreAddPluginInfo;

    assert (pOperation && pEntry);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePostAddCommitPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // entry been manipulated
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPostAddCommitPluginInfo;

    assert (pOperation && pEntry);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePreModApplyDeletePlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // pEntry is NULL in this plugin
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPreModApplyDeletePluginInfo;

    assert (pOperation);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePostDeleteCommitPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // entry been manipulated
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPostDeleteCommitPluginInfo;

    assert (pOperation);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePreModApplyModifyPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // pEntry is NULL in this plugin
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPreModApplyModifyPluginInfo;

    assert (pOperation);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePreModifyPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // entry been manipulated
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPreModifyPluginInfo;

    assert (pOperation && pEntry);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

DWORD
VmDirExecutePostModifyCommitPlugins(
    PVDIR_OPERATION     pOperation,      // current operation
    PVDIR_ENTRY         pEntry,          // entry been manipulated
    DWORD               dwResult         // latest call return value
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = gVmdirPluginGlobals.pPostModifyCommitPluginInfo;

    assert (pOperation && pEntry);

    return _VmDirOperationPlugin(
                                pPluginInfo,
                                pOperation,
                                pEntry,
                                dwResult);
}

#ifdef WINJOIN_CHECK_ENABLED
static
DWORD
_VmDirAttrAddSpnName(PVDIR_ENTRY pEntry,
                     PSTR pszHost,
                     BOOLEAN bUcHost,
                     BOOLEAN bShortHost)
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszDomainName = NULL;
    VDIR_BERVALUE    cnRdn = VDIR_BERVALUE_INIT;
    PSTR             pszRdnName = NULL;
    PSTR             pszRdnValue = NULL;
    PSTR             pszSpn = NULL;
    PSTR             pszDomainNameLc = NULL;
    PSTR             pszDomainNameUc = NULL;

    dwError = LwLdapConvertDNToDomain(
                  pEntry->dn.lberbv.bv_val,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirGetRdn(&pEntry->dn, &cnRdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&cnRdn, &pszRdnName, &pszRdnValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; pszRdnValue[i]; i++)
    {
        pszRdnValue[i] = (char) tolower((int) pszRdnValue[i]);
    }

    dwError = VmDirAllocateStringA(pszDomainName, &pszDomainNameLc);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; pszDomainNameLc[i]; i++)
    {
        pszDomainNameLc[i] = (char) tolower((int) pszDomainNameLc[i]);
    }

    dwError = VmDirAllocateStringA(pszDomainName, &pszDomainNameUc);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; pszDomainNameUc[i]; i++)
    {
        pszDomainNameUc[i] = (char) toupper((int) pszDomainNameUc[i]);
    }

    if (bShortHost)
    {
        dwError = VmDirAllocateStringPrintf(
                      &pszSpn,
                      "%s/%s@%s",
                      pszHost,
                      pszRdnValue,
                      pszDomainNameUc);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
                      &pszSpn,
                      "%s/%s.%s@%s",
                      pszHost,
                      pszRdnValue,
                      pszDomainNameLc,
                      pszDomainNameUc);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_KRB_SPN, pszSpn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszDomainNameLc);
    VMDIR_SAFE_FREE_MEMORY(pszDomainNameUc);
    VMDIR_SAFE_FREE_MEMORY(pszSpn);
    goto cleanup;
}

static
BOOLEAN
_VmDirCheckIsComputerAccount(
    PVDIR_ENTRY pEntry)
{
    BOOLEAN bIsComputerAccount = FALSE;
    PVDIR_ATTRIBUTE pAttrObjectClass = NULL;

    pAttrObjectClass = VmDirEntryFindAttribute(ATTR_OBJECT_CLASS, pEntry);

    if (pAttrObjectClass && pAttrObjectClass->vals[0].lberbv.bv_val &&
        VmDirStringCompareA(pAttrObjectClass->vals[0].lberbv.bv_val,
                            "Computer",
                            FALSE) == 0)
    {
        bIsComputerAccount = TRUE;
    }

    return bIsComputerAccount;
}

static
DWORD
_VmDirJoinCreateComputerAccount(PVDIR_ENTRY pEntry,
                                PVDIR_ATTRIBUTE *ppAttrUPN)
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszDomainName = NULL;
    PSTR pszComputerUpn = NULL;
    PVDIR_ATTRIBUTE pAttrSamAccountName = NULL;

    if (!_VmDirCheckIsComputerAccount(pEntry))
    {
        goto cleanup;
    }

    pAttrSamAccountName = VmDirEntryFindAttribute(
                              ATTR_SAM_ACCOUNT_NAME,
                              pEntry);
    if (!pAttrSamAccountName)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Construct the full Computer account name */
    dwError = LwLdapConvertDNToDomain(
                  pEntry->dn.lberbv.bv_val,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; pszDomainName[i]; i++)
    {
        pszDomainName[i] = (char) toupper((int) pszDomainName[i]);
    }
    dwError = VmDirAllocateStringPrintf(
                  &pszComputerUpn, 
                  "%s@%s",
                  pAttrSamAccountName->vals[0].lberbv.bv_val,
                  pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);


    /* Add these attributes to the directory entry */
    dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_KRB_UPN, pszComputerUpn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAttrUPN = VmDirFindAttrByName(pEntry, ATTR_KRB_UPN);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszComputerUpn);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirAddAdMachineAccount(PVDIR_ENTRY pEntry)
{
    DWORD            dwError = 0;
    VDIR_BERVALUE    cnRdn = VDIR_BERVALUE_INIT;
    PSTR             pszRdnName = NULL;
    PSTR             pszRdnValue = NULL;
    PVDIR_ATTRIBUTE  pObjectCnAttrExist = NULL;

    /* Add ATTR_CN if not present when the first RDN component is CN= */
    dwError = VmDirGetRdn(&pEntry->dn, &cnRdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&cnRdn, &pszRdnName, &pszRdnValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszRdnName && pszRdnValue && VmDirStringCompareA(pszRdnName, ATTR_CN, FALSE) == 0)
    {
        pObjectCnAttrExist = VmDirEntryFindAttribute(
                                  ATTR_CN,
                                  pEntry);
        if (!pObjectCnAttrExist)
        {
            /* Add missing cn attribute; skip over "cn=" prefix */
            dwError = VmDirEntryAddSingleValueAttribute(
                          pEntry,
                          ATTR_CN,
                          cnRdn.lberbv.bv_val + 3,
                          cnRdn.lberbv.bv_len - 3);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    /* Test for object is a computer account */
    if (_VmDirCheckIsComputerAccount(pEntry))
    {
        /* For computer accounts, add service principals */
        dwError = _VmDirAttrAddSpnName(pEntry, "host", FALSE, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirAttrAddSpnName(pEntry, "ldap", FALSE, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirAttrAddSpnName(pEntry, "cifs", FALSE, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirAttrAddSpnName(pEntry, "RPC", TRUE, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirAttrAddSpnName(pEntry, "DNS", TRUE, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirAttrAddSpnName(pEntry, "LDAP", TRUE, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszRdnName);
    VMDIR_SAFE_FREE_MEMORY(pszRdnValue);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirAttrUnicodePwdToPwd(PVDIR_ENTRY pEntry,
                          PVDIR_ATTRIBUTE *ppAttrPasswd)
{
    DWORD            dwError = 0;
    PVDIR_ATTRIBUTE  pAttrUnicodePwd = VmDirFindAttrByName(pEntry, ATTR_UNICODE_PWD);
    PWSTR            pwszUnicodePwd = NULL;
    PSTR             pszAccountPwdAlloc = NULL;
    PSTR             pszAccountPwd = NULL;
    DWORD            dwAccountPwdLen = 0;

    /* Windows join computer account passes this password */
    if (_VmDirCheckIsComputerAccount(pEntry) && pAttrUnicodePwd)
    {
        /* Create a NULL terminated wide-character string of unicodePwd */
        dwError = VmDirAllocateMemory(
                      pAttrUnicodePwd->vals[0].lberbv.bv_len + sizeof(WCHAR),
                      (PVOID *) &pwszUnicodePwd);
        BAIL_ON_VMDIR_ERROR(dwError);

        memcpy((PVOID) pwszUnicodePwd, 
               pAttrUnicodePwd->vals[0].lberbv.bv_val,
               pAttrUnicodePwd->vals[0].lberbv.bv_len);

        /* Convert unicodePwd to C String */
        dwError = LwRtlCStringAllocateFromWC16String(
                      &pszAccountPwdAlloc,
                      (PCWSTR) pwszUnicodePwd);
        dwError = LwNtStatusToWin32Error(dwError);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwAccountPwdLen = VmDirStringLenA(pszAccountPwdAlloc);

        /* Must prune off the " " around the password */
        if (pszAccountPwdAlloc[0] == '"' &&
            pszAccountPwdAlloc[dwAccountPwdLen-1] == '"')
        {
            pszAccountPwd = pszAccountPwdAlloc + 1;
            pszAccountPwd[dwAccountPwdLen-1 - 1] = '\0';
        }
        else
        {
            pszAccountPwd = pszAccountPwdAlloc;
        }

        dwError = VmDirEntryAddSingleValueStrAttribute(
                        pEntry,
                        ATTR_USER_PASSWORD,
                        pszAccountPwd);
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppAttrPasswd = VmDirFindAttrByName(pEntry, ATTR_USER_PASSWORD);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAccountPwdAlloc);
    VMDIR_SAFE_FREE_MEMORY(pwszUnicodePwd);

    return dwError;

error:
    goto cleanup;
}

#else

/* Non-Win join stub functions */
static
DWORD
_VmDirJoinCreateComputerAccount(PVDIR_ENTRY pEntry,
                                PVDIR_ATTRIBUTE *ppAttrUPN)
{
    return 0;
}

static
DWORD
_VmDirAttrUnicodePwdToPwd(PVDIR_ENTRY pEntry,
                          PVDIR_ATTRIBUTE *ppAttrPasswd)
{
    return 0;
}

static
DWORD
_VmDirAddAdMachineAccount(PVDIR_ENTRY pEntry)
{
    return 0;
}
#endif

static
DWORD
_VmDirOperationPlugin(
    PVDIR_OP_PLUGIN_INFO    pInfo,
    PVDIR_OPERATION         pOperation,
    PVDIR_ENTRY             pEntry,
    DWORD                   dwPriorResult
    )
{
    PVDIR_OP_PLUGIN_INFO    pPluginInfo = pInfo;
    DWORD                   dwError = 0;

    for (; pPluginInfo; pPluginInfo = pPluginInfo->pNext)
    {
        if ((pPluginInfo->usOpMask & pOperation->opType)
                && (!pPluginInfo->bSkipOnError || !dwPriorResult))
        {
            dwError = pPluginInfo->pPluginFunc(pOperation, pEntry, dwPriorResult);

            if (!dwPriorResult && dwError)
            {    // return the first error code encountered in all plugin calls
                dwPriorResult = dwError;
            }
        }
    }

    return dwPriorResult;
}


static
DWORD
_VmDirPluginLockoutPolicyEntryIntegrityCheck(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD           dwRtn = 0;
    PVDIR_ATTRIBUTE  pAttrOC = VmDirFindAttrByName(pEntry, ATTR_OBJECT_CLASS);

    if (pAttrOC)
    {
        unsigned int    iCnt = 0;
        BOOLEAN         bDone = FALSE;

        for (iCnt = 0; iCnt < pAttrOC->numVals && bDone == FALSE; iCnt++)
        {
            if ((VmDirStringCompareA(pAttrOC->vals[iCnt].lberbv.bv_val, OC_VMW_LOCKOUT_POLICY, FALSE)  == 0)
                ||
                (VmDirStringCompareA(pAttrOC->vals[iCnt].lberbv.bv_val, OC_VMW_PASSWORD_POLICY, FALSE) == 0)
                )
            {
                dwRtn = VdirLockoutPolicyIntegrityCheck(pEntry);
                BAIL_ON_VMDIR_ERROR(dwRtn);
                bDone = TRUE;
            }
        }
    }

error:

    return dwRtn;
}

static
DWORD
_VmDirPluginSchemaLibUpdatePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD dwRtn = 0;
    PVDIR_MODIFICATION pMod = NULL;

    if (pOperation->dwSchemaWriteOp)
    {
        pMod = pOperation->request.modifyReq.mods;
        for (; pMod; pMod = pMod->next)
        {
            // reject the following changes:
            // - objectclass
            // - cn
            PSTR pszType = pMod->attr.type.lberbv.bv_val;
            if (VmDirStringCompareA(pszType, ATTR_OBJECT_CLASS, FALSE) == 0
                    || VmDirStringCompareA(pszType, ATTR_CN, FALSE) == 0)
            {
                dwRtn = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }
        }

        dwRtn = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);
    }

error:
    return dwPriorResult ? dwPriorResult : dwRtn;
}

/*
 * Check if modified domain functional level can be supported.
 * If operation is external, check that all DCs can support dfl.
 * If operation is replication and level is above max DFL, force exit server
 * as it cannot give the functionality that the domain requires.
 */
static
DWORD
_VmDirPluginDflValidatePreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_ATTRIBUTE pAttrVer = NULL;
    PVDIR_ATTRIBUTE pAttrCn = NULL;
    PVDIR_ATTRIBUTE pAttrMaxDfl = NULL;
    PSTR pszDomainDN = NULL;
    PSTR pszDCContainerDN = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int iCnt = 0;
    DWORD dwReqDfl = 0;
    DWORD dwMaxDfl = 0;
    PSTR pszCn = NULL;

    pszDomainDN = gVmdirServerGlobals.systemDomainDN.bvnorm_val;

    // check if entry is domain object
    if (pszDomainDN && VmDirStringCompareA(BERVAL_NORM_VAL(pEntry->dn),
                                           pszDomainDN,
                                           FALSE) == 0)
    {
        // Search for vmwDomainFunctionalLevel attr
        pAttr = VmDirFindAttrByName(pEntry, ATTR_DOMAIN_FUNCTIONAL_LEVEL);
        if (pAttr != NULL)
        {
            // get requested dfl value
            dwReqDfl = atoi(BERVAL_NORM_VAL(pAttr->vals[0]));

            if (dwReqDfl < gVmdirServerGlobals.dwDomainFunctionalLevel)
            {
                // Don't allow downgrade of DFL
                if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
                {
                    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                                     "Downgrade of domain functional level not allowed (%d)",
                                     dwReqDfl);
                    BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_FUNC_LVL);
                }
                // Replicated DFL is lower
                else
                {
                    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                       "WARNING WARNING WARNING Domain functional level modification (%d) "
                                       "is lower than local DFL (%d)",
                                       dwReqDfl,
                                       gVmdirServerGlobals.dwDomainFunctionalLevel);

                }
            }

            // Can local DC support DFL?
            if(dwReqDfl > VMDIR_MAX_DFL)
            {
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                                 "Server cannot support requested domain functional level (%d)",
                                 dwReqDfl);

                if (pOperation->opType == VDIR_OPERATION_TYPE_REPL)
                {
                    // This server cannot perform as intended in this domain, shut it down.
                    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);
                    VmDirForceExit();
                }
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_FUNC_LVL);
            }

            // Check if domain can support requested DFL
            if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
            {
                dwError = VmDirAllocateStringPrintf(
                                &pszDCContainerDN,
                                "OU=%s,%s",
                                VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                                pszDomainDN);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirSimpleEqualFilterInternalSearch(
                                                pszDCContainerDN,
                                                LDAP_SCOPE_ONE,
                                                ATTR_OBJECT_CLASS,
                                                OC_COMPUTER,
                                                &entryArray);
                BAIL_ON_VMDIR_ERROR(dwError);

                // For each DC
                for (iCnt = 0; iCnt < entryArray.iSize; iCnt++)
                {
                    // Find its version number
                    pAttrVer = VmDirFindAttrByName(&(entryArray.pEntry[iCnt]), ATTR_PSC_VERSION);
                    pAttrMaxDfl = VmDirFindAttrByName(&(entryArray.pEntry[iCnt]), ATTR_MAX_DOMAIN_FUNCTIONAL_LEVEL);
                    pAttrCn = VmDirFindAttrByName(&(entryArray.pEntry[iCnt]), ATTR_CN);

                    // Get Cn of DC
                    if (pAttrCn)
                    {
                        pszCn = BERVAL_NORM_VAL(pAttrCn->vals[0]);
                    }
                    else
                    {
                        pszCn = "n/a";
                    }

                    if (pAttrMaxDfl)
                    {
                        dwMaxDfl = atoi(BERVAL_NORM_VAL(pAttrMaxDfl->vals[0]));
                    }
                    else if (pAttrVer)
                    {
                        dwMaxDfl = 0;

                        // Get its max DFL for version
                        dwError = VmDirMapVersionToMaxDFL(BERVAL_NORM_VAL(pAttrVer->vals[0]), &dwMaxDfl);
                        BAIL_ON_VMDIR_ERROR(dwError);

                    }
                    // PSC version not found for node, use default.
                    else
                    {
                        dwMaxDfl = VMDIR_DFL_DEFAULT;
                    }

                    // Can DC support requested DFL?
                    if( dwMaxDfl <  dwReqDfl)
                    {
                        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                                         "Domain Controller (%s) cannot support requested domain functional level (%d)",
                                         pszCn,
                                         dwReqDfl);
                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_FUNC_LVL);
                    }
                }
            }
        }
    }
cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCContainerDN);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    goto cleanup;

}


/*
 * Generic place to validate attribute values for LDAP_ADD operation.
 * pEntry now contains ONLY values coming from the wire.
 * i.e. before any internal logic to add/modify pEntry.
 *
 * 1. enforce UPN format - has '@'
 * 2. enforce SPN format - has '@'
 */
static
DWORD
_VmDirPluginGenericPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD            dwError = 0;
    PVDIR_ATTRIBUTE  pAttrUPN   = VmDirFindAttrByName(pEntry, ATTR_KRB_UPN);
    PVDIR_ATTRIBUTE  pAttrSPN   = VmDirFindAttrByName(pEntry, ATTR_KRB_SPN);
    PVDIR_ATTRIBUTE  pAttrSD    = VmDirFindAttrByName(pEntry, ATTR_OBJECT_SECURITY_DESCRIPTOR);
    PSTR             pszLocalErrMsg = NULL;

    dwError = _VmDirJoinCreateComputerAccount(pEntry, &pAttrUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pAttrUPN )
    {
        dwError = VmDirValidatePrincipalName( pAttrUPN, &pszLocalErrMsg );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifdef WINJOIN_CHECK_ENABLED
    /* Safe to allow multi-valued SPN's */
    pAttrSPN = NULL;
#endif
    if ( pAttrSPN )
    {
        dwError = VmDirValidatePrincipalName( pAttrSPN, &pszLocalErrMsg );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pAttrSD)
    {
        BOOLEAN bReturn = FALSE;

        bReturn = VmDirValidRelativeSecurityDescriptor(
                    (PSECURITY_DESCRIPTOR_RELATIVE)pAttrSD->vals[0].lberbv_val,
                    (ULONG)pAttrSD->vals[0].lberbv_len,
                    OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION);
        if (!bReturn)
        {
             dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
             BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );

    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszLocalErrMsg);

    goto cleanup;
}

static
DWORD
_VmDirPluginPasswordHashPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD            dwError = 0;
    VDIR_BERVALUE    bervHashDigest     = VDIR_BERVALUE_INIT;
    PVDIR_ATTRIBUTE  pAttrPasswd        = VmDirFindAttrByName(pEntry, ATTR_USER_PASSWORD);
    PVDIR_ATTRIBUTE  pAttrSchemeName    = VmDirFindAttrByName(pEntry, ATTR_PASSWORD_SCHEME);
    char             pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    PBYTE            pSalt = NULL;
    PCSTR            pszErrorContext = NULL;

    /* Windows computer joining passes the machine account pwd in unicode */
    dwError = _VmDirAttrUnicodePwdToPwd(pEntry, &pAttrPasswd);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pAttrPasswd)
    {
        if (VMDIR_IS_OP_CTRL_PASSBLOB(pOperation))
        {
            // import blob as is.
            goto cleanup;
        }
        else if (pAttrSchemeName)
        {   // during data migration, we allow client to specify which hash scheme is used in their data
            if (VmDirStringCompareA(
                    pAttrSchemeName->vals[0].lberbv.bv_val, PASSWD_SCHEME_VMDIRD, FALSE) == 0)
            {   // importing vmdird digest
                pszErrorContext = "Verify supported password scheme";
                dwError = VdirPasswordVerifySupportedScheme(&pAttrPasswd->vals[0]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {   // import foreign digest
                pszErrorContext = "Add password scheme code";
                dwError = VdirPasswordAddSchemeCode(
                         pAttrSchemeName,
                         &pAttrPasswd->vals[0],     // bv_val from wire
                         &bervHashDigest);          // output to be stored in userpassword
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            // we do not wan't to store ATTR_PASSWROD_SCHEME in entry.
            // it is just a sudo value to indicate which scheme the userpassword value is in
            pszErrorContext = "Remove password scheme attribute";
            dwError = VmDirEntryRemoveAttribute(pEntry, ATTR_PASSWORD_SCHEME);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            PVDIR_PASSWORD_HASH_SCHEME pPasswdScheme = VdirDefaultPasswordScheme();

            // handle krb password logic first.
            pszErrorContext = "Krb password add";
            dwError = VmDirKrbUPNKeySet( pOperation, pEntry, &(pAttrPasswd->vals[0]) );
            BAIL_ON_VMDIR_ERROR(dwError);

            // handle srp password logic.
            pszErrorContext = "srp password add";
            dwError = VmDirSRPSetSecret( pOperation, pEntry, &(pAttrPasswd->vals[0]) );
            BAIL_ON_VMDIR_ERROR(dwError);

            // make sure every new entry added comply with password strength policy (if defined)
            pszErrorContext = "Password strength check";
            dwError = VdirPasswordStrengthCheck(
                            &pAttrPasswd->vals[0],
                            NULL,
                            &pOperation->request.addReq.pEntry->dn);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszErrorContext = "Password gen(S)";
            dwError = VdirPasswordGenerateSalt(pPasswdScheme, &pSalt);
            BAIL_ON_VMDIR_ERROR(dwError);

            // user password is a single value attribute
            pszErrorContext = "Password gen(H)";
            dwError = VdirPasswordHash(
                        pPasswdScheme,
                        &pAttrPasswd->vals[0],      // bv_val from wire
                        &bervHashDigest,            // output to be stored in userpassword
                        pSalt);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (bervHashDigest.lberbv.bv_len > 0)
        {   // pAttrPasswd->vals[0] takes over bervHashDigest content if we have new digest
            VmDirFreeBervalContent(&pAttrPasswd->vals[0]);
            TRANSFER_BERVALUE(bervHashDigest, pAttrPasswd->vals[0]);
        }

        VmDirStringNPrintFA(pszTimeBuf, sizeof(pszTimeBuf), sizeof(pszTimeBuf) - 1, "%d", (int)time(NULL));
        // also add pwdlastSet attribute if we have userPassword
        dwError = VmDirEntryAddSingleValueStrAttribute(
                        pEntry,
                        ATTR_PWD_LAST_SET,
                        pszTimeBuf);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pSalt);

    /* Machine account pwd is sensitive; do not store in vmdir */
    VmDirEntryRemoveAttribute(pEntry, ATTR_UNICODE_PWD);

    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    VmDirFreeBervalContent(&bervHashDigest);

    goto cleanup;
}

/*
 * Only users and groups ("security principals") require a real SID. Domain
 * objects need the domain-specific SID we store there (to construct SIDs for
 * real security principals). Rather than hard-code the classes that get a
 * SID here we just let the schema definition drive the logic.
 */
DWORD
_VmDirNeedsSid(
    PVDIR_ENTRY pEntry,
    BOOLEAN *pbNeedsSid
    )
{
    BOOLEAN bMustHaveAttr = FALSE;
    BOOLEAN bMayHaveAttr = FALSE;
    DWORD dwError = 0;

    dwError = VmDirEntryIsAttrAllowed(
                pEntry,
                ATTR_OBJECT_SID,
                &bMustHaveAttr,
                &bMayHaveAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbNeedsSid = bMustHaveAttr || bMayHaveAttr;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirPluginGenerateSidPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD           dwError = 0;
    PSTR            pszObjectSid = NULL;
    BOOLEAN         bNeedsSid = FALSE;
    PVDIR_ATTRIBUTE pObjectSidAttrExist = NULL;

    PVDIR_ATTRIBUTE pObjectSidAttr = NULL;
    // Do not free ref
    PVDIR_ATTRIBUTE pOrganizationAttr = NULL;
    PCSTR           pszErrorContext = NULL;

    assert(pEntry);

    // If find objectsid do not generate again
    pObjectSidAttrExist = VmDirEntryFindAttribute(
                             ATTR_OBJECT_SID,
                             pEntry);
    if (pObjectSidAttrExist)
    {
        goto cleanup;
    }

    dwError = _VmDirNeedsSid(pEntry, &bNeedsSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    if (!bNeedsSid)
    {
        goto cleanup;
    }

    pszErrorContext = "Generate object sid";
    dwError = VmDirGenerateObjectSid(pEntry,
                                     &pszObjectSid);
    if (dwError == ERROR_NO_OBJECT_SID_GEN)
    {
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pszErrorContext = "Allocate object sid";
    dwError = VmDirAttributeAllocate(
                ATTR_OBJECT_SID,
                1,
                pEntry->pSchemaCtx,
                &pObjectSidAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                pszObjectSid,
                &pObjectSidAttr->vals[0].lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pObjectSidAttr->vals[0].bOwnBvVal = TRUE;
    pObjectSidAttr->vals[0].lberbv.bv_len = VmDirStringLenA(pObjectSidAttr->vals[0].lberbv.bv_val);

    pszErrorContext = "Add object sid attribute";
    dwError = VmDirEntryAddAttribute(
                pEntry,
                pObjectSidAttr);
    BAIL_ON_VMDIR_ERROR(dwError);
    pObjectSidAttr = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszObjectSid);

    return dwError;
error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    if (pObjectSidAttr)
    {
        VmDirFreeAttribute(pObjectSidAttr);
    }

    if (pOrganizationAttr)
    {
        VmDirFreeAttribute(pOrganizationAttr);
    }
    goto cleanup;
}

/*
 * pluginAddOpAttrsPreAdd(): Create and add operational attributes to the entry (coming from wire).
 * For now, it creates values for, and adds the following operational attributes:
 *  - uSNCreated: Update Sequence Number for the create/add operation.
 *  - uSNChanged: Update Sequence Number for the modify operation.
 *  When the entry is created, usnCreated = usnModified
 *
 * 1. createtimestamp
 * 2. modifytimestamp
 * 3. creator (TODO)
 * 4. modifier (TODO)
 *
 */
static
DWORD
_VmDirPluginAddOpAttrsPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD        dwError = 0;
    USN          usn = 0;
    char         usnStr[VMDIR_MAX_USN_STR_LEN];
    char         pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    uuid_t       guid;
    char         objectGuidStr[VMDIR_GUID_STR_LEN];
    PCSTR        pszErrorContext = NULL;
    PCSTR        pszCreatorsDN = NULL;
    PVDIR_ATTRIBUTE pAttrModifyTimeStamp = VmDirFindAttrByName(pEntry,ATTR_MODIFYTIMESTAMP);
    PVDIR_ATTRIBUTE pAttrCreateTimeStamp = VmDirFindAttrByName(pEntry,ATTR_CREATETIMESTAMP);

    // Get/create USN value
    pszErrorContext = "Get next USN";
    dwError = pOperation->pBEIF->pfnBEGetNextUSN( pOperation->pBECtx, &usn );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirStringNPrintFA( usnStr, sizeof(usnStr), sizeof(usnStr) - 1, "%" PRId64, usn);

    // Append usnCreated attribute
    pszErrorContext = "Add USN create attribute";
    dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_USN_CREATED, usnStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Append usnChanged attribute
    pszErrorContext = "Add USN change attribute";
    dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_USN_CHANGED, usnStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Append DN attribute
    pszErrorContext = "Add DN attribute";
    dwError = VmDirEntryAddSingleValueAttribute(pEntry, ATTR_DN, pEntry->dn.lberbv.bv_val, pEntry->dn.lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirCurrentGeneralizedTime(pszTimeBuf, sizeof(pszTimeBuf));

    /* Add attributes for AD compatibility */
    dwError = _VmDirAddAdMachineAccount(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // We auto generate modifytimestamp or take value from input if allowed.
    if (gVmdirGlobals.bAllowImportOpAttrs == FALSE && pAttrModifyTimeStamp)
    {
        pszErrorContext = "Operational Attribute modifytimestamp is NOT allowed";
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!pAttrModifyTimeStamp)
    {
        // ModifyTimeStamp
        pszErrorContext = "Add modify timestamp";
        dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_MODIFYTIMESTAMP, pszTimeBuf);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // We auto generate createtimestamp or take value from input if allowed.
    if (gVmdirGlobals.bAllowImportOpAttrs == FALSE && pAttrCreateTimeStamp)
    {
        pszErrorContext = "Operational Attribute createtimestamp is NOT allowed";
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!pAttrCreateTimeStamp)
    {
        // CreateTimeStamp
        pszErrorContext = "Add create timestamp";
        dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_CREATETIMESTAMP, pszTimeBuf);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszCreatorsDN = VMDIR_CURRENT_AUTHENTICATED_DN( &(pOperation->conn->AccessInfo) );
    if ( IsNullOrEmptyString( pszCreatorsDN) )
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "No creatorsName attribute");
    }
    else
    {
        dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_CREATORS_NAME, pszCreatorsDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirEntryAddSingleValueStrAttribute(pEntry, ATTR_MODIFIERS_NAME, pszCreatorsDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    assert (pEntry->pszGuid == NULL);

    /*
    The uuid_unparse function converts the supplied UUID uu from the binary
    representation into a 36-byte string (plus tailing '\0')
    of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427.
    */
    VmDirUuidGenerate (&guid);
    VmDirUuidToStringLower(&guid, objectGuidStr, sizeof(objectGuidStr));

    dwError = VmDirAllocateStringA(objectGuidStr, &pEntry->pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszErrorContext = "Add object guid";
    dwError = VmDirEntryAddSingleValueStrAttribute( pEntry, ATTR_OBJECT_GUID, pEntry->pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    goto cleanup;
}

/* If "member" attribute value(s) have "special" DN format, map attribute values to ForeignSecurityPrincipal (FSP)
 * objects's DNs, and create FSP objects, if not already exist in the DB.
 */

static
DWORD
_VmDirPluginCreateFSPsPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD            dwError = 0;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    PCSTR            pszErrorContext = NULL;

    pAttr = VmDirFindAttrByName(pEntry, ATTR_MEMBER); // SJ-TBD: ATTR_MEMBER should be "FSP enabled"
    if (pAttr)
    {
        pszErrorContext = "Handle FSP DNs";
        dwError = _VmDirHandleFSPDNs( &pEntry->dn, pAttr, pEntry->pSchemaCtx, TRUE /* create FSP objects */ );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    goto cleanup;
}

static
DWORD
_VmDirPluginSchemaEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD   dwRtn = 0;
    PVDIR_ATTRIBUTE pCnAttr = NULL;
    PSTR    pszSchemaIdGuid = NULL;

    if (pOperation->dwSchemaWriteOp)
    {
        // lDAPDisplayName attribute takes cn as default
        if (!VmDirFindAttrByName(pEntry, ATTR_LDAP_DISPLAYNAME))
        {
            pCnAttr = VmDirFindAttrByName(pEntry, ATTR_CN);
            if (!pCnAttr)
            {
                dwRtn = VMDIR_ERROR_INVALID_ENTRY;
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }

            dwRtn = VmDirEntryAddSingleValueStrAttribute(
                    pEntry,
                    ATTR_LDAP_DISPLAYNAME,
                    pCnAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwRtn);
        }

        // schemaIDGUID attribute takes a generated guid as default
        if (!VmDirFindAttrByName(pEntry, ATTR_SCHEMAID_GUID))
        {
            dwRtn = VmDirGenerateGUID(&pszSchemaIdGuid);
            BAIL_ON_VMDIR_ERROR(dwRtn);

            dwRtn = VmDirEntryAddSingleValueStrAttribute(
                    pEntry,
                    ATTR_SCHEMAID_GUID,
                    pszSchemaIdGuid);
            BAIL_ON_VMDIR_ERROR(dwRtn);
        }

        if (VmDirEntryIsObjectclass(pEntry, OC_CLASS_SCHEMA))
        {
            // defaultObjectCategory attribute takes dn as default
            if (!VmDirFindAttrByName(pEntry, ATTR_DEFAULT_OBJECT_CATEGORY))
            {
                dwRtn = VmDirEntryAddSingleValueStrAttribute(
                        pEntry,
                        ATTR_DEFAULT_OBJECT_CATEGORY,
                        pEntry->dn.lberbv.bv_val);
                BAIL_ON_VMDIR_ERROR(dwRtn);
            }
        }
    }

error:
    VMDIR_SAFE_FREE_MEMORY(pszSchemaIdGuid);
    return dwPriorResult ? dwPriorResult : dwRtn;
}

static
DWORD
_VmDirPluginSchemaLibUpdatePreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD   dwRtn = 0;

    if (pOperation->dwSchemaWriteOp)
    {
        dwRtn = VmDirSchemaCheck(pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);

        dwRtn = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
        BAIL_ON_VMDIR_ERROR(dwRtn);
    }

error:
    return dwPriorResult ? dwPriorResult : dwRtn;
}

static
DWORD
_VmDirPluginSchemaLibUpdatePostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwResult)
{
    return _VmDirPluginSchemaLibUpdatePostModifyCommit(
            pOperation, pEntry, dwResult);
}

static
DWORD
_VmDIrPluginPasswordPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,     // pEntry is NULL
    DWORD            dwPriorResult)
{
    DWORD           dwError = 0;

    dwError = VdirPasswordModifyPreCheck(pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirPluginReplaceOpAttrsPreModApplyModify(
    PVDIR_OPERATION pOperation,
    PVDIR_ENTRY     pEntry,     // pEntry is NULL
    DWORD           dwPriorResult)
{
    DWORD   dwError = 0;
    USN     usn = 0;
    char    usnStr[VMDIR_MAX_USN_STR_LEN];
    char    pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    PCSTR   pszErrorContext = NULL;
    PCSTR   pszModifiersDN = NULL;

    // Get/create USN value

    pszErrorContext = "Get next USN";
    dwError = pOperation->pBEIF->pfnBEGetNextUSN( pOperation->pBECtx, &usn );
    BAIL_ON_VMDIR_ERROR( dwError );

    VmDirStringNPrintFA( usnStr, sizeof(usnStr), sizeof(usnStr) - 1, "%" PRId64, usn);

    pszErrorContext = "Replace USN change attribute";
    dwError = VmDirAppendAMod( pOperation, MOD_OP_REPLACE, ATTR_USN_CHANGED, ATTR_USN_CHANGED_LEN, usnStr, VmDirStringLenA( usnStr ) );
    BAIL_ON_VMDIR_ERROR( dwError );

    VmDirCurrentGeneralizedTime(pszTimeBuf, sizeof(pszTimeBuf));

    pszErrorContext = "Replace modify timestamp";
    dwError = VmDirAppendAMod( pOperation, MOD_OP_REPLACE, ATTR_MODIFYTIMESTAMP, ATTR_MODIFYTIMESTAMP_LEN,
                               pszTimeBuf, VmDirStringLenA( pszTimeBuf ) );
    BAIL_ON_VMDIR_ERROR( dwError );

    pszModifiersDN = VMDIR_CURRENT_AUTHENTICATED_DN( &(pOperation->conn->AccessInfo) );
    if ( IsNullOrEmptyString( pszModifiersDN) )
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "No modifiersName attribute");
    }
    else if (VmDirStringCompareA( pOperation->request.modifyReq.dn.lberbv_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE) == 0 &&
             VmDirHaveLegacy() == TRUE)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Bypass replacing attr MODIFIERS_NAME on entry %s on Legacy compatible mode",
                        SUB_SCHEMA_SUB_ENTRY_DN);
    }
    else
    {
        dwError = VmDirAppendAMod( pOperation, MOD_OP_REPLACE, ATTR_MODIFIERS_NAME, ATTR_MODIFIERS_NAME_LEN,
                                   (PSTR)pszModifiersDN, VmDirStringLenA( pszModifiersDN ) );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    goto cleanup;
}

/* If "member" attribute value(s) have "special" DN format, map attribute values to ForeignSecurityPrincipal (FSP)
 * objects's DNs, and create FSP objects, if not already exist in the DB.
 */

static
DWORD
_VmDIrPluginHandleFSPsPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,    // pEntry is NULL
    DWORD            dwPriorResult)
{
    DWORD                dwError = 0;
    PVDIR_MODIFICATION   pMod = NULL;
    PCSTR                pszErrorContext = NULL;

    for (pMod = pOperation->request.modifyReq.mods; pMod; pMod = pMod->next)
    {
        if (VmDirStringCompareA( pMod->attr.type.lberbv.bv_val, ATTR_MEMBER, FALSE ) == 0) // SJ-TBD: ATTR_MEMBER should be "FSP enabled"
        {
            switch ( pMod->operation )
            {
                case MOD_OP_ADD:
                case MOD_OP_REPLACE:
                    pszErrorContext = "Add or replace FSP DNs";
                    dwError = _VmDirHandleFSPDNs( &pOperation->request.modifyReq.dn, &pMod->attr, pOperation->pSchemaCtx,
                                            TRUE /* create FSP objects */);
                    BAIL_ON_VMDIR_ERROR(dwError);
                    break;

                case MOD_OP_DELETE:
                    pszErrorContext = "Delete FSP DNs";
                    dwError = _VmDirHandleFSPDNs( &pOperation->request.modifyReq.dn, &pMod->attr, pOperation->pSchemaCtx,
                                            FALSE /* creation of FSP objects not involved in this case,
                                                     just attribute value mapping */ );
                    BAIL_ON_VMDIR_ERROR(dwError);
                    break;

                default:
                    assert( FALSE );
            }
        }
    }

cleanup:
    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

static
DWORD
_VmDirPluginSetDeletedObjAttrsPreModApplyDelete(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,     // pEntry is NULL
    DWORD            dwPriorResult
    )
{
    DWORD           dwError = 0;
    PCSTR           pszErrorContext = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "pluginSetDeletedObjAttrsPreModApplyDelete: Begin, entry DN = %s",
              pOperation->request.deleteReq.dn.lberbv.bv_val );

    pszErrorContext = "Add ATTR_IS_DELETED attribute";
    dwError = VmDirAppendAMod( pOperation, MOD_OP_ADD, ATTR_IS_DELETED, ATTR_IS_DELETED_LEN,
                          VMDIR_IS_DELETED_TRUE_STR, VMDIR_IS_DELETED_TRUE_STR_LEN );
    BAIL_ON_VMDIR_ERROR( dwError );

    pszErrorContext = "Add ATTR_LAST_KNOWN_DN attribute";
    dwError = VmDirAppendAMod( pOperation, MOD_OP_ADD, ATTR_LAST_KNOWN_DN, ATTR_LAST_KNOWN_DN_LEN,
                               pOperation->request.deleteReq.dn.lberbv.bv_val, pOperation->request.deleteReq.dn.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "pluginSetDeletedObjAttrsPreModApplyDelete: End." );

    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

static
DWORD
_VmDirPluginSchemaLibUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD   dwRtn = 0;

    if (pOperation->dwSchemaWriteOp)
    {
        dwRtn = VmDirSchemaLibUpdate(dwPriorResult);
    }

    return dwPriorResult ? dwPriorResult : dwRtn;
}

/*
 * Update cached domain functional level
 */
static
DWORD
_VmDirPluginDflUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    PVDIR_MODIFICATION  pMod = NULL;

    // check if domain object
    if (gVmdirServerGlobals.systemDomainDN.bvnorm_val &&
        VmDirStringCompareA(
                BERVAL_NORM_VAL(pEntry->dn),
                gVmdirServerGlobals.systemDomainDN.bvnorm_val,
                FALSE) == 0)
    {
        // search for vmwDomainFunctionalLevel mod
        pMod = pOperation->request.modifyReq.mods;
        while (pMod)
        {
            if (pMod->operation == MOD_OP_ADD &&
                VmDirStringCompareA(
                        BERVAL_NORM_VAL(pMod->attr.type),
                        ATTR_DOMAIN_FUNCTIONAL_LEVEL,
                        FALSE) == 0)
            {
                gVmdirServerGlobals.dwDomainFunctionalLevel =
                        atoi(BERVAL_NORM_VAL(pMod->attr.vals[0]));

                VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "Domain Functional Level cache changed to (%d)",
                        gVmdirServerGlobals.dwDomainFunctionalLevel);

                break;
            }

            pMod = pMod->next;
        }
    }

    return dwPriorResult;
}

/*
 * Initialize gVmdirPluginGlobals.pXXX with static init table contents.
 */
static
DWORD
_VmDirPluginInit(
    PVDIR_OP_PLUGIN_INFO*   ppHead,
    PVDIR_OP_PLUGIN_INFO    pTbl,
    int                     iSize
    )
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    PVDIR_OP_PLUGIN_INFO    pHead = NULL;

    for (iCnt = iSize; iCnt > 0; iCnt--)
    {
        PVDIR_OP_PLUGIN_INFO    pInfo = NULL;

        dwError = VmDirAllocateMemory(
                sizeof(*pInfo),
                (PVOID*)&pInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        pInfo->usOpMask = pTbl[iCnt-1].usOpMask;
        pInfo->bSkipOnError = pTbl[iCnt-1].bSkipOnError;
        pInfo->pPluginFunc = pTbl[iCnt-1].pPluginFunc;

        if (pHead)
        {
            pInfo->pNext = pHead;
            pHead = pInfo;
        }
        else
        {
            pHead = pInfo;
        }
    }

    *ppHead = pHead;

cleanup:

    return dwError;

error:

    _VmDirFreePlugins(pHead);
    goto cleanup;
}

static
VOID
_VmDirFreePlugins(
    PVDIR_OP_PLUGIN_INFO   pHead
    )
{
    PVDIR_OP_PLUGIN_INFO pCurrPluginInfo = pHead;

    while (pCurrPluginInfo)
    {
        PVDIR_OP_PLUGIN_INFO pNext = pCurrPluginInfo->pNext;

        VMDIR_SAFE_FREE_MEMORY(pCurrPluginInfo); // this free pCurrPluginInfo
        pCurrPluginInfo = pNext;
    }
}

/* Given a "special" DN, generate the corresponding FSP (ForeignSecurityPrincipal) DN.
 *
 */
static
DWORD
_VmDirConstructFSPDN(
    PVDIR_BERVALUE   pEntryDn,
    PVDIR_BERVALUE   pSpecialDn,
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PSTR *           ppszFSPDN
    )
{
    DWORD            dwError = 0;
    VDIR_BERVALUE    parentDn = VDIR_BERVALUE_INIT;
    PSTR             pszFSPDN = NULL;
    PCSTR            pszDomainDN = NULL;

    *ppszFSPDN = NULL;

    // Normalize DN
    dwError = VmDirNormalizeDN( pSpecialDn, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirGetParentDN( pSpecialDn, &parentDn );

    // if DN looks like "objectId=<something>" (FSP in AD/Openldap etc...)
    if ((parentDn.lberbv.bv_len == 0) &&
         VmDirStringNCompareA( pSpecialDn->bvnorm_val, ATTR_FSP_OBJECTID, ATTR_FSP_OBJECTID_LEN, FALSE) == 0 &&
         pSpecialDn->bvnorm_val[ATTR_FSP_OBJECTID_LEN] == '=')
    {
        // get the domain DN this entry belongs to
        assert( pEntryDn->bvnorm_val );
        pszDomainDN = VmDirFindDomainDN(pEntryDn->bvnorm_val);
        assert( pszDomainDN );

        // FSP DN looks like: objectIdd=<a SID or entryUUID or etc...>,cn=ForeignSecurityPrincipals,<domain DN>
        dwError = VmDirAllocateStringPrintf( &pszFSPDN, "%s,%s=%s,%s", pSpecialDn->lberbv.bv_val,
                                                 FSP_CONTAINER_RDN_ATTR, FSP_CONTAINER_RDN_ATTR_VALUE,
                                                 pszDomainDN );
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppszFSPDN = pszFSPDN;
    }

cleanup:
    VmDirFreeBervalContent( &parentDn );
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY( pszFSPDN );
    goto cleanup;
}

/* If "member" attribute value(s) have "special" DN format, map attribute values to ForeignSecurityPrincipal (FSP)
 * objects's DNs, and create FSP objects, if asked for (thru createFSPs parameter).
 */

static
DWORD
_VmDirHandleFSPDNs(
    PVDIR_BERVALUE   pEntryDn,
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          createFSPs)
{
#define OBJ_ID_VAL_SIZE 1024
    DWORD            dwError = 0;
    PSTR             pszFSPDN = NULL;
    unsigned int     iCnt = 0;
    char             objectIdVal[OBJ_ID_VAL_SIZE]; // SJ-TBD: Use a constant
    PSTR             ppszAttributes[] =
                        {
                            ATTR_OBJECT_CLASS,  OC_FOREIGN_SECURITY_PRINCIPAL,
                            ATTR_FSP_OBJECTID,  objectIdVal,
                            NULL
                        };

    if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_MEMBER, FALSE ) == 0) // SJ-TBD: ATTR_MEMBER should be "FSP enabled"
    {
        for (iCnt = 0; iCnt < pAttr->numVals; iCnt++)
        {
            dwError = _VmDirConstructFSPDN( pEntryDn, &pAttr->vals[iCnt], pSchemaCtx, &pszFSPDN );
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pszFSPDN != NULL)
            {
                // Replace member DN value

                if (VmDirStringNCompareA( pAttr->vals[iCnt].bvnorm_val, ATTR_FSP_OBJECTID, ATTR_FSP_OBJECTID_LEN, FALSE) == 0 &&
                        pAttr->vals[iCnt].bvnorm_val[ATTR_FSP_OBJECTID_LEN] == '=')
                {
                    VmDirStringCpyA( objectIdVal, OBJ_ID_VAL_SIZE, pAttr->vals[iCnt].bvnorm_val + ATTR_FSP_OBJECTID_LEN + 1 /* skipping '=' sign */);
                }

                VmDirFreeBervalContent(&pAttr->vals[iCnt]);
                pAttr->vals[iCnt].lberbv.bv_val = pszFSPDN;
                pAttr->vals[iCnt].lberbv.bv_len = VmDirStringLenA( pszFSPDN );
                pAttr->vals[iCnt].bOwnBvVal = TRUE;
                BAIL_ON_VMDIR_ERROR( dwError );

                if (createFSPs)
                {
                    // Create FSP object, in a transaction separate from the original transaction.
                    dwError = VmDirSimpleEntryCreate( pSchemaCtx, ppszAttributes, (PSTR)pszFSPDN, 0 /* No Fixed EID */);
                    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_EXISTS)
                    {
                        dwError = LDAP_SUCCESS;
                    }
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;

#undef OBJ_ID_VAL_SIZE
}

/*
 * after a password modify -
 * 1. if self change failed, trigger a password fail event
 * 2. if admin reset succeed, unset user account control flag and clean up lockout cache record
 *
 *
 * TBD, this should be done by a BG job instead of operation thr PostModifyCommit hook/plugin
 */
static
DWORD
_VmDirpluginPasswordPostModifyCommit(
        PVDIR_OPERATION  pOperation,
        PVDIR_ENTRY      pEntry,
        DWORD            dwPriorResult
        )
{
    DWORD   dwError = 0;

    if (pOperation->request.modifyReq.bPasswordModify)
    {
        if ( dwPriorResult == VMDIR_ERROR_PASSWORD_POLICY_VIOLATION
             ||
             dwPriorResult == VMDIR_ERROR_USER_INVALID_CREDENTIAL
           )
        {
            // could be from VdirPasswordModifyPreCheck (pEntry has NO content) or
            //          from password modify fail (bad password)
            VdirPasswordFailEvent(
                        pOperation,
                        BERVAL_NORM_VAL(pOperation->request.modifyReq.dn),
                        pEntry->eId != 0 ? pEntry : NULL);

            // should always return LDAP_INVALID_CREDENTIALS
            dwError = LDAP_INVALID_CREDENTIALS;

            VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "Password Modification Failed (%s). "
                "Bind DN: \"%s\". "
                "Modified DN: \"%s\"",
                pOperation->conn->szClientIP,
                VDIR_SAFE_STRING(pOperation->conn->AccessInfo.pszNormBindedDn),
                VDIR_SAFE_STRING(pOperation->request.modifyReq.dn.lberbv_val));
        }
        else if (pEntry->eId != 0 && dwPriorResult == 0)
        {   // ignore error for post modify handling
            if (!pOperation->conn->AccessInfo.pszNormBindedDn   ||
                VmDirStringCompareA(BERVAL_NORM_VAL(pEntry->dn),
                                    pOperation->conn->AccessInfo.pszNormBindedDn,
                                    FALSE) != 0
               )
            {
                // if reset password succeeded by admin, clear out
                //     USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG and
                //     USER_ACCOUNT_CONTROL_LOCKOUT_FLAG flags.
                VdirUserActCtlFlagUnset(pEntry, USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG |
                                                USER_ACCOUNT_CONTROL_LOCKOUT_FLAG );

                VdirLockoutCacheRemoveRec(BERVAL_NORM_VAL(pOperation->request.modifyReq.dn));
            }

            VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "Password Modification Successful (%s). "
                "Bind DN: \"%s\". "
                "Modified DN: \"%s\"",
                pOperation->conn->szClientIP,
                VDIR_SAFE_STRING(pOperation->conn->AccessInfo.pszNormBindedDn),
                VDIR_SAFE_STRING(pOperation->request.modifyReq.dn.lberbv_val));
        }

    }

    return dwError;
}

static
DWORD
_VmDirPluginLockoutCachePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pUserActCtlAttr = NULL;
    int64_t         userAccountCtrlFlags = 0;

    pUserActCtlAttr = VmDirFindAttrByName(pEntry, ATTR_USER_ACCOUNT_CONTROL);
    // In general, we do not expect an attribute w/o value(s).
    // To work around bad data in deleted container observed in CME SR case 15820658212 (PR 1644319),
    //   make sure attibute does have value by numVals check before accessing it.
    if (pUserActCtlAttr && (pUserActCtlAttr->numVals > 0) )
    {
        userAccountCtrlFlags = VmDirStringToLA(pUserActCtlAttr->vals[0].lberbv.bv_val, NULL, 10);

        if ((userAccountCtrlFlags & USER_ACCOUNT_CONTROL_LOCKOUT_FLAG) == 0) // Account is NOT locked out
        {
            // Remove LockoutCache entry, if it is there
            VdirLockoutCacheRemoveRec(BERVAL_NORM_VAL(pOperation->request.modifyReq.dn));
        }
    }
    else
    { // Remove LockoutCache entry, if it is there
        VdirLockoutCacheRemoveRec(BERVAL_NORM_VAL(pOperation->request.modifyReq.dn));
    }

    return dwError;
}

/*
 * Do NOT allow structure objectclass modification
 */
static
DWORD
_VmDirPluginHandleStructureOCPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD               dwError = 0;
    PVDIR_MODIFICATION  pModReq  = pOperation->request.modifyReq.mods;
    PVDIR_MODIFICATION  pMod = NULL;
    PVDIR_LDAP_RESULT   pLdapResult = &(pOperation->ldapResult);
    unsigned short      usCnt = 0;
    PVDIR_ENTRY         pCurrentEntry = NULL;
    BOOLEAN             bHasStrcutreOCName = FALSE;

    for (pMod = pModReq; pMod != NULL; pMod = pMod->next)
    {
        if ( !pMod->attr.type.lberbv.bv_val )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                                           "Invalid modify request: attr.type is null");
        }

        if ( VmDirStringCompareA( pMod->attr.type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE ) == 0 )
        {
            if ( pMod->operation != MOD_OP_REPLACE )
            {   // for MOD_OP_ADD and MOD_OP_DELETE, should NOT contain structure objectclass.
                for (usCnt = 0; usCnt < pMod->attr.numVals; usCnt++)
                {
                    if ( VmDirSchemaIsStructureOC( pOperation->pSchemaCtx, pMod->attr.vals[usCnt].lberbv.bv_val ) )
                    {
                        dwError = LDAP_CONSTRAINT_VIOLATION;
                        BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                                                       "Invalid modify request: modify structure objectclass");
                    }
                }
            }
            else
            {   // for MOD_OP_REPLACE, we should see leaf structure objectclass in modRequest.
                // make sure it is the same we have in DB.
                dwError = VmDirSimpleDNToEntry( pOperation->request.modifyReq.dn.lberbv_val, &pCurrentEntry );
                BAIL_ON_VMDIR_ERROR(dwError);

                for (usCnt = 0; usCnt < pMod->attr.numVals; usCnt++)
                {
                    if ( VmDirSchemaIsStructureOC( pOperation->pSchemaCtx, pMod->attr.vals[usCnt].lberbv.bv_val ) )
                    {
                        if ( VmDirSchemaIsNameEntryLeafStructureOC( pCurrentEntry, pMod->attr.vals[usCnt].lberbv.bv_val ) )
                        {
                            bHasStrcutreOCName = TRUE;
                        }
                        else
                        {   // do NOT allow any other structure objectclass except leaf one.
                            // NOTE, this may be a bit too strict.  We will error out parents in structure objectclass tree.
                            dwError = LDAP_CONSTRAINT_VIOLATION;
                            BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                                                           "Invalid modify request: modify structure objectclass");
                        }
                    }
                }

                if ( ! bHasStrcutreOCName ) // must have same leaf structure objectclass name in replace list.
                {
                    dwError = LDAP_CONSTRAINT_VIOLATION;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                                                   "Invalid modify request: modify structure objectclass");
                }
            }
        }
    }

cleanup:

    if (pCurrentEntry)
    {
        VmDirFreeEntry(pCurrentEntry);
    }

    return dwPriorResult ? dwPriorResult : dwError;

error:

    goto cleanup;
}

/*
 * Map ACLs from string format to SD format
 */

static
DWORD
_VmDirPluginMapAclStringAttributePreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD               dwError = 0;
    PVDIR_MODIFICATION  pModReq  = pOperation->request.modifyReq.mods;
    PVDIR_MODIFICATION  pMod = NULL;
    PVDIR_MODIFICATION  pPrevMod = NULL;
    PVDIR_LDAP_RESULT   pLdapResult = &(pOperation->ldapResult);
    PSTR                pSdAttrVal = NULL;
    ber_len_t           sdAttrLen = 0;

    for (pPrevMod = NULL, pMod = pModReq; pMod != NULL; )
    {
        if ( !pMod->attr.type.lberbv.bv_val )
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                "_VmDirPluginMapAclStringAttributePreModApplyModify: Invalid modify request: attr.type is null");
        }

        if ( VmDirStringCompareA( pMod->attr.type.lberbv.bv_val, ATTR_ACL_STRING, FALSE ) == 0 )
        {
            if (pMod->attr.numVals > 1)
            {
                dwError = VMDIR_ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                    "_VmDirPluginMapAclStringAttributePreModApplyModify: Invalid modify request: "
                    "> 1 values for a single valued attribute");

            }
            if (pMod->attr.numVals == 1) // there is A value to be mapped
            {
                if ((dwError = LwNtStatusToWin32Error( RtlAllocateSecurityDescriptorFromSddlCString(
                                                (PSECURITY_DESCRIPTOR_RELATIVE*)&pSdAttrVal,
                                                (PULONG)&sdAttrLen,
                                                pMod->attr.vals[0].lberbv.bv_val, SDDL_REVISION_1 ))) != 0)
                {
                    dwError = VMDIR_ERROR_INVALID_SYNTAX;
                }
                BAIL_ON_VMDIR_ERROR_WITH_MSG ( dwError, pLdapResult->pszErrMsg,
                    "_VmDirPluginMapAclStringAttributePreModApplyModify: Invalid modify request: "
                    "invalid vmwAclString attribute value");

                dwError = VmDirAppendAMod( pOperation, pMod->operation, /* original modify operation */
                                           ATTR_OBJECT_SECURITY_DESCRIPTOR, ATTR_OBJECT_SECURITY_DESCRIPTOR_LEN,
                                           pSdAttrVal, sdAttrLen );
                BAIL_ON_VMDIR_ERROR( dwError );

                // remove current ATTR_ACL_STRING mod
                if (pPrevMod == NULL)
                {
                    pOperation->request.modifyReq.mods = pMod->next;
                    VmDirModificationFree(pMod);
                    pMod = pOperation->request.modifyReq.mods;

                }
                else
                {
                    pPrevMod->next = pMod->next;
                    VmDirModificationFree(pMod);
                    pMod = pPrevMod->next;
                }

                continue;

            } // else fall-thru
        } // else fall-thru

        pPrevMod= pMod;
        pMod = pPrevMod->next;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pSdAttrVal );
    return dwPriorResult ? dwPriorResult : dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pLdapResult->pszErrMsg) );

    goto cleanup;
}

static
DWORD
_VmDirPluginVerifyAclAccess(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD               dwError = 0;
    PVDIR_MODIFICATION  pModReq  = pOperation->request.modifyReq.mods;
    PVDIR_MODIFICATION  pMod = NULL;
    PVDIR_ENTRY         pCurrentEntry = NULL;

    for (pMod = pModReq; pMod != NULL; pMod = pMod->next)
    {
        if (pMod->attr.type.lberbv.bv_val == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        //
        // In general a caller can modify an entry if they have
        // VMDIR_RIGHT_DS_WRITEPROP access. However, the entry's security
        // descriptor is special-cased and requires a separate permission
        // (VMDIR_ENTRY_WRITE_ACL). This is the same behavior as AD.
        //
        if (VmDirStringCompareA(pMod->attr.type.lberbv.bv_val, ATTR_ACL_STRING, FALSE) == 0 ||
            VmDirStringCompareA(pMod->attr.type.lberbv.bv_val, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE) == 0)
        {
            dwError = VmDirSimpleDNToEntry(pOperation->request.modifyReq.dn.lberbv_val, &pCurrentEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirSrvAccessCheck(pOperation, &pOperation->conn->AccessInfo, pCurrentEntry, VMDIR_ENTRY_WRITE_ACL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VmDirFreeEntry(pCurrentEntry);
    return dwPriorResult ? dwPriorResult : dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirPluginVerifyAclAccess failed with error %d", dwError);
    goto cleanup;
}
