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
 * Filename: check.c
 *
 * Abstract:
 *
 * Entry schema check
 *
 */

#include "includes.h"

static
PVDIR_SCHEMA_OC_DESC
_VmDirSchemaCheckOCDescLookup(
        PVDIR_SCHEMA_CTX    pCtx,
        PSTR                pOCName
        );

static
PVDIR_ATTRIBUTE
_VmDirchemaCheckFindObjectClass(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    );

static
DWORD
_VmDirSchemaCheckSyntaxAndDimension(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    );

static
DWORD
_VmDirSchemaCheckNameform(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC    pStructOCDesc
    );

static
DWORD
_VmDirSchemaCheckStructure(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    );

static
DWORD
_VmDirSchemaCheckMustAttrPresent(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_ENTRY                  pEntry,
    PBOOLEAN                pbPresentList
    );

static
DWORD
_VmDirSchemaCheckMayAttrPresent(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC*   ppTagetMayATs,
    PVDIR_ENTRY                  pEntry,
    PBOOLEAN                pbPresentList
    );

static
DWORD
_VmDirSchemaCheckEntryStructure(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_ATTRIBUTE         pOCAttr,
    BOOLEAN*                pbPresentList
    );

static
DWORD
_VmDirSchemaCheckContentRuleAuxOC(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc,
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc
    );

static
DWORD
_VmDirSchemaComputeAllowedChildClassesEffective(
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppOutAttr
    );

/*
 * reorder value of objectclass
 * 1. structureOC->parentOC->parentOC->....
 * 2. aux OCs
 */
static
DWORD
_VmDirSchemaReorderObjectClassAttr(
    PVDIR_ATTRIBUTE         pOCAttr,
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc,
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc);

DWORD
VmDirSchemaCheck(
    PVDIR_ENTRY           pEntry
    )
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pCtx = NULL;

    if ( !pEntry || !pEntry->pSchemaCtx )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCtx = pEntry->pSchemaCtx;

    // make sure attribute has descriptor
    dwError = VmDirSchemaCheckSetAttrDesc(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // syntax and dimension (single/multiple values) and max Length
    dwError = _VmDirSchemaCheckSyntaxAndDimension(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // structure + namefor + must + may
    dwError = _VmDirSchemaCheckStructure(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Entry schema check dit structure rule
 */
DWORD
VmDirSchemaCheckDITStructure(
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ENTRY           pParentEntry,
    PVDIR_ENTRY           pEntry
    )
{
    DWORD   dwError = 0;
    unsigned int     iCnt = 0;
    BOOLEAN bParentAllowed = FALSE;
    PVDIR_ATTRIBUTE              pParentOCAttr = NULL;
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc = NULL;
    VDIR_SCHEMA_OC_DESC     ocKey = {0};

    assert(pCtx && pEntry);

// BUBBUG - bypass checking until we define castle structure rules
goto cleanup;

    if (pCtx->pSchema->structureRules.usNumStructures == 0)
    {
        // schema has no structure rule defined
        goto cleanup;
    }

    if (!pEntry->pszStructureOC)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "Entry has no structure objectclass/pszStructureOC.");

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ocKey.pszName = pEntry->pszStructureOC;
    pStructureOCDesc = (PVDIR_SCHEMA_OC_DESC) bsearch(
            &ocKey,
            pCtx->pSchema->ocs.pOCSortName,
            pCtx->pSchema->ocs.usNumOCs,
            sizeof(VDIR_SCHEMA_OC_DESC),
            VdirSchemaPOCNameCmp);

    if (!pStructureOCDesc)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "Structure oc (%s) not defined.",
                VDIR_SAFE_STRING(pEntry->pszStructureOC));

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pParentEntry)
    {
        pParentOCAttr = _VmDirchemaCheckFindObjectClass(pCtx, pParentEntry);
    }

    if (pParentOCAttr)
    {
        if (pStructureOCDesc->ppszAllowedParentOCs)
        {
            // loop through parent object class to check allowedParentsOCs
            //TODO, we can arrange structure object class to be the first to eliminate loop.
            for (iCnt = 0; iCnt < pParentOCAttr->numVals; iCnt++)
            {
                int iIdx = 0;
                for (;pStructureOCDesc->ppszAllowedParentOCs[iIdx]; iIdx++)
                {
                    if (VmDirStringCompareA(pParentOCAttr->vals[iCnt].lberbv.bv_val,
                                   pStructureOCDesc->ppszAllowedParentOCs[iIdx],
                                   FALSE) == 0)
                    {
                        // allowed under this parent
                        bParentAllowed = TRUE;
                        break;
                    }
                }

                if (pStructureOCDesc->ppszAllowedParentOCs[iIdx] != NULL)
                {
                    break;
                }
            }
        }
    }
    else
    {
        if (pStructureOCDesc->bAllowedParentRoot == TRUE)
        {   // allowed under root
            bParentAllowed = TRUE;
        }
    }

    if (!bParentAllowed)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringAVsnprintf(
                &pCtx->pszErrorMsg,
                "(%s) not allowed under its parent",
                VDIR_SAFE_STRING(pEntry->pszStructureOC));

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirSchemaCheckSetAttrDesc(
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ENTRY           pEntry
    )
{
    DWORD dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert(pCtx && pEntry);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (!pAttr->pATDesc)
        {
            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_VMW_ORGANIZATION_GUID, FALSE ) == 0)
            {
                continue; // going to delete this attribute any way.
            }
            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_VMW_OBJECT_SECURITY_DESCRIPTOR, FALSE ) == 0)
            {
                pAttr->pATDesc = VmDirSchemaAttrNameToDesc(pCtx, ATTR_OBJECT_SECURITY_DESCRIPTOR);
            }
            else
            {
                pAttr->pATDesc = VmDirSchemaAttrNameToDesc(pCtx, pAttr->type.lberbv.bv_val);
            }
            if (!pAttr->pATDesc)
            {
                pCtx->dwErrorCode = ERROR_INVALID_SCHEMA;

                VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
                dwError = VmDirAllocateStringAVsnprintf(
                        &pCtx->pszErrorMsg,
                        "Attribute (%s) is not defined in schema",
                        VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val));

                dwError = ERROR_INVALID_SCHEMA;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

error:

    return dwError;
}

DWORD
VmDirSchemaGetComputedAttribute(
    PCSTR               pszComputedAttrName,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppOutAttr
    )
{
    DWORD       dwError = 0;

    if ( !pszComputedAttrName || !pEntry || !ppOutAttr )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirStringCompareA( pszComputedAttrName, ATTR_ALLOWD_CHILD_CLASSES_EFFECTIVE, FALSE ) == 0 )
    {
        dwError = _VmDirSchemaComputeAllowedChildClassesEffective( pEntry, ppOutAttr );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Find the structure objectclass of an entry and get its objectclass descriptor
 * If error, pEntry->pSchemaCtx errorcode/message could have more info.
 */
DWORD
VmDirSchemaGetEntryStructureOCDesc(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC*   ppStructureOCDesc       // caller does not own *ppStructureOCDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_ATTRIBUTE         pObjectClassAttr = NULL;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

    if (!pEntry || !pEntry->pSchemaCtx)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pObjectClassAttr = _VmDirchemaCheckFindObjectClass( pEntry->pSchemaCtx, pEntry );
    if ( !pObjectClassAttr || pObjectClassAttr->numVals < 1 )
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Baesd on Objectclass value storing convention, the first objectclass value is leaf structure objectclass.
    // See _VmDirSchemaReorderObjectClassAttr for details.
    pOCDesc = _VmDirSchemaCheckOCDescLookup(pEntry->pSchemaCtx, pObjectClassAttr->vals[0].lberbv.bv_val);
    if ( !pOCDesc || pOCDesc->type != VDIR_OC_STRUCTURAL )
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // pszStructureOC point into pEntry->attrs content.
    pEntry->pszStructureOC = pObjectClassAttr->vals[0].lberbv.bv_val;

    if ( ppStructureOCDesc )
    {
        *ppStructureOCDesc = pOCDesc;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaComputeAllowedChildClassesEffective(
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE*    ppOutAttr
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc = NULL;
    PVDIR_ATTRIBUTE         pLocalAttr = NULL;
    int                     iCnt = 0;

    dwError = VmDirSchemaGetEntryStructureOCDesc( pEntry,
                                                   &pStructureOCDesc );
    BAIL_ON_VMDIR_ERROR(dwError);
    if ( !pStructureOCDesc )
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (iCnt = 0; pStructureOCDesc->ppszAllowedChildOCs && pStructureOCDesc->ppszAllowedChildOCs[iCnt]; iCnt++) {}

    if (iCnt > 0)
    {
        dwError = VmDirAttributeAllocate( ATTR_ALLOWD_CHILD_CLASSES_EFFECTIVE,
                                          iCnt,
                                          pEntry->pSchemaCtx,
                                          &pLocalAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt = 0; pStructureOCDesc->ppszAllowedChildOCs[iCnt]; iCnt++)
        {
            dwError = VmDirAllocateStringA( pStructureOCDesc->ppszAllowedChildOCs[iCnt],
                                            &(pLocalAttr->vals[iCnt].lberbv_val));
            BAIL_ON_VMDIR_ERROR(dwError);

            pLocalAttr->vals[iCnt].lberbv_len = VmDirStringLenA(pLocalAttr->vals[iCnt].lberbv_val);
            pLocalAttr->vals[iCnt].bOwnBvVal = TRUE;
        }
    }

    *ppOutAttr = pLocalAttr;

cleanup:

    return dwError;

error:

    VmDirFreeAttribute( pLocalAttr );

    goto cleanup;
}

static
DWORD
_VmDirSchemaCheckSyntaxAndDimension(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD           dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    for (pAttr = pEntry->attrs; pAttr;  pAttr = pAttr->next)
    {
        USHORT usCnt = 0;

        if (pAttr->pATDesc->bSingleValue && pAttr->numVals != 1)
        {
            pCtx->dwErrorCode = ERROR_INVALID_ENTRY;

            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            dwError = VmDirAllocateStringAVsnprintf(
                    &pCtx->pszErrorMsg,
                    "Attribute (%s) can have at most one value",
                    VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val));

            dwError = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        for (usCnt = 0; usCnt < pAttr->numVals; usCnt++)
        {
            if (pAttr->pATDesc->uiMaxSize > 0)
            {
                //TODO, for server control/manipulate attribute, we should exclude this restriction
                // as they no longer in their original form.  (e.g. userPassword)
                if (pAttr->vals[usCnt].lberbv.bv_len > pAttr->pATDesc->uiMaxSize)
                {
                    VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
                    VmDirAllocateStringAVsnprintf(    // ignore error
                            &pCtx->pszErrorMsg,
                            "Attribute (%s) value too long, max (%d) allowed.",
                            VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val),
                            pAttr->pATDesc->uiMaxSize);

                    dwError = pCtx->dwErrorCode = LDAP_CONSTRAINT_VIOLATION;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }

            dwError = VmDirSchemaBervalSyntaxCheck(
                    pCtx,
                    pAttr->pATDesc,
                    &pAttr->vals[usCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pCtx->pszErrorMsg));

    goto cleanup;
}

static
PVDIR_SCHEMA_OC_DESC
_VmDirSchemaCheckOCDescLookup(
        PVDIR_SCHEMA_CTX    pCtx,
        PSTR                pOCName
        )
{
    VDIR_SCHEMA_OC_DESC key = {0};

    key.pszName = pOCName;

    return (PVDIR_SCHEMA_OC_DESC) bsearch(
            &key,
            pCtx->pSchema->ocs.pOCSortName,
            pCtx->pSchema->ocs.usNumOCs,
            sizeof(VDIR_SCHEMA_OC_DESC),
            VdirSchemaPOCNameCmp);
}

static
PVDIR_ATTRIBUTE
_VmDirchemaCheckFindObjectClass(
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ENTRY           pEntry
    )
{
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_SCHEMA_AT_DESC pATDesc = pCtx->pSchema->ats.ppATSortIdMap[
                                        VDIR_ATTRIBUTE_OBJECTCLASS_INDEX];
    USHORT    usId = pATDesc->usAttrID;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (pAttr->pATDesc->usAttrID == usId)
        {
            return pAttr;
        }
    }

    return NULL;
}

/*
 * Mark allowed MAY attributes presented.
 */
static
DWORD
_VmDirSchemaCheckMayAttrPresent(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_AT_DESC*   ppTagetMayATs,
    PVDIR_ENTRY                  pEntry,
    PBOOLEAN                pbPresentList
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    int     iIdx = 0;
    PVDIR_ATTRIBUTE  pAttr = NULL;

    assert(pCtx && ppTagetMayATs && pEntry && pbPresentList);
    pAttr = pEntry->attrs;

    for (iIdx = 0; pAttr != NULL; pAttr = pAttr->next, iIdx++)
    {
        if ((pAttr->pATDesc->usage != VDIR_ATTRIBUTETYPE_USER_APPLICATIONS) ||
            pbPresentList[iIdx] == TRUE)
        {
            continue;
        }

        for (iCnt = 0; ppTagetMayATs[iCnt] != NULL; iCnt++)
        {
            if (pAttr->pATDesc->usAttrID == ppTagetMayATs[iCnt]->usAttrID)
            {
                pbPresentList[iIdx] = TRUE;
                break;  // find may attribute
            }
        }
    }

    return dwError;
}

/*
 * Mark MUST attributes presented.
 */
static
DWORD
_VmDirSchemaCheckMustAttrPresent(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_ENTRY             pEntry,
    PBOOLEAN                pbPresentList
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;

    assert(pCtx && pOCDesc && pEntry && pbPresentList);

    for (iCnt = 0; pOCDesc->ppAllMustATs[iCnt] != NULL; iCnt++)
    {
        int         iIdx = 0;
        PVDIR_ATTRIBUTE  pAttr = pEntry->attrs;

        for (iIdx = 0; pAttr != NULL; pAttr = pAttr->next, iIdx++)
        {
            if (pAttr->pATDesc->usAttrID == pOCDesc->ppAllMustATs[iCnt]->usAttrID)
            {
                pbPresentList[iIdx] = TRUE;     // find must attribute
                break;
            }
        }

        // ignore missing "nTSecurityDescriptor" must attribute for now.
        // ADSI needs it to be a must attribute.  However, it is NOT easy/clean to make and
        // enforce this change in Lotus. (e.g. in VmDirInteralAddEntry, schema check is called
        //   prior to SD generation currently.)
        // TODO, clean up SD generation in bootstratp/promo/normal paths.
        if ( pAttr == NULL
             &&
             VmDirStringCompareA( pOCDesc->ppAllMustATs[iCnt]->pszName,
                                  ATTR_OBJECT_SECURITY_DESCRIPTOR,
                                  FALSE) != 0
           )
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Missing must attribute (%s)",
                    VDIR_SAFE_STRING(pOCDesc->ppAllMustATs[iCnt]->pszName));

            VmDirLog( LDAP_DEBUG_ANY, "%s", pCtx->pszErrorMsg);

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:

    return dwError;
}

/*
 * Per structure objectclass, verify contentrule auxiliary object class
 */
static
DWORD
_VmDirSchemaCheckContentRuleAuxOC(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc,
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    BOOLEAN bHasAllowedAuxOC = FALSE;

    assert(pCtx && pStructureOCDesc && ppAuxOCDesc);

    if (pCtx->pSchema->contentRules.usNumContents == 0)
    {
        // schema no content rule support
        goto cleanup;
    }

    bHasAllowedAuxOC = pStructureOCDesc->ppAllowedAuxOCs ? TRUE : FALSE;

    if (!bHasAllowedAuxOC && ppAuxOCDesc[0])
    {   // entry structure oc has NO allowedAuxOCs but entry now has aux ocs
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (; ppAuxOCDesc[iCnt]; iCnt++)
    {
        int iIdx = 0;
        for (;pStructureOCDesc->ppAllowedAuxOCs[iIdx]; iIdx++)
        {
            if (pStructureOCDesc->ppAllowedAuxOCs[iIdx] == ppAuxOCDesc[iCnt])
            {
                break;
            }
        }

        if (pStructureOCDesc->ppAllowedAuxOCs[iIdx] == NULL)
        {   // entry aux oc not found in allowed list from content rule
            dwError = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:

    pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
    VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
    VmDirAllocateStringAVsnprintf(
            &pCtx->pszErrorMsg,
            "Aux objectclass (%s) is not allowed.",
            VDIR_SAFE_STRING(ppAuxOCDesc[iCnt]->pszName));

    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pCtx->pszErrorMsg));

    goto cleanup;
}

/*
 * 1. entry must have one and only one structure object class (from same oc tree)
 * 2. auxiliary object class content rule compliance
 * 3. entry must have all MUST attributes
 * 4. entry may have allowed MAY attributes
 */
static
DWORD
_VmDirSchemaCheckEntryStructure(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_ATTRIBUTE         pOCAttr,
    BOOLEAN*                pbPresentList
    )
{
    DWORD                   dwError = 0;
    unsigned int            iCnt = 0;
    int                     iNumAuxOCs = 0;
    BOOLEAN                 bHasStructuralOC = FALSE;
    PVDIR_SCHEMA_OC_DESC    pStructOCDesc = NULL; // leaf structural OC
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc = NULL;

    assert(pCtx && pEntry && pOCAttr && pbPresentList);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_OC_DESC*) * (pOCAttr->numVals),
            (PVOID)&ppAuxOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    // walk through objectclass value to collect structure and aux ocs info
    for (iCnt = 0; iCnt < pOCAttr->numVals; iCnt++)
    {
        PVDIR_SCHEMA_OC_DESC pOCDesc =
                _VmDirSchemaCheckOCDescLookup(pCtx, pOCAttr->vals[iCnt].lberbv.bv_val);
        if (!pOCDesc)
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(
                    &pCtx->pszErrorMsg,
                    "Objectclass (%s) is not defined in schema",
                    VDIR_SAFE_STRING(pOCAttr->vals[iCnt].lberbv.bv_val));

            VmDirLog( LDAP_DEBUG_ANY, "schemaCheckStructure: (%s)", VDIR_SAFE_STRING(pCtx->pszErrorMsg));

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        switch (pOCDesc->type)
        {
        case VDIR_OC_STRUCTURAL:
            bHasStructuralOC = TRUE;

            if (!pStructOCDesc)
            {
                pStructOCDesc = pOCDesc;
                pEntry->pszStructureOC = pOCAttr->vals[iCnt].lberbv.bv_val;
            }
            else
            {   // make sure they are from the same structure tree
                PVDIR_SCHEMA_OC_DESC    pTmpOCDesc = pStructOCDesc;

                for (; pTmpOCDesc; pTmpOCDesc = pTmpOCDesc->pStructSupOC)
                {
                    if (pTmpOCDesc == pOCDesc) { break; }
                }

                if (!pTmpOCDesc) //  pOCDesc is NOT ancestor of pStructOCDesc
                {
                    for (pTmpOCDesc = pOCDesc; pTmpOCDesc; pTmpOCDesc = pTmpOCDesc->pStructSupOC)
                    {
                        if (pTmpOCDesc == pStructOCDesc)
                        {
                            // reset pStructOCDesc
                            pStructOCDesc = pOCDesc;
                            pEntry->pszStructureOC = pOCAttr->vals[iCnt].lberbv.bv_val;
                            break;
                        }
                    }
                }

                if (!pTmpOCDesc)
                {
                    VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
                    VmDirAllocateStringAVsnprintf(
                            &pCtx->pszErrorMsg,
                            "Entry can have only one structure objectclass.",
                            VDIR_SAFE_STRING(pOCDesc->pszName));

                    dwError = pCtx->dwErrorCode =  ERROR_INVALID_ENTRY;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            break;

        case VDIR_OC_AUXILIARY:
            ppAuxOCDesc[iNumAuxOCs] = pOCDesc;
            iNumAuxOCs++;
            break;

        default:
            // ABSTRACT object class
            break;
        }
    }

    // must have one structure oc
    if (bHasStructuralOC == FALSE)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        VmDirAllocateStringA(
                "Entry has no structural objectclass",
                &pCtx->pszErrorMsg);

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // enforce obejctclass value storing order structureOC->parentOC->...->TOP(not included)->AUXOC....
    dwError = _VmDirSchemaReorderObjectClassAttr( pOCAttr,
                                                  pStructOCDesc,
                                                  ppAuxOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    // enforce nameform (RDN)
    dwError = _VmDirSchemaCheckNameform( pCtx, pEntry, pStructOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    // content rule auxiliary constraint check
    if (iNumAuxOCs > 0)
    {
        dwError = _VmDirSchemaCheckContentRuleAuxOC(
                pCtx,
                pStructOCDesc,
                ppAuxOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    {   // must attributes check
        dwError = _VmDirSchemaCheckMustAttrPresent(
                pCtx,
                pStructOCDesc,
                pEntry,
                pbPresentList);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt = 0; ppAuxOCDesc[iCnt]; iCnt++)
        {
            dwError = _VmDirSchemaCheckMustAttrPresent(
                    pCtx,
                    ppAuxOCDesc[iCnt],
                    pEntry,
                    pbPresentList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    {   // may attributes check
        dwError = _VmDirSchemaCheckMayAttrPresent(
                pCtx,
                pStructOCDesc->ppAllMayATs,
                pEntry,
                pbPresentList);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt = 0; ppAuxOCDesc[iCnt]; iCnt++)
        {
            dwError = _VmDirSchemaCheckMayAttrPresent(
                    pCtx,
                    ppAuxOCDesc[iCnt]->ppAllMayATs,
                    pEntry,
                    pbPresentList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(ppAuxOCDesc);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaCheckNameform(
    PVDIR_SCHEMA_CTX        pCtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC    pStructOCDesc
    )
{
    DWORD       dwError = 0;
    PSTR        pszLocalErrorMsg = NULL;

    if ( !pCtx || !pStructOCDesc || !pEntry || !pEntry->dn.bvnorm_val )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#if 0

    // should NOT enable this until we have all namform and structure rule defined
    if ( pStructOCDesc->usNumMustRDNs > 0 )
    {   // not yet support multi RDNs case.  only check the first MUST RDN for now.
        size_t  iLen = VmDirStringLenA(pStructOCDesc->ppszMustRDNs[0]);

        if ( VmDirStringNCompareA( pEntry->dn.bvnorm_val,
                                   pStructOCDesc->ppszMustRDNs[0],
                                   iLen,
                                   FALSE ) != 0
             ||
             pEntry->dn.bvnorm_val[iLen] != '='
           )
        {
            dwError = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaCheckNameform: rdn must be (%s). (%d)",
                                          VDIR_SAFE_STRING( pStructOCDesc->ppszMustRDNs[0] ), dwError );
        }
    }

#endif

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:

    if ( !pCtx->pszErrorMsg )
    {
        pCtx->pszErrorMsg = pszLocalErrorMsg;
        pszLocalErrorMsg = NULL;

        pCtx->dwErrorCode = dwError;
    }

    goto cleanup;
}

/*
 * 1. must have objectclass value
 * 2. must have structural objectclass (done in schemaCheckStructure)
 * 3. must match contentrule allowed auxiliary objectclass definition
 * 4. must have all MUST attributes
 * 5. may have allowed MAY attributes
 */
static
DWORD
_VmDirSchemaCheckStructure(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD   dwError = 0;
    USHORT  usCnt = 0;
    BOOLEAN*    pbPresentList = NULL;
    DWORD   numAttrs = 0;
    PVDIR_ATTRIBUTE pTmpAttr = NULL;
    VDIR_ATTRIBUTE* pOCAttr = NULL;

    for (pTmpAttr = pEntry->attrs; pTmpAttr != NULL; pTmpAttr = pTmpAttr->next)
    {
        numAttrs++;
    }

    if (numAttrs == 0)
    {
        pCtx->dwErrorCode = ERROR_INVALID_ENTRY;

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringA(
                "Entry has no attributes",
                &pCtx->pszErrorMsg);

        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(BOOLEAN) * numAttrs,
            (PVOID*)&pbPresentList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pOCAttr = _VmDirchemaCheckFindObjectClass(pCtx, pEntry);

    if (!pOCAttr || pOCAttr->numVals < 1)
    {
        pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        dwError = VmDirAllocateStringA("Entry has no objectclass",&pCtx->pszErrorMsg);

        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirSchemaCheckEntryStructure(
            pCtx,
            pEntry,
            pOCAttr,
            pbPresentList);
    BAIL_ON_VMDIR_ERROR(dwError);

    {   // all VDIR_ATTRIBUTE_USER_APPLICATION attribute should be marked
        PVDIR_ATTRIBUTE pAttr = NULL;
        for (usCnt = 0, pAttr = pEntry->attrs;
             usCnt < numAttrs;
             usCnt++, pAttr = pAttr->next)
        {

            if (!pbPresentList[usCnt] && pAttr->pATDesc->usage == VDIR_ATTRIBUTETYPE_USER_APPLICATIONS)
            {
                VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
                dwError = VmDirAllocateStringAVsnprintf(
                        &pCtx->pszErrorMsg,
                        "Attribute (%s) not allowed",
                        VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val));

                dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pbPresentList);

    return dwError;

error:

    goto cleanup;
}

/*
 * reorder value of objectclass in pOCAttr->vals
 * 1. structureOC->parentOC->parentOC->....->TOP(not included)
 *    expand parent objectclass if not supplied.
 * 2. aux OCs
 * We then store its values in this order in backend.
 */
static
DWORD
_VmDirSchemaReorderObjectClassAttr(
    PVDIR_ATTRIBUTE         pOCAttr,
    PVDIR_SCHEMA_OC_DESC    pStructureOCDesc,
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc)
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    int         iAUXCnt = 0;
    int         iTotal = 0;
    PVDIR_BERVALUE          pBerv = NULL;
    PVDIR_SCHEMA_OC_DESC    pLocalDesc = pStructureOCDesc;

    if ( !pOCAttr || !pStructureOCDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // number of structure objectclass (except top)
    for (iCnt = 0; pLocalDesc != NULL; pLocalDesc = pLocalDesc->pStructSupOC, iCnt++) {}
    // number of aux objectclass
    for (iAUXCnt = 0; ppAuxOCDesc && ppAuxOCDesc[iAUXCnt] != NULL; iAUXCnt++) {};
    iTotal = iCnt + iAUXCnt;

    dwError = VmDirAllocateMemory( sizeof(VDIR_BERVALUE) * (iTotal + 1),
                                   (PVOID*)&pBerv);
    BAIL_ON_VMDIR_ERROR(dwError);

    // start with leaf structure objectclass and walk up the tree to top (NOT included)
    for (iCnt = 0, pLocalDesc = pStructureOCDesc;
         pLocalDesc != NULL;
         pLocalDesc = pLocalDesc->pStructSupOC, iCnt++)
    {
        PCSTR pszOrgName = NULL;
        unsigned int   iTmp = 0;

        for (iTmp = 0; iTmp < pOCAttr->numVals; iTmp++)
        {
            if (VmDirStringCompareA( pLocalDesc->pszName,
                                     pOCAttr->vals[iTmp].lberbv_val,
                                     FALSE) == 0)
            {   // keep whatever value provided from clients
                pszOrgName = pOCAttr->vals[iTmp].lberbv_val;
                break;
            }
        }

        dwError = VmDirAllocateStringA( pszOrgName ? pszOrgName : pLocalDesc->pszName,
                                        &(pBerv[iCnt].lberbv_val) );
        BAIL_ON_VMDIR_ERROR(dwError);

        pBerv[iCnt].lberbv_len = VmDirStringLenA(pBerv[iCnt].lberbv_val);
        pBerv[iCnt].bOwnBvVal = TRUE;
        // TODO, Do we need to normalize value here?
    }

    // append aux objectclasses after structure objectclasses
    for (iAUXCnt = 0; ppAuxOCDesc && ppAuxOCDesc[iAUXCnt] != NULL; iAUXCnt++, iCnt++)
    {
        PCSTR pszOrgName = NULL;
        unsigned int   iTmp = 0;

        for (iTmp = 0; iTmp < pOCAttr->numVals; iTmp++)
        {
            if (VmDirStringCompareA( ppAuxOCDesc[iAUXCnt]->pszName,
                                     pOCAttr->vals[iTmp].lberbv_val,
                                     FALSE) == 0)
            {   // keep whatever value provided from clients
                pszOrgName = pOCAttr->vals[iTmp].lberbv_val;
                break;
            }
        }

        dwError = VmDirAllocateStringA( pszOrgName ? pszOrgName : ppAuxOCDesc[iAUXCnt]->pszName,
                                        &(pBerv[iCnt].lberbv_val) );
        BAIL_ON_VMDIR_ERROR(dwError);

        pBerv[iCnt].lberbv_len = VmDirStringLenA(pBerv[iCnt].lberbv_val);
        pBerv[iCnt].bOwnBvVal = TRUE;
        // TODO, Do we need to normalize value here?
    }

    VmDirFreeBervalArrayContent( pOCAttr->vals, pOCAttr->numVals );
    VMDIR_SAFE_FREE_MEMORY( pOCAttr->vals );

    pOCAttr->vals = pBerv;
    pOCAttr->numVals = iTotal;

cleanup:

    return dwError;

error:

    VmDirFreeBervalArrayContent( pBerv, iTotal );
    VMDIR_SAFE_FREE_MEMORY( pBerv );

    goto cleanup;
}
