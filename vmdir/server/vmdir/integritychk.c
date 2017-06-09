/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Integrity Checking Thread
 *
 */

#include "includes.h"

static
DWORD
_VmDirIntegrityCheckingThreadFun(
    PVOID pArg
    );

static
DWORD
_VmDirIntegrityCheckJobStart(
    PVMDIR_INTEGRITY_JOB pJob
    );

static
DWORD
_VmDirIntegrityCheckJobRecheck(
    PVMDIR_INTEGRITY_JOB    pJob
    );

static
VOID
_VmDirIntegrityCheckEntry(
    PVMDIR_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY             pEntry
    );

static
VOID
_VmDirIntegrityCheckFreeJobResource(
    PVMDIR_INTEGRITY_JOB    pJob
    );

static
VOID
_VmDirIntegrityCheckFreeJobContent(
    PVMDIR_INTEGRITY_JOB    pJob
    );

static
DWORD
_VmDirInitIntegrityCheckThread(
    PVMDIR_INTEGRITY_JOB    pGlobalJob
    );

static
DWORD
_VmDirIntegrityCheckComposeStatus(
    PVMDIR_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY*            ppEntry
    );

/*
 *  Job state machine                          | -------> STOP
 *                            | -----> START   | -------> INVALID
 *  NONE -----> START -----> FINISH -----> RECHECK -----> FINISH
 *                |--------> INVALID
 *                |--------> STOP -------> INVALID -----> START
 *
 */
DWORD
VmDirIntegrityCheckStart(
    VMDIR_INTEGRITY_CHECK_JOB_STATE jobState
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;
    CHAR        finishedTimebuf[MAX_PATH] = {0};

    VMDIR_LOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    if (gVmdirIntegrityCheck.pJob == NULL)
    {
        dwError = VmDirAllocateMemory(sizeof(VMDIR_INTEGRITY_JOB), (PVOID*)&gVmdirIntegrityCheck.pJob);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // bail if not in running state
    if (gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_START   ||
        gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_RECHECK
       )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    if (jobState != INTEGRITY_CHECK_JOB_START   &&
        jobState != INTEGRITY_CHECK_JOB_RECHECK
       )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    // bail if recheck w/o finished job
    if (jobState == INTEGRITY_CHECK_JOB_RECHECK &&
        gVmdirIntegrityCheck.pJob->state != INTEGRITY_CHECK_JOB_FINISH
       )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    dwError = VmDirCopyMemory(finishedTimebuf, MAX_PATH, gVmdirIntegrityCheck.pJob->finishedTimebuf, MAX_PATH);
    BAIL_ON_VMDIR_ERROR(dwError);

    _VmDirIntegrityCheckFreeJobContent(gVmdirIntegrityCheck.pJob);

    dwError = VmDirCopyMemory(gVmdirIntegrityCheck.pJob->finishedTimebuf, MAX_PATH, finishedTimebuf, MAX_PATH);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmdirIntegrityCheck.pJob->state = jobState;
    _VmDirInitIntegrityCheckThread(gVmdirIntegrityCheck.pJob);

error:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    return dwError;
}

VOID
VmDirIntegrityCheckStop(
    VOID
    )
{
    BOOLEAN     bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    if (gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_START ||
        gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_RECHECK
       )
    {
        gVmdirIntegrityCheck.pJob->state = INTEGRITY_CHECK_JOB_STOP;
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    return;
}

DWORD
VmDirIntegrityCheckShowStatus(
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;

    if (!ppEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    if ( gVmdirIntegrityCheck.pJob &&
         ( gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_START    ||
           gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_RECHECK  ||
           gVmdirIntegrityCheck.pJob->state == INTEGRITY_CHECK_JOB_FINISH
         )
       )
    {
        // still running or finished, compose status
        dwError = _VmDirIntegrityCheckComposeStatus(gVmdirIntegrityCheck.pJob, ppEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirIntegrityCheck.pMutex);

    return dwError;
}


/*
 * Generate entry digest
 * Only cover application attributes for now.
 *
 * TODO: We would have a way to cover replication related meta data to ensure replication algorithm integrity as well.
 */
DWORD
VmDirEntrySHA1Digest(
    PVDIR_ENTRY pEntry,
    PSTR        pOutSH1DigestBuf
    )
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    DWORD           dwSize = 0;
    DWORD           dwSizeLimit = 128;
    unsigned        dwCnt = 0;
    SHA_CTX         shaCtx = {0};
    char            sha1Digest[SHA_DIGEST_LENGTH] = {0};
    VDIR_BERVALUE** ppVdirBV = NULL;


    if (!pEntry || !pOutSH1DigestBuf)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(PVDIR_BERVALUE)*dwSizeLimit, (PVOID)&ppVdirBV);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        for (dwCnt = 0;
            pAttr->pATDesc->usage == VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE && dwCnt < pAttr->numVals;
            dwCnt++)
        {
            if (dwSize+1 == dwSizeLimit)
            {
                dwSizeLimit *= 2;
                dwError = VmDirReallocateMemoryWithInit(
                                ppVdirBV,
                                (PVOID*)&ppVdirBV,
                                (dwSizeLimit) * sizeof(PVDIR_BERVALUE),
                                (dwSizeLimit/2) * sizeof(PVDIR_BERVALUE));
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            ppVdirBV[dwSize++] = pAttr->vals+dwCnt;
        }
    }

    // compact entry structure may not be in exact order across nodes, sort values before digesting.
    qsort(ppVdirBV, dwSize, sizeof(PVDIR_BERVALUE), VmDirPVdirBValCmp);

    SHA1_Init(&shaCtx);

    for (dwCnt = 0; dwCnt < dwSize; dwCnt++)
    {
        SHA1_Update(&shaCtx, ppVdirBV[dwCnt]->lberbv_val, ppVdirBV[dwCnt]->lberbv_len);
    }

    SHA1_Final(sha1Digest, &shaCtx);
    memcpy(pOutSH1DigestBuf, sha1Digest, SHA_DIGEST_LENGTH);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppVdirBV);

    return dwError;

error:
    goto cleanup;;
}

static
DWORD
_VmDirInitIntegrityCheckThread(
    PVMDIR_INTEGRITY_JOB    pGlobalJob
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirIntegrityCheckingThreadFun,
                pGlobalJob);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:

    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

static
VOID
_VmDirIntegrityCheckEntry(
    PVMDIR_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY             pEntry
    )
{
    DWORD           dwError = 0;
    LDAPMessage*    pSearchRes = NULL;
    LDAPControl     digestCtl = {0};
    LDAPControl*    srvCtrls[2] = {&digestCtl, NULL};
    CHAR            sha1Digest[SHA_DIGEST_LENGTH+1] = {0};
    DWORD           dwNodeCnt = 0;

    // TODO, should ignore server objects and replication agreements entries ???
    // or we specifically ignore some attributes (user application?)

    memset(sha1Digest, 0, SHA_DIGEST_LENGTH);
    VmDirEntrySHA1Digest(pEntry, sha1Digest); // ignore error

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "%s, iterate (%llu)(%020s)(%s)",
                       __FUNCTION__, pEntry->eId, sha1Digest, pEntry->dn.lberbv.bv_val);

    memset(&digestCtl, 0, sizeof(digestCtl));
    dwError = VmDirCreateDigestControlContent(sha1Digest, SHA_DIGEST_LENGTH, &digestCtl);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwNodeCnt=0; dwNodeCnt < pJob->dwNumJobCtx; dwNodeCnt++)
    {
        PVMDIR_INTEGRITY_JOB_CTX    pJobCtx = pJob->pJobctx+dwNodeCnt;

        if ( pJobCtx->state != INTEGRITY_CHECK_JOBCTX_VALID )
        {
            continue;
        }

        ldap_msgfree(pSearchRes);
        pSearchRes = NULL;

        dwError = ldap_search_ext_s(
                    pJobCtx->pLd,
                    pEntry->dn.lberbv_val,
                    LDAP_SCOPE_BASE,
                    NULL,
                    NULL,
                    TRUE,
                    srvCtrls, // digest control
                    NULL,
                    NULL,
                    1,
                    &pSearchRes
                    );

        if (dwError == 0)
        {
            if ( ldap_count_entries(pJobCtx->pLd, pSearchRes) > 0 )
            {
                // digest mismatch, partner send entry content back.
                pJobCtx->dwFailedDigestCnt++;

                if (fprintf(pJobCtx->fp, "I %s\n", pEntry->dn.lberbv_val) < 0)
                {
                    pJobCtx->state = INTEGRITY_CHECK_JOBCTX_ABORT;
                }

                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "%s, %s digest mismatch",
                                       __FUNCTION__, pJobCtx->pszPartnerName);
                // TODO, identify out of sync attributes/values.
            }
        }
        else if (dwError == LDAP_NO_SUCH_OBJECT)
        {
            pJobCtx->dwMissedEntryCnt++;

            if (fprintf(pJobCtx->fp, "M %s\n", pEntry->dn.lberbv_val) < 0)
            {
                pJobCtx->state = INTEGRITY_CHECK_JOBCTX_ABORT;
            }

            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "%s, %s entry missing",
                                                       __FUNCTION__, pJobCtx->pszPartnerName);
        }
        else if (dwError == LDAP_SERVER_DOWN)
        {
            pJobCtx->state = INTEGRITY_CHECK_JOBCTX_ABORT;
        }
        else
        {
            VMDIR_LOG_WARNING( LDAP_DEBUG_TRACE,"%s failed, error (%d)", __FUNCTION__, dwError );
        }
    }

cleanup:
    ldap_msgfree(pSearchRes);
    VmDirDeleteDigestControlContent(&digestCtl);

    return;

error:
    goto cleanup;
}

static
DWORD
_VmDirIntegrityCheckJobStart(
    PVMDIR_INTEGRITY_JOB    pJob
    )
{
    DWORD       dwError = 0;
    ENTRYID     eId = 0;
    VDIR_ENTRY  entry = {0};
    PVDIR_BACKEND_INTERFACE             pBE = NULL;
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR    pIterator = NULL;

    pBE = VmDirBackendSelect(NULL);

    dwError = pBE->pfnBEEntryBlobIteratorInit(0, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pJob->maxEntryID = pIterator->maxEID;

    while (pIterator->bHasNext)
    {
        dwError = pBE->pfnBEEntryBlobIterate(pIterator, &eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (++pJob->dwNumProcessed % VDIR_INTEGRITY_CHECK_BATCH == 0) // reset txn per 1000 iteration
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN || (pJob->state != INTEGRITY_CHECK_JOB_START))
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
            }

            pBE->pfnBEEntryBlobIteratorFree(pIterator);
            dwError = pBE->pfnBEEntryBlobIteratorInit(eId, &pIterator);
            BAIL_ON_VMDIR_ERROR(dwError);

            pJob->maxEntryID = pIterator->maxEID;

            continue;
        }

        pJob->currentEntryID = eId;

        dwError = pBE->pfnBESimpleIdToEntry(eId, &entry);
        if (dwError != 0)
        {
            //
            // We have seen instances in the wild where this call fails due
            // to a bad entry, so let's keep going if this fails.
            //
            dwError = 0;
            continue;
        }

        _VmDirIntegrityCheckEntry(pJob, &entry);

        VmDirFreeEntryContent(&entry);
    }

cleanup:

    pBE->pfnBEEntryBlobIteratorFree(pIterator);
    VmDirFreeEntryContent(&entry);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,"%s failed, error (%d)(%d)", __FUNCTION__, dwError, pJob->state );

    goto cleanup;
}

static
DWORD
_VmDirIntegrityCheckJobRecheck(
    PVMDIR_INTEGRITY_JOB    pJob
    )
{
    DWORD       dwError = 0;
    DWORD       dwCnt = 9;
    FILE*       fp = NULL;
    CHAR        fileNameBuf[MAX_PATH] = {0};
    CHAR        lineBuf[VMDIR_MAX_DN_LEN + 3] = {0};
    PVDIR_ENTRY pEntry = NULL;

    for (dwCnt=0; dwCnt < pJob->dwNumJobCtx; dwCnt++)
    {
        if (pJob->pJobctx[dwCnt].state != INTEGRITY_CHECK_JOBCTX_VALID)
        {
            continue;
        }

        memset(fileNameBuf, 0, sizeof(fileNameBuf));

        dwError = VmDirStringPrintFA(fileNameBuf, sizeof(fileNameBuf) - 1,
                    "%s/Integrity.%s.%s",
                    VMDIR_LOG_DIR,
                    pJob->pJobctx[dwCnt].pszPartnerName,
                    pJob->finishedTimebuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (fp)
        {
            fclose(fp);
            fp = NULL;
        }

        if (!(fp = fopen(fileNameBuf, "r")))
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s open file %s failed %d",
                             __FUNCTION__, fileNameBuf, errno);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s open file %s", __FUNCTION__, fileNameBuf);

        while (fgets(lineBuf, sizeof(lineBuf), fp))
        {
            size_t len = VmDirStringLenA(lineBuf) - 1;
            if (lineBuf[len] == '\n')
            {
                lineBuf[len] = '\0';
            }

            VmDirFreeEntry(pEntry);
            pEntry = NULL;

            dwError = VmDirSimpleDNToEntry(lineBuf+2, &pEntry); // skip first two chars "I|M " for DN
            if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
            {
                dwError = 0;
                continue;
            }
            BAIL_ON_VMDIR_ERROR(dwError);

            ++pJob->dwNumProcessed;
            _VmDirIntegrityCheckEntry(pJob, pEntry);
        }
    }

cleanup:
    if (fp)
    {
        fclose(fp);
    }

    VmDirFreeEntry(pEntry);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,"%s failed, error (%d)(%d)", __FUNCTION__, dwError, pJob->state );

    goto cleanup;
}

/*
 * job thread to handle running job (start|recheck)
 */
static
DWORD
_VmDirIntegrityCheckingThreadFun(
    PVOID pArg
    )
{
    DWORD   dwError = 0;
    DWORD   dwValidJobs = 0;
    PSTR    pszDCAccount = NULL;
    PSTR    pszDCPasswd = NULL;
    PSTR    pszDCUPN = NULL;
    PSTR*   ppszServerInfo = NULL;
    size_t  dwInfoCount = 0;
    CHAR    timeBuf[MAX_PATH] = {0};
    PVMDIR_INTEGRITY_JOB    pJob = (PVMDIR_INTEGRITY_JOB) pArg;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s begin integrity check %s job.",
                    __FUNCTION__,
                    pJob->state == INTEGRITY_CHECK_JOB_START ? "start" : "recheck");

    VmDirDropThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    /*
     * TODO, lower thread priority.  Pending PR 1860315
     */

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pszDCPasswd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(gVmdirServerGlobals.dcAccountUPN.lberbv.bv_val, &pszDCUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetHostsInternal(&ppszServerInfo, &dwInfoCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwInfoCount<2)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Domain Controllers count < 2. skip integrity job request.");
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    clock_gettime(CLOCK_REALTIME, &pJob->startTime);
    gmtime_r(&pJob->startTime.tv_sec, &pJob->startTM);

    snprintf(timeBuf, sizeof(timeBuf) - 1,
            "%4d%02d%02d%02d%02d%02d",
            pJob->startTM.tm_year+1900,
            pJob->startTM.tm_mon+1,
            pJob->startTM.tm_mday,
            pJob->startTM.tm_hour,
            pJob->startTM.tm_min,
            pJob->startTM.tm_sec);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_INTEGRITY_JOB_CTX)*dwInfoCount, (PVOID)&pJob->pJobctx);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pJob->dwNumJobCtx=0 ; pJob->dwNumJobCtx<dwInfoCount; pJob->dwNumJobCtx++)
    {
        if (VmDirStringCompareA(ppszServerInfo[pJob->dwNumJobCtx], pszDCAccount, FALSE) == 0)
        {
            pJob->pJobctx[pJob->dwNumJobCtx].state = INTEGRITY_CHECK_JOBCTX_SKIP;
            continue;
        }

        pJob->pJobctx[pJob->dwNumJobCtx].state = INTEGRITY_CHECK_JOBCTX_VALID;

        dwError = VmDirAllocateStringA(ppszServerInfo[pJob->dwNumJobCtx], &pJob->pJobctx[pJob->dwNumJobCtx].pszPartnerName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSafeLDAPBind(
                    &pJob->pJobctx[pJob->dwNumJobCtx].pLd,
                    pJob->pJobctx[pJob->dwNumJobCtx].pszPartnerName,
                    pszDCUPN,
                    pszDCPasswd);
        if (dwError)
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "%s failed to connect to DC %s, %d",
                               __FUNCTION__, pJob->pJobctx[pJob->dwNumJobCtx].pszPartnerName, dwError);

            pJob->pJobctx[pJob->dwNumJobCtx].state = INTEGRITY_CHECK_JOBCTX_INVALID;
            continue;
        }

        dwError = VmDirAllocateStringPrintf(
                    &(pJob->pJobctx[pJob->dwNumJobCtx].pszRptFileName),
                    "%s/Integrity.%s.%s",
                    VMDIR_LOG_DIR,
                    pJob->pJobctx[pJob->dwNumJobCtx].pszPartnerName,
                    timeBuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        pJob->pJobctx[pJob->dwNumJobCtx].fp = fopen(pJob->pJobctx[pJob->dwNumJobCtx].pszRptFileName, "w+");
        if (!pJob->pJobctx[pJob->dwNumJobCtx].fp)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s open file %s failed %d",
                             __FUNCTION__, pJob->pJobctx[pJob->dwNumJobCtx].pszPartnerName, errno);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }

        dwValidJobs++;
    }

    if (dwValidJobs > 0)
    {
        if (pJob->state == INTEGRITY_CHECK_JOB_START)
        {
            dwError = _VmDirIntegrityCheckJobStart(pJob);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (pJob->state == INTEGRITY_CHECK_JOB_RECHECK)
        {
            dwError = _VmDirIntegrityCheckJobRecheck(pJob);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
        }

        pJob->state = INTEGRITY_CHECK_JOB_FINISH;
        clock_gettime(CLOCK_REALTIME, &pJob->endTime);

        dwError = VmDirCopyMemory(pJob->finishedTimebuf, MAX_PATH, timeBuf, MAX_PATH);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s end integrity check job, %d entries processed", __FUNCTION__, pJob->dwNumProcessed);

cleanup:
    _VmDirIntegrityCheckFreeJobResource(pJob);
    VmDirFreeCountedStringArray(ppszServerInfo, dwInfoCount);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszDCPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszDCUPN);

    return dwError;

error:

    pJob->state = INTEGRITY_CHECK_JOB_INVALID;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s integrity check job failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

static
VOID
_VmDirIntegrityCheckFreeJobResource(
    PVMDIR_INTEGRITY_JOB    pJob
    )
{
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt < pJob->dwNumJobCtx; dwCnt++)
    {
        if (pJob->pJobctx[dwCnt].pLd)
        {
            ldap_unbind_ext_s(pJob->pJobctx[dwCnt].pLd, NULL, NULL);
            pJob->pJobctx[dwCnt].pLd = NULL;
        }

        if (pJob->pJobctx[dwCnt].fp)
        {
            fclose(pJob->pJobctx[dwCnt].fp);
            pJob->pJobctx[dwCnt].fp = NULL;
        }
    }
}

static
VOID
_VmDirIntegrityCheckFreeJobContent(
    PVMDIR_INTEGRITY_JOB    pJob
    )
{
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt < pJob->dwNumJobCtx; dwCnt++)
    {
        if (pJob->pJobctx[dwCnt].pLd)
        {
            ldap_unbind_ext_s(pJob->pJobctx[dwCnt].pLd, NULL, NULL);
        }

        if (pJob->pJobctx[dwCnt].fp)
        {
            fclose(pJob->pJobctx[dwCnt].fp);
        }

        VMDIR_SAFE_FREE_MEMORY(pJob->pJobctx[dwCnt].pszPartnerName);
        VMDIR_SAFE_FREE_MEMORY(pJob->pJobctx[dwCnt].pszRptFileName);
    }

    VMDIR_SAFE_FREE_MEMORY(pJob->pJobctx);
    memset(pJob, 0 , sizeof(*pJob));
}

static
DWORD
_VmDirIntegrityCheckComposeStatus(
    PVMDIR_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY*            ppEntry
    )
{
    DWORD               dwError = 0;
    DWORD               dwNumAttrs = 0;
    PSTR*               ppszAttrList = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    backendCtx = {0};
    DWORD               dwCnt = 0, dwGoodPartner = 0, dwIdx = 0;
    struct timespec     timeNow = {0};
    struct tm           myTM = {0};
    DWORD               dwElapseSec = 0;
    DWORD               dwRemainingSec = 0;
    PSTR                pszStartTime = NULL;
    PSTR                pszEndTime = NULL;
    PSTR                pszEstimatedEndTime = NULL;
    PSTR                pszEntryProcessed = NULL;
    PSTR*               ppszPartner = NULL;

    dwNumAttrs = 2 + // cn/oc
                 3 + // start/end/remaining time
                 1 + // processed cnt
                 pJob->dwNumJobCtx;

    dwError = VmDirAllocateStringPrintf(&pszStartTime,
                "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
                "start time: ",
                pJob->startTM.tm_year+1900,
                pJob->startTM.tm_mon+1,
                pJob->startTM.tm_mday,
                pJob->startTM.tm_hour,
                pJob->startTM.tm_min,
                pJob->startTM.tm_sec);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pJob->state == INTEGRITY_CHECK_JOB_START)
    {
        clock_gettime(CLOCK_REALTIME, &timeNow);
        dwElapseSec = VMDIR_MAX( timeNow.tv_sec - pJob->startTime.tv_sec, 1);

        dwRemainingSec = (pJob->maxEntryID - pJob->currentEntryID) / VMDIR_MAX((pJob->currentEntryID / dwElapseSec), 1);
    }
    else
    {
        gmtime_r(&pJob->endTime.tv_sec, &myTM);

        dwError = VmDirAllocateStringPrintf(&pszEndTime,
                    "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
                    "end time: ",
                    myTM.tm_year+1900,
                    myTM.tm_mon+1,
                    myTM.tm_mday,
                    myTM.tm_hour,
                    myTM.tm_min,
                    myTM.tm_sec);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (dwRemainingSec > 0)
    {
        timeNow.tv_sec += dwRemainingSec;
        gmtime_r(&timeNow.tv_sec, &myTM);

        dwError = VmDirAllocateStringPrintf(&pszEstimatedEndTime,
                    "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
                    "estimated end time: ",
                    myTM.tm_year+1900,
                    myTM.tm_mon+1,
                    myTM.tm_mday,
                    myTM.tm_hour,
                    myTM.tm_min,
                    myTM.tm_sec);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszEntryProcessed,
                    "%-20s %d",
                    "entries processed:",
                    pJob->dwNumProcessed);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(PSTR) * (pJob->dwNumJobCtx), (PVOID*)&ppszPartner);
    BAIL_ON_VMDIR_ERROR(dwError );

    for (dwCnt=0; dwCnt < pJob->dwNumJobCtx; dwCnt++)
    {
        if (pJob->pJobctx[dwCnt].state != INTEGRITY_CHECK_JOBCTX_SKIP)
        {
            dwError = VmDirAllocateStringPrintf( &ppszPartner[dwGoodPartner++],
                        "%s partner %s, digest mismatch (%d), missing entry (%d)",
                        (pJob->pJobctx[dwCnt].state == INTEGRITY_CHECK_JOBCTX_VALID) ? "Valid" :"Invalid",
                        pJob->pJobctx[dwCnt].pszPartnerName,
                        pJob->pJobctx[dwCnt].dwFailedDigestCnt,
                        pJob->pJobctx[dwCnt].dwMissedEntryCnt);
            BAIL_ON_VMDIR_ERROR(dwError);

        }
    }

    dwError = VmDirAllocateMemory( sizeof(PSTR) * ((dwNumAttrs) * 2 + 1), // add 1 for VmDirFreeStringArrayA call later
                                   (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCnt=0;

    dwError = VmDirAllocateStringA(ATTR_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(INTEGRITY_CHECK_STATUS_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_OBJECT_CLASS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(OC_SERVER_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszStartTime, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszEndTime)
    {
        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszEndTime, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszEstimatedEndTime)
    {
        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszEstimatedEndTime, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszEntryProcessed, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIdx = 0; dwIdx < dwGoodPartner; dwIdx++)
    {
        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        ppszAttrList[dwCnt++] = ppszPartner[dwIdx];
        ppszPartner[dwIdx] = NULL;
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAttrListToNewEntry( pSchemaCtx,
                                       INTEGRITY_CHECK_STATUS_DN,
                                       ppszAttrList,
                                       FALSE,
                                       &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppszPartner);
    VMDIR_SAFE_FREE_MEMORY(pszEntryProcessed);
    VMDIR_SAFE_FREE_MEMORY(pszEstimatedEndTime);
    VMDIR_SAFE_FREE_MEMORY(pszEndTime);
    VMDIR_SAFE_FREE_MEMORY(pszStartTime);
    VmDirBackendCtxContentFree( &backendCtx );

    if (ppszAttrList != NULL)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    if (pSchemaCtx != NULL)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    if (pEntry != NULL)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}
