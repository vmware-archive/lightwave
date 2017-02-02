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
        VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);
    }

    pCtx->pLdapSchema = gVdirSchemaGlobals.pLdapSchema;
    pCtx->pVdirSchema = gVdirSchemaGlobals.pVdirSchema;
    assert(pCtx->pLdapSchema);
    assert(pCtx->pVdirSchema);

    VMDIR_LOCK_MUTEX(bInLockNest, pCtx->pVdirSchema->mutex);
    pCtx->pVdirSchema->usRefCount++;

error:

    VMDIR_UNLOCK_MUTEX(bInLockNest, pCtx->pVdirSchema->mutex);
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

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

    pCtx->pLdapSchema = pOrgCtx->pLdapSchema;
    pCtx->pVdirSchema = pOrgCtx->pVdirSchema;

    VMDIR_LOCK_MUTEX(bInLock, pCtx->pVdirSchema->mutex);
    pCtx->pVdirSchema->usRefCount++;

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pCtx->pVdirSchema->mutex);

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
        if (  pCtx->pVdirSchema )
        {
            VMDIR_LOCK_MUTEX(bInLock, pCtx->pVdirSchema->mutex);

            pCtx->pVdirSchema->usRefCount--;
            usRefCnt = pCtx->pVdirSchema->usRefCount;
            usSelfRef = pCtx->pVdirSchema->usNumSelfRef;

            VMDIR_UNLOCK_MUTEX(bInLock, pCtx->pVdirSchema->mutex);

            if (usRefCnt == usSelfRef)
            {   // only self reference within pSchema exists, free pSchema itself.
                // self references are established during init before normal references.
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                                "Free unreferenced schema instance (%p)",
                                pCtx->pVdirSchema);

                VmDirFreeSchemaInstance(pCtx->pVdirSchema);
                VmDirFreeLdapSchema(pCtx->pLdapSchema);
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
         pCtx->pVdirSchema  &&
         pCtx->pVdirSchema->bIsBootStrapSchema
       )
    {
        bRtn = TRUE;
    }

    return bRtn;
}

DWORD
VmDirSchemaAttrNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    )
{
    if (!pCtx || !pszName || !ppATDesc)
    {
        return ERROR_INVALID_PARAMETER;
    }
    return VmDirSchemaInstanceGetATDescByName(
            pCtx->pVdirSchema, pszName, ppATDesc);
}

DWORD
VmDirSchemaOCNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    )
{
    if (!pCtx || !pszName || !ppOCDesc)
    {
        return ERROR_INVALID_PARAMETER;
    }
    return VmDirSchemaInstanceGetOCDescByName(
            pCtx->pVdirSchema, pszName, ppOCDesc);
}

DWORD
VmDirSchemaCRNameToDescriptor(
    PVDIR_SCHEMA_CTX        pCtx,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    )
{
    if (!pCtx || !pszName || !ppCRDesc)
    {
        return ERROR_INVALID_PARAMETER;
    }
    return VmDirSchemaInstanceGetCRDescByName(
            pCtx->pVdirSchema, pszName, ppCRDesc);
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

    dwError = VmDirSchemaOCNameToDescriptor(pCtx, pszName, &pOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pOCDesc && pOCDesc->type == VDIR_LDAP_STRUCTURAL_CLASS)
    {
        bRet = TRUE;
    }

cleanup:
    return bRet;

error:
    goto cleanup;
}

BOOLEAN
VmDirSchemaIsAncestorOC(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_SCHEMA_OC_DESC    pAncestorOCDesc
    )
{
    DWORD dwError = 0;
    BOOLEAN bRtn = FALSE;
    PVDIR_SCHEMA_OC_DESC pCurOC = NULL;

    if (!pOCDesc || !pAncestorOCDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurOC = pOCDesc;
    while (pCurOC)
    {
        if (pCurOC == pAncestorOCDesc)
        {
            bRtn = TRUE;
            goto cleanup;
        }

        if (VmDirStringCompareA(OC_TOP, pCurOC->pszName, FALSE) == 0)
        {
            pCurOC = NULL;
        }
        else
        {
            dwError = VmDirSchemaOCNameToDescriptor(
                    pCtx, pCurOC->pszSup, &pCurOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return bRtn;

error:
    bRtn = FALSE;
    goto cleanup;
}

// TODO, should retire this function and use VmDirSchemaAttrNameToDescriptor.
PVDIR_SCHEMA_AT_DESC
VmDirSchemaAttrNameToDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

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

    dwError = VmDirSchemaInstanceGetATDescByName(
            pCtx->pVdirSchema, pszName, &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

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
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

    if (pCtx)
    {
        dwError = VmDirSchemaInstanceGetATDescById(
                pCtx->pVdirSchema, usId, &pResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return pResult;

error:
    goto cleanup;
}

USHORT
VmDirSchemaAttrNameToId(
    PVDIR_SCHEMA_CTX    pCtx,
    PCSTR               pszName
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

    if (pCtx)
    {
        dwError = VmDirSchemaInstanceGetATDescByName(
                pCtx->pVdirSchema, pszName, &pResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return pResult ? pResult->usAttrID : NO_ATTR_ID_MAP;

error:
    goto cleanup;
}

PCSTR
VmDirSchemaAttrIdToName(
    PVDIR_SCHEMA_CTX    pCtx,
    USHORT              usId
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pResult = NULL;

    if (pCtx)
    {
        dwError = VmDirSchemaInstanceGetATDescById(
                pCtx->pVdirSchema, usId, &pResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return pResult ? pResult->pszName : NULL;

error:
    goto cleanup;
}

DWORD
VmDirSchemaAttrList(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC**  pppATDescList
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0, i =0;
    PVDIR_SCHEMA_AT_COLLECTION  pATColl = NULL;
    PVDIR_SCHEMA_AT_DESC*       ppATDescList = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pCtx || !pppATDescList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pATColl = &pCtx->pVdirSchema->attributeTypes;
    dwCount = LwRtlHashMapGetCount(pATColl->byName);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_AT_DESC) * (dwCount + 1),
            (PVOID*)&ppATDescList);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pATColl->byName, &iter, &pair))
    {
        ppATDescList[i++] = (PVDIR_SCHEMA_AT_DESC)pair.pValue;
    }

    *pppATDescList = ppATDescList;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(ppATDescList);
    goto cleanup;
}

DWORD
VmDirSchemaClassGetAllMayAttrs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMayAttrMap  // IN
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_SCHEMA_OC_DESC pCurOC = NULL;
    PVDIR_SCHEMA_CR_DESC pCR = NULL;

    if (!pCtx || !pOCDesc || !pAllMayAttrMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurOC = pOCDesc;
    while (pCurOC)
    {
        for (i = 0; pCurOC->ppszMayATs && pCurOC->ppszMayATs[i]; i++)
        {
            dwError = LwRtlHashMapInsert(pAllMayAttrMap,
                    pCurOC->ppszMayATs[i], pCurOC->ppszMayATs[i], NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pCurOC->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            dwError = VmDirSchemaCRNameToDescriptor(
                    pCtx, pCurOC->pszName, &pCR);

            if (dwError == 0)
            {
                for (i = 0; pCR->ppszMayATs && pCR->ppszMayATs[i]; i++)
                {
                    dwError = LwRtlHashMapInsert(pAllMayAttrMap,
                            pCR->ppszMayATs[i], pCR->ppszMayATs[i], NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            else if (dwError != ERROR_NO_SUCH_DITCONTENTRULES)
            {
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        if (VmDirStringCompareA(OC_TOP, pCurOC->pszName, FALSE) == 0)
        {
            pCurOC = NULL;
        }
        else
        {
            dwError = VmDirSchemaOCNameToDescriptor(
                    pCtx, pCurOC->pszSup, &pCurOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
}

DWORD
VmDirSchemaClassGetAllMustAttrs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMustAttrMap // IN
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_SCHEMA_OC_DESC pCurOC = NULL;
    PVDIR_SCHEMA_CR_DESC pCR = NULL;

    if (!pCtx || !pOCDesc || !pAllMustAttrMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurOC = pOCDesc;
    while (pCurOC)
    {
        for (i = 0; pCurOC->ppszMustATs && pCurOC->ppszMustATs[i]; i++)
        {
            dwError = LwRtlHashMapInsert(pAllMustAttrMap,
                    pCurOC->ppszMustATs[i], pCurOC->ppszMustATs[i], NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pCurOC->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            dwError = VmDirSchemaCRNameToDescriptor(
                    pCtx, pCurOC->pszName, &pCR);

            if (dwError == 0)
            {
                for (i = 0; pCR->ppszMustATs && pCR->ppszMustATs[i]; i++)
                {
                    dwError = LwRtlHashMapInsert(pAllMustAttrMap,
                            pCR->ppszMustATs[i], pCR->ppszMustATs[i], NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            else if (dwError != ERROR_NO_SUCH_DITCONTENTRULES)
            {
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        if (VmDirStringCompareA(OC_TOP, pCurOC->pszName, FALSE) == 0)
        {
            pCurOC = NULL;
        }
        else
        {
            dwError = VmDirSchemaOCNameToDescriptor(
                    pCtx, pCurOC->pszSup, &pCurOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
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

    dwError = VmDirSchemaGetEntryStructureOCDesc(pEntry, &pOCDesc);
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

BOOLEAN
VmDirSchemaSyntaxIsNumeric(
    PCSTR   pszSyntaxOid
    )
{
    BOOLEAN bIsNumeric = FALSE;

    if (!IsNullOrEmptyString(pszSyntaxOid) &&
         VmDirStringCompareA(pszSyntaxOid, VDIR_OID_INTERGER, FALSE) == 0)
    {
        bIsNumeric = TRUE;
    }

    return bIsNumeric;
}

BOOLEAN
VmDirSchemaAttrIsNumeric(
    PVDIR_SCHEMA_AT_DESC    pATDesc
    )
{
    BOOLEAN bIsNumeric = FALSE;

    if (pATDesc && pATDesc->pSyntax)
    {
        bIsNumeric = VmDirSchemaSyntaxIsNumeric(pATDesc->pSyntax->pszOid);
    }

    return bIsNumeric;
}

BOOLEAN
VmDirSchemaAttrIsOctetString(
    PVDIR_SCHEMA_AT_DESC    pATDesc
    )
{
    BOOLEAN bIsOctetStr = FALSE;
    if (pATDesc && pATDesc->pSyntax)
    {
        if (!IsNullOrEmptyString(pATDesc->pSyntax->pszOid) &&
                 VmDirStringCompareA(pATDesc->pSyntax->pszOid, VDIR_OID_OCTET_STRING, FALSE) == 0)
        {
            bIsOctetStr = TRUE;
        }
    }

    return bIsOctetStr;
}

/*
 * Berval syntax check
 */
DWORD
VmDirSchemaBervalSyntaxCheck(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_BERVALUE          pBerv
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

    if (pATDesc->pSyntax &&
            pATDesc->pSyntax->pValidateFunc(pBerv) == FALSE)
    {
        pCtx->dwErrorCode = ERROR_INVALID_SYNTAX;

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringPrintf(
                &pCtx->pszErrorMsg,
                "%s value (%s) is not a valid (%s) syntax",
                pATDesc->pszName,
                VDIR_SAFE_STRING(pBerv->lberbv.bv_val),
                VDIR_SAFE_STRING(pATDesc->pszSyntaxOid));

        dwError = ERROR_INVALID_SYNTAX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

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
        VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

        pLiveSchema = gVdirSchemaGlobals.pVdirSchema;

        VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.ctxMutex);

        bRtn = (pCtx->pVdirSchema == pLiveSchema) ? TRUE:FALSE;
    }

    return bRtn;
}

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
