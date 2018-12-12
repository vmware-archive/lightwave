/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
_VmDirDecodeTxnReqData(
    VDIR_BERVALUE reqData,
    PBOOLEAN pbCommit,
    PVDIR_BERVALUE ptxnid
    );


/* PerformExtended Parse the extended request on the wire, and call middle-layer associated functionality.
 *
 * From RFC 4511:
 *
 *   extendedRequest ::= [APPLICATION 23]
 *
 */

int
VmDirPerformExtended(
   PVDIR_OPERATION pOperation
   )
{
    int         dwError = LDAP_SUCCESS;
    ExtendedReq *pExReq = &(pOperation->request.extendedReq);
    VDIR_BERVALUE reqData = {0};
    VDIR_BERVALUE txnid = {0};
    ber_len_t size = 0;
    PSTR pszLocalErrorMsg = NULL;
    BOOLEAN bResultSent = FALSE;
    BOOLEAN bCommit = FALSE;

    memset( pExReq, 0, sizeof( ExtendedReq ));

    if (ber_scanf( pOperation->ber, "{m" , &(pExReq->oid.lberbv)) == LBER_ERROR ) {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "PerformExtened: ber_scanf failed" );
        dwError = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if (ber_peek_tag(pOperation->ber, &size) == LDAP_TAG_EXOP_REQ_VALUE)
    {
        if ( ber_scanf(pOperation->ber, "m", &reqData.lberbv ) == LBER_ERROR ) {
            dwError = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
                                         "Decoding error while parsing exteded reqData");
        }
    }

    dwError = ParseRequestControls(pOperation, &pOperation->ldapResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(pExReq->oid.lberbv.bv_val, VMDIR_LDAP_EXOP_TXN_START, FALSE)==0)
    {
        dwError = VmDirMLTxnStart(pOperation, &bResultSent);
        BAIL_ON_VMDIR_ERROR( dwError );
    } else if (VmDirStringCompareA(pExReq->oid.lberbv.bv_val, VMDIR_LDAP_EXOP_TXN_END, FALSE)==0)
    {
        dwError = _VmDirDecodeTxnReqData(reqData, &bCommit, &txnid);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
                                     "error decoding ReqData for exteneded operation txn-end");
        if (bCommit)
        {
            dwError = VmDirMLTxnCommit(pOperation, txnid, &bResultSent);
            BAIL_ON_VMDIR_ERROR( dwError );
        } else
        {
            dwError = VmDirMLTxnAbort(pOperation, txnid, &bResultSent);
            BAIL_ON_VMDIR_ERROR( dwError );
        }
    } else
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
                                     "unsupported extended operation");
    }

cleanup:
    if (!bResultSent)
    {
        VmDirSendLdapResult(pOperation);
    }
    VmDirFreeBervalContent(&txnid);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d", __func__, dwError);
    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult, dwError, pszLocalErrorMsg);
    goto cleanup;
}

void
VmDirFreeExtendedRequest(
   ExtendedReq * er,
   BOOLEAN     bFreeSelf
   )
{
    if (er != NULL)
    {
        if (bFreeSelf)
        {
            VMDIR_SAFE_FREE_MEMORY( er );
        }
    }

    return;
}

/*
 * RFC 5805 - decode End Transactions Request
 * txnEndReq ::= SEQUENCE {
 *     commit      BOOLEAN DEFAULT TRUE,
 *     identifier  OCTET STRING }
 */
static
DWORD
_VmDirDecodeTxnReqData(
    VDIR_BERVALUE reqData,
    PBOOLEAN pbCommit,
    PVDIR_BERVALUE ptxnid
    )
{
   DWORD dwError = 0;
   BerElementBuffer        berbuf = {0};
   BerElement *            ber = (BerElement *)&berbuf;
   ber_int_t txn_commit = {0};
   struct berval txn_id = {0};
   ber_len_t len = {0};

   ber_init2(ber, &reqData.lberbv, LBER_USE_DER);

   if (ber_scanf(ber, "{")== LBER_ERROR)
   {
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_PROTOCOL_ERROR);
   }

   if (ber_peek_tag(ber, &len ) == LBER_OCTETSTRING)
   {
      if (ber_scanf( ber, "m", &txn_id) == LBER_ERROR)
      {
         BAIL_WITH_VMDIR_ERROR(dwError, LDAP_PROTOCOL_ERROR);
      }
      txn_commit = 1;
   } else if (ber_scanf(ber, "bm}", &txn_commit, &txn_id) == LBER_ERROR)
   {
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_PROTOCOL_ERROR);
   }

   dwError = VmDirAllocateAndCopyMemory(txn_id.bv_val, txn_id.bv_len, (PVOID *)&ptxnid->lberbv.bv_val);
   BAIL_ON_VMDIR_ERROR( dwError );
   
   ptxnid->lberbv.bv_len = txn_id.bv_len;
   ptxnid->bOwnBvVal = TRUE;
   *pbCommit = (txn_commit != 0);

cleanup:
    return dwError;

error:
    goto cleanup;
}
