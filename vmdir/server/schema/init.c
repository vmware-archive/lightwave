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
 * Filename: init.c
 *
 * Abstract: initialize VDIR_SCHEMA INSTANCE (schema cache)
 *
 * Globals
 *
 */

#include "includes.h"

static
int
schemaInitPPSTRCmp(
    const void *ppStr1,
    const void *ppStr2
    );

static
DWORD
schemaReadFile(
    PCSTR     pszSchemaFilePath,
    PSTR**    pppszDescs,
    USHORT*    pdwSize
    );

static
DWORD
schemaInitFillAttrFromFile(
    PVDIR_SCHEMA_CTX    pCtx,
    PSTR*    ppszDescs,
    USHORT   dwDescSize,
    PSTR     pszTag,
    size_t   dwTagSize,
    PVDIR_ENTRY   pEntry
    );

static
DWORD
schemaInitFillAttrFromCache(
    PVDIR_SCHEMA_CTX    pCtx,
    PSTR*    ppszValues,
    USHORT   dwValueSize,
    PSTR     pszAttrName,
    PVDIR_ENTRY   pEntry
    );

static
DWORD
_VmDirSchemaInitFixBootstrapEntry(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_ENTRY             pEntry
    );

static
DWORD
VmDirSchemaPatchMerge(
    PVDIR_ENTRY pCurrentEntry,
    PVDIR_ENTRY pPatchEntry
    );

static
DWORD
_VmDirSchemaEntryToInstance(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_INSTANCE*  ppSchema
    );

static
DWORD
_VmDirNormaliseNameField(
    PSTR       pszSource ,
    size_t      dwLen
    );

static
VOID
_VmDirSchemaNormalizeAttrValue(
    PVDIR_ATTRIBUTE pAttr
    );

/*
 * This function is only for testing purpose.
 * It call load and verify to validate schema definition integrity.
 */
DWORD
UnitTestSchemaInstanceInit(
    PSTR*    ppszDescs,
    DWORD    dwDescsSize,
    PVDIR_SCHEMA_INSTANCE*    ppSchema
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;
    DWORD    dwIdx = 0;

    USHORT   dwATSize = 0;
    USHORT   dwOCSize = 0;
    USHORT   dwCRSize = 0;
    USHORT   dwSRSize = 0;
    USHORT   dwNFSize = 0;

    PVDIR_SCHEMA_INSTANCE pSchema = NULL;
    PSTR    pszTmp = NULL;

    qsort(ppszDescs, dwDescsSize, sizeof(*ppszDescs), schemaInitPPSTRCmp);

    for (dwCnt=0; dwCnt < dwDescsSize; dwCnt++)
    {
        if (IS_ATTRIBUTETYPES_TAG(ppszDescs[dwCnt]))
        {
            dwATSize++;
        }
        else if (IS_OBJECTCLASSES_TAG(ppszDescs[dwCnt]))
        {
            dwOCSize++;
        }
        else if (IS_CONTENTRULES_TAG(ppszDescs[dwCnt]))
        {
            dwCRSize++;
        }
        else if (IS_STRUCTURERULES_TAG(ppszDescs[dwCnt]))
        {
            dwSRSize++;
        }
        else if (IS_NAMEFORM_TAG(ppszDescs[dwCnt]))
        {
            dwNFSize++;
        }
    }

    dwError = VdirSchemaInstanceAllocate(&pSchema, dwATSize, dwOCSize, dwCRSize, dwSRSize, dwNFSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0, dwIdx = 0; dwCnt < dwDescsSize; dwCnt++)
    {
        if (IS_ATTRIBUTETYPES_TAG(ppszDescs[dwCnt]))
        {
            dwError = VmDirSchemaParseStrToATDesc(
                    ppszDescs[dwCnt],
                    pSchema->ats.pATSortName+dwIdx);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwIdx++;
        }
    }

    for (dwCnt=0, dwIdx = 0; dwCnt < dwDescsSize ; dwCnt++)
    {
        if (IS_OBJECTCLASSES_TAG(ppszDescs[dwCnt]))
        {
            dwError = VmDirSchemaParseStrToOCDesc(
                    ppszDescs[dwCnt],
                    pSchema->ocs.pOCSortName+dwIdx);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwIdx++;
        }
    }

    for (dwCnt=0, dwIdx = 0; dwCnt < dwDescsSize ; dwCnt++)
    {
        if (IS_CONTENTRULES_TAG(ppszDescs[dwCnt]))
        {
            dwError = VmDirSchemaParseStrToContentDesc(
                    ppszDescs[dwCnt],
                    pSchema->contentRules.pContentSortName+dwIdx);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwIdx++;
        }
    }

    for (dwCnt=0, dwIdx = 0; dwCnt < dwDescsSize ; dwCnt++)
    {
        if (IS_STRUCTURERULES_TAG(ppszDescs[dwCnt]))
        {
            dwError = VmDirSchemaParseStrToStructureDesc(
                    ppszDescs[dwCnt],
                    pSchema->structureRules.pStructureSortRuleID+dwIdx);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwIdx++;
        }
    }

    for (dwCnt=0, dwIdx = 0; dwCnt < dwDescsSize ; dwCnt++)
    {
        if (IS_NAMEFORM_TAG(ppszDescs[dwCnt]))
        {
            dwError = VmDirSchemaParseStrToNameformDesc(    ppszDescs[dwCnt],
                                                            pSchema->nameForms.pNameFormSortName + dwIdx);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwIdx++;
        }
    }

#ifdef LDAP_DEBUG
    for (dwCnt = 0; dwCnt < dwOCSize; dwCnt ++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        VdirSchemaVerifyOCDescPrint(
                pSchema->ocs.pOCSortName+dwCnt,
                &pszTmp);
        printf("%s\n", VDIR_SAFE_STRING(pszTmp));
    }
    for (dwCnt = 0; dwCnt < dwATSize; dwCnt ++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        VdirSchemaVerifyATDescPrint(
                pSchema->ats.pATSortName+dwCnt,
                &pszTmp);
        printf("%s\n", VDIR_SAFE_STRING(pszTmp));
    }
#endif

    dwError = VmDirSchemaLoadInstance(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (! VmDirSchemaVerifyIntegrity(pSchema))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSchema = pSchema;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszTmp);

    return dwError;

error:

    if (pSchema)
    {
        VdirSchemaInstanceFree(pSchema);
    }

    *ppSchema = NULL;

    goto cleanup;
}

/*
 * convert schema file into entry
 */
DWORD
VmDirSchemaInitalizeFileToEntry(
    PCSTR           pszSchemaFilePath,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD    dwError = 0;
    USHORT   dwCnt = 0;
    PVDIR_ENTRY    pEntry = NULL;

    PSTR    pszTag = NULL;
    PSTR    pszTagEnd = NULL;
    USHORT  dwTag = 0;

    PSTR*    ppszDescs = NULL;
    USHORT   dwDescSize = 0;

    PSTR*    ppszSyntaxes = NULL;
    USHORT   dwSyntaxSize = 0;

    PSTR*    ppszMRs = NULL;
    USHORT   dwMRSize = 0;

    PVDIR_SCHEMA_CTX    pCtx = NULL;

    if ( !pszSchemaFilePath || !ppEntry )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = schemaReadFile(
        pszSchemaFilePath,
        &ppszDescs,
        &dwDescSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    // sort ppszDescs by tag order (i.e. ats, ocs, syntax...)
    qsort(ppszDescs, dwDescSize, sizeof(*ppszDescs), schemaInitPPSTRCmp);

	dwError = VmDirSchemaCtxAcquire(&pCtx);
	BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
        sizeof(VDIR_ENTRY),
        (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            SUB_SCHEMA_SUB_ENTRY_DN,
            &pEntry->dn.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->dn.bOwnBvVal = TRUE;
    pEntry->dn.lberbv.bv_len = VmDirStringLenA(pEntry->dn.lberbv.bv_val);

    dwError = VmDirNormalizeDN( &(pEntry->dn), pCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    // load every thing from file except EntryDN, ldapSyntaxes and matchingRules
    {
        // By now, we know all PSTR have format - "tag: value"
        for (dwCnt=0; dwCnt < dwDescSize; dwCnt++)
        {
            PSTR pszNewTag = ppszDescs[dwCnt];
            PSTR pszNewTagEnd = VmDirStringChrA(ppszDescs[dwCnt], ':');

            if (pszTag == NULL)
            {
                pszTag = pszNewTag;
                pszTagEnd = pszNewTagEnd;
                dwTag = dwCnt;
                continue;
            }

            if (((pszTagEnd - pszTag) == (pszNewTagEnd - pszNewTag)) &&
                VmDirStringNCompareA(pszTag, pszNewTag, pszTagEnd - pszTag, TRUE) == 0)
            {
                continue;
            }
            else
            {
                dwError = schemaInitFillAttrFromFile(
                        pCtx,
                        ppszDescs+dwTag,
                        dwCnt - dwTag,
                        pszTag,
                        pszTagEnd - pszTag,
                        pEntry);
                BAIL_ON_VMDIR_ERROR(dwError);

                pszTag = pszNewTag;
                pszTagEnd = pszNewTagEnd;
                dwTag = dwCnt;
            }
        }

        dwError = schemaInitFillAttrFromFile(
                pCtx,
                ppszDescs+dwTag,
                dwCnt - dwTag,
                pszTag,
                pszTagEnd - pszTag,
                pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdirSyntaxGetDefinition(
            &ppszSyntaxes,
            &dwSyntaxSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = schemaInitFillAttrFromCache(
            pCtx,
            ppszSyntaxes,
            dwSyntaxSize,
            VDIR_ATTRIBUTE_LADPSYNTAXES,
            pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirMatchingRuleGetDefinition(
            &ppszMRs,
            &dwMRSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = schemaInitFillAttrFromCache(
            pCtx,
            ppszMRs,
            dwMRSize,
            VDIR_ATTRIBUTE_MATCHINGRULES,
            pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->eId = SUB_SCEHMA_SUB_ENTRY_ID;

    // NOTE, entry is only partially constructed to bootstrap schema
    *ppEntry = pEntry;

cleanup:

    if (pCtx)
    {
        VmDirSchemaCtxRelease(pCtx);
    }

    if (ppszDescs)
    {
        VmDirFreeStringArrayA(ppszDescs);
        VMDIR_SAFE_FREE_MEMORY(ppszDescs);
    }

    if (ppszSyntaxes)
    {
        VmDirFreeStringArrayA(ppszSyntaxes);
        VMDIR_SAFE_FREE_MEMORY(ppszSyntaxes);
    }

    if (ppszMRs)
    {
        VmDirFreeStringArrayA(ppszMRs);
        VMDIR_SAFE_FREE_MEMORY(ppszMRs);
    }

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;

}

/*
 * 1. convert patch file into pNewEntry
 * 2. merge changes in pNewEntry INTO pEntry
 *
 * NOTE, Lotus schema/patch file should NEVER delete schema definitions.
 */
DWORD
VmDirSchemaPatchFileToEntry(
    PCSTR           pszSchemaFilePath,
    PVDIR_ENTRY     pEntry
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY         pNewEntry = NULL;

    if ( !pszSchemaFilePath || !pEntry )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaInitalizeFileToEntry( pszSchemaFilePath,
                                               &pNewEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaPatchMerge( pEntry, pNewEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pNewEntry)
    {
        VmDirFreeEntry(pNewEntry);
    }

    return dwError;

error:

    goto cleanup;
}



/*
 * Create a schema cache from pEntry
 * 1. parse AT
 * 2. parse OC
 * 2.1 parse ContentRule
 * 2.2 parse StructureRule
 * 2.3 parse Nameform
 * 3. load AT and OC and Content and Structure Rule and Nameform into cache
 * 4. resolve AT and OC and Content and Structure Rule (SUP, alias, syntax, matchingrule...etc.)
 * 5. verify cache
 * 6. generate/modify vmwAttributeToIdMap (this modifies pEntry contents)
 * 7. patch pEntry to use new schema
 */
DWORD
VdirSchemaInstanceInitViaEntry(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_INSTANCE*  ppSchema
    )
{
    DWORD    dwError = 0;

    PVDIR_SCHEMA_INSTANCE pSchema = NULL;

    dwError = _VmDirSchemaEntryToInstance( pEntry,
                                     &pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLoadInstance(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSchemaAttrToIdMapInit(pSchema, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (! VmDirSchemaVerifyIntegrity(pSchema))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pSchema->usRefCount++;
    dwError = VdirSchemaADCompatibleSetup(pSchema);
    pSchema->usRefCount--;
    BAIL_ON_VMDIR_ERROR(dwError);

    // always patch at the very end of this function call
    dwError = _VmDirSchemaInitFixBootstrapEntry(
                    pSchema,
                    pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSchema = pSchema;

cleanup:

    return dwError;

error:

    //TODO, need to unpatch entry?

    if (pSchema)
    {
        VdirSchemaInstanceFree(pSchema);
    }

    *ppSchema = NULL;

    goto cleanup;
}

DWORD
VdirSchemaInstanceAllocate(
    PVDIR_SCHEMA_INSTANCE* ppSchema,
    USHORT  dwATSize,
    USHORT  dwOCSize,
    USHORT  dwContentSize,
    USHORT  dwStructureSize,
    USHORT  dwNameformSize
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_INSTANCE pSchema = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_INSTANCE),
            (PVOID*)&pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_OC_DESC) * dwOCSize,
            (PVOID*)&pSchema->ocs.pOCSortName);
    BAIL_ON_VMDIR_ERROR(dwError);
    pSchema->ocs.usNumOCs = dwOCSize;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_AT_DESC) * dwATSize,
            (PVOID*)&pSchema->ats.pATSortName);
    BAIL_ON_VMDIR_ERROR(dwError);
    pSchema->ats.usNumATs = dwATSize;

    if (dwContentSize > 0)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_SCHEMA_CR_DESC) * dwContentSize,
                (PVOID*)&pSchema->contentRules.pContentSortName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pSchema->contentRules.usNumContents = dwContentSize;

    if (dwStructureSize > 0)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_SCHEMA_SR_DESC) * dwStructureSize,
                (PVOID*)&pSchema->structureRules.pStructureSortRuleID);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pSchema->structureRules.usNumStructures = dwStructureSize;

    if (dwNameformSize > 0)
    {
        dwError = VmDirAllocateMemory(  sizeof(VDIR_SCHEMA_NF_DESC) * dwNameformSize,
                                        (PVOID*)&pSchema->nameForms.pNameFormSortName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pSchema->nameForms.usNumNameForms = dwNameformSize;

    dwError = VmDirAllocateMutex(&(pSchema->mutex));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSchema = pSchema;

cleanup:

    return dwError;

error:

    if ( pSchema )
    {
        VdirSchemaInstanceFree( pSchema );
    }

    goto cleanup;
}

VOID
VdirSchemaInstanceFree(
    PVDIR_SCHEMA_INSTANCE pSchema
    )
{
    DWORD dwCnt = 0;

    if ( pSchema )
    {
        for (dwCnt=0; dwCnt < pSchema->ocs.usNumOCs; dwCnt++)
        {
            VmDirSchemaOCDescContentFree(&pSchema->ocs.pOCSortName[dwCnt]);
        }
        VMDIR_SAFE_FREE_MEMORY(pSchema->ocs.pOCSortName);
        pSchema->ocs.usNumOCs = 0;

        for (dwCnt=0; dwCnt < pSchema->ats.usNumATs; dwCnt++)
        {
            VmDirSchemaATDescContentFree(&pSchema->ats.pATSortName[dwCnt]);
        }
        VMDIR_SAFE_FREE_MEMORY(pSchema->ats.pATSortName);
        VMDIR_SAFE_FREE_MEMORY(pSchema->ats.ppATSortIdMap);
        pSchema->ats.usNumATs = 0;

        for (dwCnt=0; dwCnt < pSchema->contentRules.usNumContents; dwCnt++)
        {
            VmDirSchemaContentDescContentFree(&pSchema->contentRules.pContentSortName[dwCnt]);
        }
        VMDIR_SAFE_FREE_MEMORY(pSchema->contentRules.pContentSortName);
        pSchema->contentRules.usNumContents = 0;

        for (dwCnt=0; dwCnt < pSchema->structureRules.usNumStructures; dwCnt++)
        {
            VmDirSchemaStructureDescContentFree(&pSchema->structureRules.pStructureSortRuleID[dwCnt]);
        }
        VMDIR_SAFE_FREE_MEMORY(pSchema->structureRules.pStructureSortRuleID);
        pSchema->structureRules.usNumStructures = 0;

        for (dwCnt=0; dwCnt < pSchema->nameForms.usNumNameForms; dwCnt++)
        {
            VmDirSchemaNameformDescContentFree( &pSchema->nameForms.pNameFormSortName[dwCnt] );
        }
        VMDIR_SAFE_FREE_MEMORY(pSchema->nameForms.pNameFormSortName);
        pSchema->nameForms.usNumNameForms = 0;

        VMDIR_SAFE_FREE_MUTEX( pSchema->mutex );

        VMDIR_SAFE_FREE_MEMORY(pSchema);
    }

    return;
}

/*
 * pEntry is created from a difference instance of schema.
 * Path it to use new instance it now associates with.
 * 1. inpalce menory of pAttr->type
 * 2. pEntry->pSchemaCtx
 */
static
DWORD
_VmDirSchemaInitFixBootstrapEntry(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_ENTRY             pEntry
    )
{
    DWORD               dwError = 0;
    VDIR_ATTRIBUTE*     pAttr = NULL;
    PVDIR_SCHEMA_CTX    pNewCtx = NULL;

    dwError = VmDirAllocateMemory( sizeof(*pNewCtx), (PVOID)&pNewCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    // create the very first ctx for pSchema
    pNewCtx->pSchema = pSchema;
    pNewCtx->pSchema->usRefCount++;

    // switch pEntry to use the new schema instance it creates.
    VmDirSchemaCtxRelease(pEntry->pSchemaCtx);
    pEntry->pSchemaCtx = pNewCtx;
    pNewCtx = NULL;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        USHORT usId = pAttr->pATDesc->usAttrID;
        PVDIR_SCHEMA_AT_DESC pResult = pSchema->ats.ppATSortIdMap[usId - 1];

        if (!pResult)
        {   // this should never happen as attribute ID never change once it is assigned.
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // patch Attribute.pATDesc
        pAttr->pATDesc = pResult;

        // patch Attribute.type in-place storage as well
        pAttr->type.lberbv.bv_val = pResult->pszName;
        pAttr->type.lberbv.bv_len = VmDirStringLenA(pAttr->type.lberbv.bv_val);
    }

cleanup:

    return dwError;

error:

    if ( pNewCtx )
    {
        VmDirSchemaCtxRelease( pNewCtx );
    }

    goto cleanup;
}

static
int
schemaInitPPSTRCmp(
    const void *ppStr1,
    const void *ppStr2
    )
{

    if ((ppStr1 == NULL || *(char * const *)ppStr1 == NULL) &&
        (ppStr2 == NULL || *(char * const *)ppStr2 == NULL))
    {
        return 0;
    }

    if (ppStr1 == NULL || *(char * const *)ppStr1 == NULL)
    {
       return -1;
    }

    if (ppStr2 == NULL || *(char * const *)ppStr2 == NULL)
    {
       return 1;
    }

   return VmDirStringCompareA(* (char * const *) ppStr1, * (char * const *) ppStr2, TRUE);
}

/*
 * Read schema definition from file
 * We ignore -
 * 1. DN/EntryDN
 * 2. ldapSyntax
 * 3. matchingRules
 */
static
DWORD
schemaReadFile(
    PCSTR   pszSchemaFilePath,
    PSTR**  pppszDescs,
    USHORT*  pdwSize
    )
{
    static PSTR pszDNTag = "DN:";
    static PSTR pszENTRYDNTag = "ENTRYDN:";
    static PSTR pszSyntaxTag = "ldapSyntaxes:";
    static PSTR pszMatchingRuleTag = "matchingRules:";
    static PSTR pszName = "NAME ";

    DWORD dwError = 0;
    USHORT dwSize = 0;
    DWORD dwCnt = 100;
    size_t iOldLen = 0 ;
    size_t iNewLen = 0 ;

    PSTR*    ppszDescs = NULL;
    PSTR    pszTmp = NULL;

    char  pbuf[2048] = {0};
    FILE* fp = NULL;


    dwError = VmDirAllocateMemory(
            sizeof(*ppszDescs) * dwCnt,
            (PVOID*)&ppszDescs);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    fp=fopen(pszSchemaFilePath, "r");
#else
    if( fopen_s(&fp, pszSchemaFilePath, "r") != 0 )
    {
        fp = NULL;
    }
#endif
    if(NULL == fp)
    {
        dwError = errno;
        VMDIR_LOG_ERROR(    VMDIR_LOG_MASK_ALL,
                            "Open schema file (%s) failed. Error (%d)",
                            pszSchemaFilePath,
                            dwError);

        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (fgets(pbuf, sizeof(pbuf), fp) != NULL)
    {
        PSTR pszTag = NULL;
        size_t len = VmDirStringLenA(pbuf)-1;

        if (pbuf[len] == '\n')
        {
            pbuf[len] = '\0';
        }

        if ((pbuf[0] == '\0')    ||
            (pbuf[0] == '#')    ||
            (pbuf[0] != ' ' && (pszTag = VmDirStringChrA(pbuf, ':')) == NULL))
        {
            continue;
        }

        if (VmDirStringNCompareA(pbuf, pszDNTag, VmDirStringLenA(pszDNTag), FALSE) == 0              ||
            VmDirStringNCompareA(pbuf, pszENTRYDNTag, VmDirStringLenA(pszENTRYDNTag), FALSE) == 0    ||
            VmDirStringNCompareA(pbuf, pszSyntaxTag, VmDirStringLenA(pszSyntaxTag), FALSE) == 0        ||
            VmDirStringNCompareA(pbuf, pszMatchingRuleTag, VmDirStringLenA(pszMatchingRuleTag), FALSE) == 0)
        {
            continue;
        }

        if (dwCnt == dwSize + 1)
        {
            DWORD    dwOldSize = dwCnt;
            dwCnt = dwCnt * 2;

            dwError = VmDirReallocateMemoryWithInit(
                    ppszDescs,
                    (PVOID*)&ppszDescs,
                    dwCnt * sizeof(*ppszDescs),
                    dwOldSize * sizeof(*ppszDescs));
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pbuf[0] != ' ')
        {
            dwError = VmDirAllocateStringA(pbuf,
                    &(ppszDescs[dwSize++]));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // consolidate leading spaces into one
            char* pszNonSpace = pbuf+1;
            while (*pszNonSpace == ' ') { pszNonSpace++; }
            {

                // Normalising NAME field for everything attributeTypes,objectClasses,ditContentRules , i.e Removing multiple spaces
                if (VmDirStringNCompareA(pszNonSpace, pszName, VmDirStringLenA(pszName), TRUE) == 0)
                 {
                     _VmDirNormaliseNameField (pszNonSpace, VmDirStringLenA(pszNonSpace));
                 }

                iOldLen = VmDirStringLenA(ppszDescs[dwSize-1]);
                iNewLen = VmDirStringLenA(pszNonSpace-1);

                dwError = VmDirReallocateMemoryWithInit(
                        ppszDescs[dwSize-1],
                        (PVOID*)&pszTmp,
                        sizeof(char) * (iOldLen + iNewLen + 1),
                        sizeof(char) * iOldLen);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirCopyMemory(
                    &pszTmp[iOldLen],
                    sizeof(char) * (iOldLen + iNewLen + 1),
                    &(pszNonSpace-1)[0],
                    iNewLen
                );
                BAIL_ON_VMDIR_ERROR(dwError);

                ppszDescs[dwSize-1] = pszTmp;
                pszTmp = NULL;
            }

        }
    }

    *pppszDescs = ppszDescs;
    *pdwSize = dwSize;

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:

    *pppszDescs = NULL;
    *pdwSize = 0;

    if (ppszDescs)
    {
        VmDirFreeStringArrayA(ppszDescs);
        VMDIR_SAFE_FREE_MEMORY(ppszDescs);
    }

    goto cleanup;
}

static
DWORD
schemaInitFillAttrFromFile(
    PVDIR_SCHEMA_CTX    pCtx,
    PSTR*   ppszDescs,
    USHORT  dwDescSize,
    PSTR    pszTag,
    size_t  dwTagSize,
    PVDIR_ENTRY  pEntry
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    pszTag[dwTagSize] = '\0';
    dwError = VmDirAttributeAllocate(
            pszTag,
            dwDescSize,
            pCtx,
            &pAttr);
    pszTag[dwTagSize] = ':';
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < dwDescSize; dwCnt++)
    {
        char* pszStr = &ppszDescs[dwCnt][dwTagSize+1];
        char* pszEndStr = &ppszDescs[dwCnt][VmDirStringLenA(ppszDescs[dwCnt])-1];

        assert(ppszDescs[dwCnt][dwTagSize] == ':');

        //ignore leading spaces
        while (*pszStr == ' ') pszStr++;

        //ignore trailing spaces
        while (*pszEndStr == ' ') pszEndStr--;
        *(pszEndStr+1) = '\0';

        dwError = VmDirAllocateStringA(
                pszStr,
                &pAttr->vals[dwCnt].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttr->vals[dwCnt].bOwnBvVal = TRUE;
        pAttr->vals[dwCnt].lberbv.bv_len = VmDirStringLenA(pAttr->vals[dwCnt].lberbv.bv_val);
    }

    dwError = VmDirEntryAddAttribute(
            pEntry,
            pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);
    pAttr = NULL;

cleanup:

    return dwError;

error:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    goto cleanup;
}

static
DWORD
schemaInitFillAttrFromCache(
    PVDIR_SCHEMA_CTX    pCtx,
    PSTR*    ppszValues,
    USHORT   dwValueSize,
    PSTR     pszAttrName,
    PVDIR_ENTRY   pEntry
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    dwError = VmDirAttributeAllocate(
            pszAttrName,
            dwValueSize,
            pCtx,
            &pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < dwValueSize; dwCnt++)
    {
        char* pszStr = &ppszValues[dwCnt][0];

        //ignore leading spaces
        while (*pszStr == ' ') pszStr++;

        dwError = VmDirAllocateStringA(
                ppszValues[dwCnt],
                &pAttr->vals[dwCnt].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttr->vals[dwCnt].bOwnBvVal = TRUE;
        pAttr->vals[dwCnt].lberbv.bv_len = VmDirStringLenA(pAttr->vals[dwCnt].lberbv.bv_val);
    }

    dwError = VmDirEntryAddAttribute(
            pEntry,
            pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);
    pAttr = NULL;

cleanup:

    return dwError;

error:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    goto cleanup;
}

static
DWORD
_VmDirSchemaAttrReplaceValue(
    PVDIR_ATTRIBUTE pAttr,
    PCSTR           pszMatchSubstr,
    PCSTR           pszValue
    )
{
#define MAX_BUF_SIZE_256    256

    DWORD       dwError = 0;
    unsigned    iCnt = 0;
    CHAR        pszBuf[MAX_BUF_SIZE_256] = {0};

    dwError = VmDirStringPrintFA( pszBuf, MAX_BUF_SIZE_256 -1 , "NAME '%s' ", pszMatchSubstr );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < pAttr->numVals; iCnt++)
    {
        if ( VmDirCaselessStrStrA( pAttr->vals[iCnt].lberbv_val, pszBuf ) != NULL )
        {
            if ( VmDirStringCompareA( VDIR_SAFE_STRING(pAttr->vals[iCnt].lberbv_val),
                                      pszValue,
                                      FALSE ) != 0
                                      )
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Merge schema, replace old - %s",
                                VDIR_SAFE_STRING(pAttr->vals[iCnt].lberbv_val));
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Merge schema, replace new - %s", pszValue );
            }

            VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "Merge schema, replace old - %s",
                             VDIR_SAFE_STRING(pAttr->vals[iCnt].lberbv_val));
            VmDirFreeBervalContent( &(pAttr->vals[iCnt]) );

            dwError = VmDirAllocateStringA( pszValue, &(pAttr->vals[iCnt].lberbv_val) );
            BAIL_ON_VMDIR_ERROR(dwError);
            pAttr->vals[iCnt].lberbv_len = VmDirStringLenA( pszValue );
            pAttr->vals[iCnt].bOwnBvVal = TRUE;

            VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "Merge schema, replace new - %s",
                             VDIR_SAFE_STRING(pAttr->vals[iCnt].lberbv_val));

            break;
        }
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
VOID
_VmDirSchemaNormalizeAttrValue(
    PVDIR_ATTRIBUTE pAttr
    )
{
    size_t  idx = 0;

    for (idx=0; idx < pAttr->numVals; idx++)
    {   // remove heading/trailing spaces and compact multiple spaces
        VmdDirSchemaParseNormalizeElement( pAttr->vals[idx].lberbv_val);
    }
}

/*
 * merge current and patch schema
 * 1. if new schema defined in patch, copy them over to current schema
 * 2. if both current and patch schema have schema definition, use value from patch schema
 *
 * The output is merged into pCurrentEntry.
 */
static
DWORD
VmDirSchemaPatchMerge(
    PVDIR_ENTRY pCurrentEntry,
    PVDIR_ENTRY pPatchEntry
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_INSTANCE   pPatchSchema = NULL;
    PVDIR_ATTRIBUTE         pAttr = NULL;
    USHORT                  usCnt = 0;

    dwError = _VmDirSchemaEntryToInstance( pPatchEntry, &pPatchSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr = VmDirEntryFindAttribute( VDIR_ATTRIBUTE_ATTRIBUTE_TYPES, pCurrentEntry );
    assert( pAttr );
    _VmDirSchemaNormalizeAttrValue( pAttr );
    for (usCnt = 0; usCnt < pPatchSchema->ats.usNumATs; usCnt++)
    {
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
        dwError = VmDirSchemaAttrNameToDescriptor( pCurrentEntry->pSchemaCtx,
                                                   pPatchSchema->ats.pATSortName[ usCnt ].pszName,
                                                   &pATDesc);
        if ( dwError == ERROR_NO_SUCH_ATTRIBUTE )
        {
            dwError = VmDirEntryAddSingleValueStrAttribute(
                            pCurrentEntry,
                            VDIR_ATTRIBUTE_ATTRIBUTE_TYPES,
                            pPatchSchema->ats.pATSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Merge schema, add - %s",
                            VDIR_SAFE_STRING(pPatchSchema->ats.pATSortName[ usCnt ].pszDefinition));
        }
        else
        {
            dwError = _VmDirSchemaAttrReplaceValue( pAttr,
                                                   pPatchSchema->ats.pATSortName[ usCnt ].pszName,
                                                   pPatchSchema->ats.pATSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    pAttr = VmDirEntryFindAttribute( VDIR_ATTRIBUTE_OBJECT_CLASSES, pCurrentEntry );
    assert( pAttr );
    _VmDirSchemaNormalizeAttrValue( pAttr );
    for (usCnt = 0; usCnt < pPatchSchema->ocs.usNumOCs; usCnt++)
    {
        PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

        dwError = VmDirSchemaOCNameToDescriptor( pCurrentEntry->pSchemaCtx,
                                                 pPatchSchema->ocs.pOCSortName[ usCnt ].pszName,
                                                 &pOCDesc);
        if ( dwError == ERROR_NO_SUCH_OBJECTCLASS )
        {
            dwError = VmDirEntryAddSingleValueStrAttribute(
                            pCurrentEntry,
                            VDIR_ATTRIBUTE_OBJECT_CLASSES,
                            pPatchSchema->ocs.pOCSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Merge schema, add - %s",
                            VDIR_SAFE_STRING(pPatchSchema->ocs.pOCSortName[ usCnt ].pszDefinition));
        }
        else
        {
            dwError = _VmDirSchemaAttrReplaceValue( pAttr,
                                                   pPatchSchema->ocs.pOCSortName[ usCnt ].pszName,
                                                   pPatchSchema->ocs.pOCSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    pAttr = VmDirEntryFindAttribute( VDIR_ATTRIBUTE_DIT_CONTENTRULES, pCurrentEntry );
    assert ( pAttr );
    _VmDirSchemaNormalizeAttrValue( pAttr );
    for (usCnt = 0; pAttr && usCnt < pPatchSchema->contentRules.usNumContents; usCnt++)
    {
        PVDIR_SCHEMA_CR_DESC    pCRDesc = NULL;

        dwError = VmDirSchemaCRNameToDescriptor( pCurrentEntry->pSchemaCtx,
                                                 pPatchSchema->contentRules.pContentSortName[ usCnt ].pszName,
                                                 &pCRDesc);
        if ( dwError == ERROR_NO_SUCH_DITCONTENTRULES )
        {
            dwError = VmDirEntryAddSingleValueStrAttribute(
                            pCurrentEntry,
                            VDIR_ATTRIBUTE_DIT_CONTENTRULES,
                            pPatchSchema->contentRules.pContentSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Merge schema, add - %s",
                            VDIR_SAFE_STRING(pPatchSchema->contentRules.pContentSortName[ usCnt ].pszDefinition));
        }
        else
        {
            dwError = _VmDirSchemaAttrReplaceValue( pAttr,
                                                   pPatchSchema->contentRules.pContentSortName[ usCnt ].pszName,
                                                   pPatchSchema->contentRules.pContentSortName[ usCnt ].pszDefinition);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    if ( pPatchSchema )
    {
        VdirSchemaInstanceFree(pPatchSchema);
    }

    return dwError;

error:

    goto cleanup;
}

/*
 * convert schema definition string to descriptor
 */
static
DWORD
_VmDirSchemaEntryToInstance(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_INSTANCE*  ppSchema
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;

    PVDIR_SCHEMA_INSTANCE pSchema = NULL;
    VDIR_ATTRIBUTE* pAttr = NULL;
    VDIR_ATTRIBUTE* pATAttr = NULL;
    VDIR_ATTRIBUTE* pOCAttr = NULL;
    VDIR_ATTRIBUTE* pContentRuleAttr = NULL;
    VDIR_ATTRIBUTE* pStructureRuleAttr = NULL;
    VDIR_ATTRIBUTE* pNameformAttr = NULL;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_OBJECT_CLASSES, FALSE) == 0)
        {
            pOCAttr = pAttr;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_ATTRIBUTE_TYPES, FALSE) == 0)
        {
            pATAttr = pAttr;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_DIT_CONTENTRULES, FALSE) == 0)
        {
            pContentRuleAttr = pAttr;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_DIT_STRUCTURERULES, FALSE) == 0)
        {
            pStructureRuleAttr = pAttr;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_NAMEFORMS, FALSE) == 0)
        {
            pNameformAttr = pAttr;
        }
    }

    if (!pATAttr || !pOCAttr)
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdirSchemaInstanceAllocate(   &pSchema,
                                            pATAttr->numVals,
                                            pOCAttr->numVals,
                                            pContentRuleAttr ? pContentRuleAttr->numVals : 0,
                                            pStructureRuleAttr ? pStructureRuleAttr->numVals : 0,
                                            pNameformAttr ? pNameformAttr->numVals : 0 );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < pATAttr->numVals; dwCnt++)
    {
        dwError = VmDirSchemaParseStrToATDesc(  pATAttr->vals[dwCnt].lberbv.bv_val,
                                                pSchema->ats.pATSortName+dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (dwCnt=0; dwCnt < pOCAttr->numVals; dwCnt++)
    {
        dwError = VmDirSchemaParseStrToOCDesc(  pOCAttr->vals[dwCnt].lberbv.bv_val,
                                                pSchema->ocs.pOCSortName+dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pContentRuleAttr)
    {
        for (dwCnt=0; dwCnt < pContentRuleAttr->numVals; dwCnt++)
        {
            dwError = VmDirSchemaParseStrToContentDesc( pContentRuleAttr->vals[dwCnt].lberbv.bv_val,
                                                        pSchema->contentRules.pContentSortName+dwCnt);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (pStructureRuleAttr)
    {
        for (dwCnt=0; dwCnt < pStructureRuleAttr->numVals; dwCnt++)
        {
            dwError = VmDirSchemaParseStrToStructureDesc(   pStructureRuleAttr->vals[dwCnt].lberbv.bv_val,
                                                            pSchema->structureRules.pStructureSortRuleID+dwCnt);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (pNameformAttr)
    {
        for (dwCnt=0; dwCnt < pNameformAttr->numVals; dwCnt++)
        {
            dwError = VmDirSchemaParseStrToNameformDesc(    pNameformAttr->vals[dwCnt].lberbv.bv_val,
                                                            pSchema->nameForms.pNameFormSortName + dwCnt);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppSchema = pSchema;

cleanup:

    return dwError;

error:

    if (pSchema)
    {
        VdirSchemaInstanceFree(pSchema);
    }

    *ppSchema = NULL;

    goto cleanup;
}

static
DWORD
_VmDirNormaliseNameField(
    PSTR    pszSource ,
    size_t      dwLen
    )
{

    DWORD    i        = 0;
    DWORD    j        = 0;
    DWORD    dwError  = 0;
    DWORD    dwState  = 0;

    for (i=0 ; i<dwLen ;i++ )
    {
        if (pszSource[i] == ' ')
        {
            if (dwState == 0 )
            {
                pszSource[j++] = pszSource[i];
                dwState = 1;
            }
            else
            {
               // Do Nothing , Ignore Extra Spaces ..
            }
        }
        else
        {
            pszSource[j++] = pszSource[i];
            dwState = 0;
        }
    }

    pszSource[j++] = '\0' ;
    VMDIR_LOG_DEBUG( VMDIR_LOG_MASK_ALL, "Formatted NAME string is  - %s",
            VDIR_SAFE_STRING(pszSource) );

    return dwError ;

}
