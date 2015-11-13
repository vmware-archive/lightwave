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
 * Module Name: Directory Schema
 *
 * Filename: api.c
 *
 * Abstract:
 *
 * Schema api
 *
 */

#include "includes.h"

static
BOOLEAN
vdirIsLiveSchema(
    PVDIR_SCHEMA_INSTANCE    pSchema
    );

static
BOOLEAN
_VmDirIsNameInCaseIgnoreList(
    PCSTR   pszName,
    PCSTR*  ppszList,
    size_t  iSize
    );

DWORD
VdirSchemaCtxAcquireInLock(
    BOOLEAN    bHasLock,    // TRUE if owns gVdirSchemaGlobals.mutex already
    PVDIR_SCHEMA_CTX* ppSchemaCtx
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bInLockNest = FALSE;
    PVDIR_SCHEMA_CTX    pCtx = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_CTX),
            (PVOID*)&pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bHasLock)
    {
        VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);
    }

    pCtx->pSchema = gVdirSchemaGlobals.pSchema;
    assert(pCtx->pSchema);

    VMDIR_LOCK_MUTEX(bInLockNest, pCtx->pSchema->mutex);
    pCtx->pSchema->usRefCount++;

error:

    VMDIR_UNLOCK_MUTEX(bInLockNest, pCtx->pSchema->mutex);
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    if (dwError != ERROR_SUCCESS)
    {
        dwError = ERROR_NO_SCHEMA;
        VMDIR_SAFE_FREE_MEMORY(pCtx);
        pCtx = NULL;
    }

    *ppSchemaCtx = pCtx;

    return dwError;
}

/*
 * Caller acquire schema ctx
 */
DWORD
VmDirSchemaCtxAcquire(
    PVDIR_SCHEMA_CTX* ppSchemaCtx
    )
{
	return VdirSchemaCtxAcquireInLock(FALSE, ppSchemaCtx);
}

PVDIR_SCHEMA_CTX
VmDirSchemaCtxClone(
    PVDIR_SCHEMA_CTX    pOrgCtx
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_SCHEMA_CTX    pCtx = NULL;

    assert(pOrgCtx);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_CTX),
            (PVOID*)&pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCtx->pSchema = pOrgCtx->pSchema;

    VMDIR_LOCK_MUTEX(bInLock, pCtx->pSchema->mutex);
    pCtx->pSchema->usRefCount++;

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pCtx->pSchema->mutex);

    return pCtx;

error:

    VMDIR_SAFE_FREE_MEMORY(pCtx);

    goto cleanup;
}

/*
 * Caller release schema ctx
 */
VOID
VmDirSchemaCtxRelease(
    PVDIR_SCHEMA_CTX    pCtx
    )
{
    BOOLEAN    bInLock = FALSE;
    USHORT     usRefCnt = 0;
    USHORT     usSelfRef = 0;

    if ( pCtx )
    {
        if (  pCtx->pSchema )
        {
            VMDIR_LOCK_MUTEX(bInLock, pCtx->pSchema->mutex);

            pCtx->pSchema->usRefCount--;
            usRefCnt = pCtx->pSchema->usRefCount;
            usSelfRef = pCtx->pSchema->usNumSelfRef;

            VMDIR_UNLOCK_MUTEX(bInLock, pCtx->pSchema->mutex);

            if (usRefCnt == usSelfRef)
            {   // only self reference within pSchema exists, free pSchema itself.
                // self references are established during init before normal references.
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                                "Free unreferenced schema instance (%p)",
                                pCtx->pSchema);
#if 0 /* BUGBUG - reenable this when Purify report is clean */
                VdirSchemaInstanceFree(pCtx->pSchema);
#endif
            }
        }

        VMDIR_SAFE_FREE_STRINGA(pCtx->pszErrorMsg);
        VMDIR_SAFE_FREE_MEMORY(pCtx);
    }

    return;
}

/*
 * Get context error message.
 */
PCSTR
VmDirSchemaCtxGetErrorMsg(
    PVDIR_SCHEMA_CTX    pCtx
    )
{
    if (pCtx && pCtx->pszErrorMsg)
    {
        return pCtx->pszErrorMsg;
    }

    return NULL;
}

/*
 * Get context error code.
*/
DWORD
VmDirSchemaCtxGetErrorCode(
    PVDIR_SCHEMA_CTX    pCtx
    )
{
    if (pCtx)
    {
        return pCtx->dwErrorCode;
    }

    return ERROR_NO_SCHEMA;
}

BOOLEAN
VmDirSchemaCtxIsBootStrap(
    PVDIR_SCHEMA_CTX    pCtx
    )
{
    BOOLEAN bRtn = FALSE;

    if ( pCtx           &&
         pCtx->pSchema  &&
         pCtx->pSchema->bIsBootStrapSchema
       )
    {
        bRtn = TRUE;
    }

    return bRtn;
}

/*
 * Before modify schema cache, make sure new schema is valid.
 * 1. schema of pSchemaEntry must be live one
 * 2. create new schema instance via pEntry
 * 3. check active and new schema compatibility
 *    NOT compatible - reject this operation
 *    Compatible but NO semantic change - update schema entry
 *    Compatible and has semantic change - update schema entry and cache
 * 4. make new instance pending in gVdirSchemaGlobals
 */
DWORD
VmDirSchemaCacheModifyPrepare(
    PVDIR_OPERATION      pOperation,
    VDIR_MODIFICATION*   pMods,
    PVDIR_ENTRY          pSchemaEntry
    )
{
    DWORD dwError = 0;
    BOOLEAN                 bInLock = FALSE;
    PSTR                    pszLocalErrMsg = NULL;
    BOOLEAN                 bOwnNewInstance = FALSE;
    BOOLEAN                 bCompatible = FALSE;            // schema compatible
    BOOLEAN                 bNeedCachePatch = FALSE;        // schema semantic/cache change
    PVDIR_SCHEMA_INSTANCE   pNewInstance = NULL;            // DO NOT free, pEntry should take over it.
    PVDIR_SCHEMA_INSTANCE   pEntryOrgSchemaInstance = NULL;

    if ( !pMods || !pSchemaEntry || !pOperation )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
    {
        PVDIR_MODIFICATION  pLocalMods = NULL;
        PCSTR               immutableList[] = VDIR_IMMUTABLE_SCHEMA_ELEMENT_INITIALIZER;
        int                 iImmutableSize = sizeof(immutableList)/sizeof(immutableList[0]);

        for (pLocalMods = pMods; pLocalMods; pLocalMods = pLocalMods->next)
        {
            BOOLEAN bImmutableElement = _VmDirIsNameInCaseIgnoreList( pLocalMods->attr.pATDesc->pszName,
                                                                      immutableList,
                                                                      iImmutableSize);
            if ( bImmutableElement )
            {
                dwError = ERROR_OPERATION_NOT_PERMITTED;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                             "modify (%s) not allowed", pLocalMods->attr.pATDesc->pszName);
            }
        }
    }

    // make sure pEntry uses live schema
    if (! vdirIsLiveSchema(pSchemaEntry->pSchemaCtx->pSchema))
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Out dated schema");
    }

    pEntryOrgSchemaInstance = pSchemaEntry->pSchemaCtx->pSchema;
    // instantiate a schema cache - pNewInstance
    // If this call succeed, do NOT free pNewInstance.  pEntry->pSchemaCtx takes it over.
    dwError = VdirSchemaInstanceInitViaEntry(   pSchemaEntry,
                                                &pNewInstance);
    if ( dwError != 0
         &&
         pSchemaEntry->pSchemaCtx->pSchema != pNewInstance )
    {
        // we still own pNewInstance and need to free it.
        bOwnNewInstance = TRUE;
    }
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Entry to schema instance failed (%d)", dwError);

    // check if two instances are compatible and if schema patching is needed
    dwError = VmDirSchemaInstancePatchCheck( pEntryOrgSchemaInstance,
                                             pNewInstance,
                                             &bCompatible,
                                             &bNeedCachePatch);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "VmDirSchemaInstancePatch (%d)", dwError);

    if ( !bCompatible )
    {
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Schema NOT compatible (%d)", dwError);
    }

    if ( bNeedCachePatch )
    {
        // need schema entry and cache update
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Prepare schema entry and instance update (%p)", pNewInstance);

        VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);
        gVdirSchemaGlobals.bHasPendingChange = TRUE;
        VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);
    }
    else
    {
        // no semantic change,
        // 1. do not modify schema entry if coming from INTERNAL (schema patch -u route).
        //    to avoid unecessary metadata version bump.
        // 2. allow modify schema entry if coming from REPL (copypartnerschema).
        // 3. allow modify schema entry if coming from external LDAP (force sync metadata version).
        if ( pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL )
        {
            dwError = VMDIR_ERROR_SCHEMA_UPDATE_PASSTHROUGH;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Schema pass through (%d)", dwError);
        }
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Prepare schema entry update");
    }


cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    if ( bOwnNewInstance )
    {
        VdirSchemaInstanceFree( pNewInstance );
    }

    VMDIR_SET_LDAP_RESULT_ERROR( &pOperation->ldapResult, dwError, pszLocalErrMsg );

    goto cleanup;
}


/*
 * Commit schema modification into cache.
 *
 * 1. commit pending cache to go live
 * 2. create self reference ctx
 * 3. release old self reference ctx
 * 4. update pEntry to have new live schema ctx association
 */
VOID
VmDirSchemaCacheModifyCommit(
    PVDIR_ENTRY  pSchemaEntry
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bInLock = FALSE;
    PVDIR_SCHEMA_CTX    pOldCtx = NULL;

    if ( !pSchemaEntry || !pSchemaEntry->pSchemaCtx )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    if ( ! gVdirSchemaGlobals.bHasPendingChange )
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Schema cache update pass through" );
        goto cleanup;   // no schema definition change, just pass through.
    }

    gVdirSchemaGlobals.bHasPendingChange = FALSE;
    gVdirSchemaGlobals.pSchema = pSchemaEntry->pSchemaCtx->pSchema;

    pOldCtx = gVdirSchemaGlobals.pCtx;
    gVdirSchemaGlobals.pCtx = NULL;

    VdirSchemaCtxAcquireInLock(TRUE, &gVdirSchemaGlobals.pCtx); // add global instance self reference
    assert(gVdirSchemaGlobals.pCtx);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Enable schema instance (%p)", gVdirSchemaGlobals.pSchema);

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    if (pOldCtx)
    {
        VmDirSchemaCtxRelease(pOldCtx);
    }

    return;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSchemaCacheModifyCommit failed (%d)", dwError);

    goto cleanup;
}

/*
 * PVDIR_SCHEMA_AT_DESC lookup
 */
DWORD
VmDirSchemaAttrNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_AT_DESC    pLocalATDesc = NULL;
    VDIR_SCHEMA_AT_DESC     key = {0};

    if (!pCtx || !pszName || !ppATDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszName;

    pLocalATDesc = (PVDIR_SCHEMA_AT_DESC) bsearch(  &key,
                                                    pCtx->pSchema->ats.pATSortName,
                                                    pCtx->pSchema->ats.usNumATs,
                                                    sizeof(VDIR_SCHEMA_AT_DESC),
                                                    VdirSchemaPATNameCmp);

    if (!pLocalATDesc)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppATDesc = pLocalATDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirSchemaOCNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_OC_DESC    pLocalOCDesc = NULL;
    VDIR_SCHEMA_OC_DESC     key = {0};

    if (!pCtx || !pszName || !ppOCDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszName;

    pLocalOCDesc = (PVDIR_SCHEMA_OC_DESC) bsearch(  &key,
                                                    pCtx->pSchema->ocs.pOCSortName,
                                                    pCtx->pSchema->ocs.usNumOCs,
                                                    sizeof(VDIR_SCHEMA_OC_DESC),
                                                    VdirSchemaPOCNameCmp);

    if (!pLocalOCDesc)
    {
        dwError = ERROR_NO_SUCH_OBJECTCLASS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOCDesc = pLocalOCDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirSchemaCRNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_CR_DESC    pLocalCRDesc = NULL;
    VDIR_SCHEMA_CR_DESC     key = {0};

    if (!pCtx || !pszName || !ppCRDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszName;

    pLocalCRDesc = (PVDIR_SCHEMA_CR_DESC) bsearch(  &key,
                                                    pCtx->pSchema->contentRules.pContentSortName,
                                                    pCtx->pSchema->contentRules.usNumContents,
                                                    sizeof(VDIR_SCHEMA_CR_DESC),
                                                    VdirSchemaPCRNameCmp);

    if (!pLocalCRDesc)
    {
        dwError = ERROR_NO_SUCH_DITCONTENTRULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppCRDesc = pLocalCRDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}

BOOLEAN
VmDirSchemaIsStructureOC(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bRet = FALSE;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

    if ( !pCtx || !pszName )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaOCNameToDescriptor( pCtx, pszName, &pOCDesc );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pOCDesc && pOCDesc->type == VDIR_OC_STRUCTURAL )
    {
        bRet = TRUE;
    }

cleanup:

    return bRet;

error:

    goto cleanup;
}

// TODO, should retire this function and use VmDirSchemaAttrNameToDescriptor.
PVDIR_SCHEMA_AT_DESC
VmDirSchemaAttrNameToDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    DWORD    dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;
    VDIR_SCHEMA_AT_DESC key = {0};

    if (!pCtx || !pszName)
    {
        if (pCtx)
        {
            pCtx->dwErrorCode = ERROR_INVALID_PARAMETER;

            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            dwError = VmDirAllocateStringA(
                    " No schema context or attribute name",
                    &pCtx->pszErrorMsg);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        return NULL;
    }

    key.pszName = (PSTR)pszName;

    pResult = (PVDIR_SCHEMA_AT_DESC) bsearch(
            &key,
            pCtx->pSchema->ats.pATSortName,
            pCtx->pSchema->ats.usNumATs,
            sizeof(VDIR_SCHEMA_AT_DESC),
            VdirSchemaPATNameCmp);

cleanup:

    return pResult;

error:

    goto cleanup;
}

PVDIR_SCHEMA_AT_DESC
VmDirSchemaAttrIdToDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    USHORT              usId
    )
{
    DWORD    dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

    if (!pCtx)
    {
        return NULL;
    }

    if (usId >= pCtx->pSchema->ats.usNextId)
    {
        pCtx->dwErrorCode = ERROR_INVALID_PARAMETER;

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "Lookup id(%d) > max id(%d)",
                usId,
                pCtx->pSchema->ats.usNextId - 1);
        BAIL_ON_VMDIR_ERROR(dwError);

        return NULL;
    }

    pResult = pCtx->pSchema->ats.ppATSortIdMap[usId - 1];

cleanup:

    return pResult ? pResult : NULL;

error:

    goto cleanup;
}

USHORT
VmDirSchemaAttrNameToId(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    PVDIR_SCHEMA_AT_DESC pResult = VmDirSchemaAttrNameToDesc(pCtx,pszName);

    return pResult ? pResult->usAttrID : NO_ATTR_ID_MAP;
}

PCSTR
VmDirSchemaAttrIdToName(
    PVDIR_SCHEMA_CTX    pCtx,
    USHORT              usId
    )
{
    DWORD    dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

    if (!pCtx)
    {
        return NULL;
    }

    if (usId >= pCtx->pSchema->ats.usNextId)
    {
        pCtx->dwErrorCode = ERROR_INVALID_PARAMETER;

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "Lookup id(%d) > max id(%d)",
                usId,
                pCtx->pSchema->ats.usNextId - 1);
        BAIL_ON_VMDIR_ERROR(dwError);

        return NULL;
    }

    pResult = pCtx->pSchema->ats.ppATSortIdMap[usId - 1];

cleanup:

    return pResult ? pResult->pszName : NULL;

error:

    goto cleanup;
}

BOOLEAN
VmDirSchemaIsNameEntryLeafStructureOC(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszName
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bRet = FALSE;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

    if ( !pEntry || !pszName )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaGetEntryStructureOCDesc( pEntry, &pOCDesc );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pOCDesc &&
         VmDirStringCompareA( pszName, pOCDesc->pszName, FALSE ) == 0
       )
    {
        bRet = TRUE;
    }

cleanup:

    return bRet;

error:
    goto cleanup;
}

/*
 * Numeric ordering attribute has matching rule integerMatch or integerOrderingMatch
 */
BOOLEAN
VmDirSchemaAttrHasIntegerMatchingRule(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bIsNumericOrdering = FALSE;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

    if (pCtx && pszName)
    {
        pATDesc = VmDirSchemaAttrNameToDesc(pCtx,pszName);
        if (!pATDesc)
        {
            dwError = ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if ((pATDesc->pszEqualityMRName &&
             VmDirStringCompareA(pATDesc->pszEqualityMRName, VDIR_MATCHING_RULE_INTEGER_MATCH, FALSE) == 0)
            ||
            (pATDesc->pszOrderingMRName &&
             VmDirStringCompareA(pATDesc->pszOrderingMRName, VDIR_MATCHING_RULE_INTEGER_ORDERING_MATCH, FALSE) == 0)
           )
        {
            bIsNumericOrdering = TRUE;
        }
    }

error:

    return bIsNumericOrdering;
}

/*
 * Get the Entry used to startup/load schema for the very first time from file.
 * Caller owns pEntry.
 */
PVDIR_ENTRY
VmDirSchemaAcquireAndOwnStartupEntry(
    VOID
    )
{
    BOOLEAN     bInLock = FALSE;
    PVDIR_ENTRY      pEntry = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    pEntry = gVdirSchemaGlobals.pLoadFromFileEntry;
    gVdirSchemaGlobals.pLoadFromFileEntry = NULL;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    return pEntry;
}

/* in check.c
 * Entry schema check
 *
DWORD
VmDirSchemaCheck(
    PVDIR_SCHEMA_CTX pCtx,
    PEntry           pEntry
    )
 */


/*
 * Berval syntax check
 */

DWORD
VmDirSchemaBervalSyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE                 pBerv
    )
{
    DWORD dwError = 0;

    if (!pCtx || !pATDesc || !pBerv)
    {
        if (pCtx)
        {
            pCtx->dwErrorCode = ERROR_INVALID_PARAMETER;

            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            dwError = VmDirAllocateStringA(
                    "No descriptor or value",
                    &pCtx->pszErrorMsg);
        }

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //TODO, if NO pSyntax, i.e. this syntax is NOT supported yet.
    // just bypass checking and move on.
    if (pATDesc->pSyntax &&
        pATDesc->pSyntax->pValidateFunc(pBerv) == FALSE)
    {
        pCtx->dwErrorCode = ERROR_INVALID_SYNTAX;

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "%s value (%s) is not a valid (%s) syntax",
                pATDesc->pszName,
                VDIR_SAFE_STRING(pBerv->lberbv.bv_val),
                VDIR_SAFE_STRING(pATDesc->pszSyntaxName));

        dwError = ERROR_INVALID_SYNTAX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
DWORD
VmDirSchemaBervalArraySyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    DWORD                   dwNumBervs,
    PBerval*                ppBervs
    )
{
    DWORD dwError = 0;

    if (!pCtx || !pATDesc || !ppBervs)
    {
        //
    }
cleanup:

    return dwError;

error:

    goto cleanup;
}
*/

/*
 * Berval value normalization
 */
DWORD
VmDirSchemaBervalNormalize(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE          pBerv
    )
{
    DWORD dwError = 0;

    if (!pCtx || !pATDesc || !pBerv)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //TODO, add matchingruleuse support
    //TODO, make sure there is only one normalizer for EQ/OD/SUBSTR match
    if (pATDesc->pEqualityMR && pATDesc->pEqualityMR->pNormalizeFunc)
    {
        dwError = pATDesc->pEqualityMR->pNormalizeFunc(pBerv);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pATDesc->pOrderingMR && pATDesc->pOrderingMR->pNormalizeFunc)
    {
        dwError = pATDesc->pOrderingMR->pNormalizeFunc(pBerv);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pATDesc->pSubStringMR && pATDesc->pSubStringMR->pNormalizeFunc)
    {
        dwError = pATDesc->pSubStringMR->pNormalizeFunc(pBerv);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pBerv->bvnorm_val = pBerv->lberbv.bv_val;
        pBerv->bvnorm_len = pBerv->lberbv.bv_len;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

BOOLEAN
VmDirIsLiveSchemaCtx(
    PVDIR_SCHEMA_CTX        pCtx
    )
{
    BOOLEAN                 bInLock = FALSE;
    BOOLEAN                 bRtn = FALSE;
    PVDIR_SCHEMA_INSTANCE   pLiveSchema = NULL;

    if ( pCtx )
    {
        VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

        pLiveSchema = gVdirSchemaGlobals.pSchema;

        VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

        bRtn = (pCtx->pSchema == pLiveSchema) ? TRUE:FALSE;
    }

    return bRtn;
}

/*
DWORD
VmdirSchemaBervalArrayNormalize(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    DWORD                   dwNumBervs,
    PBerval*                ppBervs
    )
{
    DWORD dwError = 0;

cleanup:

    return dwError;

error:

    goto cleanup;
}
*/

/*
 * Berval value comparison
 */
BOOLEAN
VmDirSchemaBervalCompare(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    VDIR_SCHEMA_MATCH_TYPE  matchType,
    PVDIR_BERVALUE          pBervAssert,
    PVDIR_BERVALUE          pBerv
    )
{

    if (!pCtx || !pATDesc || !pBervAssert || !pBerv)
    {
        return FALSE;
    }

    switch (matchType)
    {
    case VDIR_SCHEMA_MATCH_EQUAL:
        if (pATDesc->pEqualityMR && pATDesc->pEqualityMR->pCompareFunc)
        {
            return pATDesc->pEqualityMR->pCompareFunc(
                    matchType,
                    pBervAssert,
                    pBerv);
        }

        break;

    case VDIR_SCHEMA_MATCH_GE:
    case VDIR_SCHEMA_MATCH_LE:
        if (pATDesc->pOrderingMR && pATDesc->pOrderingMR->pCompareFunc)
        {
            return pATDesc->pOrderingMR->pCompareFunc(
                    matchType,
                    pBervAssert,
                    pBerv);
        }

        break;

    case VDIR_SCHEMA_MATCH_SUBSTR_INIT:
    case VDIR_SCHEMA_MATCH_SUBSTR_ANY:
    case VDIR_SCHEMA_MATCH_SUBSTR_FINAL:
        if (pATDesc->pSubStringMR && pATDesc->pSubStringMR->pCompareFunc)
        {
            return pATDesc->pSubStringMR->pCompareFunc(
                    matchType,
                    pBervAssert,
                    pBerv);
        }

        break;

    default:

        break;
    }

    return FALSE;
}

static
BOOLEAN
vdirIsLiveSchema(
    PVDIR_SCHEMA_INSTANCE    pSchema
    )
{
    BOOLEAN bInLock = FALSE;
    PVDIR_SCHEMA_INSTANCE    pLiveSchema = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    pLiveSchema = gVdirSchemaGlobals.pSchema;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    return (pSchema == pLiveSchema);
}

static
BOOLEAN
_VmDirIsNameInCaseIgnoreList(
    PCSTR   pszName,
    PCSTR*  ppszList,
    size_t  iSize
    )
{
    BOOLEAN     bRtn = FALSE;
    size_t      iCnt = 0;

    if ( pszName && ppszList )
    {
        for (iCnt=0; iCnt < iSize; iCnt++)
        {
            if (VmDirStringCompareA( pszName, ppszList[iCnt], FALSE) == 0)
            {
                bRtn = TRUE;
                break;
            }
        }
    }

    return bRtn;
}
