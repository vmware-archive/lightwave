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
DWORD
_VmDirMLTxnEnd(
    PVDIR_OPERATION pOperation,
    VDIR_BERVALUE txnid,
    BOOLEAN isCommit,
    BOOLEAN *pBresultSent
    );

static
VOID
_VmDirSetSockTimeout(
    PVDIR_OPERATION pOperation,
    int timeout_ms,
    const char *from);

static
DWORD
_VmDirAllocTxnId(
    PSTR *ppszTxnId
    )
{
    DWORD   dwError = 0;
    char szGuidStr[VMDIR_GUID_STR_LEN] = {0};
    uuid_t guid = {0};

    dwError = VmDirUuidGenerate(&guid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidToStringLower(&guid, szGuidStr, sizeof(szGuidStr));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(szGuidStr, VmDirStringLenA(szGuidStr)+1, (PVOID*)ppszTxnId);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

int
VmDirMLTxnStart(
    PVDIR_OPERATION pOperation,
    BOOLEAN         *pBresultSent
    )
{
    DWORD   dwError = 0;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PSTR pszLocalErrMsg = NULL;
    BOOLEAN bResultSent = FALSE;

    // AnonymousBind Or in case of a failed bind, do not grant user transaction
    if (pOperation->conn->bIsAnonymousBind || VmDirIsFailedAccessInfo(&pOperation->conn->AccessInfo))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_INSUFFICIENT_ACCESS);
    }

    dwError = VmDirAllocAndSetThrTxnCtx(&pThreadTxnContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(pThreadTxnContext);

    pTxnContext = &pThreadTxnContext->mainDbTxnCtx;
    if (pTxnContext->txnState == TXN_USER_IN_PROGRESS ||
        pTxnContext->txnState == TXN_PENDING_START)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: invaid request - user transaction in progress", __func__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    } else if (pTxnContext->txnState != TXN_NONE)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "wrong state - a standalong transaction on mainDB");
    } else
    {
        VMDIR_SAFE_FREE_MEMORY(pTxnContext->pszTxnId);
        pTxnContext->pBE = NULL;
        pTxnContext->pTxnParent = NULL;
        pTxnContext->pTxnChild = NULL;
        dwError = _VmDirAllocTxnId(&pTxnContext->pszTxnId);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTxnContext->txnState = TXN_PENDING_START;
        VmDirSendLdapTxnResult(pOperation, pTxnContext->pszTxnId, "", &bResultSent);
        bResultSent = TRUE;

        _VmDirSetSockTimeout(pOperation, gVmdirGlobals.dwLdapUserTxnRecvTimeoutMS, __func__);
        pTxnContext->txnStartTime = VmDirGetTimeInMilliSec();
    }

cleanup:
     *pBresultSent = bResultSent;
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VmDirMDBTxnAbortAll();
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: failed %s error %d", __func__,
      VDIR_SAFE_STRING(pszLocalErrMsg), dwError);
    VMDIR_SET_LDAP_RESULT_ERROR(&(pOperation->ldapResult), dwError, pszLocalErrMsg);
    goto cleanup;
}

static
DWORD
_VmDirMLTxnEnd(
    PVDIR_OPERATION pOperation,
    VDIR_BERVALUE   txnid,
    BOOLEAN         isCommit,
    BOOLEAN         *pBresultSent
    )
{
    DWORD   dwError = 0;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PSTR    pszLocalErrMsg = NULL;
    BOOLEAN bResultSent = FALSE;

    if (!pOperation->pBECtx)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid Backend");
    }

    dwError = VmDirGetThreadTxnContextValue(&pThreadTxnContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(pThreadTxnContext);

    pTxnContext = &pThreadTxnContext->mainDbTxnCtx;

    if (!pTxnContext->pBE)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid Backend");
    }

    if (pTxnContext->txnState == TXN_USER_IN_PROGRESS)
    {
        if(pTxnContext->pszTxnId && 
           VmDirStringCompareA(txnid.lberbv.bv_val, pTxnContext->pszTxnId, FALSE)== 0)
        {
           pOperation->pBECtx->pBE = pTxnContext->pBE;

           _VmDirSetSockTimeout(pOperation, gVmdirGlobals.dwLdapRecvTimeoutSec * 1000, __func__);

           if (isCommit)
           {
               dwError = ((PVDIR_BACKEND_INTERFACE)pTxnContext->pBE)->pfnBETxnCommit(pOperation->pBECtx);
               BAIL_ON_VMDIR_ERROR(dwError);
           } else
           {
              ((PVDIR_BACKEND_INTERFACE)pTxnContext->pBE)->pfnBETxnAbort(pOperation->pBECtx);
           }

           VmDirSendLdapTxnResult(pOperation, pTxnContext->pszTxnId, "", &bResultSent);
           VMDIR_SAFE_FREE_MEMORY(pTxnContext->pszTxnId);
        } else
        {
           dwError = VMDIR_ERROR_INVALID_PARAMETER;
           BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "missing or mismatched user txn id");
        }
    } else
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "No outstanding user transaction");
    }

cleanup:
    *pBresultSent = bResultSent;
    if (dwError==0)
    {
      VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "%s: %s succeeded", __func__, isCommit?"commit":"abort");
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:
    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult, dwError, pszLocalErrMsg);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: %s error %d", __func__, isCommit?"commit":"abort", dwError);
    goto cleanup ;
}

int
VmDirMLTxnCommit(
    PVDIR_OPERATION pOperation,
    VDIR_BERVALUE   txnid,
    BOOLEAN *       pBresultSent
    )
{
    return _VmDirMLTxnEnd(pOperation, txnid, TRUE, pBresultSent);
}

int
VmDirMLTxnAbort(
    PVDIR_OPERATION pOperation,
    VDIR_BERVALUE   txnid,
    BOOLEAN *       pBresultSent
    )
{
    return _VmDirMLTxnEnd(pOperation, txnid, FALSE, pBresultSent);
}

static
VOID
_VmDirSetSockTimeout(
    PVDIR_OPERATION pOperation,
    int             timeout_ms,
    const char *    from
    )
{
    struct timeval sTimeout = {0};
    int sd = -1;
    DWORD dwError = 0;

    if (pOperation->conn == NULL || pOperation->conn->sb == NULL)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s(%s): invalid sock number", __func__, from);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (ber_sockbuf_ctrl(pOperation->conn->sb, LBER_SB_OPT_GET_FD, &sd) == 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ber_sockbuf_ctrl LBER_SB_OPT_GET_FD error %d ", errno);
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    sTimeout.tv_sec = timeout_ms/1000;
    sTimeout.tv_usec = (timeout_ms % 1000) * 1000;
    if (setsockopt(pOperation->conn->sd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &sTimeout, sizeof(sTimeout)) < 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: setsockopt SO_RCVTIMEO error %d", __func__, errno);
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "%s(%s): setsockopt SO_RCVTIMEO %d ms", __func__, from, timeout_ms);

cleanup:
    return;

error:
    goto cleanup;
}
