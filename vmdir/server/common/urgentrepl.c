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


VOID
VmDirPerformUrgentReplication(
    PVDIR_OPERATION pOperation
    )
{
    BOOLEAN   bSuccess = FALSE;
    DWORD     dwError = 0;
    UINT64    timeout = 0;
    UINT64    newTimeout = 0;
    USN       currentTxnUSN = 0;
    UINT64    startTime = 0;
    UINT64    endTime = 0;
    PVDIR_ATTRIBUTE   pAttrUSNCreated = NULL;

    pAttrUSNCreated = VmDirEntryFindAttribute(ATTR_USN_CHANGED, pOperation->request.addReq.pEntry);
    assert( pAttrUSNCreated != NULL );
    currentTxnUSN = VmDirStringToLA(pAttrUSNCreated->vals[0].lberbv.bv_val, NULL, 10);

    /*
     * If urgent replication request is already active then set this boolean
     * and wait for the response
     */
    VmDirSetUrgentReplicationPending(TRUE);
    VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart();

    //TODO: populate time out value via registry and move the timeout init to appropriate place
    VmDirSetUrgentReplTimeout(60000); // 60 seconds
    newTimeout = timeout = VmDirGetUrgentReplTimeout();
    startTime = VmDirGetTimeInMilliSec();

    /*
     * During server shutdown state, irrespective of whether urgent repl
     * request is successful or not, urgent repl thread will signal all
     * the waiting writer threads.
     *
     * If Signalled but USN not updated and not a time out case.
     * This function would calculate the new time out, retry urgent repl cycle and wait again.
     */
    while (bSuccess == FALSE &&
           VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        dwError = VmDirTimedWaitForUrgentReplDone(newTimeout, startTime);
        if (dwError != 0)
        {
            break;
        }

        endTime = VmDirGetTimeInMilliSec();

        if (currentTxnUSN <= VmDirGetUrgentReplConsensus())
        {
            bSuccess = TRUE;
        }
        else if((startTime + timeout) > endTime)
        {
            // Retry until it times out or succeeds
            VmDirRetryUrgentReplication();
            //Revise the time out before blocking again
            newTimeout = (startTime + timeout) - endTime;
        }
        else
        {
            //time out - don't wait again
            break;
        }
    }

    // 0 - success, 1 - failure (does not differentiate between partial failure or complete failure)
    pOperation->strongConsistencyWriteCtrl->value.scwDoneCtrlVal.status = ((bSuccess) ? VDIR_SCW_SUCCEEDED: VDIR_SCW_FAILED);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
        "VmDirPerformUrgentReplication completed successfully");

    return;
}

VOID
VmDirRetryUrgentReplication(
    VOID
    )
{
    VmDirSetUrgentReplicationPending(TRUE);
    VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart();
}
