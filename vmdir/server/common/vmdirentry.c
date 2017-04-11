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



#include "includes.h"

static
DWORD
AttributeAppendBervArray(
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_BERVALUE     pBervs,
    USHORT      usBervSize
    );

static
DWORD
AttrListToEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR                pszDN,
    PSTR*               ppszAttrList,
    PVDIR_ENTRY         pEntry
    );


DWORD
VmDirInitializeEntry(
   VDIR_ENTRY *               e,
   VDIR_ENTRY_ALLOCATION_TYPE allocType,
   int                        nAttrs,
   int                        nVals
   )
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pLocalAttrs = NULL;
    PVDIR_BERVALUE  pLocalBervs = NULL;
    VDIR_BERVALUE   localDNBerv = VDIR_BERVALUE_INIT;

    VmDirLog( LDAP_DEBUG_TRACE, "InitializeEntry: Begin, allocType = %d", allocType );

    e->allocType = allocType;
    e->dn = localDNBerv;
    dwError = VmDirAllocateMemory( nAttrs * sizeof(VDIR_ATTRIBUTE), (PVOID *)&pLocalAttrs);
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirAllocateMemory( nVals * sizeof( VDIR_BERVALUE ), (PVOID *)&pLocalBervs );
    BAIL_ON_VMDIR_ERROR( dwError );

    e->pAclCtx = NULL;
    e->pszGuid = NULL;

    e->attrs = pLocalAttrs;
    e->savedAttrsPtr = e->attrs;

    e->bvs = pLocalBervs;
    e->usNumBVs = nVals;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "InitializeEntry: End" );
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pLocalAttrs);
    VMDIR_SAFE_FREE_MEMORY(pLocalBervs);
    goto cleanup;
}

void
VmDirFreeEntryContent(
   VDIR_ENTRY * e
   )
{
   VmDirLog( LDAP_DEBUG_TRACE, "DeleteEntry: Begin" );

   if ( e )
   {
      VmDirLog( LDAP_DEBUG_TRACE, "DeleteEntry: DN = %s",
              e ? (e->dn.lberbv.bv_val ? e->dn.lberbv.bv_val : "") : "" );

      if (e->pParentEntry)
      {
          VmDirFreeEntryContent(e->pParentEntry);
          VMDIR_SAFE_FREE_MEMORY(e->pParentEntry);
      }

      VmDirFreeBervalContent( &(e->dn) );
      VmDirFreeBervalContent( &(e->pdn) );
      VmDirFreeBervalContent( &(e->newpdn) );

      if (e->allocType == ENTRY_STORAGE_FORMAT_PACK)
      {
         VMDIR_SAFE_FREE_MEMORY( e->encodedEntry );
         VmDirFreeBervalArrayContent(e->bvs, e->usNumBVs);
         VMDIR_SAFE_FREE_MEMORY( e->bvs );
         VMDIR_SAFE_FREE_MEMORY( e->savedAttrsPtr );
      }
      else if (e->allocType == ENTRY_STORAGE_FORMAT_NORMAL)
      {
         VDIR_ATTRIBUTE * currAttr = NULL;
         VDIR_ATTRIBUTE * tmpAttr = NULL;

         VMDIR_SAFE_FREE_MEMORY( e->encodedEntry );

         for (currAttr = e->attrs; currAttr != NULL; )
         {
            tmpAttr = currAttr->next;
            VmDirFreeAttribute(currAttr);
            currAttr = tmpAttr;
         }
      }

      if (e->pComputedAttrs)
      {
          VDIR_ATTRIBUTE * currAttr = NULL;
          VDIR_ATTRIBUTE * tmpAttr = NULL;

          for (currAttr = e->pComputedAttrs; currAttr != NULL; )
          {
             tmpAttr = currAttr->next;
             VmDirFreeAttribute(currAttr);
             currAttr = tmpAttr;
          }
      }

      VmDirSchemaCtxRelease(e->pSchemaCtx);
      VmDirAclCtxContentFree(e->pAclCtx);
      VMDIR_SAFE_FREE_MEMORY(e->pAclCtx);
      VMDIR_SAFE_FREE_MEMORY(e->pszGuid);

      memset(e, 0, sizeof(*e));
   }

   VmDirLog( LDAP_DEBUG_TRACE, "DeleteEntry: End" );
}

/*
 * Convert entry allocType from FORMAT_PACK to FORMAT_NORMAL
 */
DWORD
VmDirEntryUnpack(
    PVDIR_ENTRY  pEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_ATTRIBUTE      pAttr = NULL;
    PVDIR_ATTRIBUTE      pDupAttr = NULL;
    VDIR_ENTRY           newEntry = {0};

    assert(pEntry);

    if (pEntry->allocType != ENTRY_STORAGE_FORMAT_PACK)
    {
        return 0;
    }

    newEntry.allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    newEntry.eId = pEntry->eId;

    dwError = VmDirBervalContentDup(&pEntry->dn, &newEntry.dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // pdn.lberbv.bv_val is always in-place of dn.lberbv.bv_val (compatible with DeleteEntry)
    if (pEntry->pdn.lberbv.bv_val && pEntry->pdn.lberbv.bv_len > 0)
    {
        newEntry.pdn.lberbv.bv_val = newEntry.dn.lberbv.bv_val + (pEntry->pdn.lberbv.bv_val - pEntry->dn.lberbv.bv_val);
    }

    // pdn.bvnorm_val is ok to heap allocated
    if (pEntry->pdn.bvnorm_val && pEntry->pdn.bvnorm_len > 0)
    {
        dwError = VmDirAllocateStringA(
                pEntry->pdn.bvnorm_val,
                &newEntry.pdn.bvnorm_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        newEntry.pdn.bvnorm_len = VmDirStringLenA(newEntry.pdn.bvnorm_val);
    }

    // copy attribute
    for (pAttr = pEntry->attrs, pDupAttr = NULL; pAttr; pAttr = pAttr->next)
    {
        dwError = VmDirAttributeDup(pAttr, &pDupAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirEntryAddAttribute(&newEntry, pDupAttr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pDupAttr = NULL;
    }

    /////////////////////////////////////////////////////////////////////
    // should never fail from here to the end
    /////////////////////////////////////////////////////////////////////

    // takes over schema ctx (we copy attributes)
    newEntry.pSchemaCtx = pEntry->pSchemaCtx;
    assert(newEntry.pSchemaCtx);
    pEntry->pSchemaCtx = NULL;

// TODO, other fields?
    // takes over pParentEntry
    newEntry.pParentEntry = pEntry->pParentEntry;
    pEntry->pParentEntry = NULL;

    // fee resources
    VmDirFreeEntryContent(pEntry);

    // takes over newEntry content
    VmDirCopyMemory(pEntry, sizeof(VDIR_ENTRY), &newEntry, sizeof(newEntry));

cleanup:

    return dwError;

error:

    if (pDupAttr)
    {
        VmDirFreeAttribute(pDupAttr);
    }

    VmDirFreeEntryContent(&newEntry);

    goto cleanup;
}

void
VmDirFreeEntry(
    PVDIR_ENTRY pEntry
    )
{
    if (pEntry)
    {
        VmDirFreeEntryContent(pEntry);
        VMDIR_SAFE_FREE_MEMORY(pEntry);
    }
}

void
VmDirFreeEntryArrayContent(
    PVDIR_ENTRY_ARRAY   pEntryAry
    )
{
    size_t  iCnt = 0;

    if ( pEntryAry )
    {
        for (iCnt = 0; iCnt < pEntryAry->iSize; iCnt++)
        {
            VmDirFreeEntryContent((pEntryAry->pEntry)+iCnt);
        }

        VMDIR_SAFE_FREE_MEMORY(pEntryAry->pEntry);
        pEntryAry->iSize = 0;
    }
}

void
VmDirFreeEntryArray(
    PVDIR_ENTRY_ARRAY   pEntryAry
    )
{
    if (pEntryAry)
    {
        VmDirFreeEntryArrayContent(pEntryAry);
        VMDIR_SAFE_FREE_MEMORY(pEntryAry);
    }
}

/* ***************************************************
 * if success, pEntry takes ownership of pAttr.
 * ***************************************************
 * if pEntry already has this attribute
 *    append to existing berval
 * else
 *    add new attribute to entry
 */
DWORD
VmDirEntryAddAttribute(
    PVDIR_ENTRY        pEntry,
    PVDIR_ATTRIBUTE    pAttr
    )
{
    DWORD    dwError = 0;
    PVDIR_ATTRIBUTE    pOrgAttr = NULL;

    if (!pEntry                                             ||
        pEntry->allocType != ENTRY_STORAGE_FORMAT_NORMAL    ||
        !pAttr                                              ||
        pAttr->next)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (pOrgAttr = pEntry->attrs; pOrgAttr; pOrgAttr = pOrgAttr->next)
    {
        if (pOrgAttr->pATDesc->usAttrID == pAttr->pATDesc->usAttrID)
        {
            break;
        }
    }

    if (pOrgAttr)
    {
        dwError = AttributeAppendBervArray(pOrgAttr, pAttr->vals, pAttr->numVals);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pAttr);
    }
    else
    {
        pAttr->next = pEntry->attrs;
        pEntry->attrs = pAttr;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Add an array of bervalue attribute values into an entry.
 */
DWORD
VmDirEntryAddBervArrayAttribute(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszAttrName,
    VDIR_BERVARRAY  attrVals,
    USHORT          usNumVals
    )
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    USHORT          usCnt = 0;

    if (!pEntry || !pEntry->pSchemaCtx || !pszAttrName || !attrVals)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (usNumVals)
    {
        dwError = VmDirAttributeAllocate(
                pszAttrName,
                usNumVals,
                pEntry->pSchemaCtx,
                &pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (usCnt=0; usCnt < usNumVals; usCnt++)
        {
            dwError = VmDirBervalContentDup(
                    &attrVals[usCnt], &pAttr->vals[usCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirEntryAddAttribute(pEntry, pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VmDirFreeAttribute(pAttr);
    goto cleanup;
}

/*
 * Convenient function to add a single "string" type attribute to pEntry.
 */
DWORD
VmDirEntryAddSingleValueStrAttribute(
    PVDIR_ENTRY pEntry,
    PCSTR pszAttrName,
    PCSTR pszAttrValue)
{
    DWORD       dwError = 0;

    if (!pEntry || !pEntry->pSchemaCtx || !pszAttrName || !pszAttrValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirEntryAddSingleValueAttribute(
            pEntry,
            pszAttrName,
            pszAttrValue,
            VmDirStringLenA(pszAttrValue));
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

/*
 * Convenient function to add a single attribute to pEntry.
 */
DWORD
VmDirEntryAddSingleValueAttribute(
    PVDIR_ENTRY pEntry,
    PCSTR pszAttrName,
    PCSTR pszAttrValue,
    size_t   iAttrValueLen
    )
{
    DWORD dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!pEntry || !pEntry->pSchemaCtx || !pszAttrName || !pszAttrValue || iAttrValueLen < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAttributeAllocate(
                pszAttrName,
                1,
                pEntry->pSchemaCtx,
                &pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            iAttrValueLen + 1,                  // want string null terminated.
            (PVOID*)&pAttr->vals[0].lberbv.bv_val );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
        pAttr->vals[0].lberbv.bv_val,
        (iAttrValueLen + 1),
        (PCVOID)pszAttrValue,
        iAttrValueLen
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr->vals[0].bOwnBvVal = TRUE;
    pAttr->vals[0].lberbv.bv_len = iAttrValueLen;

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

/*
 * replace attribute in pEntry if such attribute exists.
 * pEntry takes over ownership of pNewAttr if success.
 */
DWORD
VmDirEntryReplaceAttribute(
    PVDIR_ENTRY     pEntry,
    PVDIR_ATTRIBUTE pNewAttr)
{
    DWORD   dwError = 0;
    BOOLEAN bFound = FALSE;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    PVDIR_ATTRIBUTE  pPriorAttr = NULL;

    assert ( pEntry && pEntry->allocType == ENTRY_STORAGE_FORMAT_NORMAL && pNewAttr);

    for ( pAttr = pEntry->attrs;
          pAttr;
          pPriorAttr = pAttr, pAttr = pAttr->next
        )
    {
        if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, pNewAttr->type.lberbv.bv_val, FALSE))
        {
            if ( pEntry->attrs == pAttr )
            {
                pEntry->attrs = pNewAttr;
            }
            else
            {
                pPriorAttr->next = pNewAttr;
            }
            pNewAttr->next = pAttr->next;

            VmDirFreeAttribute(pAttr);
            bFound = TRUE;
            break;
        }
    }

    if ( bFound == FALSE )
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
    }

    return dwError;
}

/*
 * Remove an attribute of an entry.
 * Only handle ENTRY_STORAGE_FORMAT_NORMAL.
 */
DWORD
VmDirEntryRemoveAttribute(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszName
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pAttr = NULL;
    PVDIR_ATTRIBUTE     pPrevAttr = NULL;

    if (!pEntry || !pszName || pEntry->allocType != ENTRY_STORAGE_FORMAT_NORMAL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, pszName, FALSE) == 0)
        {
            break;
        }

        pPrevAttr = pAttr;
    }

    if (pAttr)
    {
        if (pPrevAttr)
        {
            pPrevAttr->next = pAttr->next;
        }
        else
        {
            assert(pAttr == pEntry->attrs);
            pEntry->attrs = pAttr->next;
        }

        VmDirFreeAttribute(pAttr);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

PVDIR_ATTRIBUTE
VmDirEntryFindAttribute(
    PSTR        pszName,
    PVDIR_ENTRY pEntry
    )
{
    PVDIR_ATTRIBUTE  pAttr = NULL;

    assert( pszName && pEntry && pEntry->pSchemaCtx);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pszName, pAttr->type.lberbv.bv_val, FALSE) == 0)
        {
            return pAttr;
        }
    }

    return NULL;
}

/*
 * FROM and TO Attributes use same schema context
 */
DWORD
VmDirAttributeDup(
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_ATTRIBUTE* ppDupAttr
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_ATTRIBUTE    pAttribute = NULL;

    assert(pAttr && ppDupAttr);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ATTRIBUTE),
            (PVOID*)&pAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

    // add one more BerValue as Encode/Decode entry in data store layer needs it.
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (pAttr->numVals + 1),
            (PVOID*)&pAttribute->vals);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0 ; dwCnt < pAttr->numVals; dwCnt++)
    {
        dwError = VmDirBervalContentDup(
                &pAttr->vals[dwCnt],
                &pAttribute->vals[dwCnt]);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttribute->numVals = dwCnt + 1;
    }

    // use the same pATDesc and type from pAttr
    pAttribute->pATDesc = pAttr->pATDesc;
    // type.lberbv.bv_val always store in-place
    pAttribute->type.lberbv.bv_val = pAttr->pATDesc->pszName;
    pAttribute->type.lberbv.bv_len = VmDirStringLenA(pAttr->type.lberbv.bv_val);

    *ppDupAttr = pAttribute;

cleanup:

    return dwError;

error:

    if (pAttribute)
    {
        VmDirFreeAttribute(pAttribute);
    }

    goto cleanup;
}

/*
 * Initialize an attribute structure
 */
DWORD
VmDirAttributeInitialize(
    PSTR    pszName,
    USHORT  usBerSize,
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ATTRIBUTE pAttr
    )
{
    DWORD    dwError = 0;

    if (!pAttr)
    {
        return 0;
    }

    // add one more BerValue as Encode/Decode entry in data store layer needs it.
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (usBerSize + 1),
            (PVOID*)&pAttr->vals);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr->numVals = usBerSize;

    pAttr->pATDesc = VmDirSchemaAttrNameToDesc(
                    pCtx,
                    pszName);
    if (!pAttr->pATDesc)
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // pAttr->type.lberbv.bv_val always store in-place
    pAttr->type.lberbv.bv_val = pAttr->pATDesc->pszName;
    pAttr->type.lberbv.bv_len = VmDirStringLenA(pAttr->type.lberbv.bv_val);

cleanup:

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirAttributeInitialize failed (%d)(%s)",
              dwError, VDIR_SAFE_STRING(pszName));

    goto cleanup;
}


/*
 * Create an Attribute on the heap and establish its pATDesc
 * two memory allocate
 * 1. pAttribute
 * 2. pAttribute->vals (BerValue array is one more then requested)
 */
DWORD
VmDirAttributeAllocate(
    PCSTR   pszName,
    USHORT  usBerSize,
    PVDIR_SCHEMA_CTX pCtx,
    PVDIR_ATTRIBUTE* ppOutAttr)
{
    DWORD    dwError = 0;
    PVDIR_ATTRIBUTE    pAttr = NULL;

    if (!ppOutAttr)
    {
        return 0;
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ATTRIBUTE),
            (PVOID*)&pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // add one more BerValue as Encode/Decode entry in data store layer needs it.
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (usBerSize + 1),
            (PVOID*)&pAttr->vals);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr->numVals = usBerSize;

    pAttr->pATDesc = VmDirSchemaAttrNameToDesc(
                    pCtx,
                    pszName);
    if (!pAttr->pATDesc)
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // pAttr->type.lberbv.bv_val always store in-place
    pAttr->type.lberbv.bv_val = pAttr->pATDesc->pszName;
    pAttr->type.lberbv.bv_len = VmDirStringLenA(pAttr->type.lberbv.bv_val);

    *ppOutAttr = pAttr;

cleanup:

    return dwError;

error:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    VmDirLog( LDAP_DEBUG_ANY, "AllocateAttribute failed (%d)(%s)",
              dwError, VDIR_SAFE_STRING(pszName));

    goto cleanup;
}

/*
 * Free attr-value-meta-data in an DEQUE
 * which is a linked list of PVDIR_BERVALUE
 */
VOID
VmDirFreeAttrValueMetaDataContent(
    PDEQUE  pValueMetaData
    )
{
    PVDIR_BERVALUE pAVmeta = NULL;
    while(!dequeIsEmpty(pValueMetaData))
    {
        dequePopLeft(pValueMetaData, (PVOID*)&pAVmeta);
        VmDirFreeBerval(pAVmeta);
    }
}

/*
 * Free a heap Attribute
 * (ATTENTION: if the pAttr is within a pEntry, only when pEntry is constructed as
 * ENTRY_STORAGE_FORMAT_NORMAL allocation type, its attribute can be freed using this function;
 * otherwise, an entry's attribute free is taken care of by 'pEntry->bvs'
 */
VOID
VmDirFreeAttribute(
    PVDIR_ATTRIBUTE pAttr
    )
{
    if (!pAttr)
    {
        return;
    }

    // pAttr->type is always store in place and has NO bvnorm_val.  no need to free here.
    if (!dequeIsEmpty(&pAttr->valueMetaDataToAdd))
    {
        VmDirFreeAttrValueMetaDataContent(&pAttr->valueMetaDataToAdd);
    }
    if (!dequeIsEmpty(&pAttr->valueMetaDataToDelete))
    {
        VmDirFreeAttrValueMetaDataContent(&pAttr->valueMetaDataToDelete);
    }
    VmDirFreeBervalContent(&pAttr->type);
    VmDirFreeBervalArrayContent(pAttr->vals, pAttr->numVals);
    VMDIR_SAFE_FREE_MEMORY(pAttr->vals);
    VMDIR_SAFE_FREE_MEMORY(pAttr);
}

/*
 * Allocate string into pDupBerval
 */
DWORD
VmDirStringToBervalContent(
    PCSTR              pszBerval,
    PVDIR_BERVALUE     pDupBerval
    )
{
    DWORD    dwError = 0;

    VmDirFreeBervalContent(pDupBerval);

    dwError = VmDirAllocateStringA(pszBerval, &pDupBerval->lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDupBerval->bOwnBvVal = TRUE;

    pDupBerval->lberbv.bv_len = VmDirStringLenA(pDupBerval->lberbv.bv_val);

cleanup:

    return dwError;

error:

    VmDirFreeBervalContent(pDupBerval);

    goto cleanup;
}

/*
 * Duplicate content of pBerval into pDupBerval
 */
DWORD
VmDirBervalContentDup(
    PVDIR_BERVALUE     pBerval,
    PVDIR_BERVALUE     pDupBerval
    )
{
    DWORD    dwError = 0;

    assert(pBerval && pDupBerval);

    VmDirFreeBervalContent(pDupBerval);

    dwError = VmDirAllocateMemory(pBerval->lberbv.bv_len+1, (PVOID*)&pDupBerval->lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);
    if (pBerval->lberbv.bv_len > 0)
    {
        dwError = VmDirCopyMemory(
            pDupBerval->lberbv.bv_val,
            (pBerval->lberbv.bv_len+1),
            pBerval->lberbv.bv_val,
            pBerval->lberbv.bv_len
        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pDupBerval->bOwnBvVal = TRUE;
    pDupBerval->lberbv.bv_len = pBerval->lberbv.bv_len;

    if (pBerval->lberbv.bv_val == pBerval->bvnorm_val)
    {
        pDupBerval->bvnorm_val = pDupBerval->lberbv.bv_val;
        pDupBerval->bvnorm_len = pDupBerval->lberbv.bv_len;
    }
    else if (pBerval->bvnorm_val == NULL)
    {
        pDupBerval->bvnorm_val = NULL;
        pDupBerval->bvnorm_len = 0;
    }
    else
    {
        dwError = VmDirAllocateMemory(pBerval->bvnorm_len+1, (PVOID*)&pDupBerval->bvnorm_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        if (pBerval->bvnorm_len > 0)
        {
            dwError = VmDirCopyMemory(
                pDupBerval->bvnorm_val,
                pBerval->bvnorm_len+1,
                pBerval->bvnorm_val,
                pBerval->bvnorm_len
            );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pDupBerval->bvnorm_len = pBerval->bvnorm_len;
    }

cleanup:

    return dwError;

error:

    VmDirFreeBervalContent(pDupBerval);

    goto cleanup;
}

/*
 * Free a heap BerValue
 */
VOID
VmDirFreeBerval(
    VDIR_BERVALUE* pBerv
    )
{
    VmDirFreeBervalContent(pBerv);
    VMDIR_SAFE_FREE_MEMORY(pBerv);

    return;
}

VOID
VmDirFreeBervalContent(
    VDIR_BERVALUE* pBerv
    )
{
    if (!pBerv)
    {
        return;
    }

    if (pBerv->bvnorm_val != NULL &&
        pBerv->bvnorm_val != pBerv->lberbv.bv_val)
    {
        VMDIR_SAFE_FREE_MEMORY(pBerv->bvnorm_val);
    }

    if (pBerv->bOwnBvVal)
    {
        VMDIR_SAFE_FREE_MEMORY(pBerv->lberbv.bv_val);
    }

    memset(pBerv, 0, sizeof(*pBerv));

    return;
}

DWORD
VmDirCreateTransientSecurityDescriptor(
    BOOL bAllowAnonymousRead,
    PVMDIR_SECURITY_DESCRIPTOR pvsd
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszAdminsGroupSid = NULL;
    PSTR pszDomainAdminsGroupSid = NULL;
    PSTR pszDomainClientsGroupSid = NULL;
    PSTR pszUsersGroupSid = NULL;
    VMDIR_SECURITY_DESCRIPTOR SecDesc = {0};

    pszDomainDN = BERVAL_NORM_VAL(gVmdirServerGlobals.systemDomainDN);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                                        &pszAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ADMINS_RID,
                                        &pszDomainAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_CLIENTS_RID,
                                        &pszDomainClientsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ALIAS_RID_USERS,
                                        &pszUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);


    //
    // Create default security descriptor for internally-created entries.
    //
    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD_BUT_DELETE_OBJECT,
                BERVAL_NORM_VAL(gVmdirServerGlobals.bvDefaultAdminDN),
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                pszUsersGroupSid,
                FALSE,
                bAllowAnonymousRead,
                FALSE,
                FALSE,
                &SecDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pvsd->pSecDesc = SecDesc.pSecDesc;
    pvsd->ulSecDesc = SecDesc.ulSecDesc;
    pvsd->SecInfo = SecDesc.SecInfo;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAdminsGroupSid);
    VMDIR_SAFE_FREE_STRINGA(pszDomainAdminsGroupSid);
    VMDIR_SAFE_FREE_STRINGA(pszDomainClientsGroupSid);
    VMDIR_SAFE_FREE_STRINGA(pszUsersGroupSid);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirAttrListToNewEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR                pszDN,
    PSTR*               ppszAttrList,
    BOOLEAN             bAllowAnonymousRead,
    PVDIR_ENTRY*        ppEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY  pEntry = NULL;
    VMDIR_SECURITY_DESCRIPTOR vsd = {0};

    assert(pSchemaCtx && pszDN && ppszAttrList && ppEntry);

    dwError = VmDirAllocateMemory(
        sizeof(VDIR_ENTRY),
        (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AttrListToEntry(
        pSchemaCtx,
        pszDN,
        ppszAttrList,
        pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateTransientSecurityDescriptor(
                bAllowAnonymousRead,
                &vsd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryCacheSecurityDescriptor(
                pEntry,
                vsd.pSecDesc,
                vsd.ulSecDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(vsd.pSecDesc);
    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

PVDIR_ATTRIBUTE
VmDirFindAttrByName(
    PVDIR_ENTRY      pEntry,
    PSTR        pszName)
{
    PVDIR_ATTRIBUTE  pAttr = NULL;

    assert (pEntry && pszName);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, pszName, FALSE))
        {
            break;
        }
    }

    return pAttr;
}

DWORD
VmDirSimpleEntryCreate(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR*               ppszEntryInitializer,
    PSTR                pszDN,
    ENTRYID             ulEntryId
    )
{
    return VmDirSimpleEntryCreateWithGuid(pSchemaCtx,
                                          ppszEntryInitializer,
                                          pszDN,
                                          ulEntryId,
                                          NULL);
}

DWORD
VmDirSimpleEntryCreateWithGuid(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR*               ppszEntryInitializer,
    PSTR                pszDN,
    ENTRYID             ulEntryId,
    PSTR                pszGuid /* Optional */
    )
{
    DWORD                   dwError = 0;
    VDIR_OPERATION          ldapOp = {0};

    dwError = VmDirInitStackOperation( &ldapOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_ADD,
                                       pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    ldapOp.reqDn.lberbv.bv_val = pszDN;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(pszDN);

    dwError = AttrListToEntry(
            pSchemaCtx,
            pszDN,
            ppszEntryInitializer,
            ldapOp.request.addReq.pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.request.addReq.pEntry->eId = ulEntryId;

    if (!IsNullOrEmptyString(pszGuid))
    {
        dwError = VmDirAllocateStringA(pszGuid, &ldapOp.request.addReq.pEntry->pszGuid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInternalAddEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&ldapOp);

    return dwError;

error:

    goto cleanup;
}

VOID
VmDirFreeBervalArrayContent(
    PVDIR_BERVALUE pBervs,
    USHORT  usSize
    )
{
    USHORT usCnt = 0;

    if (pBervs != NULL)
    {
        for (usCnt = 0; usCnt < usSize; usCnt++)
        {
            if (pBervs[usCnt].bvnorm_val != NULL &&
                pBervs[usCnt].bvnorm_val != pBervs[usCnt].lberbv.bv_val)
            {
                VMDIR_SAFE_FREE_MEMORY(pBervs[usCnt].bvnorm_val);
            }

            if (pBervs[usCnt].bOwnBvVal)
            {
                VMDIR_SAFE_FREE_MEMORY(pBervs[usCnt].lberbv.bv_val);
            }

        }

        memset(pBervs, 0, sizeof(*pBervs) * usSize);
    }
}

BOOLEAN
VmDirIsInternalEntry(
    PVDIR_ENTRY pEntry
    )
{
    return pEntry->eId < ENTRY_ID_SEQ_INITIAL_VALUE;
}

BOOLEAN
VmDirEntryIsObjectclass(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszOCName
    )
{
    BOOLEAN             bResult = FALSE;
    unsigned int        iCnt = 0;

    if (pEntry && pszOCName)
    {
        PVDIR_ATTRIBUTE     pAttrOC = VmDirFindAttrByName(pEntry, ATTR_OBJECT_CLASS);

        for (iCnt = 0; (pAttrOC != NULL) && (iCnt < pAttrOC->numVals); iCnt++)
        {
            if (VmDirStringCompareA( pAttrOC->vals[iCnt].lberbv.bv_val, pszOCName, FALSE) == 0)
            {
                bResult = TRUE;
                break;
            }
        }
    }

    return bResult;
}

DWORD
VmDirEntryIsAttrAllowed(
    PVDIR_ENTRY pEntry,
    PSTR        pszAttrName,
    PBOOLEAN    pbMust,
    PBOOLEAN    pbMay
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_ATTRIBUTE         pAttrOC = NULL;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;
    PLW_HASHMAP pAllMustAttrMap = NULL;
    PLW_HASHMAP pAllMayAttrMap = NULL;

    if (!pEntry || IsNullOrEmptyString(pszAttrName))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlCreateHashMap(&pAllMustAttrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pAllMayAttrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttrOC = VmDirFindAttrByName(pEntry, ATTR_OBJECT_CLASS);

    for (i = 0; pAttrOC && i < pAttrOC->numVals; i++)
    {
        dwError = VmDirSchemaOCNameToDescriptor(
                pEntry->pSchemaCtx, pAttrOC->vals[i].lberbv.bv_val, &pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaClassGetAllMustAttrs(
                pEntry->pSchemaCtx, pOCDesc, pAllMustAttrMap);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaClassGetAllMayAttrs(
                pEntry->pSchemaCtx, pOCDesc, pAllMayAttrMap);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pbMust)
    {
        *pbMust = FALSE;
        if (LwRtlHashMapFindKey(pAllMustAttrMap, NULL, pszAttrName) == 0)
        {
            *pbMust = TRUE;
        }
    }

    if (pbMay)
    {
        *pbMay = FALSE;
        if (LwRtlHashMapFindKey(pAllMayAttrMap, NULL, pszAttrName) == 0)
        {
            *pbMay = TRUE;
        }
    }

cleanup:
    LwRtlHashMapClear(pAllMustAttrMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pAllMustAttrMap);
    LwRtlHashMapClear(pAllMayAttrMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pAllMayAttrMap);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/* *************************************************************
 * if success, pAttr takes ownership of pBervs and its contents
 * *************************************************************
 * Assume pBervs itself is from ONE memory allocate.
 */
static
DWORD
AttributeAppendBervArray(
    PVDIR_ATTRIBUTE  pAttr,
    PVDIR_BERVALUE     pBervs,
    USHORT      usBervSize
    )
{
    DWORD   dwError = 0;
    PVDIR_BERVALUE pNewBerv = NULL;

    if (!pAttr || !pBervs)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // add one more BerValue in size as bdb-store/entry.c needs that.
    dwError = VmDirReallocateMemoryWithInit(
            pAttr->vals,
            (PVOID*) &pNewBerv,
            sizeof(VDIR_BERVALUE) * (pAttr->numVals + usBervSize + 1),
            sizeof(VDIR_BERVALUE) * (pAttr->numVals));
    BAIL_ON_VMDIR_ERROR(dwError);

    // takes over BerValue contents (i.e. lberbv.bv_val and lberbv.bv_len)
    dwError = VmDirCopyMemory(
        pNewBerv + pAttr->numVals,
        sizeof(VDIR_BERVALUE) * (pAttr->numVals + usBervSize + 1),
        pBervs,
        sizeof(VDIR_BERVALUE) * usBervSize
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // free array of BerValue itself
    VMDIR_SAFE_FREE_MEMORY(pBervs);

    pAttr->numVals += usBervSize;
    pAttr->vals = pNewBerv;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pNewBerv);

    goto cleanup;
}

/*
 * Simple (but not efficient) function to convert list of attribute,value
 * into an ADD_REQUEST type entry.
 * (e.g. ppszAttrList = { "objectclass","person",
 *                        "objectclass", "masterdiver",
 *                        "cn","my common name",
 *                         NULL};
 */
static
DWORD
AttrListToEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PSTR                pszDN,
    PSTR*               ppszAttrList,
    PVDIR_ENTRY         pEntry)
{
    DWORD   dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;
    PSTR*    ppszNext = NULL;

    assert(pSchemaCtx && pszDN && ppszAttrList && pEntry);
    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    // establish entry schema context association
    pEntry->pSchemaCtx = VmDirSchemaCtxClone(pSchemaCtx);
    assert(pEntry->pSchemaCtx);

    dwError = VmDirAllocateStringA(
            pszDN,
            &pEntry->dn.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->dn.bOwnBvVal = TRUE;
    pEntry->dn.lberbv.bv_len = VmDirStringLenA(pEntry->dn.lberbv.bv_val);

    dwError = VmDirNormalizeDN( &(pEntry->dn), pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (ppszNext = ppszAttrList; *ppszNext && *(ppszNext+1); ppszNext += 2)
    {
        assert (!pAttr);

        if ( (*(ppszNext+1))[0] != '\0')    // skip attribute with empty value
        {
            dwError = VmDirAttributeAllocate(
                    *ppszNext,
                    1,
                    pSchemaCtx,
                    &pAttr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    *(ppszNext+1),
                    &pAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            pAttr->vals[0].bOwnBvVal = TRUE;
            pAttr->vals[0].lberbv.bv_len = VmDirStringLenA(pAttr->vals[0].lberbv.bv_val);

            dwError = VmDirEntryAddAttribute(
                    pEntry,
                    pAttr);
            BAIL_ON_VMDIR_ERROR(dwError);
            pAttr = NULL; // pEntry takes over pAttr
        }
    }

cleanup:

    return dwError;

error:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    VmDirFreeEntryContent(pEntry);
    memset(pEntry, 0, sizeof(*pEntry));

    goto cleanup;
}

DWORD
VmDirDeleteEntry(
    PVDIR_ENTRY pEntry
    )
{
    DWORD dwError = 0;
    VDIR_OPERATION op = {0};
    DeleteReq *dr = NULL;

    dwError = VmDirInitStackOperation(&op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_DELETE, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    op.pBEIF = VmDirBackendSelect(NULL);
    op.reqDn.lberbv_val = pEntry->dn.lberbv.bv_val;
    op.reqDn.lberbv_len = pEntry->dn.lberbv.bv_len;

    dr = &op.request.deleteReq;
    dr->dn.lberbv.bv_val = op.reqDn.lberbv.bv_val;
    dr->dn.lberbv.bv_len = op.reqDn.lberbv.bv_len;

    dwError = VmDirInternalDeleteEntry(&op);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&op);
    return dwError;
error:
    goto cleanup;
}
