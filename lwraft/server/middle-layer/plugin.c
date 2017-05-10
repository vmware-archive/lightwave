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
}

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_POST_MODIFY_COMMIT_PLUGIN_INITIALIZER                  \
{                                                                   \
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
// NOTE 2: _VmDirPluginRaftProxyPostAddCommit() plugin is called only when Add commit has succeeded.
#define VDIR_POST_ADD_COMMIT_PLUGIN_INITIALIZER                     \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginRaftProxyPostAddCommit),  \
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
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                            \
    VMDIR_SF_INIT(.pPluginFunc, VmDirPluginIndexEntryPostAdd),      \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// function parameter pEntry is NULL in this case!!
// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_PRE_MODAPPLY_DELETE_PLUGIN_INITIALIZER                 \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginSchemaLibUpdatePreModApplyDelete), \
    VMDIR_SF_INIT(.pNext, NULL )                                    \
    },                                                              \
}

// NOTE: order of fields MUST stay in sync with struct definition...
#define VDIR_POST_DELETE_COMMIT_PLUGIN_INITIALIZER                  \
{                                                                   \
    {                                                               \
    VMDIR_SF_INIT(.usOpMask, VDIR_NOT_REPL_OPERATIONS),             \
    VMDIR_SF_INIT(.bSkipOnError, TRUE),                             \
    VMDIR_SF_INIT(.pPluginFunc, _VmDirPluginReplAgrPostDeleteCommit), \
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
_VmDirPluginRaftProxyPostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult);

static
DWORD
_VmDirPluginReplAgrPostDeleteCommit(
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
_VmDirPluginSchemaLibUpdatePreModApplyDelete(
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

    if (pOperation->bSchemaWriteOp)
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

    if ( pAttrUPN )
    {
        dwError = VmDirValidatePrincipalName( pAttrUPN, &pszLocalErrMsg );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( pAttrSPN )
    {
        dwError = VmDirValidatePrincipalName( pAttrSPN, &pszLocalErrMsg );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pAttrSD)
    {
        BOOLEAN bReturn = FALSE;

        bReturn = VmDirValidRelativeSecurityDescriptor(
                    (PSECURITY_DESCRIPTOR_RELATIVE)pAttrSD->vals[0].bvnorm_val,
                    (ULONG)pAttrSD->vals[0].bvnorm_len,
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

    if (pAttrPasswd)
    {
        if (pAttrSchemeName)
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

    return dwError;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);

    VmDirFreeBervalContent(&bervHashDigest);

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
    char         pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    uuid_t       guid;
    char         objectGuidStr[VMDIR_GUID_STR_LEN];
    PCSTR        pszErrorContext = NULL;
    PCSTR        pszCreatorsDN = NULL;
    PVDIR_ATTRIBUTE pAttrModifyTimeStamp = VmDirFindAttrByName(pEntry,ATTR_MODIFYTIMESTAMP);
    PVDIR_ATTRIBUTE pAttrCreateTimeStamp = VmDirFindAttrByName(pEntry,ATTR_CREATETIMESTAMP);

    // Append DN attribute
    pszErrorContext = "Add DN attribute";
    dwError = VmDirEntryAddSingleValueAttribute(pEntry, ATTR_DN, pEntry->dn.lberbv.bv_val, pEntry->dn.lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirCurrentGeneralizedTime(pszTimeBuf, sizeof(pszTimeBuf));

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

    if (pOperation->bSchemaWriteOp)
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

        if (VmDirIsEntryWithObjectclass(pEntry, OC_CLASS_SCHEMA))
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

    if (pOperation->bSchemaWriteOp)
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

// Handle (ADD to replication agreements in-memory cache) MY replication agreements.

static
DWORD
_VmDirPluginRaftProxyPostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD dwError = 0;
    PCSTR pszErrorContext = "VmDirAddRaftProxy";

    dwError = VmDirAddRaftProxy(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

// Mark RA isDeleted = TRUE if present in gVmdirReplAgrs.

static
DWORD
_VmDirPluginReplAgrPostDeleteCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    //No repl agr for raft.
    return 0;
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
    char    pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    PCSTR   pszErrorContext = NULL;
    PCSTR   pszModifiersDN = NULL;

    // Get/create USN value

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
_VmDirPluginSchemaLibUpdatePreModApplyDelete(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    // reject delete if it's a schema entry
    DWORD   dwError = 0;
    PSTR    pszDN = BERVAL_NORM_VAL(pOperation->request.deleteReq.dn);

    if (VmDirStringEndsWith(pszDN, SCHEMA_NAMING_CONTEXT_DN, FALSE))
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_VmDirPluginSchemaLibUpdatePostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult)
{
    DWORD   dwRtn = 0;

    if (pOperation->bSchemaWriteOp)
    {
        dwRtn = VmDirSchemaLibUpdate(dwPriorResult);
    }

    return dwPriorResult ? dwPriorResult : dwRtn;
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
        dwError = VmDirAllocateStringAVsnprintf( &pszFSPDN, "%s,%s=%s,%s", pSpecialDn->lberbv.bv_val,
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
            // if reset password succeeded (either by admin or self), clear out
            //     USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG and
            //     USER_ACCOUNT_CONTROL_LOCKOUT_FLAG flags.
            VdirUserActCtlFlagUnset(pEntry, USER_ACCOUNT_CONTROL_PASSWORD_EXPIRE_FLAG |
                                            USER_ACCOUNT_CONTROL_LOCKOUT_FLAG );

            VdirLockoutCacheRemoveRec(BERVAL_NORM_VAL(pOperation->request.modifyReq.dn));

            dwError = 0;

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

/* The following functions (prefixed with VmDirRepl) perform pre/post operations (plugins)
 * at a Raft follower. Those plugins are much simpler than those executed at Raft leader because:
 *  1. All prechecks and validations are not neeed at the follower (have done at raft leader).
 *  2. All derived attributes (through plugin at Raft leader) are included in the Raft log,
 *     and have applied to the entry structure before posted to the backend.
 * However some functions in pre-plugin are still needed, such as Attribute Reindexing Schedule,
 * Rollback schema unique scope if the existing entries have violated the uniqueness.
 *
 * Fixme: Any other post plugin functions need to be included?
 */
DWORD
VmDirReplSchemaEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    DWORD dwError = 0;

    if (!pOperation->bSchemaWriteOp)
    {
        goto cleanup;
    }

    dwError = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirReplSchemaEntryPostAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    DWORD dwError = 0;

    if (!pOperation->bSchemaWriteOp)
    {
        goto cleanup;
    }

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPluginIndexEntryPostAdd(pOperation, pEntry, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirReplSchemaEntryPreMoidify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    DWORD dwError = 0;

    if (!pOperation->bSchemaWriteOp)
    {
        goto cleanup;
    }

    dwError = VmDirSchemaLibPrepareUpdateViaModify(pOperation, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPluginIndexEntryPreModify(pOperation, pEntry, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirReplSchemaEntryPostMoidify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    DWORD dwError = 0;

    if (!pOperation->bSchemaWriteOp)
    {
        goto cleanup;
    }

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}
