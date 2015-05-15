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
 * Filename: load.c
 *
 * Abstract: load schema into VDIR_SCHEMA INSTANCE
 *
 *
 */
#include "includes.h"

static
int
_VmDirSchemaPSRRuleIDCmp(
    const void *p1,
    const void *p2
    );

static
int
_VmDirSchemaPNFNameCmp(
    const void *p1,
    const void *p2
    );

static
int
_VmDirSchemaStrCaselessCmp(
    const void *p1,
    const void *p2
    );

static
DWORD
_VmDirSchemaATDupAliases(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
DWORD
_VmDirSchemaATDescSup(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
DWORD
schemaLoadATDescResolveInheritance(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
DWORD
_VmDirSchemaATDescResolveMatchingRule(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
DWORD
_VmDirSchemaATDescResolveSyntax(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
DWORD
_VmDirSchemaOCDescATs(
    PVDIR_SCHEMA_INSTANCE    pSchema,
    VDIR_SCHEMA_OC_ATTRS_TYPE attrType
    );

static
DWORD
_VmDirSchemaTraceATs(
    PVDIR_SCHEMA_INSTANCE     pSchema,
    PVDIR_SCHEMA_OC_DESC      pOCDesc,
    VDIR_SCHEMA_OC_ATTRS_TYPE attrType,
    PVDIR_SCHEMA_FILL_ALL_ATS pAllATs
    );

static
DWORD
_VmDirSchemaOCDescAllATs(
    PVDIR_SCHEMA_INSTANCE     pSchema,
    VDIR_SCHEMA_OC_ATTRS_TYPE attrTyp
    );

static
DWORD
_VmDirSchemaOCDescSupOCs(
    PVDIR_SCHEMA_OC_COLLECTION pOCCollection
    );

static
VOID
_VmDirSchemaSortDesc(
    PVDIR_SCHEMA_INSTANCE    pSchema
    );

static
DWORD
_VmDirSchemaCRIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    );

static
DWORD
_VmDirSchemaSRIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    );

static
DWORD
_VmDirSchemaSRSetAllowedParent(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_SCHEMA_SR_DESC    pSRDesc,
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE pTuple
    );

static
DWORD
_VmDirSchemaSRSetAllowedChild(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_SR_DESC    pSRDesc,
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE pTuple
    );

static
DWORD
_VmDirSchemaNFIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    );

static
DWORD
_VmDirSchemaIDToSRDesc(
    PVDIR_SCHEMA_SR_COLLECTION  pSRCollection,
    PCSTR                       pszSRName,
    PVDIR_SCHEMA_SR_DESC*       ppSRDesc        // caller does NOT own out *ppSRDesc
    );

static
DWORD
_VmDirSchemaNameToNFDesc(
    PVDIR_SCHEMA_NF_COLLECTION  pNFCollection,
    PCSTR                       pszNFName,
    PVDIR_SCHEMA_NF_DESC*       ppNFDesc        // caller does NOT own out *ppNFDesc
    );

static
VOID
_VmDirSchemaSortAndUnique(
    USHORT*     pusSize,
    PSTR*       ppszList
    );

DWORD
VmDirSchemaLoadInstance(
    PVDIR_SCHEMA_INSTANCE    pSchema
    )
{
    DWORD dwError = 0;

    // duplicate aliases into separate VDIR_SCHEMA_ATTR_DESC,
    // so they are represented by individual internal id.
    dwError = _VmDirSchemaATDupAliases(&pSchema->ats);
    BAIL_ON_VMDIR_ERROR(dwError);

    // sort AT/OC/CR/SR/NF DESC SortByName array
    _VmDirSchemaSortDesc(pSchema);

    // resolve VDIR_SCHEMA_ATTR_DESC.pSup
    dwError = _VmDirSchemaATDescSup(&pSchema->ats);
    BAIL_ON_VMDIR_ERROR(dwError);

    // resolve attribute definition inheritance
    dwError = schemaLoadATDescResolveInheritance(&pSchema->ats);
    BAIL_ON_VMDIR_ERROR(dwError);

    // resolve VDIR_SCHEMA_ATTR_DESC.pSyntax
    dwError = _VmDirSchemaATDescResolveSyntax(&pSchema->ats);
    BAIL_ON_VMDIR_ERROR(dwError);

    // resolve VDIR_SCHEMA_ATTR_DESC.p matching rules
    dwError = _VmDirSchemaATDescResolveMatchingRule(&pSchema->ats);
    BAIL_ON_VMDIR_ERROR(dwError);

    // resolve VDIR_SCHEMA_OC_DESC.ppSupOCs
    dwError = _VmDirSchemaOCDescSupOCs(&pSchema->ocs);
    BAIL_ON_VMDIR_ERROR(dwError);

    // merge CONTENT_DESC information into OC_DESC
    dwError = _VmDirSchemaCRIntoOCDesc(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    // merge NAMEFORM_DESC information into OC_DESC
    dwError = _VmDirSchemaNFIntoOCDesc(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    // merge STRUCTURE_DESC information into OC_DESC
    dwError = _VmDirSchemaSRIntoOCDesc(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    // resolve VDIR_SCHEMA_OC_DESC may/must/allmay/allmust attributes
    dwError = _VmDirSchemaOCDescATs(
            pSchema,
            VDIR_OC_ATTR_MAY);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaOCDescATs(
            pSchema,
            VDIR_OC_ATTR_MUST);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaOCDescAllATs(
            pSchema,
            VDIR_OC_ATTR_MAY);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaOCDescAllATs(
            pSchema,
            VDIR_OC_ATTR_MUST);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO, enforce oid unique among all (oc,at,syntax, mr...etc)

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Sort function -
 * Array of VDIR_SCHEMA_CR_DESC
 * String compare PVDIR_SCHEMA_CR_DESC->pszName
 */
int
VdirSchemaPCRNameCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_SCHEMA_CR_DESC pCRDesc1 = (PVDIR_SCHEMA_CR_DESC) p1;
    PVDIR_SCHEMA_CR_DESC pCRDesc2 = (PVDIR_SCHEMA_CR_DESC) p2;

    if ((pCRDesc1 == NULL || pCRDesc1->pszName == NULL) &&
        (pCRDesc2 == NULL || pCRDesc2->pszName == NULL))
    {
        return 0;
    }

    if (pCRDesc1 == NULL || pCRDesc1->pszName == NULL)
    {
        return -1;
    }

    if (pCRDesc2 == NULL || pCRDesc2->pszName == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pCRDesc1->pszName, pCRDesc2->pszName, FALSE);
}

/*
 * Sort function -
 * Array of VDIR_SCHEMA_AT_DESC
 * String compare PVDIR_SCHEMA_AT_DESC->pszName
 */
int
VdirSchemaPATNameCmp(
    const void *p1,
    const void *p2
    )
{

    PVDIR_SCHEMA_AT_DESC pATDesc1 = (PVDIR_SCHEMA_AT_DESC) p1;
    PVDIR_SCHEMA_AT_DESC pATDesc2 = (PVDIR_SCHEMA_AT_DESC) p2;

    if ((pATDesc1 == NULL || pATDesc1->pszName == NULL) &&
        (pATDesc2 == NULL || pATDesc2->pszName == NULL))
    {
        return 0;
    }

    if (pATDesc1 == NULL || pATDesc1->pszName == NULL)
    {
        return -1;
    }

    if (pATDesc2 == NULL || pATDesc2->pszName == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pATDesc1->pszName, pATDesc2->pszName, FALSE);
}

/*
 * Sort function -
 * Array of VDIR_SCHEMA_OC_DESC
 * String compare PVDIR_SCHEMA_OC_DESC->pszName
 */
int
VdirSchemaPOCNameCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_SCHEMA_OC_DESC pOCDesc1 = (PVDIR_SCHEMA_OC_DESC) p1;
    PVDIR_SCHEMA_OC_DESC pOCDesc2 = (PVDIR_SCHEMA_OC_DESC) p2;

    if ((pOCDesc1 == NULL || pOCDesc1->pszName == NULL) &&
        (pOCDesc2 == NULL || pOCDesc2->pszName == NULL))
    {
        return 0;
    }

    if (pOCDesc1 == NULL || pOCDesc1->pszName == NULL)
    {
        return -1;
    }

    if (pOCDesc2 == NULL || pOCDesc2->pszName == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pOCDesc1->pszName, pOCDesc2->pszName, FALSE);
}

/*
 * Sort function -
 * Array of VDIR_SCHEMA_NF_DESC
 * String compare PVDIR_SCHEMA_NF_DESC->pszName
 */
static
int
_VmDirSchemaPNFNameCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_SCHEMA_NF_DESC pNFDesc1 = (PVDIR_SCHEMA_NF_DESC) p1;
    PVDIR_SCHEMA_NF_DESC pNFDesc2 = (PVDIR_SCHEMA_NF_DESC) p2;

    if ((pNFDesc1 == NULL || pNFDesc1->pszName == NULL) &&
        (pNFDesc2 == NULL || pNFDesc2->pszName == NULL))
    {
        return 0;
    }

    if (pNFDesc1 == NULL || pNFDesc1->pszName == NULL)
    {
        return -1;
    }

    if (pNFDesc2 == NULL || pNFDesc2->pszName == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pNFDesc1->pszName, pNFDesc2->pszName, FALSE);
}

DWORD
VmDirSchemaNameToOCDesc(
    PVDIR_SCHEMA_OC_COLLECTION  pOCCollection,
    PCSTR                       pszOCName,
    PVDIR_SCHEMA_OC_DESC*       ppOCDesc        // caller does NOT own out *ppOCDesc
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;
    VDIR_SCHEMA_OC_DESC     key = {0};

    if ( !pOCCollection || !pszOCName || !ppOCDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszOCName;

    pOCDesc = (PVDIR_SCHEMA_OC_DESC) bsearch(
            &key,
            pOCCollection->pOCSortName,
            pOCCollection->usNumOCs,
            sizeof(VDIR_SCHEMA_OC_DESC),
            VdirSchemaPOCNameCmp);

    if (!pOCDesc)
    {
        dwError = ERROR_NO_SUCH_OBJECTCLASS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOCDesc = pOCDesc;

error:

    return dwError;
}

DWORD
VmDirSchemaNameToATDesc(
    PVDIR_SCHEMA_AT_COLLECTION  pATCollection,
    PCSTR                       pszATName,
    PVDIR_SCHEMA_AT_DESC*       ppATDesc        // caller does NOT own out *ppATDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
    VDIR_SCHEMA_AT_DESC     key = {0};

    if ( !pATCollection || !pszATName || !ppATDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszATName;

    pATDesc = (PVDIR_SCHEMA_AT_DESC) bsearch(   &key,
                                                pATCollection->pATSortName,
                                                pATCollection->usNumATs,
                                                sizeof(VDIR_SCHEMA_AT_DESC),
                                                VdirSchemaPATNameCmp);
    if ( !pATDesc )
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppATDesc = pATDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaIDToSRDesc(
    PVDIR_SCHEMA_SR_COLLECTION  pSRCollection,
    PCSTR                       pszSRName,
    PVDIR_SCHEMA_SR_DESC*       ppSRDesc        // caller does NOT own out *ppSRDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_SR_DESC    pSRDesc = NULL;
    VDIR_SCHEMA_SR_DESC     key = {0};

    if ( !pSRCollection || !pszSRName || !ppSRDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszRuleID = (PSTR)pszSRName;

    pSRDesc = (PVDIR_SCHEMA_SR_DESC) bsearch(   &key,
                                                pSRCollection->pStructureSortRuleID,
                                                pSRCollection->usNumStructures,
                                                sizeof(VDIR_SCHEMA_SR_DESC),
                                                _VmDirSchemaPSRRuleIDCmp);
    if ( !pSRDesc )
    {
        dwError = ERROR_NO_SUCH_DITSTRUCTURERULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSRDesc = pSRDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaNameToNFDesc(
    PVDIR_SCHEMA_NF_COLLECTION  pNFCollection,
    PCSTR                       pszNFName,
    PVDIR_SCHEMA_NF_DESC*       ppNFDesc        // caller does NOT own out *ppNFDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_NF_DESC    pNFDesc = NULL;
    VDIR_SCHEMA_NF_DESC     key = {0};

    if ( !pNFCollection || !pszNFName || !ppNFDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszName = (PSTR)pszNFName;

    pNFDesc = (PVDIR_SCHEMA_NF_DESC) bsearch(   &key,
                                                pNFCollection->pNameFormSortName,
                                                pNFCollection->usNumNameForms,
                                                sizeof(VDIR_SCHEMA_NF_DESC),
                                                _VmDirSchemaPNFNameCmp);
    if ( !pNFDesc )
    {
        dwError = ERROR_NO_SUCH_NAMEFORMS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppNFDesc = pNFDesc;

cleanup:

    return dwError;

error:

    goto cleanup;
}


/*
 * Sort function -
 * Array of VDIR_SCHEMA_SR_DESC
 * String compare PVDIR_SCHEMA_SR_DESC->pszRuleID
 */
static
int
_VmDirSchemaPSRRuleIDCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_SCHEMA_SR_DESC pSRDesc1 = (PVDIR_SCHEMA_SR_DESC) p1;
    PVDIR_SCHEMA_SR_DESC pSRDesc2 = (PVDIR_SCHEMA_SR_DESC) p2;

    if ((pSRDesc1 == NULL || pSRDesc1->pszRuleID == NULL) &&
        (pSRDesc2 == NULL || pSRDesc2->pszRuleID == NULL))
    {
        return 0;
    }

    if (pSRDesc1 == NULL || pSRDesc1->pszRuleID == NULL)
    {
        return -1;
    }

    if (pSRDesc2 == NULL || pSRDesc2->pszRuleID == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pSRDesc1->pszRuleID, pSRDesc2->pszRuleID, FALSE);
}

/*
 * duplicate ppszAliases into VDIR_SCHEMA_AT_DESC
 *
 * populate ppATSortByInternalId
 */
static
DWORD
_VmDirSchemaATDupAliases(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD  dwError = 0;
    DWORD  dwCnt = 0;
    USHORT dwSize = 0;
    DWORD  dwIdx = 0;
    USHORT dwOldSize = pATCollection->usNumATs;
    DWORD  dwNext = 0;

    for (dwCnt=0, dwSize=0 ; dwCnt < dwOldSize; dwCnt++, dwSize++)
    {
        PSTR* ppszAlias = pATCollection->pATSortName[dwCnt].ppszAliases;
        if (ppszAlias)
        {
            for (dwIdx = 0; ppszAlias[dwIdx]; dwIdx++)
            {
                dwSize++;
            }
        }
    }

    if (dwSize != dwOldSize)
    {
        dwError = VmDirReallocateMemoryWithInit(
                pATCollection->pATSortName,
                (PVOID*)&pATCollection->pATSortName,
                sizeof(VDIR_SCHEMA_AT_DESC) * (dwSize),
                sizeof(VDIR_SCHEMA_AT_DESC) * (dwOldSize));
        BAIL_ON_VMDIR_ERROR(dwError);

        pATCollection->usNumATs = dwSize;

        for (dwCnt = 0, dwNext = dwOldSize; dwCnt < dwOldSize; dwCnt++)
        {
            PVDIR_SCHEMA_AT_DESC pNow = &pATCollection->pATSortName[dwCnt];
            PSTR* ppszAlias = pATCollection->pATSortName[dwCnt].ppszAliases;
            if (ppszAlias)
            {
                for (dwIdx = 0; ppszAlias[dwIdx]; dwIdx++)
                {
                    PVDIR_SCHEMA_AT_DESC pNext =
                            &pATCollection->pATSortName[dwNext++];

                    pNext->uiMaxSize        = pNow->uiMaxSize;
                    pNext->bSingleValue     = pNow->bSingleValue;
                    pNext->bCollective      = pNow->bCollective;
                    pNext->bNoUserModifiable  = pNow->bNoUserModifiable;
                    pNext->bNoUserModifiable  = pNow->bNoUserModifiable;
                    pNext->usage            = pNow->usage;

                    dwError = VmDirAllocateStringA(
                            ppszAlias[dwIdx],
                            &pNext->pszName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszOid,
                            &pNext->pszOid);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszDesc,
                            &pNext->pszDesc);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszSup,
                            &pNext->pszSup);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszSyntaxName,
                            &pNext->pszSyntaxName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszEqualityMRName,
                            &pNext->pszEqualityMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszOrderingMRName,
                            &pNext->pszOrderingMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(
                            pNow->pszSubStringMRName,
                            &pNext->pszSubStringMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaATDescSup(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;

    for (dwCnt = 0; dwCnt < pATCollection->usNumATs; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC pATDesc = pATCollection->pATSortName+dwCnt;

        if (pATDesc->pszSup)
        {
            PVDIR_SCHEMA_AT_DESC pResult = NULL;

            dwError = VmDirSchemaNameToATDesc(  pATCollection,
                                                pATDesc->pszSup,
                                                &pResult);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaATDescSup: at (%s) not defined. (%d)",
                                          VDIR_SAFE_STRING( pATDesc->pszSup ), dwError );

            pATDesc->pSup = pResult;
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING(pszLocalErrorMsg) );

    goto cleanup;
}

/*
 * Attribute can inherit syntax from parent
 */
static
DWORD
schemaLoadATDescResolveInheritance(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;

    for (dwCnt = 0; dwCnt < pATCollection->usNumATs; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC pATDesc = pATCollection->pATSortName+dwCnt;

        if (!pATDesc->pszSyntaxName && pATDesc->pSup)
        {
            PVDIR_SCHEMA_AT_DESC pParentATDesc = pATDesc->pSup;
            while (pParentATDesc)
            {
                if (pParentATDesc->pszSyntaxName)
                {
                    dwError = VmDirAllocateStringA(
                        pParentATDesc->pszSyntaxName,
                        &pATDesc->pszSyntaxName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;
                }

                pParentATDesc = pParentATDesc->pSup;
            }
        }

        if (!pATDesc->pszEqualityMRName && pATDesc->pSup)
        {
            PVDIR_SCHEMA_AT_DESC pParentATDesc = pATDesc->pSup;
            while (pParentATDesc)
            {
                if (pParentATDesc->pszEqualityMRName)
                {
                    dwError = VmDirAllocateStringA(
                            pParentATDesc->pszEqualityMRName,
                            &pATDesc->pszEqualityMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;
                }

                pParentATDesc = pParentATDesc->pSup;
            }
        }

        if (!pATDesc->pszOrderingMRName && pATDesc->pSup)
        {
            PVDIR_SCHEMA_AT_DESC pParentATDesc = pATDesc->pSup;
            while (pParentATDesc)
            {
                if (pParentATDesc->pszOrderingMRName)
                {
                    dwError = VmDirAllocateStringA(
                            pParentATDesc->pszOrderingMRName,
                            &pATDesc->pszOrderingMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;
                }

                pParentATDesc = pParentATDesc->pSup;
            }
        }

        if (!pATDesc->pszSubStringMRName && pATDesc->pSup)
        {
            PVDIR_SCHEMA_AT_DESC pParentATDesc = pATDesc->pSup;
            while (pParentATDesc)
            {
                if (pParentATDesc->pszSubStringMRName)
                {
                    dwError = VmDirAllocateStringA(
                            pParentATDesc->pszSubStringMRName,
                            &pATDesc->pszSubStringMRName);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;
                }

                pParentATDesc = pParentATDesc->pSup;
            }
        }

    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaATDescResolveMatchingRule(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;

    for (dwCnt = 0; dwCnt < pATCollection->usNumATs; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC pATDesc = pATCollection->pATSortName+dwCnt;

        if (pATDesc->pszEqualityMRName)
        {
            pATDesc->pEqualityMR = VdirMatchingRuleLookupByName(
                    pATDesc->pszEqualityMRName);

            if (!pATDesc->pEqualityMR)
            {
                //dwError = ERROR_INVALID_SCHEMA;
                //BAIL_ON_VMDIR_ERROR(dwError);
                VmDirLog( LDAP_DEBUG_TRACE,
                        "MatchingRule (%s) NOT supported in attribute (%s)",
                        pATDesc->pszEqualityMRName,
                        pATDesc->pszName);
            }
        }

        if (pATDesc->pszOrderingMRName)
        {
            pATDesc->pOrderingMR = VdirMatchingRuleLookupByName(
                    pATDesc->pszOrderingMRName);

            if (!pATDesc->pOrderingMR)
            {
                //dwError = ERROR_INVALID_SCHEMA;
                //BAIL_ON_VMDIR_ERROR(dwError);
                VmDirLog( LDAP_DEBUG_TRACE,
                        "MatchingRule (%s) NOT supported in attribute (%s)",
                        pATDesc->pszOrderingMRName,
                        pATDesc->pszName);
            }
        }

        if (pATDesc->pszSubStringMRName)
        {
            pATDesc->pSubStringMR = VdirMatchingRuleLookupByName(
                    pATDesc->pszSubStringMRName);

            if (!pATDesc->pSubStringMR)
            {
                //dwError = ERROR_INVALID_SCHEMA;
                //BAIL_ON_VMDIR_ERROR(dwError);
                VmDirLog( LDAP_DEBUG_TRACE,
                        "MatchingRule (%s) NOT supported in attribute (%s)",
                        pATDesc->pszSubStringMRName,
                        pATDesc->pszName);
            }
        }
    }

//cleanup:

    return dwError;

//error:

//    goto cleanup;
}

static
DWORD
_VmDirSchemaATDescResolveSyntax(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;

    for (dwCnt = 0; dwCnt < pATCollection->usNumATs; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC pATDesc = pATCollection->pATSortName+dwCnt;

        pATDesc->pSyntax = VdirSyntaxLookupByOid(pATDesc->pszSyntaxName);
        if (!pATDesc->pSyntax)
        {
            //dwError = ERROR_INVALID_SCHEMA;
            //BAIL_ON_VMDIR_ERROR(dwError);
            VmDirLog( LDAP_DEBUG_TRACE,
                    "Syntax (%s) NOT supported in attribute (%s)",
                    pATDesc->pszSyntaxName,
                    pATDesc->pszName);
        }
    }

//cleanup:

    return dwError;

//error:

//    goto cleanup;
}

/*
 * Populate VDIR_SCHEMA_OC_DESC ppMustATs/ppMayATs
 * i.e. object class's must/may attributes
 * This includes definitions from both objectclasses as well as ditcontentrules.
 */
static
DWORD
_VmDirSchemaOCDescATs(
    PVDIR_SCHEMA_INSTANCE      pSchema,
    VDIR_SCHEMA_OC_ATTRS_TYPE  attrType
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;

    for (dwCnt = 0; dwCnt < pSchema->ocs.usNumOCs; dwCnt++)
    {
        PSTR*  ppszThisATs  = NULL;
        USHORT usNumThisATs = 0;
        PVDIR_SCHEMA_AT_DESC** pppCurrentATs = NULL;

        PVDIR_SCHEMA_OC_DESC pOCDesc = pSchema->ocs.pOCSortName+dwCnt;

        if (attrType == VDIR_OC_ATTR_MAY)
        {
            usNumThisATs = pOCDesc->usNumMayATs;
            ppszThisATs  = pOCDesc->ppszMayATs;
            pppCurrentATs = &pOCDesc->ppMayATs;
        }
        else if (attrType == VDIR_OC_ATTR_MUST)
        {
            usNumThisATs = pOCDesc->usNumMustATs;
            ppszThisATs  = pOCDesc->ppszMustATs;
            pppCurrentATs = &pOCDesc->ppMustATs;
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (ppszThisATs)
        {
            DWORD dwIdx = 0;

            dwError = VmDirAllocateMemory(
                    sizeof(PVDIR_SCHEMA_AT_DESC) * (usNumThisATs+1),
                    (PVOID*)pppCurrentATs);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (dwIdx = 0; ppszThisATs[dwIdx]; dwIdx++)
            {
                PVDIR_SCHEMA_AT_DESC pResult = NULL;

                dwError = VmDirSchemaNameToATDesc(  &(pSchema->ats),
                                                    ppszThisATs[dwIdx],
                                                    &pResult);
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                              "_VmDirSchemaOCDescATs: at (%s) not defined. (%d)",
                                              VDIR_SAFE_STRING( ppszThisATs[dwIdx] ), dwError );

                (*pppCurrentATs)[dwIdx] = pResult;
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING( pszLocalErrorMsg ) );

    goto cleanup;
}

/*
 * trace the ancestor tree to collect every one's MUST/MAY into ppAllMust/MayAts
 */
static
DWORD
_VmDirSchemaTraceATs(
    PVDIR_SCHEMA_INSTANCE     pSchema,
    PVDIR_SCHEMA_OC_DESC      pOCDesc,
    VDIR_SCHEMA_OC_ATTRS_TYPE attrType,
    PVDIR_SCHEMA_FILL_ALL_ATS pAllATs
    )
{
    DWORD dwError = 0;
    USHORT dwIdx = 0;
    USHORT dwCurrentATSize = 0;
    PVDIR_SCHEMA_AT_DESC* ppCurrentATs = NULL;
    DWORD dwOldSize = pAllATs->usSize;

    if (attrType == VDIR_OC_ATTR_MAY)
    {
        ppCurrentATs = pOCDesc->ppMayATs;
    }
    else if (attrType == VDIR_OC_ATTR_MUST)
    {
        ppCurrentATs = pOCDesc->ppMustATs;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ppCurrentATs)
    {
        for (dwIdx=0; ppCurrentATs[dwIdx]; dwIdx++) {}
        dwCurrentATSize = dwIdx;

        while (dwCurrentATSize >= (pAllATs->usSize - pAllATs->usCnt))
        {
                pAllATs->usSize = pAllATs->usSize * 2;
        }

        if (dwOldSize != pAllATs->usSize)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    *pAllATs->pppAllATs,
                    (PVOID*)pAllATs->pppAllATs,
                    sizeof(PVDIR_SCHEMA_AT_DESC) * (pAllATs->usSize + 1),
                    sizeof(PVDIR_SCHEMA_AT_DESC) * dwOldSize);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirCopyMemory(&(*pAllATs->pppAllATs)[pAllATs->usCnt],
                sizeof(PVDIR_SCHEMA_AT_DESC) * (pAllATs->usSize + 1),
                ppCurrentATs,
                sizeof(PVDIR_SCHEMA_AT_DESC) * dwCurrentATSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAllATs->usCnt += dwCurrentATSize;
    }

    for (dwIdx=0; pOCDesc->ppSupOCs && pOCDesc->ppSupOCs[dwIdx]; dwIdx++)
    {
        dwError = _VmDirSchemaTraceATs(
                pSchema,
                pOCDesc->ppSupOCs[dwIdx],
                attrType,
                pAllATs);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirSchemaOCDescAllATs(
    PVDIR_SCHEMA_INSTANCE     pSchema,
    VDIR_SCHEMA_OC_ATTRS_TYPE attrType
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;

    for (dwCnt = 0; dwCnt < pSchema->ocs.usNumOCs; dwCnt++)
    {
        PVDIR_SCHEMA_OC_DESC pOCDesc = pSchema->ocs.pOCSortName+dwCnt;
        VDIR_SCHEMA_FILL_ALL_ATS allATs = {0};

        if (attrType == VDIR_OC_ATTR_MAY)
        {
            allATs.pppAllATs = &pOCDesc->ppAllMayATs;
        }
        else if (attrType == VDIR_OC_ATTR_MUST)
        {
            allATs.pppAllATs = &pOCDesc->ppAllMustATs;
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        allATs.usCnt = 0;
        allATs.usSize = 10;

        dwError = VmDirAllocateMemory(
                sizeof(PVDIR_SCHEMA_AT_DESC) * (allATs.usSize+1),
                (PVOID*)allATs.pppAllATs);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSchemaTraceATs(
                pSchema,
                pOCDesc,
                attrType,
                &allATs);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (attrType == VDIR_OC_ATTR_MAY)
        {
            pOCDesc->usNumAllMayATs = allATs.usCnt;
        }
        else if (attrType == VDIR_OC_ATTR_MUST)
        {
            pOCDesc->usNumAllMustATs = allATs.usCnt;
        }
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}



/*
 * 1. Convert VMDIR_SCHEMA_OC_DESC.ppszSupOCs into .psSupOCs.
 * 2. Make sure Object Class inheritance tree is well defined.
 * TODO, check circular dependency
 */
static
DWORD
_VmDirSchemaOCDescSupOCs(
    PVDIR_SCHEMA_OC_COLLECTION pOCCollection
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;

    for (dwCnt = 0; dwCnt < pOCCollection->usNumOCs; dwCnt++)
    {
        PVDIR_SCHEMA_OC_DESC pOCDesc = pOCCollection->pOCSortName+dwCnt;

        if (pOCDesc->ppszSupOCs)
        {
            DWORD dwIdx = 0;
            DWORD dwSize = 0;

            dwError = VmDirAllocateMemory(
                    sizeof(*(pOCDesc->ppSupOCs)) * (pOCDesc->usNumSupOCs+1),
                    (PVOID*)&pOCDesc->ppSupOCs);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (dwIdx = 0, dwSize = 0; pOCDesc->ppszSupOCs[dwIdx]; dwIdx++)
            {
                PVDIR_SCHEMA_OC_DESC pParent = NULL;

                dwError = VmDirSchemaNameToOCDesc(  pOCCollection,
                                                    pOCDesc->ppszSupOCs[dwIdx],
                                                    &pParent);
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                              "_VmDirSchemaOCDescSupOCs: oc (%s) not defined. (%d)",
                                              VDIR_SAFE_STRING( pOCDesc->ppszSupOCs[dwIdx] ), dwError );

                if (pOCDesc->type == VDIR_OC_STRUCTURAL)
                {
                    if (pParent->type == VDIR_OC_AUXILIARY)
                    {
                        VmDirLog( LDAP_DEBUG_ANY,
                                "schemaLoadOCDescSupOCs: (%s) can't have aux (%s) parent.",
                                pOCDesc->pszName, pOCDesc->ppszSupOCs[dwIdx]);
                        dwError = ERROR_INVALID_SCHEMA;
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }

                    if (pParent->type == VDIR_OC_STRUCTURAL)
                    {
                        if (pOCDesc->pStructSupOC)
                        {
                            VmDirLog( LDAP_DEBUG_ANY,
                                    "schemaLoadOCDescSupOCs: (%s) have 2+ structure parents.",
                                    pOCDesc->pszName);
                            dwError = ERROR_INVALID_SCHEMA;
                            BAIL_ON_VMDIR_ERROR(dwError);
                        }
                        pOCDesc->pStructSupOC = pParent;
                    }
                }
                else if (pOCDesc->type == VDIR_OC_AUXILIARY)
                {
                    if (pParent->type == VDIR_OC_STRUCTURAL)
                    {
                        VmDirLog( LDAP_DEBUG_ANY,
                                "schemaLoadOCDescSupOCs: (%s) can't have structure parent.",
                                pOCDesc->pszName);
                        dwError = ERROR_INVALID_SCHEMA;
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }
                }
                else    // VDIR_OC_ABSTRACT
                {
                    if (pParent->type != VDIR_OC_ABSTRACT)
                    {
                        VmDirLog( LDAP_DEBUG_ANY,
                                "schemaLoadOCDescSupOCs: (%s) can have only abstract parent.",
                                pOCDesc->pszName);
                        dwError = ERROR_INVALID_SCHEMA;
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }
                }

                pOCDesc->ppSupOCs[dwSize++] = pParent;
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING( pszLocalErrorMsg ) );

    goto cleanup;

}

static
VOID
_VmDirSchemaSortDesc(
    PVDIR_SCHEMA_INSTANCE    pSchema
    )
{
    PVDIR_SCHEMA_AT_DESC    pATDescName = pSchema->ats.pATSortName;
    PVDIR_SCHEMA_OC_DESC    pOCDescName = pSchema->ocs.pOCSortName;
    PVDIR_SCHEMA_CR_DESC    pCRDescName = pSchema->contentRules.pContentSortName;
    PVDIR_SCHEMA_SR_DESC    pSRDescName = pSchema->structureRules.pStructureSortRuleID;
    PVDIR_SCHEMA_NF_DESC    pNFDescName = pSchema->nameForms.pNameFormSortName;

    qsort(  pATDescName,
            pSchema->ats.usNumATs,
            sizeof(VDIR_SCHEMA_AT_DESC),
            VdirSchemaPATNameCmp);

    qsort(  pOCDescName,
            pSchema->ocs.usNumOCs,
            sizeof(VDIR_SCHEMA_OC_DESC),
            VdirSchemaPOCNameCmp);

    if (pCRDescName)
    {
        qsort(  pCRDescName,
                pSchema->contentRules.usNumContents,
                sizeof(VDIR_SCHEMA_CR_DESC),
                VdirSchemaPCRNameCmp);
    }

    if (pSRDescName)
    {
        qsort(  pSRDescName,
                pSchema->structureRules.usNumStructures,
                sizeof(VDIR_SCHEMA_SR_DESC),
                _VmDirSchemaPSRRuleIDCmp);
    }

    if (pNFDescName)
    {
        qsort(  pNFDescName,
                pSchema->nameForms.usNumNameForms,
                sizeof(VDIR_SCHEMA_NF_DESC),
                _VmDirSchemaPNFNameCmp);
    }

    return;
}

/*
 * Move ContentRule.ppszMustATs/.ppszMayATs/.ppszAuxOCs
 * TO   ObjectClass.ppszMustATs/.ppszMayATs/.ppszAuxOCs
 *
 * ObjectClassDesc takes PSTRs memory ownership.
 *
 * At the end of the merge, sort and unique OC MUST/MAY/OC list
 */
static
DWORD
_VmDirSchemaCRIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;

    for (iCnt=0; iCnt < pSchema->contentRules.usNumContents; iCnt++)
    {
        PVDIR_SCHEMA_CR_DESC    pCRDesc = pSchema->contentRules.pContentSortName+iCnt;
        PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

        dwError = VmDirSchemaNameToOCDesc(  &(pSchema->ocs),
                                            pCRDesc->pszName,
                                            &pOCDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaCRIntoOCDesc: oc (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pCRDesc->pszName ), dwError );

        // only structure object class could have content rule defined
        if (pOCDesc->type != VDIR_OC_STRUCTURAL)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaCRIntoOCDesc: not structure oc (%s) (%d)",
                                          VDIR_SAFE_STRING( pCRDesc->pszName ), dwError );
        }

        // merge must attributes (transfer PSTRs and adjust size)
        if (pCRDesc->usNumMustATs > 0)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    pOCDesc->ppszMustATs,
                    (PVOID*)&pOCDesc->ppszMustATs,
                    (pOCDesc->usNumMustATs + pCRDesc->usNumMustATs + 1) * sizeof(PSTR),
                    (pOCDesc->usNumMustATs) * sizeof(PSTR));
            BAIL_ON_VMDIR_ERROR(dwError);

            // OCDecs takes over PSTRs
            dwError = VmDirCopyMemory(
                &pOCDesc->ppszMustATs[pOCDesc->usNumMustATs],
                (pOCDesc->usNumMustATs + pCRDesc->usNumMustATs + 1) * sizeof(PSTR),
                pCRDesc->ppszMustATs,
                sizeof(PSTR) * pCRDesc->usNumMustATs);
            BAIL_ON_VMDIR_ERROR(dwError);

            memset(pCRDesc->ppszMustATs, 0, sizeof(PSTR) * pCRDesc->usNumMustATs);
            pOCDesc->usNumMustATs += pCRDesc->usNumMustATs;
            pCRDesc->usNumMustATs = 0;
        }

        // merge may attributes (transfer PSTRs and adjust size)
        if (pCRDesc->usNumMayATs > 0)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    pOCDesc->ppszMayATs,
                    (PVOID*)&pOCDesc->ppszMayATs,
                    (pOCDesc->usNumMayATs + pCRDesc->usNumMayATs + 1) * sizeof(PSTR),
                    (pOCDesc->usNumMayATs) * sizeof(PSTR));
            BAIL_ON_VMDIR_ERROR(dwError);

            // OCDecs takes over PSTRs
            dwError = VmDirCopyMemory(
                &pOCDesc->ppszMayATs[pOCDesc->usNumMayATs],
                (pOCDesc->usNumMayATs + pCRDesc->usNumMayATs + 1) * sizeof(PSTR),
                pCRDesc->ppszMayATs,
                sizeof(PSTR) * pCRDesc->usNumMayATs);
            BAIL_ON_VMDIR_ERROR(dwError);

            memset(pCRDesc->ppszMayATs, 0, sizeof(PSTR) * pCRDesc->usNumMayATs);
            pOCDesc->usNumMayATs += pCRDesc->usNumMayATs;
            pCRDesc->usNumMayATs = 0;
        }

        // add allowed aux
        if (pCRDesc->usNumAuxOCs > 0)
        {
            int iCnt = 0;

            // OCDecs takes over PSTRs
            pOCDesc->ppszAuxOCs = pCRDesc->ppszAuxOCs;
            pOCDesc->usNumAuxOCs = pCRDesc->usNumAuxOCs;
            pCRDesc->ppszAuxOCs = NULL;
            pCRDesc->usNumAuxOCs = 0;

            // resolve ppszAuxOCs into ppAllowedAuxOCs
            dwError = VmDirAllocateMemory(
                    sizeof(PVDIR_SCHEMA_OC_DESC) * (pOCDesc->usNumAuxOCs+1),
                    (PVOID)&pOCDesc->ppAllowedAuxOCs);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (iCnt = 0; iCnt < pOCDesc->usNumAuxOCs; iCnt++)
            {
                PVDIR_SCHEMA_OC_DESC    pAuxOCDesc = NULL;

                dwError = VmDirSchemaNameToOCDesc(  &(pSchema->ocs),
                                                    pOCDesc->ppszAuxOCs[iCnt],
                                                    &pAuxOCDesc);
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                              "_VmDirSchemaCRIntoOCDesc: oc (%s) not defined. (%d)",
                                              VDIR_SAFE_STRING( pOCDesc->ppszAuxOCs[iCnt] ), dwError );

                if (pAuxOCDesc->type == VDIR_OC_AUXILIARY)
                {
                    pOCDesc->ppAllowedAuxOCs[iCnt] = pAuxOCDesc;
                }
                else
                {
                    dwError = ERROR_INVALID_SCHEMA;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                                dwError, pszLocalErrorMsg,
                              "_VmDirSchemaCRIntoOCDesc:(%s) allowed oc (%s) not AUX type (%d)",
                              VDIR_SAFE_STRING( pOCDesc->pszName ),
                              VDIR_SAFE_STRING( pOCDesc->ppszAuxOCs[iCnt] ),
                              dwError );
                }
            }

        }

    }

    for (iCnt=0; iCnt < pSchema->ocs.usNumOCs; iCnt++)
    {
        PVDIR_SCHEMA_OC_DESC    pOCDesc = pSchema->ocs.pOCSortName + iCnt;

        _VmDirSchemaSortAndUnique( &(pOCDesc->usNumMustATs), pOCDesc->ppszMustATs );
        _VmDirSchemaSortAndUnique( &(pOCDesc->usNumMayATs), pOCDesc->ppszMayATs );
        _VmDirSchemaSortAndUnique( &(pOCDesc->usNumAuxOCs), pOCDesc->ppszAuxOCs );
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING( pszLocalErrorMsg ) );

    goto cleanup;
}

static
DWORD
_VmDirSchemaSRSetAllowedParent(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PVDIR_SCHEMA_SR_DESC    pSRDesc,
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE pTuple
    )
{
    DWORD   dwError = 0;
    int     iLoopCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;
    size_t  iTupleIdx = (pOCDesc - pSchema->ocs.pOCSortName);
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE  pTarget = pTuple + iTupleIdx;

    if (pSRDesc->usNumSupRulesID == 0)
    {
        pOCDesc->bAllowedParentRoot = TRUE;
    }

    for (iLoopCnt = 0; iLoopCnt < pSRDesc->usNumSupRulesID; iLoopCnt++)
    {
        PVDIR_SCHEMA_SR_DESC    pTargetSRDesc = NULL;
        PVDIR_SCHEMA_NF_DESC    pTargetNFDesc = NULL;

        dwError = _VmDirSchemaIDToSRDesc( &(pSchema->structureRules),
                                          pSRDesc->ppszSupRulesID[iLoopCnt],
                                          &pTargetSRDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRSetAllowedParent: structure rule (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pSRDesc->ppszSupRulesID[iLoopCnt] ), dwError );

        dwError = _VmDirSchemaNameToNFDesc( &(pSchema->nameForms),
                                            pTargetSRDesc->pszNameform,
                                            &pTargetNFDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRSetAllowedParent: nameform (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pTargetSRDesc->pszNameform ), dwError );

        if (pTarget->iAllowedParentSize - pTarget->iAllowedParentCnt <= 1)
        {
            dwError = VmDirReallocateMemoryWithInit(    pTarget->ppszAllowedParentOCs,
                                                        (PVOID*)&pTarget->ppszAllowedParentOCs,
                                                        sizeof(PSTR) * (pTarget->iAllowedParentSize + SCHEMA_SIZE_5),
                                                        sizeof(PSTR) * (pTarget->iAllowedParentSize));
            BAIL_ON_VMDIR_ERROR(dwError);

            pTarget->iAllowedParentSize += 5;
        }

        dwError = VmDirAllocateStringA( pTargetNFDesc->pszStructOC,
                                        &pTarget->ppszAllowedParentOCs[pTarget->iAllowedParentCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirLog( LDAP_DEBUG_TRACE, "oc (%s) allowed Parent (%s)",
                                     VDIR_SAFE_STRING(pOCDesc->pszName),
                                     VDIR_SAFE_STRING(pTargetNFDesc->pszStructOC) );
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING( pszLocalErrorMsg ) );

    goto cleanup;
}

static
DWORD
_VmDirSchemaSRSetAllowedChild(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_SR_DESC    pSRDesc,
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE pTuple
    )
{
    DWORD                   dwError = 0;
    int                     iLoopCnt = 0;
    PSTR                    pszLocalErrorMsg = NULL;
    PVDIR_SCHEMA_NF_DESC    pSourceNFDesc = NULL;

    dwError = _VmDirSchemaNameToNFDesc( &(pSchema->nameForms),
                                        pSRDesc->pszNameform,
                                        &pSourceNFDesc);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                  "_VmDirSchemaSRSetAllowedChild: name (%s) not defined. (%d)",
                                  VDIR_SAFE_STRING( pSRDesc->pszNameform ), dwError );

    for (iLoopCnt = 0; iLoopCnt < pSRDesc->usNumSupRulesID; iLoopCnt++)
    {
        PVDIR_SCHEMA_SR_DESC    pTargetSRDesc = NULL;
        PVDIR_SCHEMA_OC_DESC    pTargetOCDesc = NULL;
        PVDIR_SCHEMA_NF_DESC    pTargetNFDesc = NULL;

        dwError = _VmDirSchemaIDToSRDesc( &(pSchema->structureRules),
                                          pSRDesc->ppszSupRulesID[iLoopCnt],
                                          &pTargetSRDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRSetAllowedChild: structure rule (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pSRDesc->ppszSupRulesID[iLoopCnt] ), dwError );

        dwError = _VmDirSchemaNameToNFDesc( &(pSchema->nameForms),
                                            pTargetSRDesc->pszNameform,
                                            &pTargetNFDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRSetAllowedChild: nameform (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pTargetSRDesc->pszNameform ), dwError );

        dwError = VmDirSchemaNameToOCDesc(   &(pSchema->ocs),
                                            pTargetNFDesc->pszStructOC,
                                            &pTargetOCDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRSetAllowedChild: oc (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pTargetNFDesc->pszStructOC ), dwError );

        {

        size_t iTupleIdx = (pTargetOCDesc - pSchema->ocs.pOCSortName);
        PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE  pTarget = pTuple + iTupleIdx;

        if (pTarget->iAllowedChildSize - pTarget->iAllowedChildCnt <= 1)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    pTarget->ppszAllowedChildOCs,
                    (PVOID*)&pTarget->ppszAllowedChildOCs,
                    sizeof(PSTR) * (pTarget->iAllowedChildSize + SCHEMA_SIZE_5),
                    sizeof(PSTR) * (pTarget->iAllowedChildSize));
            BAIL_ON_VMDIR_ERROR(dwError);

            pTarget->iAllowedChildSize += 5;
        }

        dwError = VmDirAllocateStringA( pSourceNFDesc->pszStructOC,
                                        &pTarget->ppszAllowedChildOCs[pTarget->iAllowedChildCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirLog( LDAP_DEBUG_TRACE, "oc (%s) allowed Child (%s)",
                                    VDIR_SAFE_STRING(pTargetOCDesc->pszName),
                                    VDIR_SAFE_STRING(pSourceNFDesc->pszStructOC) );
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;
error:

    VmDirLog( LDAP_DEBUG_ANY, VDIR_SAFE_STRING( pszLocalErrorMsg ) );

    goto cleanup;
}

/*
 *  Based on structure rule definitions, construct
 *  -> VDIR_SCHEMA_OC_DESC.ppszAllowedParentOCs
 *  -> VDIR_SCHEMA_OC_DESC.ppszAllowedChildOCs
 */
static
DWORD
_VmDirSchemaSRIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    )
{
    DWORD                               dwError = 0;
    int                                 iCnt = 0;
    PVDIR_SCHEMA_NAME_SIZE_CNT_TUPLE    pTuple = NULL;
    PSTR                                pszLocalErrorMsg = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_NAME_SIZE_CNT_TUPLE) * (pSchema->ocs.usNumOCs),
            (PVOID*)&pTuple);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt=0; iCnt < pSchema->structureRules.usNumStructures; iCnt++)
    {
        PVDIR_SCHEMA_SR_DESC    pSRDesc = pSchema->structureRules.pStructureSortRuleID + iCnt;
        PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;
        PVDIR_SCHEMA_NF_DESC    pNFDesc = NULL;

        // lookup nameform
        dwError = _VmDirSchemaNameToNFDesc( &(pSchema->nameForms),
                                            pSRDesc->pszNameform,
                                            &pNFDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaLoadNameToNFDesc: nameform (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pSRDesc->pszNameform ), dwError );

        // lookup structure object class
        dwError = VmDirSchemaNameToOCDesc(  &(pSchema->ocs),
                                            pNFDesc->pszStructOC,
                                            &pOCDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaSRIntoOCDesc: oc (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pNFDesc->pszStructOC ), dwError );

        // only structure object class could have structure rule defined
        if (pOCDesc->type != VDIR_OC_STRUCTURAL)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaSRIntoOCDesc: not structure oc (%s) (%d)",
                                          VDIR_SAFE_STRING( pSRDesc->pszNameform ), dwError );
        }

        // add allowed parent for this oc
        dwError = _VmDirSchemaSRSetAllowedParent(   pSchema,
                                                    pOCDesc,
                                                    pSRDesc,
                                                    pTuple);
        BAIL_ON_VMDIR_ERROR(dwError);

        // add allowed child of this oc
        dwError = _VmDirSchemaSRSetAllowedChild(    pSchema,
                                                    pSRDesc,
                                                    pTuple);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // transfer pTuple.ppszAllowedParentOCs/.ppszAllowedChildOCs to corresponding ocs
    for (iCnt=0; iCnt < pSchema->ocs.usNumOCs; iCnt++)
    {
        pSchema->ocs.pOCSortName[iCnt].ppszAllowedParentOCs = pTuple[iCnt].ppszAllowedParentOCs;
        pSchema->ocs.pOCSortName[iCnt].ppszAllowedChildOCs = pTuple[iCnt].ppszAllowedChildOCs;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pTuple);
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrorMsg );

    return dwError;

error:

    for (iCnt=0; pTuple && iCnt < pSchema->ocs.usNumOCs; iCnt++)
    {
        VmDirFreeStringArrayA(pTuple[iCnt].ppszAllowedParentOCs);
        VMDIR_SAFE_FREE_MEMORY(pTuple[iCnt].ppszAllowedParentOCs);

        VmDirFreeStringArrayA(pTuple[iCnt].ppszAllowedChildOCs);
        VMDIR_SAFE_FREE_MEMORY(pTuple[iCnt].ppszAllowedChildOCs);
    }

    if ( pszLocalErrorMsg )
    {
        VmDirLog( LDAP_DEBUG_ANY, pszLocalErrorMsg );
    }

    goto cleanup;
}

/*
 *  Based on nameform definitions, construct
 *  -> VDIR_SCHEMA_OC_DESC.ppszMustRDNs
 *  -> VDIR_SCHEMA_OC_DESC.ppszMayRDNs
 */
static
DWORD
_VmDirSchemaNFIntoOCDesc(
    PVDIR_SCHEMA_INSTANCE   pSchema
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    PSTR    pszLocalErrorMsg = NULL;

    for (iCnt=0; iCnt < pSchema->nameForms.usNumNameForms; iCnt++)
    {
        PVDIR_SCHEMA_NF_DESC  pNFDesc = pSchema->nameForms.pNameFormSortName + iCnt;
        PVDIR_SCHEMA_OC_DESC        pOCDesc = NULL;
        USHORT                      usIdx = 0;

        dwError = VmDirSchemaNameToOCDesc(  &(pSchema->ocs),
                                            pNFDesc->pszStructOC,
                                            &pOCDesc);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                      "_VmDirSchemaNFIntoOCDesc: oc (%s) not defined. (%d)",
                                      VDIR_SAFE_STRING( pNFDesc->pszStructOC ), dwError );

        // only structure object class could have structurerule and nameform defined
        if (pOCDesc->type != VDIR_OC_STRUCTURAL)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaNFIntoOCDesc: not structure oc (%s)",
                                          VDIR_SAFE_STRING( pNFDesc->pszStructOC) );
        }

        // MUST RDN attributes verification
        for (usIdx = 0; usIdx < pNFDesc->usNumMustATs; usIdx++)
        {
            PVDIR_SCHEMA_AT_DESC pResult = NULL;

            dwError = VmDirSchemaNameToATDesc(  &(pSchema->ats),
                                                pNFDesc->ppszMustATs[usIdx],
                                                &pResult);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaNFIntoOCDesc: at (%s) not defined. (%d)",
                                          VDIR_SAFE_STRING( pNFDesc->ppszMustATs[usIdx] ), dwError );

            // Support only ONE must RDN in AD mode.
            // TODO, should make this configurable.  e.g. AD or FULL
            if ( usIdx > 1 )
            {
                dwError = ERROR_INVALID_SCHEMA;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                              "_VmDirSchemaNFIntoOCDesc: only one Must RDN allowed.");
            }
        }

        // Disable support of may RDN in AD mode.
        // TODO, should make this configurable.  e.g. AD or FULL
        if ( pNFDesc->usNumMayATs > 0 )
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaNFIntoOCDesc: May RDN not supported.");
        }

        // May RDN attributes verification
        for (usIdx = 0; usIdx < pNFDesc->usNumMayATs; usIdx++)
        {
            PVDIR_SCHEMA_AT_DESC pResult = NULL;

            dwError = VmDirSchemaNameToATDesc(  &(pSchema->ats),
                                                pNFDesc->ppszMayATs[usIdx],
                                                &pResult);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg,
                                          "_VmDirSchemaNFIntoOCDesc: at (%s) not defined. (%d)",
                                          VDIR_SAFE_STRING( pNFDesc->ppszMayATs[usIdx]), dwError );
        }

        // pOCDesc takes over pNFDesc contents
        pOCDesc->ppszMustRDNs  = pNFDesc->ppszMustATs;
        pOCDesc->usNumMustRDNs = pNFDesc->usNumMustATs;
        pOCDesc->ppszMayRDNs   = pNFDesc->ppszMayATs;
        pOCDesc->usNumMayRDNs  = pNFDesc->usNumMayATs;

        pNFDesc->ppszMustATs  = NULL;
        pNFDesc->usNumMustATs = 0;
        pNFDesc->ppszMayATs   = NULL;
        pNFDesc->usNumMayATs  = 0;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:

    if ( pszLocalErrorMsg )
    {
        VmDirLog( LDAP_DEBUG_ANY, pszLocalErrorMsg );
    }

    goto cleanup;
}

static
int
_VmDirSchemaStrCaselessCmp(
    const void *p1,
    const void *p2
    )
{
    PSTR pszStr1 = *((PSTR*) p1);
    PSTR pszStr2 = *((PSTR*) p2);

    if ( pszStr1 == NULL && pszStr2 == NULL )
    {
        return 0;
    }

    if ( pszStr1 == NULL && pszStr2 != NULL)
    {
        return -1;
    }

    if ( pszStr2 == NULL && pszStr1 != NULL)
    {
        return 1;
    }

    return VmDirStringCompareA( pszStr1, pszStr2, FALSE);
}

/*
 * Sort and make ppszList elements unique
 * NOTE: we own memory of ppszList[] elements.
 */
static
VOID
_VmDirSchemaSortAndUnique(
    USHORT*     pusSize,
    PSTR*       ppszList
    )
{
    USHORT    usCnt = 0;
    USHORT    usGap = 0;

    // sort the list
    qsort(  ppszList, *pusSize, sizeof(PSTR), _VmDirSchemaStrCaselessCmp);

    for (usCnt = 1; usCnt < *pusSize; usCnt++ )
    {
        if ( VmDirStringCompareA( ppszList[usCnt - 1 - usGap], ppszList[usCnt], FALSE ) == 0 )
        {
            VMDIR_SAFE_FREE_MEMORY( ppszList[usCnt] );
            usGap++;
        }
        else
        {
            if ( usGap > 0 )
            {
                ppszList[usCnt - usGap] = ppszList[usCnt];
                ppszList[usCnt] = NULL;
            }
        }
    }

    *pusSize = *pusSize - usGap;

    return;
}
