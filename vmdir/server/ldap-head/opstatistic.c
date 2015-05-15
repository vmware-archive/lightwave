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


DWORD
VmDirInitOPStatisticGlobals(
    VOID
    )
{
    DWORD       dwError = 0;

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opBind.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opAdd.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opSearch.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opModify.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opDelete.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirOPStatisticGlobals.opUnbind.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;

}

//TODO, shutdown clean up op.pmutex

DWORD
VmDirOPStatisticUpdate(
    PVMDIR_OPERATION_STATISTIC   pStatistic,
    uint64_t                    iThisTimeInMilliSec
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;
    uint64_t    iNewTotalTime = 0;

    assert(pStatistic != NULL);

    if (iThisTimeInMilliSec <=  0)
    {
        iThisTimeInMilliSec = 1;
    }

    VMDIR_LOCK_MUTEX(bInLock, pStatistic->pmutex);
    pStatistic->iTotalCount++;

    iNewTotalTime = pStatistic->iTimeInMilliSec + iThisTimeInMilliSec;

    if (iNewTotalTime < pStatistic->iTimeInMilliSec)
    {
        // overflow, reset time and counter
        pStatistic->iTimeInMilliSec = iThisTimeInMilliSec;
        pStatistic->iCount = 1;
    }
    else
    {
        pStatistic->iTimeInMilliSec = iNewTotalTime;
        pStatistic->iCount++;
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pStatistic->pmutex);

    return dwError;
}

uint16_t
VmDirOPStatisticGetAvgTime(
    PVMDIR_OPERATION_STATISTIC   pStatistic
    )
{
    BOOLEAN     bInLock = FALSE;
    uint64_t    iCurrentTotalTimeInMSec = 0;
    uint64_t    iCurrentCount = 0;
    uint64_t    iAvgTimeInMSecs = 0;

    assert(pStatistic != NULL);

    VMDIR_LOCK_MUTEX(bInLock, pStatistic->pmutex);

    iCurrentTotalTimeInMSec = pStatistic->iTimeInMilliSec;
    iCurrentCount           = pStatistic->iCount;

    VMDIR_UNLOCK_MUTEX(bInLock, pStatistic->pmutex);

    iAvgTimeInMSecs = iCurrentCount > 0 ? (iCurrentTotalTimeInMSec/iCurrentCount) : 0;

    return iAvgTimeInMSecs > UINT16_MAX ? UINT16_MAX : (uint16_t)iAvgTimeInMSecs;
}

uint64_t
VmDirOPStatisticGetCount(
    PVMDIR_OPERATION_STATISTIC   pStatistic
    )
{
    BOOLEAN     bInLock = FALSE;
    uint64_t    iCurrentCount = 0;

    assert(pStatistic != NULL);

    VMDIR_LOCK_MUTEX(bInLock, pStatistic->pmutex);

    iCurrentCount           = pStatistic->iCount;

    VMDIR_UNLOCK_MUTEX(bInLock, pStatistic->pmutex);

    return iCurrentCount;
}

/*
 * caller owns return PSTR
 */
PSTR
VmDirOPStatistic(
    ber_tag_t      opTag
    )
{
    DWORD   dwError = 0;
    PSTR    pszStatistic = NULL;
    PCSTR   pszOPName = NULL;
    PVMDIR_OPERATION_STATISTIC   pOPStatistic = NULL;

    switch (opTag)
    {
       case LDAP_REQ_BIND:
           pszOPName = "Bind";
           pOPStatistic = &gVmdirOPStatisticGlobals.opBind;
          break;

       case LDAP_REQ_ADD:
           pszOPName = "Add";
           pOPStatistic = &gVmdirOPStatisticGlobals.opAdd;
          break;

       case LDAP_REQ_SEARCH:
           pszOPName = "Search";
           pOPStatistic = &gVmdirOPStatisticGlobals.opSearch;
          break;

       case LDAP_REQ_MODIFY:
           pszOPName = "Modify";
           pOPStatistic = &gVmdirOPStatisticGlobals.opModify;
           break;

       case LDAP_REQ_DELETE:
           pszOPName = "Delete";
           pOPStatistic = &gVmdirOPStatisticGlobals.opDelete;
           break;

       case LDAP_REQ_UNBIND:
           pszOPName = "Unbind";
           pOPStatistic = &gVmdirOPStatisticGlobals.opUnbind;
           break;

       case LDAP_REQ_MODDN:
       case LDAP_REQ_COMPARE:
       case LDAP_REQ_ABANDON:
       case LDAP_REQ_EXTENDED:
       default:

          break;
    }

    if (pOPStatistic != NULL)
    {
        dwError = VmDirAllocateStringAVsnprintf(
                        &pszStatistic,
                        "LDAP %10s - count:(%ld), Avg response time in MS:(%ld)",
                        pszOPName,
                        VmDirOPStatisticGetCount(pOPStatistic),
                        VmDirOPStatisticGetAvgTime(pOPStatistic));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return pszStatistic;

error:

    VMDIR_SAFE_FREE_MEMORY(pszStatistic);

    goto cleanup;
}

