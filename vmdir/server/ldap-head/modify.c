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

/* PerformModify: Parse the modify request on the wire, and call middle-layer Modify functionality.
 *
 * From RFC 4511, section 4.6:
 *
 *   ModifyRequest ::= [APPLICATION 6] SEQUENCE {
 *                          object      LDAPDN,
 *                          changes     SEQUENCE OF change SEQUENCE {
 *                              operation   ENUMERATED {
 *                                              add     (0),
 *                                              delete  (1),
 *                                              replace (2), ...
 *                                          },
 *                              modification    PartialAttribute
 *                          }
 *                     }
 *   PartialAttribute ::= SEQUENCE {
 *                            type AttributeDescription,
 *                            vals SET OF value AttributeValue
 *                        }
 *
 */

int
VmDirPerformModify(
   PVDIR_OPERATION pOperation
   )
{
   char *             endOfModsMarker = NULL;
   ber_len_t          len = 0;
   ber_tag_t          tag = LBER_ERROR;
   PVDIR_MODIFICATION pMod = NULL;
   ModifyReq *        mr = &(pOperation->request.modifyReq);
   int                retVal = LDAP_SUCCESS;
   PVDIR_LDAP_RESULT  pResult = &(pOperation->ldapResult);
   ber_len_t          size = 0;
   BerValue*          pLberBerv = NULL;
   PSTR               pszLocalErrorMsg = NULL;
   DWORD              dwModCount = 0;

   // Get entry DN. 'm' => reqDn.bv_val points to DN within (in-place) ber
   if ( ber_scanf( pOperation->ber, "{m", &(pOperation->reqDn.lberbv) ) == LBER_ERROR )
   {
      VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "VmDirPerformModify: ber_scanf failed" );
      pResult->errCode = LDAP_PROTOCOL_ERROR;
      retVal = LDAP_NOTICE_OF_DISCONNECT;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                      "Decoding error while parsing the target DN");
   }

   memset( mr, 0, sizeof( ModifyReq ));

   // NOTE: bv_val is NULL terminated (TODO, verify this)
   mr->dn.lberbv.bv_val            = pOperation->reqDn.lberbv.bv_val;
   mr->dn.lberbv.bv_len            = pOperation->reqDn.lberbv.bv_len;

   VMDIR_LOG_INFO( LDAP_DEBUG_ARGS, "VmDirPerformModify: dn (%s)", VDIR_SAFE_STRING(pOperation->reqDn.lberbv.bv_val) );

   // Get the list of modifications. ber_first_element => skip the sequence header, set the cursor at the 1st mod in the
   // SEQ of SEQ
   for ( tag = ber_first_element( pOperation->ber, &len, &endOfModsMarker ); tag != LBER_DEFAULT;
         tag = ber_next_element( pOperation->ber, &len, endOfModsMarker ) )
   {
      if (VmDirAllocateMemory( sizeof(VDIR_MODIFICATION), (PVOID *)&pMod ) != 0)
      {
          retVal = pResult->errCode = LDAP_OPERATIONS_ERROR;;
          BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");
      }

      VMDIR_SAFE_FREE_MEMORY(pLberBerv);
      // 'm' => type.bv_val points to attribute type within (in-place) ber
      // 'M' => Build array of BerValues by allocating memory for BerValue structures, attribute value strings are
      //        in-place. Last BerValue.bv_val points to NULL.
      size = sizeof( BerValue ); // Size of the structure is passed-in, and number of attribute values are returned
                                 // back in the same parameter.
      if ( ber_scanf( pOperation->ber, "{i{m[M]}}", &pMod->operation, &(pMod->attr.type.lberbv), &pLberBerv, &size,
                     (ber_len_t) 0 ) == LBER_ERROR )
      {
         VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "VmDirPerformModify: decoding error" );
         pResult->errCode = LDAP_PROTOCOL_ERROR;
         retVal = LDAP_NOTICE_OF_DISCONNECT;
         VmDirModificationFree( pMod );
         pMod = NULL;
         BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Decoding error while parsing an attribute");
      }

      if ( pMod->operation == MOD_OP_ADD && (pLberBerv == NULL || size == 0 ) )
      {
         VMDIR_LOG_VERBOSE( LDAP_DEBUG_ARGS, "VmDirPerformModify: modify operation is ADD, no values for type: %s",
                   pMod->attr.type.lberbv.bv_val );
         pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
         BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "An Add modification has no values");
      }
      else if ( size > UINT16_MAX )
      {   // currently, we only support 65535 attribute values due to encode/decode format constraint.
          pResult->errCode = retVal = LDAP_PROTOCOL_ERROR;
          BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                          "Too many %s attribute values, max %u allowed.",
                                          VDIR_SAFE_STRING(pMod->attr.type.lberbv_val), UINT16_MAX);
      }

      VMDIR_LOG_VERBOSE( LDAP_DEBUG_ARGS, "MOD TYPE: (%d)(%s)", pMod->operation, VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val));

      // copy pBervArray content into pAttr->vals
      if (pLberBerv && size > 0)
      {
          int iCnt = 0;

          if (VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (size+1), (PVOID*)&pMod->attr.vals) != 0)
          {
              retVal = pResult->errCode = LDAP_OPERATIONS_ERROR;
              BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");

          }

          for (iCnt = 0; iCnt < size; iCnt++)
          {
              PCSTR pszLogValue = (0 == VmDirStringCompareA( pMod->attr.type.lberbv.bv_val, ATTR_USER_PASSWORD, FALSE)) ?
                                    "XXX" : pLberBerv[iCnt].bv_val;

              if (iCnt < MAX_NUM_MOD_CONTENT_LOG)
              {
                  VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "MOD %d,%s,%s: (%.*s)",
                            ++dwModCount,
                            VmDirLdapModOpTypeToName(pMod->operation),
                            VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val),
                            VMDIR_MIN(pLberBerv[iCnt].bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                            VDIR_SAFE_STRING( pszLogValue ));
              }
              else if (iCnt == MAX_NUM_MOD_CONTENT_LOG)
              {
                  VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "MOD %d,%s,%s: .... Total MOD %d)",
                            ++dwModCount,
                            VmDirLdapModOpTypeToName(pMod->operation),
                            VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val),
                            size);
              }

              pMod->attr.vals[iCnt].lberbv.bv_val = pLberBerv[iCnt].bv_val;
              pMod->attr.vals[iCnt].lberbv.bv_len = pLberBerv[iCnt].bv_len;
          }
      }
      else
      {   // delete attribute
          VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "MOD %d,%s,%s",
                    ++dwModCount,
                    VmDirLdapModOpTypeToName(pMod->operation),
                    VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val));
      }

      // we should be safe to cast ...
      pMod->attr.numVals = (unsigned int)size;
      pMod->next = mr->mods;
      mr->mods = pMod;
      pMod = NULL;
      mr->numMods++;
   }

    retVal = ParseRequestControls(pOperation, pResult);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                                 "Strong consistency request control parsing failed");

   if ( ber_scanf( pOperation->ber, "}") == LBER_ERROR )
   {
      VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "PerformModify: ber_scanf failed" );
      pResult->errCode = LDAP_PROTOCOL_ERROR;
      retVal = LDAP_NOTICE_OF_DISCONNECT;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Decoding error while parsing the end of message.");
   }

   retVal = pResult->errCode = VmDirMLModify( pOperation );
   BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    if (retVal != LDAP_NOTICE_OF_DISCONNECT)
    {
        VmDirSendLdapResult( pOperation );
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(pLberBerv);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(pResult->pszErrMsg, pszLocalErrorMsg);
    VmDirModificationFree( pMod );
    goto cleanup;
}

void
VmDirFreeModifyRequest(
   ModifyReq * mr,
   BOOLEAN     freeSelf)
{
    if (mr != NULL)
    {
        VDIR_MODIFICATION * currMod = NULL;
        VDIR_MODIFICATION * tmpMod = NULL;

        VmDirFreeBervalContent( &(mr->dn) );
        VmDirFreeBervalContent( &(mr->newrdn) );
        VmDirFreeBervalContent( &(mr->newSuperior) );
        VmDirFreeBervalContent( &(mr->newdn) );

        for (currMod = mr->mods; currMod != NULL; )
        {
            tmpMod = currMod->next;
            VmDirModificationFree(currMod);
            currMod = tmpMod;
        }
        if (freeSelf)
        {
            VMDIR_SAFE_FREE_MEMORY( mr );
        }
    }

    return;
}


