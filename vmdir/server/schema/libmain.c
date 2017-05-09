/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Library Entry points
 *
 */

#include "includes.h"

/*
 *    Bootstrap initial schema from default data.
 *    Not a full schema as ones that initialize from file or entry,
 *    but with limited information that we can bootstrap schema entry from
 *    data store or file.
 *
 */
DWORD
VmDirSchemaLibInit(
    PVMDIR_MUTEX*   ppModMutex
    )
{
    DWORD   dwError = 0;

    // legacy support
    // - replace with VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER
    //   when legacy support is no longer required
    VDIR_SCHEMA_BOOTSTRAP_TABLE ATTable[] =
            VDIR_LEGACY_SCHEMA_BOOTSTRP_ATTR_INITIALIZER;

    if (!ppModMutex)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdirSyntaxLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirMatchingRuleLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVdirSchemaGlobals.ctxMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVdirSchemaGlobals.cacheModMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaInit(&gVdirSchemaGlobals.pLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaInstanceCreate(
            gVdirSchemaGlobals.pLdapSchema,
            &gVdirSchemaGlobals.pVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSchemaCtxAcquireInLock(TRUE, &gVdirSchemaGlobals.pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaAttrIdMapInit(&gVdirSchemaGlobals.pAttrIdMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    // read attribute Ids from DB
    dwError = VmDirSchemaAttrIdMapReadDB(gVdirSchemaGlobals.pAttrIdMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    // read attributes from bootstrap table
    dwError = VmDirSchemaLibLoadBootstrapTable(ATTable);
    BAIL_ON_VMDIR_ERROR(dwError);

    // legacy support
    dwError = VmDirSchemaLibInitLegacy();
    BAIL_ON_VMDIR_ERROR(dwError);

    // repl status globals
    dwError = VmDirSchemaReplStatusGlobalsInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppModMutex = gVdirSchemaGlobals.cacheModMutex;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaLibLoadBootstrapTable(
    VDIR_SCHEMA_BOOTSTRAP_TABLE bootstrapTable[]
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap = NULL;

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;
    pAttrIdMap = gVdirSchemaGlobals.pAttrIdMap;

    assert(pCurLdapSchema && pAttrIdMap);

    dwError = VmDirLdapSchemaCopy(pCurLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0 ; bootstrapTable[i].usAttrID; i++)
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = NULL;

        dwError = VmDirLdapAtParseStr(bootstrapTable[i].pszDesc, &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pNewLdapSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirSchemaAttrIdMapGetAttrId(pAttrIdMap, pAt->pszName, NULL) != 0)
        {
            dwError = VmDirSchemaAttrIdMapAddNewAttr(
                    pAttrIdMap, pAt->pszName, bootstrapTable[i].usAttrID);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    pNewLdapSchema = NULL;

    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;
    pNewVdirSchema = NULL;

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}

DWORD
VmDirSchemaLibLoadFile(
    PCSTR   pszSchemaFilePath
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pTmpLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;

    dwError = VmDirLdapSchemaInit(&pTmpLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadFile(pTmpLdapSchema, pszSchemaFilePath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaMerge(
            pCurLdapSchema, pTmpLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pNewLdapSchema->attributeTypes, &iter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;

        if (VmDirSchemaAttrIdMapGetAttrId(
                gVdirSchemaGlobals.pAttrIdMap, pAt->pszName, NULL) != 0)
        {
            dwError = VmDirSchemaAttrIdMapAddNewAttr(
                    gVdirSchemaGlobals.pAttrIdMap, pAt->pszName, 0);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    pNewLdapSchema = NULL;

    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;
    pNewVdirSchema = NULL;

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeLdapSchema(pTmpLdapSchema);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}

DWORD
VmDirSchemaLibLoadAttributeSchemaEntries(
    PVDIR_ENTRY_ARRAY   pAtEntries
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;

    if (!pAtEntries)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;

    dwError = VmDirLdapSchemaCopy(pCurLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAtEntries->iSize; i++)
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = NULL;

        dwError = VmDirLdapAtParseVdirEntry(&pAtEntries->pEntry[i], &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pNewLdapSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    pNewLdapSchema = NULL;

    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;
    pNewVdirSchema = NULL;

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}

DWORD
VmDirSchemaLibLoadClassSchemaEntries(
    PVDIR_ENTRY_ARRAY   pOcEntries
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;

    if (!pOcEntries)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;

    dwError = VmDirLdapSchemaCopy(pCurLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pOcEntries->iSize; i++)
    {
        PVDIR_LDAP_OBJECT_CLASS pOc = NULL;
        PVDIR_LDAP_CONTENT_RULE pCr = NULL;

        dwError = VmDirLdapOcParseVdirEntry(&pOcEntries->pEntry[i], &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrParseVdirEntry(&pOcEntries->pEntry[i], &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pNewLdapSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pCr)
        {
            dwError = VmDirLdapSchemaAddCr(pNewLdapSchema, pCr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    pNewLdapSchema = NULL;

    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;
    pNewVdirSchema = NULL;

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}

DWORD
VmDirSchemaLibPrepareUpdateViaModify(
    PVDIR_OPERATION      pOperation,
    PVDIR_ENTRY          pSchemaEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszLocalErrMsg = NULL;
    PVDIR_ATTRIBUTE pClassAttr = NULL;
    PSTR            pszClass = NULL;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;

    if (!pSchemaEntry || !pOperation)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!VDIR_CUSTOM_SCHEMA_MODIFICATION_ENABLED)
    {
        dwError = VMDIR_ERROR_OPERATION_NOT_PERMITTED;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                "custom schema modification is not enabled");
    }

    pClassAttr = VmDirFindAttrByName(pSchemaEntry, ATTR_OBJECT_CLASS);
    if (!pClassAttr)
    {
        dwError = VMDIR_ERROR_OBJECTCLASS_VIOLATION;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                "missing objectclass attribute");
    }

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;

    dwError = VmDirLdapSchemaCopy(pCurLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pClassAttr->numVals; i ++)
    {
        pszClass = pClassAttr->vals[i].lberbv.bv_val;
        if (VmDirStringCompareA(pszClass, OC_ATTRIBUTE_SCHEMA, FALSE) == 0)
        {
            dwError = VmDirLdapAtParseVdirEntry(pSchemaEntry, &pAt);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLdapAtVerify(pAt);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirSchemaAttrIdMapGetAttrId(
                    gVdirSchemaGlobals.pAttrIdMap, pAt->pszName, NULL) != 0)
            {
                dwError = VmDirSchemaAttrIdMapAddNewAttr(
                        gVdirSchemaGlobals.pAttrIdMap, pAt->pszName, 0);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwError = VmDirLdapSchemaAddAt(pNewLdapSchema, pAt);
            BAIL_ON_VMDIR_ERROR(dwError);
            pAt = NULL;

            goto updatelib;
        }
        else if (VmDirStringCompareA(pszClass, OC_CLASS_SCHEMA, FALSE) == 0)
        {
            dwError = VmDirLdapOcParseVdirEntry(pSchemaEntry, &pOc);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLdapCrParseVdirEntry(pSchemaEntry, &pCr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLdapOcResolveSup(pNewLdapSchema, pOc);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLdapOcVerify(pNewLdapSchema, pOc);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLdapSchemaAddOc(pNewLdapSchema, pOc);
            BAIL_ON_VMDIR_ERROR(dwError);
            pOc = NULL;

            if (pCr)
            {
                dwError = VmDirLdapCrVerify(pNewLdapSchema, pCr);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirLdapSchemaAddCr(pNewLdapSchema, pCr);
                BAIL_ON_VMDIR_ERROR(dwError);
                pCr = NULL;
            }

            goto updatelib;
        }
        else
        {
            // We don't handle other types yet
        }
    }

    dwError = ERROR_INVALID_ENTRY;
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
            "Not a schema object entry");

updatelib:
    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) (%s)",
            __FUNCTION__, dwError, pszLocalErrMsg );

    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult,
            dwError, pszLocalErrMsg);

    VmDirFreeLdapAt(pAt);
    VmDirFreeLdapOc(pOc);
    VmDirFreeLdapCr(pCr);
    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}

DWORD
VmDirSchemaLibUpdate(
    DWORD   dwPriorResult
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD   dwError = dwPriorResult;
    PVDIR_SCHEMA_CTX        pLiveCtx = NULL;
    PVDIR_LDAP_SCHEMA       pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;

    // no schema definition change, just pass through.
    if (!gVdirSchemaGlobals.pPendingLdapSchema)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Schema cache update pass through");
        goto error;
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

    // replace LdapSchema and VdirSchema with pending ones
    pNewLdapSchema = gVdirSchemaGlobals.pPendingLdapSchema;
    pNewVdirSchema = gVdirSchemaGlobals.pPendingVdirSchema;
    gVdirSchemaGlobals.pPendingLdapSchema = NULL;
    gVdirSchemaGlobals.pPendingVdirSchema = NULL;
    gVdirSchemaGlobals.pLdapSchema = pNewLdapSchema;
    gVdirSchemaGlobals.pVdirSchema = pNewVdirSchema;

    // get reference to current schema context, to clean up later
    pLiveCtx = gVdirSchemaGlobals.pCtx;

    // clean up if there was a prior error
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSchemaCtxAcquireInLock(TRUE, &gVdirSchemaGlobals.pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(gVdirSchemaGlobals.pCtx);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
            "New schema instance (%p)",
            gVdirSchemaGlobals.pVdirSchema);

cleanup:
    // release global ctxMutex before updating IdMap DB
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);
    VmDirSchemaCtxRelease(pLiveCtx);

    // no cleanup can be done if IdMapUpdateDB fails
    (VOID)VmDirSchemaAttrIdMapUpdateDB(gVdirSchemaGlobals.pAttrIdMap);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirSchemaAttrIdMapRemoveAllPending(gVdirSchemaGlobals.pAttrIdMap);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    VmDirFreeLdapSchema(pNewLdapSchema);
    if (pLiveCtx)
    {
        gVdirSchemaGlobals.pVdirSchema = pLiveCtx->pVdirSchema;
        gVdirSchemaGlobals.pLdapSchema = pLiveCtx->pLdapSchema;
        gVdirSchemaGlobals.pCtx = pLiveCtx;
        pLiveCtx = NULL;
    }
    goto cleanup;
}

VOID
VmDirSchemaLibShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VmDirSchemaReplStatusGlobalsShutdown();

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

    // release live context and live schema
    VmDirSchemaCtxRelease(gVdirSchemaGlobals.pCtx);
    gVdirSchemaGlobals.pCtx = NULL;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

    VMDIR_SAFE_FREE_MUTEX(gVdirSchemaGlobals.ctxMutex);
    gVdirSchemaGlobals.ctxMutex = NULL;

    VMDIR_SAFE_FREE_MUTEX(gVdirSchemaGlobals.cacheModMutex);
    gVdirSchemaGlobals.cacheModMutex = NULL;

    VmDirFreeSchemaAttrIdMap(gVdirSchemaGlobals.pAttrIdMap);
    gVdirSchemaGlobals.pAttrIdMap = NULL;
}
