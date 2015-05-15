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
 * Filename: attr2idmap.c
 *
 * Abstract: handle attribute to id mapping
 *
 *
 */
#include "includes.h"

static
DWORD
schemaAttributeIdMapToStr(
    PVDIR_SCHEMA_INSTANCE    pSchema,
    PSTR*                    ppszStr
    );

static
VOID
schemaAttributeToIdCarryAndAutoGen(
    PVDIR_SCHEMA_INSTANCE    pCurrentSchema,
    PVDIR_SCHEMA_INSTANCE    pSchema
    );

static
DWORD
schemaLoadIdMapFromAttrStr(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszAttrStr
    );

static
DWORD
schemaAttrIdMapModify(
    PVDIR_ENTRY  pEntry,
    PCSTR   pszStr
    );

static
DWORD
schemaAttrIdMapAdd(
    PVDIR_ENTRY  pEntry,
    PCSTR   pszStr
    );

/*
 *
 * Handle Attribute internal id value assignment.
 * ---> set pSchema->ats.pATSortName[].usIdMap
 * ---> set pSchema->ats.ppATSortIdMap
 ***************************************************************
 *  IMPORTANT: we never delete existing attributes
 ***************************************************************
 * Here are cases we handle -
 * 1. bootstarp schema - no op
 * 2. load schema from file (pIdMapAttr is NULL)
 * 3. load schema from storage
 * 4. load schema from ldapmodify schema entry to add AT/OC
 *
 * The format of attributeToIdMap is -
 * "id=attrName:id=attrName:....:id=attrName:nextId"
 */
DWORD
VdirSchemaAttrToIdMapInit(
    PVDIR_SCHEMA_INSTANCE    pSchema,
    PVDIR_ENTRY              pEntry
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;
    PSTR     pszIdMapAttr = NULL;
    PVDIR_ATTRIBUTE     pIdMapAttr = NULL;

    PVDIR_SCHEMA_INSTANCE pCurrentSchema = gVdirSchemaGlobals.pSchema;

    if (pSchema->bIsBootStrapSchema)
    {
        return 0; // No op for bootstrap schema
    }

    for (pIdMapAttr = pEntry->attrs; pIdMapAttr; pIdMapAttr = pIdMapAttr->next)
    {
        if (VmDirStringCompareA(VDIR_ATTRIBUTE_ATTRIBUTETOIDMAP, pIdMapAttr->type.lberbv.bv_val, FALSE) == 0)
        {
            break;
        }
    }

    if (!pIdMapAttr)
    {    // Load from file for the first time, carry and auto gen
        if (!pCurrentSchema->bIsBootStrapSchema)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        schemaAttributeToIdCarryAndAutoGen(pCurrentSchema, pSchema);
    }
    else
    {
        if (pCurrentSchema->bIsBootStrapSchema)
        {   // startup case
            dwError = schemaLoadIdMapFromAttrStr(
                    pSchema,
                    pIdMapAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {   // schema modify case
            schemaAttributeToIdCarryAndAutoGen(pCurrentSchema, pSchema);
        }

    }

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_AT_DESC) * pSchema->ats.usNextId,
            (PVOID*)&pSchema->ats.ppATSortIdMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < pSchema->ats.usNumATs; dwCnt++)
    {
        if (pSchema->ats.pATSortName[dwCnt].usAttrID == 0)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pSchema->ats.ppATSortIdMap[pSchema->ats.pATSortName[dwCnt].usAttrID - 1] =
                &pSchema->ats.pATSortName[dwCnt];
    }

    // first time server startup, create idmap.
    if (!pIdMapAttr && pCurrentSchema->bIsBootStrapSchema)
    {
        dwError = schemaAttributeIdMapToStr(pSchema, &pszIdMapAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = schemaAttrIdMapAdd(pEntry, pszIdMapAttr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // schema entry modification, update idmap
    if (pIdMapAttr && !pCurrentSchema->bIsBootStrapSchema)
    {
        dwError = schemaAttributeIdMapToStr(pSchema, &pszIdMapAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = schemaAttrIdMapModify(pEntry, pszIdMapAttr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszIdMapAttr);

    return dwError;

error:

    goto cleanup;
}

/*
 * Create attributeToIdMap attribute to be stored in schema entry.
 * Format - "id=name:id=name:id=name:....:id=name;nextid"
 */
static
DWORD
schemaAttributeIdMapToStr(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    USHORT  usPadSize = 7; // (max 65,535) pad 5 digits + ':' + '='
    size_t  usLen = 0;
    size_t  bufLen = 0;
    size_t  strLen = 0;

    PSTR    pszBuf = NULL;

    assert(pSchema && ppszStr);

    for (dwCnt = 0, usLen = 0; dwCnt < pSchema->ats.usNumATs; dwCnt++)
    {
        usLen = usLen + VmDirStringLenA(pSchema->ats.pATSortName[dwCnt].pszName) + usPadSize;
    }

    bufLen = sizeof(char) * usLen;

    dwError = VmDirAllocateMemory(
                bufLen,
                (PVOID*)&pszBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < pSchema->ats.usNextId; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC pDesc = pSchema->ats.ppATSortIdMap[dwCnt];
        if (pDesc)
        {
            strLen = VmDirStringLenA(pszBuf);
            dwError = VmDirStringPrintFA(
                pszBuf + strLen,
                bufLen - strLen,
                "%d=%s:",
                pDesc->usAttrID,
                pDesc->pszName
            );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    strLen = VmDirStringLenA(pszBuf);
    dwError = VmDirStringPrintFA(
        pszBuf + strLen, bufLen - strLen, "%d", pSchema->ats.usNextId);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszBuf;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszBuf);

    goto cleanup;
}

/*
 * Modify schema entry, update vmwAttributeToIdMap contents.
 */
static
DWORD
schemaAttrIdMapModify(
    PVDIR_ENTRY  pEntry,
    PCSTR   pszStr
    )
{
    DWORD       dwError = 0;
    PVDIR_ATTRIBUTE  pAttr = NULL;

    assert(pEntry);

    // unpack entry, so we can manipulate its contents.
    // TODO, after having berval.bOwnBvVal, may not need to unpack..
    dwError = VmDirEntryUnpack(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, VDIR_ATTRIBUTE_ATTRIBUTETOIDMAP, FALSE) == 0)
        {
            break;
        }
    }

    assert(pAttr && pAttr->numVals == 1);

    if (VmDirStringCompareA(pAttr->vals[0].lberbv.bv_val, pszStr, TRUE) != 0)
    {
        VmDirFreeBervalArrayContent(pAttr->vals, 1);

        dwError = VmDirAllocateStringA(
                pszStr,
                &pAttr->vals[0].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttr->vals[0].bOwnBvVal = TRUE;
        pAttr->vals[0].lberbv.bv_len = VmDirStringLenA(pAttr->vals[0].lberbv.bv_val);

        dwError = VmDirSchemaBervalNormalize(
                pEntry->pSchemaCtx,
                pAttr->pATDesc,
                &pAttr->vals[0]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
schemaAttrIdMapAdd(
    PVDIR_ENTRY  pEntry,
    PCSTR   pszStr
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE          pNewAttr = NULL;
    PVDIR_SCHEMA_CTX    pCtx = NULL;


    // use gVdirSchemaGlobals.pCtx ( it is safe because we are either in start up or
    // in schema modify, where is serialized by middle layer.)
    pCtx = gVdirSchemaGlobals.pCtx;

    // NOTE, we are using current context to build attribute into pEntry.
    // we will patch pEntry to sync with its schema as the last step in
    // VdirSchemaInstanceInitViaEntry.  So, at the end, pEntry should hold NO
    // reference to pCtx here.
    dwError = VmDirAttributeAllocate(
                VDIR_ATTRIBUTE_ATTRIBUTETOIDMAP,
                1,
                pCtx,
                &pNewAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            pszStr,
            &pNewAttr->vals[0].lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNewAttr->vals[0].bOwnBvVal = TRUE;
    pNewAttr->vals[0].lberbv.bv_len = VmDirStringLenA(pNewAttr->vals[0].lberbv.bv_val);

    dwError = VmDirEntryAddAttribute(
                pEntry,
                pNewAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (pNewAttr)
    {
        VmDirFreeAttribute(pNewAttr);
    }

    goto cleanup;
}

/*
 * Set VDIR_SCHEMA_AT_DESC[].usIdMap and pSchema->ats.usNextId
 *
 * 1. Carry forward bootstrap schema id
 * 2. Generate new id for additional attributes
 */
static
VOID
schemaAttributeToIdCarryAndAutoGen(
    PVDIR_SCHEMA_INSTANCE    pCurrentSchema,
    PVDIR_SCHEMA_INSTANCE    pSchema
    )
{
    DWORD dwCnt = 0;

    pSchema->ats.usNextId = pCurrentSchema->ats.usNextId;

    for (dwCnt = 0; dwCnt < pSchema->ats.usNumATs; dwCnt++)
    {
        PVDIR_SCHEMA_AT_DESC    pResult = NULL;
        VDIR_SCHEMA_AT_DESC     key = {0};

        key.pszName = pSchema->ats.pATSortName[dwCnt].pszName;

        pResult = (PVDIR_SCHEMA_AT_DESC) bsearch(
                &key,
                pCurrentSchema->ats.pATSortName,
                pCurrentSchema->ats.usNumATs,
                sizeof(VDIR_SCHEMA_AT_DESC),
                VdirSchemaPATNameCmp);

        if (pResult)
        {
            pSchema->ats.pATSortName[dwCnt].usAttrID = pResult->usAttrID;
        }
        else
        {
            pSchema->ats.pATSortName[dwCnt].usAttrID = pSchema->ats.usNextId++;
        }
    }

    return;
}

static
DWORD
schemaLoadIdMapFromAttrStr(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PCSTR                   pszAttrStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszBuf = NULL;
    PSTR    pToken = NULL;
    PSTR    pRest = NULL;
    BOOLEAN bLastToken = FALSE;
    int64_t id64 = 0;

    dwError = VmDirAllocateStringA(pszAttrStr, &pszBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pToken = VmDirStringTokA(pszBuf, SCHEMA_ATTRIBUTE_ID_MAP_TOKEN_SEP, &pRest);
         pToken && !bLastToken;
         pToken = VmDirStringTokA(NULL, SCHEMA_ATTRIBUTE_ID_MAP_TOKEN_SEP, &pRest))
    {
        PSTR    pChar = NULL;
        PSTR    pEnd = NULL;
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
        VDIR_SCHEMA_AT_DESC     key = {0};

        pChar = VmDirStringChrA(pToken, SCHEMA_ATTRIBUTE_ID_MAP_SEP_CHAR);
        if (NULL == pChar)
        {
            bLastToken = TRUE;  // no '=', must be the nextid
            id64 = VmDirStringToLA(pToken, &pEnd, 10);

            if( id64 < 0 || id64 > USHRT_MAX )
            {
                dwError = ERROR_INVALID_SCHEMA;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            pSchema->ats.usNextId = (USHORT)id64;

            if (*pEnd != '\0')
            {
                dwError = ERROR_INVALID_SCHEMA;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            continue;
        }

        key.pszName = pChar+1;

        pATDesc = (PVDIR_SCHEMA_AT_DESC) bsearch(
                &key,
                pSchema->ats.pATSortName,
                pSchema->ats.usNumATs,
                sizeof(VDIR_SCHEMA_AT_DESC),
                VdirSchemaPATNameCmp);

        if (!pATDesc)
        {
            // TODO, Should bail if we do not allow attribute deletion?
            assert(0);
            continue;
        }

        *pChar = '\0';

        id64 = VmDirStringToLA(pToken, &pEnd, 10);

        if( id64 < 0 || id64 > USHRT_MAX )
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pATDesc->usAttrID = (USHORT)id64;

        if (*pEnd != '\0')
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if ((pToken && bLastToken) || pSchema->ats.usNextId == 0)
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszBuf);

    return dwError;

error:

    goto cleanup;
}
