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

/* PerformBind: Parse the bind request on the wire, and call middle-layer Bind functionality.
 *
 * From RFC 4511:
 *
 *   BindRequest ::= [APPLICATION 0] SEQUENCE {
 *             version                 INTEGER (1 ..  127),
 *             name                    LDAPDN,
 *             authentication          AuthenticationChoice }
 *
 *  AuthenticationChoice ::= CHOICE {
 *             simple                  [0] OCTET STRING, -- 1 and 2 reserved
 *             sasl                    [3] SaslCredentials,
 *             ...  }
 *
 *  SaslCredentials ::= SEQUENCE {
 *             mechanism               LDAPString,
 *             credentials             OCTET STRING OPTIONAL }
 */

int
VmDirPerformBind(
    PVDIR_OPERATION   pOperation
    )
{
   BindReq *    pBindReq = &(pOperation->request.bindReq);
   int          retVal = LDAP_SUCCESS;
   ber_len_t    berLen = 0;
   ber_tag_t    berTag = 0;
   BOOLEAN      bResultAlreadySent = FALSE;

   memset( pBindReq, 0, sizeof( BindReq ));

   if (ber_scanf( pOperation->ber, "{imt", &pOperation->protocol, &(pOperation->reqDn.lberbv), &pBindReq->method ) == LBER_ERROR )
   {
      VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "PerformBind: ber_scanf failed" );

      BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_PROTOCOL_ERROR, (pOperation->ldapResult.pszErrMsg),
                                 "Decoding error while parsing protocol version, bind DN, bind method.");
   }

   VMDIR_LOG_VERBOSE( LDAP_DEBUG_ARGS,
       "Bind Request (%s): Protocol version: %d, Bind DN: \"%s\", Method: %ld",
        pOperation->conn->szClientIP,
        pOperation->protocol,
        pOperation->reqDn.lberbv.bv_val, pBindReq->method );

   if (pOperation->protocol != LDAP_VERSION3)
   {
      VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "PerformBind: Non-ldap-v3 version." );
      BAIL_ON_STATIC_LDAP_ERROR(    retVal, LDAP_PROTOCOL_ERROR, (pOperation->ldapResult.pszErrMsg),
                                  "Currently, only LDAP V3 is supported.");
   }

   switch (pBindReq->method)
   {
       case LDAP_AUTH_SIMPLE:
               if ( ber_scanf( pOperation->ber, "m}", &(pBindReq->cred.lberbv) ) == LBER_ERROR )
               {
                   VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "PerformBind: ber_scanf failed" );
                  BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_PROTOCOL_ERROR, (pOperation->ldapResult.pszErrMsg),
                                             "Decoding error while parsing simple credentials.");
               }

               break;

       case LDAP_AUTH_SASL:
               if ( ber_scanf( pOperation->ber, "{m" /*}*/, &(pBindReq->bvMechanism.lberbv) ) == LBER_ERROR)
               {
                   BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_PROTOCOL_ERROR, (pOperation->ldapResult.pszErrMsg),
                                              "Decoding error while parsing sasl mechanism.");
               }

               berTag = ber_peek_tag( pOperation->ber, &berLen );
               if ( berTag == LDAP_TAG_LDAPCRED ) {
                   if ( ber_scanf( pOperation->ber, "m", &(pBindReq->cred.lberbv)) == LBER_ERROR )
                   {
                       BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_PROTOCOL_ERROR,  (pOperation->ldapResult.pszErrMsg),
                                                  "Decoding error while parsing sasl creds.");
                   }
               }

               if ( ber_scanf( pOperation->ber, /*{{*/ "}}" ) == LBER_ERROR )
               {
                   BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_PROTOCOL_ERROR, (pOperation->ldapResult.pszErrMsg),
                                              "Decoding error.");
               }

               break;

       default:
           BAIL_ON_STATIC_LDAP_ERROR( retVal, LDAP_UNWILLING_TO_PERFORM,  (pOperation->ldapResult.pszErrMsg),
                                      "bind method not supported.");
   }

    retVal = VmDirMLBind( pOperation );
    bResultAlreadySent = TRUE;
    if (retVal && retVal != LDAP_SASL_BIND_IN_PROGRESS)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "Bind Request Failed (%s) error %u: Protocol version: %d, Bind DN: \"%s\", Method: %ld",
            pOperation->conn->szClientIP,
            retVal,
            pOperation->protocol,
            VDIR_SAFE_STRING(pOperation->reqDn.lberbv.bv_val),
            pBindReq->method );
    }
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);


    VMDIR_LOG_INFO(LDAP_DEBUG_AUTH,
            "Bind Request Succeeded (%s) Bind DN: \"%s\", Method: %ld",
            pOperation->conn->szClientIP,
            VDIR_SAFE_STRING(pOperation->reqDn.lberbv.bv_val),
            pBindReq->method );

cleanup:

    return retVal;

ldaperror:
    pOperation->ldapResult.errCode = retVal;

    if (retVal != LDAP_NOTICE_OF_DISCONNECT && bResultAlreadySent == FALSE)
    {
        VmDirSendLdapResult( pOperation );
    }

    goto cleanup;
}


void
VmDirFreeBindRequest(
    BindReq*     pBindReq,
    BOOLEAN      bFreeSelf)
{
    if (pBindReq != NULL)
    {
        VmDirFreeBervalContent( &(pBindReq->cred) );
        VmDirFreeBervalContent( &(pBindReq->bvMechanism) );

        if (bFreeSelf)
        {
            VMDIR_SAFE_FREE_MEMORY( pBindReq );
        }
    }
}
