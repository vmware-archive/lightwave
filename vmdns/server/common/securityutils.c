/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * File:   securityutils.c
 * Author: Neel Shah
 *
 * Created on August 15, 2016
 */

#include "includes.h"

static
DWORD
VmDnsSecGssAcceptSecCtx(
    PVMDNS_RECORD               pReqTkey,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_RECORD               *ppRespTkey,
    PBOOL                       pbAuthSuccess
    );

static
VOID
VmDnsSecGssDeleteSecCtx(
    gss_ctx_id_t    *gss_ctx_hdl
    );

static
DWORD
VmDnsSecGssCheckSecCtxExpired(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PBOOL                       pbExpired
    );

static
DWORD
VmDnsSecGssGetMic(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_BLOB                 pMessage,
    PVMDNS_BLOB                 *ppSignature
    );

static
DWORD
VmDnsSecGssVerifyMic(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_BLOB                 pMessage,
    PVMDNS_BLOB                 pSignature,
    PBOOL                       pbIsVerified
    );

static
VOID
VmDnsSecGssReleaseBuffer(
    gss_buffer_t    buffer
    );

static
VOID
VmDnsSecGssReleaseName(
    gss_name_t  *name
    );

static
DWORD
VmDnsSecGssGetPrincipalName(
    gss_name_t  client_name,
    PSTR        *ppszPrincipalName
    );

static
DWORD
VmDnsSecCreateTkeyRR(
    PCSTR           pszKeyName,
    PVMDNS_BLOB     pKey,
    VM_DNS_RCODE    unRCode,
    PVMDNS_RECORD   *ppOutTkey
    );

static
DWORD
VmDnsSecCreateTsigRR(
    PCSTR           pszKeyName,
    PVMDNS_BLOB     pSignature,
    UINT16          wOriginalXid,
    VM_DNS_RCODE    unRCode,
    PVMDNS_RECORD   *ppOutTsig
    );

static
DWORD
VmDnsSecAppendToRecordArray(
    DWORD           dwSize,
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   **pppRecords
    );

static
DWORD
VmDnsSecStripTsigFromMessage(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_RECORD   *ppTsig
    );

static
DWORD
VmDnsSecStripTsigFromUpdateMessage(
    PVMDNS_UPDATE_MESSAGE   pRequest,
    PVMDNS_RECORD           *ppTsig
    );

static
DWORD
VmDnsSecSerializeTsigVariable(
    PVMDNS_RECORD           pTsig,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );

static
DWORD
VmDnsSecCreateMacMessage(
    PVMDNS_BLOB             pRequestSig,
    PVMDNS_MESSAGE          pResponse,
    PVMDNS_RECORD           pTsig,
    PVMDNS_BLOB             *ppMessage
    );

static
DWORD
VmDnsSecCreateMacUpdateMessage(
    PVMDNS_BLOB             pRequestSig,
    PVMDNS_UPDATE_MESSAGE   pDnsMessage,
    PVMDNS_RECORD           pTsig,
    PVMDNS_BLOB             *ppMessage
    );

static
DWORD
VmDnsSecBlobToGssBuffer(
    PVMDNS_BLOB     pBlob,
    gss_buffer_t    gss_buffer_out
    );

static
DWORD
VmDnsSecGssBufferToBlob(
    gss_buffer_t    gss_buffer,
    PVMDNS_BLOB     *ppBlob
    );

static
DWORD
VmDnsSecFQDNToShort(
    PCSTR   pszSrc,
    PSTR    *ppszDst
    );


DWORD
VmDnsSecInitialize(
    PVMDNS_SECURITY_CONTEXT     *ppContext
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_SECURITY_CONTEXT pContext = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppContext, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_SECURITY_CONTEXT),
                        (PVOID*)&pContext
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsHashTableAllocate(
                        &pContext->pActiveGSSContexts,
                        DEFAULT_SECURITY_HASHTABLE_SIZE
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(
                        &pContext->pLock
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppContext = pContext;


cleanup:

    return dwError;

error:

    VmDnsSecCleanup(pContext);
    if (ppContext)
    {
        *ppContext = NULL;
    }

    goto cleanup;
}

VOID
VmDnsSecCleanup(
    PVMDNS_SECURITY_CONTEXT     pContext
    )
{
    if (pContext)
    {
        if (pContext->pActiveGSSContexts)
        {
            VmDnsHashTableFree(pContext->pActiveGSSContexts);
        }

        if (pContext->pLock)
        {
            VmDnsFreeRWLock(pContext->pLock);
        }

        VmDnsFreeMemory(pContext);
    }
}

DWORD
VmDnsSecCreateGssCtx(
    PCSTR                       pszKeyName,
    gss_ctx_id_t                gss_ctx_hdl,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx
    )
{
    DWORD dwError = 0;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszKeyName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(gss_ctx_hdl, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppGssSecCtx, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_GSS_CONTEXT_HANDLE),
                        (PVOID*)&pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                        pszKeyName,
                        &pGssSecCtx->pszKeyName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pGssSecCtx->gss_ctx_hdl = gss_ctx_hdl;
    pGssSecCtx->state = VM_DNS_GSS_CTX_UNINITIALIZED;
    pGssSecCtx->pLastRecvSig = NULL;

    *ppGssSecCtx = pGssSecCtx;


cleanup:

    return dwError;

error:

    VmDnsSecCleanupGssCtx(pGssSecCtx);
    if (ppGssSecCtx)
    {
        *ppGssSecCtx = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsSecAddGssCtx(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);

    VMDNS_LOCKWRITE(gpSrvContext->pSecurityContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsHashTableInsert(
                        gpSrvContext->pSecurityContext->pActiveGSSContexts,
                        pGssSecCtx->pszKeyName,
                        pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
             "[%s,%s,%d]: GSS_DEBUG: ctx for key %s added to context map",
             __FILE__,
             __FUNCTION__,
             __LINE__,
             pGssSecCtx->pszKeyName
             );


cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(gpSrvContext->pSecurityContext->pLock);
    }

    return dwError;

error:

    goto cleanup;

}

DWORD
VmDnsSecGetGssCtx(
    PCSTR                       pszKeyName,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszKeyName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppGssSecCtx, dwError);

    VMDNS_LOCKREAD(gpSrvContext->pSecurityContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsHashTableGet(
                        gpSrvContext->pSecurityContext->pActiveGSSContexts,
                        pszKeyName,
                        (PVOID *)&pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppGssSecCtx = pGssSecCtx;


cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(gpSrvContext->pSecurityContext->pLock);
    }

    return dwError;

error:

    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
             "[%s,%s,%d]: GSS_DEBUG: ctx for key %s not found in context map",
             __FILE__,
             __FUNCTION__,
             __LINE__,
              pszKeyName
             );

    VmDnsSecCleanupGssCtx(pGssSecCtx);
    if (ppGssSecCtx)
    {
        *ppGssSecCtx = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsSecDeleteGssCtx(
    PSTR   pszKeyName
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszKeyName, dwError);

    VMDNS_LOCKWRITE(gpSrvContext->pSecurityContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsSecGetGssCtx(
                    pszKeyName,
                    &pGssSecCtx
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsSecCleanupGssCtx(pGssSecCtx);

    dwError = VmDnsHashTableRemove(
                        gpSrvContext->pSecurityContext->pActiveGSSContexts,
                        pszKeyName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(gpSrvContext->pSecurityContext->pLock);
    }

    return dwError;

error:

    VmDnsSecCleanupGssCtx(pGssSecCtx);
    goto cleanup;
}

VOID
VmDnsSecCleanupGssCtx(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx
    )
{
    if (pGssSecCtx)
    {
        VMDNS_SAFE_FREE_STRINGA(pGssSecCtx->pszKeyName);
        VmDnsSecGssDeleteSecCtx(&pGssSecCtx->gss_ctx_hdl);
        VmDnsFreeBlob(pGssSecCtx->pLastRecvSig);
        VMDNS_SAFE_FREE_MEMORY(pGssSecCtx);
    }
}

DWORD
VmDnsSecProcessTkeyQuery(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_MESSAGE  pResponse
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwTkeyCount = 0;
    BOOL bAuthSuccess = FALSE;
    BOOL bIsVerified = FALSE;
    PVMDNS_RECORD *pAdditionalRRs = NULL;
    PSTR pszAlgName = NULL;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    PVMDNS_RECORD pTkeyRR = NULL;
    PVMDNS_RECORD pRespTkey = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pResponse, dwError);

    pAdditionalRRs = pRequest->pAdditional;

    for (; dwIndex < pRequest->pHeader->usADCount; ++dwIndex)
    {
        if (pAdditionalRRs[dwIndex]->dwType == VMDNS_RR_MTYPE_TKEY)
        {
            pTkeyRR = pAdditionalRRs[dwIndex];
            dwTkeyCount++;
        }
    }

    if (dwTkeyCount != 1)
    {
        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_FORMAT_ERROR;
        goto response;
    }

    dwError = VmDnsSecFQDNToShort(
                        pTkeyRR->Data.TKEY.pNameAlgorithm,
                        &pszAlgName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (VmDnsStringCompareA(pszAlgName,
                            VMDNS_SEC_ALG_NAME,
                            FALSE))
    {
        dwError = VmDnsDuplicateRecord(
                            pTkeyRR,
                            &pRespTkey
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        pRespTkey->Data.TKEY.wError = VM_DNS_RCODE_BADALG;

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usANCount,
                                pRespTkey,
                                &pResponse->pAnswers
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usANCount++;

        goto response;
    }

    if (pTkeyRR->Data.TKEY.wMode == VM_DNS_TKEY_MODE_GSS_API)
    {
        dwError = VmDnsSecAuthNegotiate(
                            pTkeyRR,
                            &pGssSecCtx,
                            &pRespTkey,
                            &bAuthSuccess
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usANCount,
                                pRespTkey,
                                &pResponse->pAnswers
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usANCount++;

        if (bAuthSuccess)
        {
            dwError = VmDnsSecSignMessage(
                                pGssSecCtx,
                                pResponse
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
    else if (pTkeyRR->Data.TKEY.wMode == VM_DNS_TKEY_MODE_KEY_DELETION)
    {
        dwError = VmDnsSecCheckDeletePermissions(
                                pRequest,
                                pResponse,
                                &bIsVerified
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (bIsVerified)
        {
            dwError = VmDnsSecDeleteGssCtx(
                                pTkeyRR->pszName
                                );
            BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

            if (dwError == ERROR_NOT_FOUND)
            {
                dwError = ERROR_SUCCESS;

                dwError = VmDnsSecCreateTkeyRR(
                                    pTkeyRR->pszName,
                                    NULL,
                                    VM_DNS_RCODE_BADNAME,
                                    &pRespTkey
                                    );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else
            {
                dwError = VmDnsSecCreateTkeyRR(
                                    pTkeyRR->pszName,
                                    NULL,
                                    VM_DNS_RCODE_NOERROR,
                                    &pRespTkey
                                    );
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            pRespTkey->Data.TKEY.wMode = VM_DNS_TKEY_MODE_KEY_DELETION;
        }
    }
    else
    {
        dwError = VmDnsDuplicateRecord(
                            pTkeyRR,
                            &pRespTkey
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        pRespTkey->Data.TKEY.wError = VM_DNS_RCODE_BADMODE;

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usANCount,
                                pRespTkey,
                                &pResponse->pAnswers
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usANCount++;
    }


response:

    VmDnsClearRecord(pRespTkey);
    VMDNS_SAFE_FREE_STRINGA(pszAlgName);
    VMDNS_SAFE_FREE_MEMORY(pRespTkey);

    return dwError;

error:

    pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;

    goto response;
}

DWORD
VmDnsSecCheckDeletePermissions(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_MESSAGE  pResponse,
    PBOOL           pbIsVerified
    )
{
    DWORD dwError = 0;
    DWORD dwARCount = 0;
    UINT64 unTime = 0;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    PVMDNS_RECORD pTsig = NULL;
    PVMDNS_RECORD pRespTsig = NULL;
    BOOL bIsVerified = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pbIsVerified, dwError);

    dwARCount = pRequest->pHeader->usARCount;

    if (dwARCount < 0 || !pRequest->pAdditional)
    {
        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_REFUSED;
        goto response;
    }

    if (pRequest->pAdditional[dwARCount - 1]->dwType != VMDNS_RR_MTYPE_TSIG)
    {
        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_REFUSED;
        goto response;
    }

    dwError = VmDnsSecStripTsigFromMessage(
                        pRequest,
                        &pTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGetGssCtx(
                        pTsig->pszName,
                        &pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (dwError == ERROR_NOT_FOUND)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Attempted DNS ctx delete with unknown key.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = ERROR_SUCCESS;

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADKEY,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usARCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usARCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;

        goto response;
    }

    unTime = time(NULL);

    if (((pTsig->Data.TSIG.unCreateTime + pTsig->Data.TSIG.wFudgeTime) < unTime) ||
        ((pTsig->Data.TSIG.unCreateTime - pTsig->Data.TSIG.wFudgeTime) > unTime))
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Attempted DNS ctx delete with fudged TSIG.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADTIME,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usARCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usARCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;

        goto response;
    }

    dwError = VmDnsSecVerifyMessage(
                        pGssSecCtx,
                        pRequest,
                        pTsig,
                        &bIsVerified
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!bIsVerified)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: DNS ctx delete TSIG failed to verify.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADSIG,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usARCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usARCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;
    }


response:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);
    VmDnsClearRecord(pRespTsig);
    VMDNS_SAFE_FREE_MEMORY(pRespTsig);

    *pbIsVerified = bIsVerified;
    return dwError;

error:

    pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    bIsVerified = FALSE;

    goto response;
}

DWORD
VmDnsSecCheckUpdatePermissions(
    PVMDNS_UPDATE_MESSAGE       pRequest,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pResponse,
    PBOOL                       pbIsVerified
    )
{
    DWORD dwError = 0;
    DWORD dwADCount = 0;
    UINT64 unTime = 0;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    PVMDNS_RECORD pTsig = NULL;
    PVMDNS_RECORD pRespTsig = NULL;
    BOOL bIsVerified = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pbIsVerified, dwError);

    dwADCount = pRequest->pHeader->usADCount;

    if (dwADCount < 0 || !pRequest->pAdditional)
    {
        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_REFUSED;
        goto response;
    }

    if (pRequest->pAdditional[dwADCount - 1]->dwType != VMDNS_RR_MTYPE_TSIG)
    {
        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_REFUSED;
        goto response;
    }

    dwError = VmDnsSecStripTsigFromUpdateMessage(
                                pRequest,
                                &pTsig
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGetGssCtx(
                        pTsig->pszName,
                        &pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (dwError == ERROR_NOT_FOUND)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Attempted DNS Update with unknown key.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = ERROR_SUCCESS;

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADKEY,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usADCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usADCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;

        goto response;
    }

    unTime = time(NULL);

    if (((pTsig->Data.TSIG.unCreateTime + pTsig->Data.TSIG.wFudgeTime) < unTime) ||
        ((pTsig->Data.TSIG.unCreateTime - pTsig->Data.TSIG.wFudgeTime) > unTime))
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Attempted DNS Update with fudged TSIG.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADTIME,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usADCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usADCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;

        goto response;
    }

    dwError = VmDnsSecVerifyUpdateMessage(
                            pGssSecCtx,
                            pRequest,
                            pTsig,
                            &bIsVerified
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!bIsVerified)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: DNS update TSIG failed to verify.  Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pTsig->pszName
                 );

        dwError = VmDnsSecCreateTsigRR(
                            pTsig->pszName,
                            NULL,
                            pRequest->pHeader->usId,
                            VM_DNS_RCODE_BADSIG,
                            &pRespTsig
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecAppendToRecordArray(
                                pResponse->pHeader->usADCount,
                                pRespTsig,
                                &pResponse->pAdditional
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pResponse->pHeader->usADCount++;

        pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_NOTAUTH;
    }


response:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);
    VmDnsClearRecord(pRespTsig);
    VMDNS_SAFE_FREE_MEMORY(pRespTsig);

    *ppGssSecCtx = pGssSecCtx;
    *pbIsVerified = bIsVerified;

    return dwError;

error:

    pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    bIsVerified = FALSE;
    if (ppGssSecCtx)
    {
        *ppGssSecCtx = NULL;
    }

    goto response;
}

DWORD
VmDnsSecAuthNegotiate(
    PVMDNS_RECORD               pReqTkey,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_RECORD               *ppRespTkey,
    PBOOL                       pbAuthSuccess
    )
{
    DWORD dwError = 0;
    PSTR pszKeyName = NULL;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    BOOL bCtxExpired = FALSE;
    PVMDNS_RECORD pRespTkey = NULL;
    BOOL bAuthSuccess = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pReqTkey, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRespTkey, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pbAuthSuccess, dwError);

    pszKeyName = pReqTkey->pszName;

    dwError = VmDnsSecGetGssCtx(
                        pszKeyName,
                        &pGssSecCtx
                        );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (!pGssSecCtx || dwError == ERROR_NOT_FOUND)
    {
        dwError = ERROR_SUCCESS;

        dwError = VmDnsSecGssAcceptSecCtx(
                            pReqTkey,
                            &pGssSecCtx,
                            &pRespTkey,
                            &bAuthSuccess
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsSecGssCheckSecCtxExpired(
                                pGssSecCtx,
                                &bCtxExpired
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (pGssSecCtx->state != VM_DNS_GSS_CTX_ESTABLISHED && !bCtxExpired)
        {
            pGssSecCtx->state = VM_DNS_GSS_CTX_ESTABLISHED;

            bAuthSuccess = TRUE;
        }

        if (pGssSecCtx->state != VM_DNS_GSS_CTX_ESTABLISHED && bCtxExpired)
        {
            dwError = VmDnsSecDeleteGssCtx(
                                pGssSecCtx->pszKeyName
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSecGssAcceptSecCtx(
                                pReqTkey,
                                &pGssSecCtx,
                                &pRespTkey,
                                &bAuthSuccess
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        if (pGssSecCtx->state == VM_DNS_GSS_CTX_ESTABLISHED && bCtxExpired)
        {
            dwError = VmDnsDuplicateRecord(
                                pReqTkey,
                                &pRespTkey
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            pRespTkey->Data.TKEY.wError = VM_DNS_RCODE_BADNAME;
        }
    }

    *ppGssSecCtx = pGssSecCtx;
    *ppRespTkey = pRespTkey;
    *pbAuthSuccess = bAuthSuccess;


cleanup:

    return dwError;

error:

    VmDnsClearRecord(pRespTkey);
    VMDNS_SAFE_FREE_MEMORY(pRespTkey);
    if (ppGssSecCtx)
    {
        *ppGssSecCtx = NULL;
    }
    if (ppRespTkey)
    {
        *ppRespTkey = NULL;
    }
    if (pbAuthSuccess)
    {
        *pbAuthSuccess = FALSE;
    }

    goto cleanup;
}

DWORD
VmDnsSecSignMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_MESSAGE              pResponse
    )
{
    DWORD dwError = 0;
    PVMDNS_BLOB pMessage = NULL;
    PVMDNS_BLOB pSignature = NULL;
    PVMDNS_RECORD pTsig = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pResponse, dwError);

    dwError = VmDnsSecCreateTsigRR(
                        pGssSecCtx->pszKeyName,
                        NULL,
                        pResponse->pHeader->usId,
                        VM_DNS_RCODE_NOERROR,
                        &pTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecCreateMacMessage(
                        pGssSecCtx->pLastRecvSig,
                        pResponse,
                        pTsig,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGssGetMic(
                        pGssSecCtx,
                        pMessage,
                        &pSignature
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pSignature)
    {
        VmDnsFreeBlob(pTsig->Data.TSIG.pSignature);

        dwError = VmDnsCopyBlob(
                        pSignature,
                        &pTsig->Data.TSIG.pSignature
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        pTsig->Data.TSIG.wError = VM_DNS_RCODE_BADKEY;
    }

    dwError = VmDnsSecAppendToRecordArray(
                            pResponse->pHeader->usARCount,
                            pTsig,
                            &pResponse->pAdditional
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pResponse->pHeader->usARCount++;


response:

    VmDnsFreeBlob(pMessage);
    VmDnsFreeBlob(pSignature);

    return dwError;

error:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);

    pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

DWORD
VmDnsSecSignUpdateMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pResponse
    )
{
    DWORD dwError = 0;
    PVMDNS_BLOB pMessage = NULL;
    PVMDNS_BLOB pSignature = NULL;
    PVMDNS_RECORD pTsig = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pResponse, dwError);

    dwError = VmDnsSecCreateTsigRR(
                        pGssSecCtx->pszKeyName,
                        NULL,
                        pResponse->pHeader->usId,
                        VM_DNS_RCODE_NOERROR,
                        &pTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecCreateMacUpdateMessage(
                        pGssSecCtx->pLastRecvSig,
                        pResponse,
                        pTsig,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGssGetMic(
                        pGssSecCtx,
                        pMessage,
                        &pSignature
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pSignature)
    {
        VmDnsFreeBlob(pTsig->Data.TSIG.pSignature);

        dwError = VmDnsCopyBlob(
                        pSignature,
                        &pTsig->Data.TSIG.pSignature
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        pTsig->Data.TSIG.wError = VM_DNS_RCODE_BADKEY;
    }

    dwError = VmDnsSecAppendToRecordArray(
                            pResponse->pHeader->usADCount,
                            pTsig,
                            &pResponse->pAdditional
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pResponse->pHeader->usADCount++;


response:

    VmDnsFreeBlob(pMessage);
    VmDnsFreeBlob(pSignature);

    return dwError;

error:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);

    pResponse->pHeader->codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

DWORD
VmDnsSecVerifyMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_MESSAGE              pRequest,
    PVMDNS_RECORD               pTsig,
    PBOOL                       pbIsVerified
    )
{
    DWORD dwError = 0;
    PVMDNS_BLOB pMessage = NULL;
    PVMDNS_BLOB pSignature = NULL;
    BOOL bIsVerified = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);

    dwError = VmDnsSecCreateMacMessage(
                        NULL,
                        pRequest,
                        pTsig,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGssVerifyMic(
                        pGssSecCtx,
                        pMessage,
                        pTsig->Data.TSIG.pSignature,
                        &bIsVerified
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bIsVerified)
    {
        if (pGssSecCtx->pLastRecvSig)
        {
            VmDnsFreeBlob(pGssSecCtx->pLastRecvSig);
        }

        dwError = VmDnsCopyBlob(
                        pTsig->Data.TSIG.pSignature,
                        &pGssSecCtx->pLastRecvSig
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pbIsVerified = bIsVerified;


response:

    VmDnsFreeBlob(pMessage);
    VmDnsFreeBlob(pSignature);

    return dwError;

error:

    if (pbIsVerified)
    {
        *pbIsVerified = FALSE;
    }

    goto response;
}

DWORD
VmDnsSecVerifyUpdateMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pRequest,
    PVMDNS_RECORD               pTsig,
    PBOOL                       pbIsVerified
    )
{
    DWORD dwError = 0;
    PVMDNS_BLOB pMessage = NULL;
    PVMDNS_BLOB pSignature = NULL;
    BOOL bIsVerified = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);

    dwError = VmDnsSecCreateMacUpdateMessage(
                        NULL,
                        pRequest,
                        pTsig,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecGssVerifyMic(
                        pGssSecCtx,
                        pMessage,
                        pTsig->Data.TSIG.pSignature,
                        &bIsVerified
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bIsVerified)
    {
        if (pGssSecCtx->pLastRecvSig)
        {
            VmDnsFreeBlob(pGssSecCtx->pLastRecvSig);
        }

        dwError = VmDnsCopyBlob(
                        pTsig->Data.TSIG.pSignature,
                        &pGssSecCtx->pLastRecvSig
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pbIsVerified = bIsVerified;


response:

    VmDnsFreeBlob(pMessage);
    VmDnsFreeBlob(pSignature);

    return dwError;

error:

    if (pbIsVerified)
    {
        *pbIsVerified = FALSE;
    }

    goto response;
}

BOOL
VmDnsSecIsRRTypeSec(
    DWORD dwRecordType
    )
{
    return  dwRecordType == VMDNS_RR_MTYPE_TKEY ||
            dwRecordType == VMDNS_RR_MTYPE_TSIG;
}


static
DWORD
VmDnsSecGssAcceptSecCtx(
    PVMDNS_RECORD               pReqTkey,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_RECORD               *ppRespTkey,
    PBOOL                       pbAuthSuccess
    )
{
    DWORD dwError = 0;
    PSTR pszKeyName = NULL;
    PSTR pszPrincipalName = NULL;
    PVMDNS_BLOB pOutputKey = NULL;
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    OM_uint32 major_status = 0;
    OM_uint32 minor_status = 0;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    gss_ctx_id_t gss_ctx_hdl = GSS_C_NO_CONTEXT;
    gss_name_t client_name = GSS_C_NO_NAME;
    PVMDNS_RECORD pRespTkey = NULL;
    BOOL bAuthSuccess = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pReqTkey, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRespTkey, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pbAuthSuccess, dwError);

    dwError = VmDnsStringToLower(pReqTkey->pszName, &pszKeyName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecBlobToGssBuffer(
                            pReqTkey->Data.TKEY.pKey,
                            &input_token
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    major_status = gss_accept_sec_context(
                            &minor_status,
                            &gss_ctx_hdl,
                            NULL,
                            &input_token,
                            NULL,
                            &client_name,
                            NULL,
                            &output_token,
                            NULL,
                            NULL,
                            NULL
                            );

    dwError = VmDnsSecGssGetPrincipalName(
                            client_name,
                            &pszPrincipalName
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (output_token.length)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                 "[%s,%s,%d]: GSS_DEBUG: Output token is non-null, creating TKEY with it",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__
                 );

        dwError = VmDnsSecGssBufferToBlob(
                            &output_token,
                            &pOutputKey
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSecCreateTkeyRR(
                            pszKeyName,
                            pOutputKey,
                            VM_DNS_RCODE_NOERROR,
                            &pRespTkey
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                 "[%s,%s,%d]: GSS_DEBUG: Output token is null, duplicate request TKEY",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__
                 );

        dwError = VmDnsDuplicateRecord(
                            pReqTkey,
                            &pRespTkey
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    switch(major_status)
    {
        case GSS_S_COMPLETE:
            VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                     "[%s,%s,%d]: GSS_ALERT: Accepted key, ctx established. Key name: %s Principal: %s",
                     __FILE__,
                     __FUNCTION__,
                     __LINE__,
                     pszKeyName,
                     pszPrincipalName
                     );

            dwError = VmDnsSecCreateGssCtx(
                                pszKeyName,
                                gss_ctx_hdl,
                                &pGssSecCtx
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            pGssSecCtx->state = VM_DNS_GSS_CTX_ESTABLISHED;

            dwError = VmDnsSecAddGssCtx(pGssSecCtx);
            BAIL_ON_VMDNS_ERROR(dwError);

            bAuthSuccess = TRUE;

            break;
        case GSS_S_CONTINUE_NEEDED:
            VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                     "[%s,%s,%d]: GSS_ALERT: GSS_S_Continue_Needed on accept. Key name: %s Principal: %s",
                     __FILE__,
                     __FUNCTION__,
                     __LINE__,
                     pszKeyName,
                     pszPrincipalName
                     );
            VmDnsSecGssDeleteSecCtx(&gss_ctx_hdl);

           break;
        case GSS_S_DEFECTIVE_TOKEN:
        case GSS_S_DEFECTIVE_CREDENTIAL:
        case GSS_S_BAD_SIG:
        case GSS_S_DUPLICATE_TOKEN:
        case GSS_S_OLD_TOKEN:
        case GSS_S_NO_CRED:
        case GSS_S_CREDENTIALS_EXPIRED:
        case GSS_S_BAD_BINDINGS:
        case GSS_S_NO_CONTEXT:
        case GSS_S_BAD_MECH:
        case GSS_S_FAILURE:
            VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                     "[%s,%s,%d]: GSS_ALERT: Cannot accept, TKEY error BADKEY. Key name: %s Principal: %s",
                     __FILE__,
                     __FUNCTION__,
                     __LINE__,
                     pszKeyName,
                     pszPrincipalName
                     );

            pRespTkey->Data.TKEY.wError = VM_DNS_RCODE_BADKEY;
            VmDnsSecGssDeleteSecCtx(&gss_ctx_hdl);

            break;
        default:
            VmDnsLog(VMDNS_LOG_LEVEL_ERROR,
                     "[%s,%s,%d]: GSS_ERROR: Unknown accept key error. No ctx. Key name: %s Principal: %s",
                     __FILE__,
                     __FUNCTION__,
                     __LINE__,
                     pszKeyName,
                     pszPrincipalName
                     );

            dwError = minor_status ? minor_status : major_status;
            BAIL_ON_VMDNS_ERROR(dwError);
            break;
    }

    *ppGssSecCtx = pGssSecCtx;
    *ppRespTkey = pRespTkey;
    *pbAuthSuccess = bAuthSuccess;


cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszPrincipalName);
    VmDnsFreeBlob(pOutputKey);
    VmDnsSecGssReleaseName(&client_name);

    return dwError;

error:

    VmDnsSecGssDeleteSecCtx(&gss_ctx_hdl);
    VmDnsClearRecord(pRespTkey);
    VMDNS_SAFE_FREE_MEMORY(pRespTkey);
    if (ppGssSecCtx)
    {
        *ppGssSecCtx = NULL;
    }
    if (ppRespTkey)
    {
        *ppRespTkey = NULL;
    }
    if (pbAuthSuccess)
    {
        *pbAuthSuccess = FALSE;
    }

    goto cleanup;
}

static
VOID
VmDnsSecGssDeleteSecCtx(
    gss_ctx_id_t    *gss_ctx_hdl
    )
{
    OM_uint32 minor_status = 0;

    gss_delete_sec_context(
                &minor_status,
                gss_ctx_hdl,
                GSS_C_NO_BUFFER
                );
}

static
DWORD
VmDnsSecGssCheckSecCtxExpired(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PBOOL                       pbExpired
    )
{
    DWORD dwError = 0;
    BOOL bExpired = FALSE;
    OM_uint32 major_status = 0;
    OM_uint32 minor_status = 0;
    OM_uint32 time_rec = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);

    major_status = gss_context_time(
                        &minor_status,
                        pGssSecCtx->gss_ctx_hdl,
                        &time_rec
                        );

    switch (major_status)
    {
        case GSS_S_COMPLETE:
            bExpired = FALSE;
            break;
        case GSS_S_CONTEXT_EXPIRED:
            bExpired = TRUE;
            break;
        case GSS_S_NO_CONTEXT:
        default:
            dwError = minor_status ? minor_status : major_status;
            BAIL_ON_VMDNS_ERROR(dwError);
            break;
    }

    *pbExpired = bExpired;


cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsSecGssGetMic(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_BLOB                 pMessage,
    PVMDNS_BLOB                 *ppSignature
    )
{
    DWORD dwError = 0;
    OM_uint32 major_status = 0;
    OM_uint32 minor_status = 0;
    gss_buffer_desc message = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc signature = GSS_C_EMPTY_BUFFER;
    PVMDNS_BLOB pSignature = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppSignature, dwError);

    dwError = VmDnsSecBlobToGssBuffer(
                            pMessage,
                            &message
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    major_status = gss_get_mic(
                        &minor_status,
                        pGssSecCtx->gss_ctx_hdl,
                        GSS_C_QOP_DEFAULT,
                        &message,
                        &signature
                        );

    if (major_status == GSS_S_CONTEXT_EXPIRED ||
        major_status == GSS_S_NO_CONTEXT)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Failed signature creation with expired or no context. Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pGssSecCtx->pszKeyName,
                 major_status,
                 minor_status
                 );

        dwError = VmDnsSecDeleteGssCtx(pGssSecCtx->pszKeyName);
        goto error;
    }

    if (major_status == GSS_S_COMPLETE)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                     "[%s,%s,%d]: GSS_ALERT: Signature created. Key name: %s",
                     __FILE__,
                     __FUNCTION__,
                     __LINE__,
                     pGssSecCtx->pszKeyName
                     );

        dwError = VmDnsSecGssBufferToBlob(
                            &signature,
                            &pSignature
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

    }

    *ppSignature = pSignature;


cleanup:

    return dwError;

error:

    VmDnsFreeBlob(pSignature);
    if (ppSignature)
    {
        *ppSignature = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecGssVerifyMic(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_BLOB                 pMessage,
    PVMDNS_BLOB                 pSignature,
    PBOOL                       pbIsVerified
    )
{
    DWORD dwError = 0;
    OM_uint32 major_status = 0;
    OM_uint32 minor_status = 0;
    gss_buffer_desc message = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc signature = GSS_C_EMPTY_BUFFER;
    BOOL bIsVerified = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pGssSecCtx, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pSignature, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pbIsVerified, dwError);

    dwError = VmDnsSecBlobToGssBuffer(
                            pMessage,
                            &message
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecBlobToGssBuffer(
                            pSignature,
                            &signature
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    major_status = gss_verify_mic(
                        &minor_status,
                        pGssSecCtx->gss_ctx_hdl,
                        &message,
                        &signature,
                        NULL
                        );

    if (major_status == GSS_S_COMPLETE)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                 "[%s,%s,%d]: GSS_DEBUG: Verified signature. Key name: %s",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pGssSecCtx->pszKeyName
                 );

        bIsVerified = TRUE;
    }
    else
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ALERT,
                 "[%s,%s,%d]: GSS_ALERT: Failed to verify signature. Key name: %s.  Routine Error %u",
                 __FILE__,
                 __FUNCTION__,
                 __LINE__,
                 pGssSecCtx->pszKeyName,
                 GSS_ROUTINE_ERROR(major_status)
                 );

        bIsVerified = FALSE;
    }

    *pbIsVerified = bIsVerified;


cleanup:

    return dwError;

error:

    if (pbIsVerified)
    {
        *pbIsVerified = FALSE;
    }
    goto cleanup;
}

static
VOID
VmDnsSecGssReleaseBuffer(
    gss_buffer_t    buffer
    )
{
    OM_uint32 minor_status = 0;

    gss_release_buffer(
            &minor_status,
            buffer
            );
}

static
VOID
VmDnsSecGssReleaseName(
    gss_name_t  *name
    )
{
    OM_uint32 minor_status = 0;

    gss_release_name(
            &minor_status,
            name
            );
}

static
DWORD
VmDnsSecGssGetPrincipalName(
    gss_name_t  client_name,
    PSTR        *ppszPrincipalName
    )
{
    DWORD dwError = 0;
    OM_uint32 major_status = 0;
    OM_uint32 minor_status = 0;
    gss_buffer_desc display_name = GSS_C_EMPTY_BUFFER;
    PSTR pszPrincipalName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppszPrincipalName, dwError);

    major_status = gss_display_name(
                    &minor_status,
                    client_name,
                    &display_name,
                    NULL
                    );
    if (major_status)
    {
        dwError = minor_status ? minor_status : major_status;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                        (char *)display_name.value,
                        &pszPrincipalName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszPrincipalName = pszPrincipalName;


cleanup:

    VmDnsSecGssReleaseBuffer(&display_name);

    return dwError;

error:

    VMDNS_SAFE_FREE_STRINGA(pszPrincipalName);
    if (ppszPrincipalName)
    {
        *ppszPrincipalName = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecCreateTkeyRR(
    PCSTR           pszKeyName,
    PVMDNS_BLOB     pKey,
    VM_DNS_RCODE    unRCode,
    PVMDNS_RECORD   *ppOutTkey
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pOutTkey = NULL;
    PVMDNS_TKEY_DATAA pTkeyData = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszKeyName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pKey, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppOutTkey, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD),
                        (PVOID*)&pOutTkey
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pTkeyData = &pOutTkey->Data.TKEY;

    dwError = VmDnsAllocateStringA(
                        pszKeyName,
                        &pOutTkey->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pOutTkey->dwType = VMDNS_RR_MTYPE_TKEY;
    pOutTkey->iClass = VMDNS_CLASS_ANY;
    pOutTkey->dwTtl = 0;

    dwError = VmDnsAllocateStringA(
                        VMDNS_SEC_ALG_NAME,
                        &pTkeyData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pKey)
    {
        dwError = VmDnsCopyBlob(
                        pKey,
                        &pTkeyData->pKey
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsAllocateBlob(
                        0,
                        &pTkeyData->pKey
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBlob(
                    0,
                    &pTkeyData->pOtherData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pTkeyData->wMode = VM_DNS_TKEY_MODE_GSS_API;
    pTkeyData->wError = unRCode;

    *ppOutTkey = pOutTkey;


cleanup:

    return dwError;

error:

    VmDnsClearRecord(pOutTkey);
    VMDNS_SAFE_FREE_MEMORY(pTkeyData);
    VMDNS_SAFE_FREE_MEMORY(pOutTkey);
    if (ppOutTkey)
    {
        *ppOutTkey = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecCreateTsigRR(
    PCSTR           pszKeyName,
    PVMDNS_BLOB     pSignature,
    UINT16          wOriginalXid,
    VM_DNS_RCODE    unRCode,
    PVMDNS_RECORD   *ppOutTsig
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pOutTsig = NULL;
    PVMDNS_TSIG_DATAA pTsigData = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszKeyName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppOutTsig, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD),
                        (PVOID*)&pOutTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pTsigData = &pOutTsig->Data.TSIG;

    dwError = VmDnsAllocateStringA(
                        pszKeyName,
                        &pOutTsig->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pOutTsig->dwType = VMDNS_RR_MTYPE_TSIG;
    pOutTsig->iClass = VMDNS_CLASS_ANY;
    pOutTsig->dwTtl = 0;

    dwError = VmDnsAllocateStringA(
                        VMDNS_SEC_ALG_NAME,
                        &pTsigData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pTsigData->unCreateTime = time(NULL);
    pTsigData->wFudgeTime = VMDNS_SEC_DEFAULT_FUDGE_TIME;

    if (pSignature)
    {
        dwError = VmDnsCopyBlob(
                        pSignature,
                        &pTsigData->pSignature
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsAllocateBlob(
                            0,
                            &pTsigData->pSignature
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pTsigData->wOriginalXid = wOriginalXid;
    pTsigData->wError = unRCode;

    dwError = VmDnsAllocateBlob(
                        0,
                        &pTsigData->pOtherData
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppOutTsig = pOutTsig;


cleanup:

    return dwError;

error:

    VmDnsClearRecord(pOutTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsigData);
    VMDNS_SAFE_FREE_MEMORY(pOutTsig);
    if (ppOutTsig)
    {
        *ppOutTsig = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecAppendToRecordArray(
    DWORD           dwSize,
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   **pppRecords
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    PVMDNS_RECORD *ppRecords = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppRecords, dwError);

    dwCount = dwSize + 1;

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD) * dwCount,
                        (PVOID*)&ppRecords
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < dwCount; ++dwIndex)
    {
        if (*pppRecords && dwIndex < dwSize)
        {
            dwError = VmDnsDuplicateRecord(
                                *pppRecords[dwIndex],
                                &ppRecords[dwIndex]
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        if (dwIndex == dwSize)
        {
            dwError = VmDnsDuplicateRecord(
                                pRecord,
                                &ppRecords[dwIndex]
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    VmDnsFreeRecordsArray(*pppRecords, dwSize);

    *pppRecords = ppRecords;


cleanup:

    return dwError;

error:

    VmDnsFreeRecordsArray(ppRecords, dwSize + 1);

    goto cleanup;
}

static
DWORD
VmDnsSecStripTsigFromMessage(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_RECORD   *ppTsig
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pTsig = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppTsig, dwError);

    dwError = VmDnsDuplicateRecord(
                        pRequest->pAdditional[pRequest->pHeader->usARCount - 1],
                        &pTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsClearRecord(pRequest->pAdditional[pRequest->pHeader->usARCount - 1]);
    VMDNS_SAFE_FREE_MEMORY(pRequest->pAdditional[pRequest->pHeader->usARCount - 1]);
    pRequest->pHeader->usARCount--;

    *ppTsig = pTsig;


cleanup:

    return dwError;

error:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);
    if (ppTsig)
    {
        *ppTsig = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecStripTsigFromUpdateMessage(
    PVMDNS_UPDATE_MESSAGE   pRequest,
    PVMDNS_RECORD           *ppTsig
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pTsig = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRequest, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppTsig, dwError);

    dwError = VmDnsDuplicateRecord(
                        pRequest->pAdditional[pRequest->pHeader->usADCount - 1],
                        &pTsig
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsClearRecord(pRequest->pAdditional[pRequest->pHeader->usADCount - 1]);
    VMDNS_SAFE_FREE_MEMORY(pRequest->pAdditional[pRequest->pHeader->usADCount - 1]);
    pRequest->pHeader->usADCount--;

    *ppTsig = pTsig;


cleanup:

    return dwError;

error:

    VmDnsClearRecord(pTsig);
    VMDNS_SAFE_FREE_MEMORY(pTsig);
    if (ppTsig)
    {
        *ppTsig = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecSerializeTsigVariable(
    PVMDNS_RECORD           pTsig,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    )
{
    DWORD dwError = 0;
    UINT64 unTsigCombinedTimes = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pTsig, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pMessageBuffer, dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                            pTsig->pszName,
                            pMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                            pTsig->iClass,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                            pTsig->dwTtl,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                            pTsig->Data.TSIG.pNameAlgorithm,
                            pMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    unTsigCombinedTimes = VMDNS_TSIG_COMBINE_TIMES(
                                    pTsig->Data.TSIG.unCreateTime,
                                    pTsig->Data.TSIG.wFudgeTime
                                    );

    dwError = VmDnsWriteUINT64ToBuffer(
                            unTsigCombinedTimes,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                            pTsig->Data.TSIG.wError,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteBlobToBuffer(
                            pTsig->Data.TSIG.pOtherData,
                            TRUE,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsSecCreateMacMessage(
    PVMDNS_BLOB             pRequestSig,
    PVMDNS_MESSAGE          pDnsMessage,
    PVMDNS_RECORD           pTsig,
    PVMDNS_BLOB             *ppMessage
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    PVMDNS_MESSAGE_BUFFER pMessageBuffer = NULL;
    PVMDNS_BLOB pMessage = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pTsig, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppMessage, dwError);

    dwError = VmDnsAllocateBufferStream(
                            0,
                            &pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSetBufferTokenizedFlag(
                            pMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pRequestSig)
    {
        dwError = VmDnsWriteBlobToBuffer(
                            pRequestSig,
                            TRUE,
                            pMessageBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDnsHeaderToBuffer(
                        pDnsMessage->pHeader,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsMessage->pRawDnsMessage)
    {
        dwError = VmDnsWriteBlobToBuffer(
                            pDnsMessage->pRawDnsMessage,
                            FALSE,
                            pMessageBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsWriteQueryMessageToBuffer(
                                pDnsMessage,
                                pMessageBuffer
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSecSerializeTsigVariable(
                            pTsig,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pMessageBuffer,
                            NULL,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateBlob(
                        dwSize,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pMessageBuffer,
                            pMessage->pData,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppMessage = pMessage;

cleanup:

    VmDnsFreeBufferStream(pMessageBuffer);
    return dwError;

error:

    VmDnsFreeBlob(pMessage);
    if (ppMessage)
    {
        *ppMessage = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecCreateMacUpdateMessage(
    PVMDNS_BLOB             pRequestSig,
    PVMDNS_UPDATE_MESSAGE   pDnsMessage,
    PVMDNS_RECORD           pTsig,
    PVMDNS_BLOB             *ppMessage
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    PVMDNS_MESSAGE_BUFFER pMessageBuffer = NULL;
    PVMDNS_BLOB pMessage = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pTsig, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppMessage, dwError);

    dwError = VmDnsAllocateBufferStream(
                            0,
                            &pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSetBufferTokenizedFlag(
                            pMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pRequestSig)
    {
        dwError = VmDnsWriteBlobToBuffer(
                            pRequestSig,
                            TRUE,
                            pMessageBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDnsHeaderToBuffer(
                        pDnsMessage->pHeader,
                        pMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsMessage->pRawDnsMessage)
    {
        dwError = VmDnsWriteBlobToBuffer(
                            pDnsMessage->pRawDnsMessage,
                            FALSE,
                            pMessageBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsWriteUpdateMessageToBuffer(
                                pDnsMessage,
                                pMessageBuffer
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSecSerializeTsigVariable(
                            pTsig,
                            pMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pMessageBuffer,
                            NULL,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateBlob(
                        dwSize,
                        &pMessage
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pMessageBuffer,
                            pMessage->pData,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppMessage = pMessage;

cleanup:

    VmDnsFreeBufferStream(pMessageBuffer);
    return dwError;

error:

    VmDnsFreeBlob(pMessage);
    if (ppMessage)
    {
        *ppMessage = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecBlobToGssBuffer(
    PVMDNS_BLOB pBlob,
    gss_buffer_t gss_buffer_out
    )
{
    DWORD dwError = 0;
    gss_buffer_desc gss_buffer = GSS_C_EMPTY_BUFFER;

    BAIL_ON_VMDNS_INVALID_POINTER(pBlob, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(gss_buffer_out, dwError);

    gss_buffer.length = (size_t)pBlob->unSize;
    gss_buffer.value = (PVOID)pBlob->pData;

    *gss_buffer_out = gss_buffer;


cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsSecGssBufferToBlob(
    gss_buffer_t gss_buffer,
    PVMDNS_BLOB *ppBlob
    )
{
    DWORD dwError = 0;
    PVMDNS_BLOB pBlob = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(gss_buffer, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppBlob, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_BLOB),
                        (PVOID)&pBlob
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pBlob->unSize = gss_buffer->length;
    pBlob->pData = (PBYTE)gss_buffer->value;

    *ppBlob = pBlob;


cleanup:

    return dwError;

error:

    VMDNS_SAFE_FREE_MEMORY(pBlob);
    if (ppBlob)
    {
        *ppBlob = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDnsSecFQDNToShort(
    PCSTR   pszSrc,
    PSTR    *ppszDst
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    PSTR pszDst = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pszSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszDst, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszSrc, dwError);

    dwSize = VmDnsStringLenA(pszSrc);

    dwError = VmDnsAllocateMemory(
                        dwSize,
                        (PVOID*)&pszDst
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                    pszDst,
                    dwSize,
                    pszSrc,
                    dwSize - 1
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszDst = pszDst;


cleanup:

    return dwError;

error:

    VMDNS_SAFE_FREE_STRINGA(pszDst);
    if (ppszDst)
    {
        *ppszDst = NULL;
    }

    goto cleanup;
}
