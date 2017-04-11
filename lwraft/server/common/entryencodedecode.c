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



#include "includes.h"

/* Numbers/length in the encoded entry are stored in 2 fixed bytes.
 * Current limitation for the length of strings (DN, and attribute values) in an Entry is 2 bytes (unsigned short),
 * 0xffff = 65,535
 * Also, number of total attribute values and number of total attributes in an entry have same limitation of 65,535.
 */
#define LEN_OF_LEN        (2)

DWORD
VmDirDecodeMods(
   PVDIR_SCHEMA_CTX pSchemaCtx,
   char * encodedmods,
   PVDIR_MODIFICATION *ppmods
   );

DWORD
VmDirComputeEncodedModsSize(
    PVDIR_MODIFICATION pmods,
    int *           nAttrs,
    int *           nVals,
    ber_len_t*      pEncodedEntrySize);

static
DWORD
_VmDirEntrySanityCheck(
    PVDIR_ENTRY     pEntry
    );

/* Compute the total size of all the strings (bv_vals) in an entry using the following scheme:
 *
 * - Length required to store a string is calculated based on the string storage as: <bv_len><bv_val><\0>
 * - Total number of attributes and total number of attribute values (BerValues) involved are also stored.
 * - Total number of Attribute Values is computed as, for each attribute: number of attribute values + an empty
 * (bv_val = NULL) BerValue structure.
 *
 * Also return the number of attributes and total number of attribute values in an entry.
 * */

DWORD
VmDirComputeEncodedEntrySize(
    PVDIR_ENTRY     pEntry,
    int *           nAttrs,
    int *           nVals,
    ber_len_t*      pEncodedEntrySize)
{
    DWORD        dwError = 0;
    ber_len_t    size = 0;
    int          i = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    int          iLocalnAttrs = 0;
    int          iLocalnVals = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "ComputeEncodedEntrySize: Begin, DN: %s", pEntry->dn.lberbv.bv_val );

    if ( pEntry == NULL || nAttrs == NULL || nVals == NULL || pEncodedEntrySize == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *nAttrs = *nVals = 0;

    size += LEN_OF_LEN; // to store number of attributes
    size += LEN_OF_LEN; // to store number of attribute values

   for (pAttr = pEntry->attrs; pAttr != NULL; pAttr = pAttr->next)
   {
        // NOTE: using bv_val != NULL check below causes compiler optimizer issue on windows => this check confuses the
        // "for" loop below
        if (pAttr->vals[0].lberbv.bv_len != 0) // at least one attribute value is present.
        {
            size += LEN_OF_LEN;        // store attribute id map in 2 bytes

            size += LEN_OF_LEN;            // to store number of attribute values
            for (i = 0; pAttr->vals[i].lberbv.bv_val; i++, iLocalnVals++)
            {
                if (pAttr->vals[i].lberbv.bv_len > (UINT16_MAX - 1))
                {
                    dwError = VMDIR_ERROR_INVALID_ENTRY;
                    VMDIR_LOG_ERROR ( VMDIR_LOG_MASK_ALL, "VmDirComputeEncodedEntrySize failed (%u) DN:(%s) attribute maximum length exceeded:(%d > %d)",
                        dwError, pEntry->dn.lberbv.bv_val, pAttr->vals[i].lberbv.bv_len, UINT16_MAX - 1);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
                size += LEN_OF_LEN;                // to store attribute value len
                size += pAttr->vals[i].lberbv.bv_len + 1; // trailing NUL byte
            }
            iLocalnVals++;            // Empty (with bv_val = NULL) BerValue at the end
            iLocalnAttrs++;
        }
    }

    *pEncodedEntrySize = size;
    *nAttrs = iLocalnAttrs;
    *nVals = iLocalnVals;

error:

    VmDirLog( LDAP_DEBUG_TRACE, "ComputeEncodedEntrySize: End, Size: %ld", size );

    return dwError;
}



/* EncodeEntry: Store all strings (DN, attribute types, and attribute values) involved in an Entry in a buffer,
 * to be written as data value in Entry DB.
 * - All strings are preceded by their length (2 bytes are used to store the length).
 * - All strings are explicitly \0 terminated.
 *
 * So, the encoded entry looks like:
    <number of attrs><number of values>
    <Attr1 ID><number of Attr1 values>
    <Len of Attr1 Value1><Attr1 Value 1>\0
    <Len of Attr1 Value2><Attr1 Value 2>\0
    ...
    <Len of Attr1 Valuei><Attr1 Valuen>\0
    <Attr2 ID><number of Attr2 values>
    <Len of Attr2 Value1><Attr2 Value 1>\0
    <Len of Attr2 Value2><Attr2 Value 2>\0
    ...
    <Len of Attr2 Valuei><Attr2 Valuen>\0
    ...
    ...
    ...
    <Attrm ID><number of Attrm values>
    <Len of Attrm Value1><Attrm Value 1>\0
    <Len of Attrm Value2><Attrm Value 2>\0
    ...
    <Len of Attrm Valuei><Attrm Valuen>\0

 * Entry information is extracted from argument 'pEntry', and the encoded entry is returned in 'pEncodedBerval'.
 */
DWORD
VmDirEncodeEntry(
    PVDIR_ENTRY              pEntry,
    VDIR_BERVALUE*           pEncodedBerval)
{
    DWORD            dwError = 0;
    int              i = 0;
    int              nAttrs = 0;
    int              nVals = 0;
    VDIR_ATTRIBUTE *      attr = NULL;
    unsigned char *  writer = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "=> EncodeEntry(%ld) Begin: %s", (long) pEntry->eId, pEntry->dn.lberbv.bv_val );

    dwError = _VmDirEntrySanityCheck(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Calculate required length for all the strings in e
    dwError = VmDirComputeEncodedEntrySize( pEntry, &nAttrs, &nVals, &(pEncodedBerval->lberbv.bv_len) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory( pEncodedBerval->lberbv.bv_len, (PVOID *)&pEncodedBerval->lberbv.bv_val );
    BAIL_ON_VMDIR_ERROR( dwError );
    writer = (unsigned char *)pEncodedBerval->lberbv.bv_val;

    // Put number of attributes and number of attribute values in front, speeds up (e.g. memory allocation) decoding.
    VmDirEncodeShort(&writer, nAttrs);
    VmDirEncodeShort(&writer, nVals);

    // Put Attributes
    for (attr = pEntry->attrs; attr; attr = attr->next)
    {
        // Put id
        if (attr->pATDesc == NULL)
        {
            dwError = ERROR_INVALID_ATTRIBUTETYPES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // Put values
        if (attr->vals)
        {
            for (i=0; attr->vals[i].lberbv.bv_val; i++)
            {
                ;
            }

            if (i > 0)
            {
                VmDirEncodeShort(&writer, attr->pATDesc->usAttrID);
                VmDirEncodeShort(&writer, i);

                for (i=0; attr->vals[i].lberbv.bv_val; i++)
                {
                    VmDirEncodeShort(&writer, attr->vals[i].lberbv.bv_len);

                    // TODO: this needs to provide proper buffer size -
                    // i.e. what's left off of buffer, not whole original size
                    dwError = VmDirCopyMemory(writer, pEncodedBerval->lberbv.bv_len, attr->vals[i].lberbv.bv_val,
                                              attr->vals[i].lberbv.bv_len);
                    BAIL_ON_VMDIR_ERROR( dwError );

                    writer += attr->vals[i].lberbv.bv_len;
                    *writer++ = '\0';
                }
            }
        }
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "<= EncodeEntry End:" );
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY( pEncodedBerval->lberbv.bv_val );
    pEncodedBerval->lberbv.bv_len = 0;
    goto cleanup;
}

/* DecodeEntry: Decodes an Entry that was stored using EncodeEntry. Create the Entry structure from entry->encodedEntry
 * string.
 *
 */
DWORD
VmDirDecodeEntry(
   PVDIR_SCHEMA_CTX     pSchemaCtx,
   PVDIR_ENTRY          pEntry
   )
{
    DWORD               dwError = 0;
    int                 len = 0;
    int                 i = 0;
    int                 j = 0;
    int                 nAttrs = 0;
    int                 nVals = 0;
    VDIR_ATTRIBUTE *    pBeginAttr = NULL;
    VDIR_ATTRIBUTE *    pEndAttr = NULL;
    VDIR_ATTRIBUTE *    attr = NULL;
    unsigned char *     strPtr = NULL;
    VDIR_BERVALUE *     bvPtr = NULL;
    BOOLEAN             bKeepAttr = TRUE;

    if (pEntry == NULL || pSchemaCtx == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry->pSchemaCtx = VmDirSchemaCtxClone(pSchemaCtx);

    strPtr = pEntry->encodedEntry;
    nAttrs = VmDirDecodeShort(&strPtr);
    nVals = VmDirDecodeShort(&strPtr);

    dwError = VmDirInitializeEntry( pEntry, ENTRY_STORAGE_FORMAT_PACK, nAttrs, nVals);
    BAIL_ON_VMDIR_ERROR( dwError );

    bvPtr = (VDIR_BERVALUE *)pEntry->bvs;

   for (i = 0, attr = pEntry->attrs; i < nAttrs; attr = &(pEntry->attrs[++i]))
   {
       unsigned short usIdMap = VmDirDecodeShort(&strPtr);

       bKeepAttr = TRUE;

       // set pATDesc
       attr->pATDesc = VmDirSchemaAttrIdToDesc(pEntry->pSchemaCtx, usIdMap);
       if (attr->pATDesc == NULL)
       {
           if ( pEntry->eId == SUB_SCEHMA_SUB_ENTRY_ID    &&
                VmDirSchemaCtxIsBootStrap(pSchemaCtx) == TRUE   // bootstrap ctx
              )
           {    // bootstrap schema case, ignore attributes not defined in
                // schema/defines.h VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER table
                // while decoding schema entry during InitializeSchema call.
                bKeepAttr = FALSE;
           }
           else
           {
               dwError = VMDIR_ERROR_INVALID_ATTRIBUTETYPES;
               BAIL_ON_VMDIR_ERROR(dwError);
           }
       }

       if ( bKeepAttr )
       {
           // type.bv_val always uses in-place storage
           attr->type.lberbv.bv_val = attr->pATDesc->pszName;
           attr->type.lberbv.bv_len = VmDirStringLenA(attr->type.lberbv.bv_val);
       }

        // Keep consuming strPtr even if bKeepAttr is FALSE
        // Get number of attribute values
        j = VmDirDecodeShort(&strPtr);
        attr->numVals = j;
        attr->vals = bvPtr;

        // Set vals array
        while (j)
        {
            len = VmDirDecodeShort(&strPtr);
            bvPtr->lberbv.bv_len = len;
            bvPtr->lberbv.bv_val = (char *)strPtr;
            strPtr += len + 1; // Skipping \0
            bvPtr++;
            j--;
        }
        // Set the last BerValue in vals array.
        bvPtr->lberbv.bv_val = NULL;
        bvPtr->lberbv.bv_len = 0;
        bvPtr++;

        if ( bKeepAttr )
        {
            if (VmDirStringCompareA( attr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
            {
                pEntry->dn = attr->vals[0];
            }

            if ( pBeginAttr == NULL )
            {
                pBeginAttr = attr;
                pEndAttr   = attr;
            }
            else
            {
                pEndAttr->next = attr;
                pEndAttr = attr;
            }
        }
    }

    if ( pBeginAttr == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // reposition pEntry->attrs to the first kept attr
    pEntry->attrs = pBeginAttr;
    pEndAttr->next = NULL;

    dwError = VmDirNormalizeDN( &(pEntry->dn), pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR ( VMDIR_LOG_MASK_ALL, "DecodeEntry failed (%u) DN:(%s)",
                      dwError, pEntry ? VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val) : "unknown" );

    goto cleanup;
}

void
VmDirEncodeShort(
    unsigned char ** ppbuf,
    ber_len_t        len)
{
    **ppbuf = (unsigned char) (len >> 8); // higher order byte.
    (*ppbuf)++;
    **ppbuf = (unsigned char) len;        // lower order byte.
    (*ppbuf)++;
}

unsigned short
VmDirDecodeShort(
    unsigned char ** ppbuf)
{
    unsigned short len = 0;

    len = **ppbuf;  // higher order byte.
    (*ppbuf)++;
    len <<= 8;
    len |= **ppbuf; // lower order byte.
    (*ppbuf)++;
    return len;
}

/*
 * Make sure entry has minimum required content before writing into database
 * Currently, we check:
 * 1. attribute has its descriptor
 * 2. must has ATTR_DN
 * 3. must has ATTR_OBJECT_CLASS
 */
static
DWORD
_VmDirEntrySanityCheck(
    PVDIR_ENTRY     pEntry
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bHasDN = FALSE;
    BOOLEAN     bHasObjectclass = FALSE;
    PVDIR_ATTRIBUTE pAttr = NULL;

    for (pAttr = pEntry->attrs; pAttr != NULL ; pAttr = pAttr->next)
    {
        if (pAttr->pATDesc == NULL)
        {
            dwError = ERROR_INVALID_ATTRIBUTETYPES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if ( VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0
             &&
             pAttr->numVals == 1
             &&
             pAttr->vals[0].lberbv.bv_val != NULL
           )
        {
            bHasDN = TRUE;
        }
        else if ( VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE) == 0
                  &&
                  pAttr->numVals > 0
                )
        {
            bHasObjectclass = TRUE;
        }
    }

    if ( bHasDN == FALSE
         ||
         bHasObjectclass == FALSE
       )
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirEntrySanityCheck failed DN:(%s)", VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));

    VmDirLogStackFrame(LDAP_DEBUG_ANY);

    goto cleanup;
}

/* EncodeMods:
 * - All strings are preceded by their length (2 bytes are used to store the length).
 * - All strings are explicitly \0 terminated.
 *
 * So, the EncodeModis looks like:
    <number of attrs><number of values>
    <Attr1 ID><number of Attr1 values><mod_type>
    <Len of Attr1 Value1><Attr1 Value 1>\0
    <Len of Attr1 Value2><Attr1 Value 2>\0
    ...
    <Len of Attr1 Valuei><Attr1 Valuen>\0
    <Attr2 ID><number of Attr2 values><mod_type>
    <Len of Attr2 Value1><Attr2 Value 1>\0
    <Len of Attr2 Value2><Attr2 Value 2>\0
    ...
    <Len of Attr2 Valuei><Attr2 Valuen>\0
    ...
    ...
    ...
    <Attrm ID><number of Attrm values><mod_type>
    <Len of Attrm Value1><Attrm Value 1>\0
    <Len of Attrm Value2><Attrm Value 2>\0
    ...
    <Len of Attrm Valuei><Attrm Valuen>\0

 * Mods information is extracted reurned in 'pModsBerval'.
 */
DWORD
VmDirEncodeMods(
    PVDIR_MODIFICATION pmods,
    VDIR_BERVALUE*     pEncodedBerval)
{
    DWORD dwError = 0;
    int i = 0;
    int nAttrs = 0;
    int nVals = 0;
    VDIR_ATTRIBUTE * attr = NULL;
    unsigned char * writer = NULL;
    PVDIR_MODIFICATION pMod = NULL;

    // Calculate required length for all the strings in e
    dwError = VmDirComputeEncodedModsSize(pmods, &nAttrs, &nVals, &(pEncodedBerval->lberbv.bv_len));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory( pEncodedBerval->lberbv.bv_len, (PVOID *)&pEncodedBerval->lberbv.bv_val );
    BAIL_ON_VMDIR_ERROR( dwError );
    pEncodedBerval->bOwnBvVal = TRUE;

    writer = (unsigned char *)pEncodedBerval->lberbv.bv_val;

    // Put number of attributes and number of attribute values in front, speeds up (e.g. memory allocation) decoding.
    VmDirEncodeShort(&writer, nAttrs);
    VmDirEncodeShort(&writer, nVals);

    // Put Attributes
    for (pMod = pmods; pMod; pMod = pMod->next)
    {
        // Put id
        if (pMod->attr.pATDesc== NULL)
        {
            dwError = ERROR_INVALID_ATTRIBUTETYPES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        attr = &pMod->attr;
        // Put values
        if (attr->vals)
        {
            for (i=0; attr->vals[i].lberbv.bv_val; i++)
            {
                ;
            }

            if (i > 0)
            {
                VmDirEncodeShort(&writer, attr->pATDesc->usAttrID);
                VmDirEncodeShort(&writer, i);
                *writer = (unsigned char)pMod->operation;
                writer++;

                for (i=0; attr->vals[i].lberbv.bv_val; i++)
                {
                    VmDirEncodeShort(&writer, attr->vals[i].lberbv.bv_len);

                    // TODO: this needs to provide proper buffer size -
                    // i.e. what's left off of buffer, not whole original size
                    dwError = VmDirCopyMemory(writer, pEncodedBerval->lberbv.bv_len, attr->vals[i].lberbv.bv_val,
                                              attr->vals[i].lberbv.bv_len);
                    BAIL_ON_VMDIR_ERROR( dwError );

                    writer += attr->vals[i].lberbv.bv_len;
                    *writer++ = '\0';
                }
            }
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY( pEncodedBerval->lberbv.bv_val );
    pEncodedBerval->lberbv.bv_len = 0;
    goto cleanup;
}

/* DecodeMods: Decodes an Mods for Raft to replicate LDAP modify
 * string.
 *
 */
DWORD
VmDirDecodeMods(
   PVDIR_SCHEMA_CTX pSchemaCtx,
   char * encodedmods,
   PVDIR_MODIFICATION *ppmods
   )
{
    DWORD               dwError = 0;
    int                 len = 0;
    int                 i = 0;
    int                 j = 0;
    int                 nAttrs = 0;
    int                 nVals = 0;
    VDIR_ATTRIBUTE *    attr = NULL;
    unsigned char *     strPtr = NULL;
    VDIR_BERVALUE *     bvPtr = NULL;
    PVDIR_MODIFICATION  pMods = NULL;
    PVDIR_MODIFICATION  prevMod = NULL;

    if (ppmods == NULL || pSchemaCtx == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    strPtr = encodedmods;
    nAttrs = VmDirDecodeShort(&strPtr);
    nVals = VmDirDecodeShort(&strPtr);

    for (i=0; i<nAttrs; i++)
    {
        unsigned short usIdMap = VmDirDecodeShort(&strPtr);
        PVDIR_MODIFICATION  mod = NULL;

        dwError = VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&mod);
        BAIL_ON_VMDIR_ERROR( dwError );

        if (pMods == NULL)
        {
            pMods = mod;
        }

        if (prevMod)
        {
            prevMod->next = mod;
        }

        dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID *)&attr);
        BAIL_ON_VMDIR_ERROR( dwError );

        // set pATDesc
        attr->pATDesc = VmDirSchemaAttrIdToDesc(pSchemaCtx, usIdMap);

        // type.bv_val always uses in-place storage
        attr->type.lberbv.bv_val = attr->pATDesc->pszName;
        attr->type.lberbv.bv_len = VmDirStringLenA(attr->type.lberbv.bv_val);

        // Get number of attribute values
        j = VmDirDecodeShort(&strPtr);
        attr->numVals = j;
       
        dwError = VmDirAllocateMemory((j +1 )* sizeof( VDIR_BERVALUE ), (PVOID *)&bvPtr);
        BAIL_ON_VMDIR_ERROR( dwError );

        attr->vals = bvPtr;
        mod->operation = *strPtr;
        strPtr++;

        // Set vals array
        while (j)
        {
            VDIR_BERVALUE bv = {0};

            len = VmDirDecodeShort(&strPtr);
            bv.lberbv.bv_len = len;
            bv.lberbv.bv_val = (char *)strPtr;

            dwError = VmDirBervalContentDup(&bv, bvPtr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirSchemaBervalNormalize(pSchemaCtx, attr->pATDesc, bvPtr);
            BAIL_ON_VMDIR_ERROR(dwError);

            strPtr += len + 1; // Skipping \0
            bvPtr++;
            j--;
        }
        // Set the last BerValue in vals array.
        bvPtr->lberbv.bv_val = NULL;
        bvPtr->lberbv.bv_len = 0;
        bvPtr++;
        mod->attr = *attr;
        prevMod = mod;
    }

    *ppmods = pMods;
cleanup:
    return dwError;

error:

    VMDIR_LOG_ERROR ( VMDIR_LOG_MASK_ALL, "DecodeMods failed (%u)", dwError);

    goto cleanup;
}

DWORD
VmDirComputeEncodedModsSize(
    PVDIR_MODIFICATION pmods,
    int *           nAttrs,
    int *           nVals,
    ber_len_t*      pEncodedEntrySize)
{
    DWORD        dwError = 0;
    ber_len_t    size = 0;
    int          i = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    int          iLocalnAttrs = 0;
    int          iLocalnVals = 0;
    PVDIR_MODIFICATION  pMod = NULL;


    if (nAttrs == NULL || nVals == NULL || pEncodedEntrySize == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *nAttrs = *nVals = 0;

    size += LEN_OF_LEN; // to store number of attributes
    size += LEN_OF_LEN; // to store number of attribute values

   for (pMod = pmods; pMod; pMod=pMod->next)
   {
        // NOTE: using bv_val != NULL check below causes compiler optimizer issue on windows => this check confuses the
        // "for" loop below
        pAttr = &pMod->attr;
        if (pAttr->vals[0].lberbv.bv_len != 0) // at least one attribute value is present.
        {
            size += LEN_OF_LEN;        // store attribute id map in 2 bytes

            size += LEN_OF_LEN;            // to store number of attribute values
            size++;                        // to store mod operation
            for (i = 0; pAttr->vals[i].lberbv.bv_val; i++, iLocalnVals++)
            {
                if (pAttr->vals[i].lberbv.bv_len > (UINT16_MAX - 1))
                {
                    dwError = VMDIR_ERROR_INVALID_ENTRY;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
                size += LEN_OF_LEN;                // to store attribute value len
                size += pAttr->vals[i].lberbv.bv_len + 1; // trailing NUL byte
            }
            iLocalnVals++;            // Empty (with bv_val = NULL) BerValue at the end
            iLocalnAttrs++;
        }
    }

    *pEncodedEntrySize = size;
    *nAttrs = iLocalnAttrs;
    *nVals = iLocalnVals;

error:

    VmDirLog( LDAP_DEBUG_TRACE, "ComputeEncodedEntrySize: End, Size: %ld", size );

    return dwError;
}
