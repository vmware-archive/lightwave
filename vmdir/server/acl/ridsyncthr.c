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
_VmDirRidSyncThr(
    PVOID    pArg
    );

DWORD
VmDirInitRidSynchThr(
    PVDIR_THREAD_INFO* ppThrInfo
    )
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirRidSyncThr,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);
    *ppThrInfo = pThrInfo;

cleanup:

    return dwError;

error:
    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

static
DWORD
_VmDirRidSyncThr(
    PVOID    pArg
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bInLock = FALSE;
    PVDIR_THREAD_INFO   pThrInfo = (PVDIR_THREAD_INFO)pArg;
    PVMDIR_SID_GEN_STACK_NODE pSidGenStackNode = NULL;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirRidSyc thr started" );

    while (1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        VMDIR_SAFE_FREE_MEMORY(pSidGenStackNode);
        while (VmDirPopTSStack(gSidGenState.pStack, (PVOID*)&pSidGenStackNode) == 0 &&
               pSidGenStackNode != NULL)
        {
            (VOID)VmDirSyncRIDSeqToDB(
                    pSidGenStackNode->pszDomainDn,
                    pSidGenStackNode->dwDomainRidSequence);

            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                //
                // Any pending updates will be performed by VmDirVmAclShutdown.
                //
                goto cleanup;
            }

            VMDIR_SAFE_FREE_MEMORY(pSidGenStackNode);
        }

        VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutexUsed);

        VmDirConditionTimedWait(
            pThrInfo->conditionUsed,
            pThrInfo->mutexUsed,
            3 * 1000);          // time wait 3 seconds
        // ignore error

        VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutexUsed);
    }

cleanup:

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirRidSyc thr stopped (%d)", dwError );

    VMDIR_SAFE_FREE_MEMORY(pSidGenStackNode);

    return dwError;
}
