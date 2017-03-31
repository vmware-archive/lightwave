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

static
int
_VmDirSASLBind(
    PVDIR_OPERATION pOperation
    );

static
int
_VmDirBindSetupACL(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry
    );

static
int
_VmDirBindHandleFailedPassword(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry
    );

int
VmDirMLBind(
   PVDIR_OPERATION   pOperation
   )
{
    DWORD       dwError = 0;
    BOOLEAN     bSendResult = TRUE;
    PSTR        pszLocalErrMsg = NULL;

    assert(pOperation->conn);
    pOperation->conn->bIsAnonymousBind = TRUE;  // default to anonymous bind

    switch (pOperation->request.bindReq.method)
    {
        case LDAP_AUTH_SIMPLE:

                if (!VmDirdGetAllowInsecureAuth() &&
                    !pOperation->conn->bIsLdaps &&
                    pOperation->request.bindReq.cred.lberbv.bv_len > 0)
                {
                    // Discourage clients from using insecure connections
                    dwError = pOperation->ldapResult.errCode = LDAP_UNWILLING_TO_PERFORM;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Simple bind not allowed");
                }
                if (pOperation->reqDn.lberbv.bv_len > 0) // anonymous bind has empty DN
                {
                    pOperation->conn->bIsAnonymousBind = FALSE;
                    dwError = VmDirInternalBindEntry(pOperation);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
                else
                {
                    VmDirFreeAccessInfo(&pOperation->conn->AccessInfo);
                }

                break;

        case LDAP_AUTH_SASL:
                pOperation->conn->bIsAnonymousBind = FALSE;

                dwError = _VmDirSASLBind(pOperation);
                BAIL_ON_VMDIR_ERROR(dwError);

                if (pOperation->ldapResult.errCode == LDAP_SASL_BIND_IN_PROGRESS)
                {
                    VmDirSendSASLBindResponse(pOperation);
                    bSendResult = FALSE;
                }
                else if (pOperation->ldapResult.errCode == LDAP_SUCCESS)
                {   // if SASL negotiation completes successfully, it sets pOpeartion->reqDn.
                    dwError = VmDirInternalBindEntry(pOperation);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                break;

        default:
               dwError = pOperation->ldapResult.errCode = LDAP_AUTH_METHOD_NOT_SUPPORTED;
               BAIL_ON_VMDIR_ERROR(dwError);

               break;
    }


cleanup:

    if ( bSendResult )
    {
        VmDirSendLdapResult( pOperation );

        if (pOperation->ldapResult.errCode == LDAP_SUCCESS)
        {
            if (pOperation->request.bindReq.method == LDAP_AUTH_SASL)
            {
                // install sasl encode/decode sockbuf i/o
                pOperation->ldapResult.errCode = VmDirSASLSockbufInstall(
							pOperation->conn->sb,
                                                        pOperation->conn->pSaslInfo
							);
                // do not bail in cleanup section.  we return ldapResult.errCode directly.
            }

            if (gVmdirGlobals.bTrackLastLoginTime)
            {
                VmDirAddTrackLastLoginItem(pOperation->reqDn.lberbv_val);
            }
        }
    }

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:

    if (pszLocalErrMsg)
    {
        pOperation->ldapResult.pszErrMsg = pszLocalErrMsg;
        pszLocalErrMsg = NULL;
    }
    goto cleanup;
}


// After a successful bind, we'll have pOperation->conn->pAccesstoken setup
// using the creds to do the bind
/*
 * Return: VmDir level error code.  Also, pOperation->ldapResult content is set.
 */
int
VmDirInternalBindEntry(
    PVDIR_OPERATION  pOperation
    )
{
    DWORD                   retVal = LDAP_SUCCESS;
    int                     deadLockRetries = 0;
    BOOLEAN                 bHasTxn = FALSE;
    VDIR_ENTRY              entry = {0};
    PVDIR_ENTRY             pEntry = NULL;
    PSTR                    pszLocalErrMsg = NULL;

    assert(pOperation);

    if (pOperation->pBEIF == NULL)
    {
        pOperation->pBEIF = VmDirBackendSelect(pOperation->reqDn.lberbv.bv_val);
        assert(pOperation->pBEIF);
    }

    // Normalize DN
    retVal = VmDirNormalizeDN( &(pOperation->reqDn), pOperation->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    // ************************************************************************************
    // transaction retry loop begin.  make sure all function within are retry agnostic.
    // ************************************************************************************
txnretry:
    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
        bHasTxn = FALSE;
    }

    deadLockRetries++;
    if (deadLockRetries > MAX_DEADLOCK_RETRIES)
    {
        retVal = VMDIR_ERROR_LOCK_DEADLOCK;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    else
    {
        if (pEntry)
        {
            VmDirFreeEntryContent(pEntry);
            memset(pEntry, 0, sizeof(VDIR_ENTRY));
            pEntry = NULL;
        }

        retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_READ );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = TRUE;

        // Read current entry from DB
        retVal = pOperation->pBEIF->pfnBEDNToEntry(
                                    pOperation->pBECtx,
                                    pOperation->pSchemaCtx,
                                    &pOperation->reqDn,
                                    &entry,
                                    VDIR_BACKEND_ENTRY_LOCK_READ);
        if (retVal != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                    goto txnretry;

                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%u)(%s)",
                                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

        retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = FALSE;

        pEntry = &entry;
    }
    // ************************************************************************************
    // transaction retry loop end.
    // ************************************************************************************

    retVal = _VmDirBindSetupACL( pOperation, pEntry );
    BAIL_ON_VMDIR_ERROR(retVal );

    retVal = _VmDirBindHandleFailedPassword( pOperation, pEntry );
    BAIL_ON_VMDIR_ERROR(retVal);

    // deny access if login is blocked.
    retVal = VdirLoginBlocked(pOperation, pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );
    VmDirFreeEntryContent ( &entry );

    return retVal;

error:

    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
    }

    if (retVal)
    {
        VmDirFreeAccessInfo(&pOperation->conn->AccessInfo);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                       "Bind failed (%s) (%u)",
                        VDIR_SAFE_STRING(pszLocalErrMsg), retVal);

        retVal = LDAP_INVALID_CREDENTIALS;
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    }

    VMDIR_SET_LDAP_RESULT_ERROR(&(pOperation->ldapResult), retVal, pszLocalErrMsg);

    goto cleanup;
}

static
int
_VmDirBindSetupACL(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    int     retVal = 0;

    assert( pOperation && pEntry );

    // For instance a bind trying to overwrite a previous bind on the same connection
    // and the previous bind's token is still in-use, the new bind request should fail
    // return LDAP_UNWILLING_TO_PERFORM

    VmDirFreeAccessInfo(&pOperation->conn->AccessInfo);

    retVal = VmDirSrvCreateAccessTokenWithEntry(pEntry,
                                                &pOperation->conn->AccessInfo.pAccessToken,
                                                &pOperation->conn->AccessInfo.pszBindedObjectSid);
    BAIL_ON_VMDIR_ERROR(retVal);

    pOperation->conn->AccessInfo.bindEID = pEntry->eId;

    retVal = VmDirAllocateStringA(BERVAL_NORM_VAL(pEntry->dn),
                                  &pOperation->conn->AccessInfo.pszNormBindedDn);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAllocateStringA(pEntry->dn.lberbv.bv_val,
                                  &pOperation->conn->AccessInfo.pszBindedDn);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "(%s) failed: (%u)", __FUNCTION__, retVal);
    goto cleanup;
}

static
int
_VmDirBindHandleFailedPassword(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry)
{
    int     retVal = 0;

    assert( pOperation && pEntry );

    // Perform login in check after ACL
    if (pOperation->request.bindReq.method == LDAP_AUTH_SIMPLE)
    {   // for simple bind, verify user password.
        retVal = VdirPasswordCheck( &pOperation->request.bindReq.cred, pEntry);
    }
    else if (pOperation->request.bindReq.method == LDAP_AUTH_SASL)
    {
        retVal = pOperation->conn->pSaslInfo->vmdirCode;
    }

    if (retVal == VMDIR_ERROR_USER_INVALID_CREDENTIAL)
    {   // handle password fail event but ignore error
        VdirPasswordFailEvent(
                pOperation,
                BERVAL_NORM_VAL(pEntry->dn),
                pEntry);
        // ignore error
    }

    return retVal;
}

static
int
_VmDirSASLBind(
    PVDIR_OPERATION pOperation
    )
{
    DWORD                   retVal = 0;
    PVDIR_BIND_REQ          pBindReq = &pOperation->request.bindReq;
    PVDIR_SASL_BIND_INFO    pSaslInfo = pOperation->conn->pSaslInfo;
    PVDIR_SASL_BIND_INFO    pLocalInfo = NULL;
    PSTR                    pszLocalErrMsg = NULL;
    PVDIR_ENTRY             pLocalEntry = NULL;

    if ( IS_BERVALUE_EMPTY_OR_NULL(pBindReq->bvMechanism.lberbv)
         ||
         (! VmDirIsSupportedSASLMechanism(pBindReq->bvMechanism.lberbv.bv_val) )
       )
    {
        retVal = VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Invalid or unsupported sasl mechanism" );
    }

    if (pSaslInfo == NULL)  // start brand new sasl session
    {
        retVal = VmDirAllocateMemory(sizeof(*pSaslInfo), (PVOID*)&pLocalInfo);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirBervalContentDup(&pBindReq->bvMechanism, &pLocalInfo->bvMechnism);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirSASLSessionInit(pLocalInfo);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%u)(%s)", retVal, "SASL init failed.");

        retVal = VmDirSASLSessionStart( pLocalInfo,
                                        &(pOperation->request.bindReq),
                                        &(pOperation->ldapResult.replyInfo.replyData.bvSaslReply),
                                        &(pOperation->ldapResult)
                                        );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%u)(%s)", retVal, "SASL start failed.");

        pLocalInfo->pSockbuf = pOperation->conn->sb;
        pLocalInfo->saslStatus = SASL_STATUS_IN_PROGRESS;
    }
    else
    {
        if ( pSaslInfo->saslStatus != SASL_STATUS_IN_PROGRESS)
        {
            retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%s)", "Invalid sasl state.");
        }

        if ( VmDirStringCompareA(  pSaslInfo->bvMechnism.lberbv.bv_val,   // subsequent sasl bind
                                   pBindReq->bvMechanism.lberbv.bv_val,   // should use same mech
                                   FALSE) != 0
           )
        {
            // sasl mechanism mismatch, bail. (is this allowed by standard?)
            retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%s)", "Invalid sasl mechanism or state.");
        }

        retVal = VmDirSASLSessionStep(  pSaslInfo,
                                        &(pOperation->request.bindReq),
                                        &(pOperation->ldapResult.replyInfo.replyData.bvSaslReply),
                                        &(pOperation->ldapResult)
                                        );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%u)(%s)", retVal, "SASL step failed.");
    }

    if  (pOperation->conn->pSaslInfo)
    {
        if (pOperation->conn->pSaslInfo->saslStatus == SASL_STATUS_DONE)
        {
            // all done with SASL, let's lookup entry via UPN
            retVal = VmDirUPNToDNBerWrap( pOperation->conn->pSaslInfo->pszBindUserName, &pOperation->reqDn);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "SASL bind UPN (%s) lookup failed.",
                                          VDIR_SAFE_STRING(pOperation->conn->pSaslInfo->pszBindUserName));

        }
    }
    else
    {   // hand pLocalInfo to connection
        pOperation->conn->pSaslInfo = pLocalInfo;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    if ( pLocalEntry )
    {
        VmDirFreeEntry( pLocalEntry );
    }

    return retVal;

error:

    if ( retVal == LDAP_INVALID_CREDENTIALS
         &&
         pOperation->conn->pSaslInfo
         &&
         pOperation->conn->pSaslInfo->pszBindUserName
       )
    {
        // bad password - lookup entry and handle failed password
        if ( VmDirUPNToDNBerWrap( pOperation->conn->pSaslInfo->pszBindUserName, &pOperation->reqDn ) == 0
             &&
             VmDirSimpleDNToEntry( pOperation->reqDn.lberbv_val, &pLocalEntry ) == 0
             &&
             _VmDirBindSetupACL( pOperation, pLocalEntry ) == 0
           )
        {
            pOperation->conn->pSaslInfo->vmdirCode = VMDIR_ERROR_USER_INVALID_CREDENTIAL;
            _VmDirBindHandleFailedPassword( pOperation, pLocalEntry );
            // ignore error
        }
    }

    if (pLocalInfo)
    {
        VmDirSASLSessionClose(pLocalInfo);
        VMDIR_SAFE_FREE_MEMORY(pLocalInfo);
    }

    // SASL bind failed, clear up pSaslInfo to reset SASL state.
    VmDirSASLSessionClose(pOperation->conn->pSaslInfo);
    VMDIR_SAFE_FREE_MEMORY(pOperation->conn->pSaslInfo);

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);

    goto cleanup;
}
