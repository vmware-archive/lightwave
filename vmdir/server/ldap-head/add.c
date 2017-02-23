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

/* ParseEntry: Parse the add request on the wire.
 *
 * From RFC 4511:
 *
 *   AddRequest ::= [APPLICATION 8] SEQUENCE {
 *                      entry  LDAPDN,
 *                      attributes   AttributeList }
 *
 *   AttributeList ::= SEQUENCE OF attribute Attribute
 *
 *   Attribute ::= PartialAttribute(WITH COMPONENTS { ..., vals (SIZE(1..MAX))})
 *
 *   PartialAttribute ::= SEQUENCE {
 *                            type AttributeDescription,
 *                            vals SET OF value AttributeValue }
 */

int
VmDirParseBerToEntry(
    BerElement *ber,
    PVDIR_ENTRY pEntry,
    ber_int_t *pErrCode,
    PSTR *ppszErrMsg
    )
{
    int              retVal = LDAP_SUCCESS;
    ber_int_t        errCode = 0;
    VDIR_ENTRY       entry = {0};
    PVDIR_ENTRY      e = &entry;
    char *           endOfAttrsMarker = NULL;
    ber_len_t        len = 0;
    ber_tag_t        tag = LBER_ERROR;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    ber_len_t        size = 0;
    BerValue*        plberBerv = NULL;
    PSTR             pszLocalErrorMsg = NULL;

    // Get entry DN. 'm' => pOperation->reqDn.lberbv.bv_val points to DN within (in-place) ber
    if ( ber_scanf( ber, "{m", &(e->dn.lberbv) ) == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: ber_scanf failed" );
        errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the target DN");
    }

    e->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    // Get entry attributes. ber_first_element => skip the sequence header, set the cursor at the 1st attribute in the
    // SEQ of SEQ
    for ( tag = ber_first_element( ber, &len, &endOfAttrsMarker ); tag != LBER_DEFAULT;
          tag = ber_next_element( ber, &len, endOfAttrsMarker ) )
    {
        if (VmDirAllocateMemory( sizeof(VDIR_ATTRIBUTE), (PVOID *)&pAttr ) != 0)
        {
            errCode = retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        VMDIR_SAFE_FREE_MEMORY(plberBerv);
        // 'm' => bervAttrType points to attribute type within (in-place) ber
        // 'M' => Build array of BerValues by allocating memory for BerValue structures, attribute value strings are
        //        in-place. Last BerValue.bv_val points to NULL.
        size = sizeof( BerValue ); // Size of the structure is passed-in, and number of attribute values are returned
                                   // back in the same parameter.
        if ( ber_scanf( ber, "{m[M]}", &(pAttr->type.lberbv), &plberBerv, &size, (ber_len_t) 0 ) == LBER_ERROR )
        {
            VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: decoding error" );
            errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Decoding error while parsing an attribute");
        }

        if ( plberBerv == NULL || size == 0 )
        {
            VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: no values for type %s", VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val) );
            errCode = retVal = LDAP_PROTOCOL_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Attribute type has no values");
        }
        else if ( size > UINT16_MAX )
        {   // currently, we only support 65535 attribute values due to encode/decode format constraint.
            errCode = retVal = LDAP_PROTOCOL_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Too many %s attribute values, max %u allowed.",
                                            VDIR_SAFE_STRING(pAttr->type.lberbv_val), UINT16_MAX);
        }
        else
        {    // copy pBervArray content into pAttr->vals
            int iCnt = 0;

            if (VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (size+1), (PVOID*)&pAttr->vals) != 0)
            {
                errCode = retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }

            for (iCnt = 0; iCnt < size; iCnt++)
            {
                pAttr->vals[iCnt].lberbv.bv_val = plberBerv[iCnt].bv_val;
                pAttr->vals[iCnt].lberbv.bv_len = plberBerv[iCnt].bv_len;
            }
        }

        // we should be safe to cast...
        pAttr->numVals = (unsigned int)size;
        pAttr->next = e->attrs;
        e->attrs = pAttr;
        pAttr = NULL;
    }

    if ( ber_scanf( ber, "}") == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: ber_scanf failed" );
        errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the end of message.");
    }

    if (pEntry)
    {
        *pEntry = *e;
        memset( e, 0, sizeof( VDIR_ENTRY ));
    }

cleanup:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    VmDirFreeEntryContent(e);

    if (ppszErrMsg)
    {
        *ppszErrMsg = pszLocalErrorMsg;
        pszLocalErrorMsg = NULL;
    }

    if (pErrCode)
    {
        *pErrCode = errCode;
    }

    VMDIR_SAFE_FREE_STRINGA(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(plberBerv);

    return retVal;

error:

    goto cleanup;
}

int
VmDirParseEntry(
    VDIR_OPERATION *     op
    )
{
    char *           endOfAttrsMarker = NULL;
    ber_len_t        len = 0;
    ber_tag_t        tag = LBER_ERROR;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    VDIR_ENTRY *     e = op->request.addReq.pEntry;
    int              retVal = LDAP_SUCCESS;
    ber_len_t        size = 0;
    BerValue*        plberBerv = NULL;
    PSTR             pszLocalErrorMsg = NULL;
    PVDIR_LDAP_RESULT   pResult = &(op->ldapResult);

    // Get entry DN. 'm' => pOperation->reqDn.lberbv.bv_val points to DN within (in-place) ber
    if ( ber_scanf( op->ber, "{m", &(op->reqDn.lberbv) ) == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: ber_scanf failed" );
        pResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the target DN");
    }

    VMDIR_LOG_INFO( LDAP_DEBUG_ARGS, "Add Request: dn (%s)", op->reqDn.lberbv.bv_val );

    memset( e, 0, sizeof( VDIR_ENTRY ));

    // NOTE: bv_val from BER is NULL terminated (TODO, verify this)
    e->dn.lberbv.bv_val     = op->reqDn.lberbv.bv_val;
    e->dn.lberbv.bv_len     = op->reqDn.lberbv.bv_len;

    e->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    // Get entry attributes. ber_first_element => skip the sequence header, set the cursor at the 1st attribute in the
    // SEQ of SEQ
    for ( tag = ber_first_element( op->ber, &len, &endOfAttrsMarker ); tag != LBER_DEFAULT;
          tag = ber_next_element( op->ber, &len, endOfAttrsMarker ) )
    {
        if (VmDirAllocateMemory( sizeof(VDIR_ATTRIBUTE), (PVOID *)&pAttr ) != 0)
        {
            pResult->errCode = retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        VMDIR_SAFE_FREE_MEMORY(plberBerv);
        // 'm' => bervAttrType points to attribute type within (in-place) ber
        // 'M' => Build array of BerValues by allocating memory for BerValue structures, attribute value strings are
        //        in-place. Last BerValue.bv_val points to NULL.
        size = sizeof( BerValue ); // Size of the structure is passed-in, and number of attribute values are returned
                                   // back in the same parameter.
        if ( ber_scanf( op->ber, "{m[M]}", &(pAttr->type.lberbv), &plberBerv, &size, (ber_len_t) 0 ) == LBER_ERROR )
        {
            VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: decoding error" );
            pResult->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Decoding error while parsing an attribute");
        }

        if ( plberBerv == NULL || size == 0 )
        {
            VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: no values for type %s", VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val) );
            pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Attribute type has no values");
        }
        else if ( size > UINT16_MAX )
        {   // currently, we only support 65535 attribute values due to encode/decode format constraint.
            pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Too many %s attribute values, max %u allowed.",
                                            VDIR_SAFE_STRING(pAttr->type.lberbv_val), UINT16_MAX);
        }
        else
        {    // copy pBervArray content into pAttr->vals
            int iCnt = 0;

            if (VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (size+1), (PVOID*)&pAttr->vals) != 0)
            {
                pResult->errCode = retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }

            for (iCnt = 0; iCnt < size; iCnt++)
            {
                pAttr->vals[iCnt].lberbv.bv_val = plberBerv[iCnt].bv_val;
                pAttr->vals[iCnt].lberbv.bv_len = plberBerv[iCnt].bv_len;
            }
        }

        // we should be safe to cast...
        pAttr->numVals = (unsigned int)size;
        pAttr->next = e->attrs;
        e->attrs = pAttr;
        pAttr = NULL;
    }

    /* StrongConsistencyWrite Parse LDAP write controls (if any) in the request. */
    retVal = ParseRequestControls(op, pResult);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                                 "write request control parsing failed");

    if ( ber_scanf( op->ber, "}") == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "ParseEntry: ber_scanf failed" );
        pResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the end of message.");
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    VMDIR_SAFE_FREE_MEMORY(plberBerv);

    return retVal;

error:

    if (pAttr)
    {
        VmDirFreeAttribute(pAttr);
    }

    VMDIR_APPEND_ERROR_MSG(pResult->pszErrMsg, pszLocalErrorMsg);

    goto cleanup;
}

int
VmDirPerformAdd(
   PVDIR_OPERATION pOperation
   )
{
    int retVal = LDAP_SUCCESS;

    retVal = VmDirParseEntry( pOperation );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirMLAdd( pOperation );
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    if (retVal != LDAP_NOTICE_OF_DISCONNECT)
    {
        VmDirSendLdapResult( pOperation );
    }
    return retVal;

error:
    goto cleanup;
}

void
VmDirFreeAddRequest(
   AddReq * pAddReq,
   BOOLEAN  freeSelf
   )
{
   if (pAddReq != NULL && pAddReq->pEntry)
   {
       VmDirFreeEntry( pAddReq->pEntry );
   }
}

/*
 *  use supplied pEntry in LDAP_ADD_REQ operation.
 *  pOp takes over pEntry.
 */
DWORD
VmDirResetAddRequestEntry(
    PVDIR_OPERATION     pOp,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD   dwError = 0;

    if ( !pOp || !pEntry || pOp->reqCode != LDAP_REQ_ADD )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VmDirFreeEntry(pOp->request.addReq.pEntry);
    pOp->request.addReq.pEntry = pEntry;

    dwError = VmDirBervalContentDup(&pEntry->dn, &pOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

