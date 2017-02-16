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


void
VmDirModificationFree(
    PVDIR_MODIFICATION pMod
    )
{
    if (pMod)
    {
        VmDirFreeBervalContent(&pMod->attr.type);
        VmDirFreeBervalArrayContent(pMod->attr.vals, pMod->attr.numVals);
        VMDIR_SAFE_FREE_MEMORY(pMod->attr.vals);
        VmDirFreeAttrValueMetaDataContent(&pMod->attr.valueMetaDataToAdd);
        VmDirFreeAttrValueMetaDataContent(&pMod->attr.valueMetaDataToDelete);
        VMDIR_SAFE_FREE_MEMORY(pMod);
    }
}

/*
 * Add a ModifyRequest to pOperation with-
 * 1. modOp
 * 2. create attribute with values from pBerValue
 *
 * i.e. this function call is equivalent of a modify section in LDIF file
 * changetype: modify
 * add/delete/replace: attributeXYZ  <<<<<<<<<<<<<<<< modOp
 * attributeXYZ: value1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<< pBervalue
 * attributeXYZ: valueN <<<<<<<<<<<<<<<<<<<<<<<<<<<<< iBerValueSize
 */
DWORD
VmDirOperationAddModReq(
    PVDIR_OPERATION   pOperation,
    int               modOp,
    char *            pszAttrName,
    PVDIR_BERVALUE    pBerValue,
    size_t            iBerValueSize
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalErrMsg = NULL;
    VDIR_MODIFICATION * pMod = NULL;
    ModifyReq *         pModReq = &(pOperation->request.modifyReq);
    size_t              iCnt = 0;

    if ( !pOperation || !pszAttrName || !pBerValue )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof( *pMod ), (PVOID *)&(pMod) );
    BAIL_ON_VMDIR_ERROR( dwError );

    pMod->operation = modOp;
    pMod->attr.next = NULL;
    pMod->attr.pATDesc = VmDirSchemaAttrNameToDesc( pOperation->pSchemaCtx, pszAttrName);
    if ( !pMod->attr.pATDesc )
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "attibute (%s) not defined", pszAttrName );
    }

    // attribute.type.lberbv use (schema cache) inplace memory (we don't fee it).
    pMod->attr.type.lberbv.bv_val = pMod->attr.pATDesc->pszName;
    pMod->attr.type.lberbv.bv_len = VmDirStringLenA(pMod->attr.pATDesc->pszName);

    dwError = VmDirAllocateMemory(  sizeof( VDIR_BERVALUE ) * (iBerValueSize + 1 ),
                                    (PVOID *)&(pMod->attr.vals) );
    BAIL_ON_VMDIR_ERROR( dwError );

    for (iCnt = 0; iCnt < iBerValueSize; iCnt++)
    {
        dwError = VmDirAllocateAndCopyMemory( pBerValue[iCnt].lberbv_val,
                                              pBerValue[iCnt].lberbv_len,
                                              (PVOID *)&(pMod->attr.vals[iCnt].lberbv.bv_val));
        BAIL_ON_VMDIR_ERROR( dwError );

        pMod->attr.vals[iCnt].lberbv.bv_len = pBerValue[iCnt].lberbv_len;
        pMod->attr.vals[iCnt].bOwnBvVal = TRUE;
        pMod->attr.numVals++;
    }

    pMod->next = pModReq->mods;
    pModReq->mods = pMod;   // pOpeartion->request.modifyRequest takes over pMod
    pModReq->numMods++;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    if (pMod)
    {
        VmDirModificationFree( pMod );
    }

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg );

    goto cleanup;
}
DWORD
VmDirAppendAMod(
    PVDIR_OPERATION   pOperation,
    int               modOp,
    const char *      attrName,
    int               attrNameLen,
    const char *      attrVal,
    size_t            attrValLen
    )
{
    DWORD          dwError = 0;
    VDIR_MODIFICATION * mod = NULL;
    ModifyReq *    modReq = &(pOperation->request.modifyReq);

    VmDirLog( LDAP_DEBUG_TRACE, "appendAMod: Begin, entry DN = %s", modReq->dn.lberbv.bv_val );

    dwError = VmDirAllocateMemory( sizeof( VDIR_MODIFICATION ), (PVOID *)&(mod) );
    BAIL_ON_VMDIR_ERROR( dwError );

    mod->operation = modOp;

    mod->attr.next = NULL;
    if ((mod->attr.pATDesc = VmDirSchemaAttrNameToDesc( pOperation->pSchemaCtx, attrName)) == NULL)
    {
        VmDirLog( LDAP_DEBUG_ANY, "appendAMod: VmDirSchemaAttrNameToDesc failed.");
        dwError = -1; /* Any value except 0 should do. */
        BAIL_ON_VMDIR_ERROR( dwError );
    }
    mod->attr.type.lberbv.bv_val = mod->attr.pATDesc->pszName;
    mod->attr.type.lberbv.bv_len = VmDirStringLenA(mod->attr.pATDesc->pszName);

    dwError = VmDirAllocateMemory( 2 * sizeof( VDIR_BERVALUE ), (PVOID *)&(mod->attr.vals) );
    BAIL_ON_VMDIR_ERROR( dwError );

    if ( attrVal && attrValLen > 0 )
    {
        dwError = VmDirAllocateMemory( attrValLen + 1, (PVOID *)&(mod->attr.vals[0].lberbv.bv_val) );
        BAIL_ON_VMDIR_ERROR( dwError );

        dwError = VmDirCopyMemory( mod->attr.vals[0].lberbv.bv_val, attrValLen + 1, (const PVOID)attrVal, attrValLen);
        BAIL_ON_VMDIR_ERROR( dwError );

        mod->attr.vals[0].lberbv.bv_len = attrValLen;
        mod->attr.vals[0].bOwnBvVal = TRUE;

        mod->attr.numVals = 1;
    }

    mod->next = modReq->mods;
    modReq->mods = mod;
    modReq->numMods++;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "appendAMod: End." );
    return dwError;

error:
    goto cleanup;
}

/*
 * Convenient function to add a single attribute to pMod.
 *
 * Currently, only handle adding content to EMPTY pMod->attr and use with
 * pMod->operation = MOD_OP_REPLACE or MOD_OP_ADD or MOD_OP_DELETE
 * to manipulate ONE attribute value
 *
 */
DWORD
VmDirModAddSingleValueAttribute(
    PVDIR_MODIFICATION      pMod,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PCSTR                   pszAttrName,
    PCSTR                   pszAttrValue,
    size_t                  iAttrValueLen
    )
{
    DWORD dwError = 0;

    if (!pMod || pMod->attr.pATDesc || pMod->attr.vals || !pSchemaCtx ||
        !pszAttrName || !pszAttrValue || iAttrValueLen < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // add one more BerValue as Encode/Decode entry in data store layer needs it.
    dwError = VmDirAllocateMemory(
                    sizeof(VDIR_BERVALUE) * (1 + 1),
                    (PVOID*)&pMod->attr.vals);
    BAIL_ON_VMDIR_ERROR(dwError);
    pMod->attr.numVals = 1;

    pMod->attr.pATDesc = VmDirSchemaAttrNameToDesc(pSchemaCtx, pszAttrName);
    if (!pMod->attr.pATDesc)
    {
        dwError = ERROR_NO_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // pAttr->type.lberbv.bv_val always store in-place
    pMod->attr.type.lberbv.bv_val = pMod->attr.pATDesc->pszName;
    pMod->attr.type.lberbv.bv_len = VmDirStringLenA(pMod->attr.type.lberbv.bv_val);

    dwError = VmDirAllocateMemory(
                    iAttrValueLen + 1,                  // want string null terminated.
                    (PVOID*)&pMod->attr.vals[0].lberbv.bv_val );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory( pMod->attr.vals[0].lberbv.bv_val, iAttrValueLen + 1, pszAttrValue, iAttrValueLen );
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod->attr.vals[0].bOwnBvVal = TRUE;
    pMod->attr.vals[0].lberbv.bv_len = iAttrValueLen;

cleanup:

    return dwError;

error:

    // pMod owns the memory allocated in this function.

    goto cleanup;
}

/*
 * Convenient function to add a single "string" type attribute to pMod.
 */
DWORD
VmDirModAddSingleStrValueAttribute(
    PVDIR_MODIFICATION      pMod,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PCSTR                   pszAttrName,
    PCSTR                   pszAttrValue
    )
{
    DWORD       dwError = 0;

    dwError = VmDirModAddSingleValueAttribute(
                    pMod,
                    pSchemaCtx,
                    pszAttrName,
                    pszAttrValue,
                    pszAttrValue ? VmDirStringLenA(pszAttrValue) : 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

