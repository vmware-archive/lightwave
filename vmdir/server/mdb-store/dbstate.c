/*
 * Copyright © 208 VMware, Inc.  All Rights Reserved.
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
_VmDirSetMdbStateInLock(
    MDB_state_op        op,
    DWORD*              pdwLogNum,
    DWORD*              pdwDbSizeMb,
    DWORD*              pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize
    );

DWORD
VmDirInitMdbStateGlobals(
    VOID
    )
{
    DWORD   dwError = 0;

    memset(&gVDirMdbStateGlobals, 0, sizeof(gVDirMdbStateGlobals));

    dwError = VmDirAllocateMutex(&gVDirMdbStateGlobals.pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDirFreeMdbStateGlobals();
    goto cleanup;
}

VOID
VmDirFreeMdbStateGlobals(
    VOID
    )
{
    if (gVDirMdbStateGlobals.pMutex)
    {
        VmDirFreeMutex(gVDirMdbStateGlobals.pMutex);
    }

    memset(&gVDirMdbStateGlobals, 0, sizeof(gVDirMdbStateGlobals));
}

/*
 * change MDB state to allow hot database copy
 */
DWORD
VmDirSetMdbBackendState(
    MDB_state_op        op,
    DWORD*              pdwLogNum,
    DWORD*              pdwDbSizeMb,
    DWORD*              pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize)
{
    DWORD dwError = 0;
    DWORD lognum = 0L;
    DWORD dbSizeMb = 0L;
    DWORD dbMapSizeMb = 0L;
    BOOLEAN bInLock = FALSE;

    if (!pdwLogNum      ||
        !pdwDbSizeMb    ||
        !pdwDbMapSizeMb ||
        !pszDbPath  ||
        (op < MDB_STATE_CLEAR || op > MDB_STATE_GETXLOGNUM))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, set (%s) called", __FUNCTION__, VmDirMdbStateToName(op));

    VMDIR_LOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);

    dwError = _VmDirSetMdbStateInLock(op, &lognum, &dbSizeMb, &dbMapSizeMb, pszDbPath, dwDbPathSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwLogNum = lognum;
    *pdwDbSizeMb = dbSizeMb;
    *pdwDbMapSizeMb = dbMapSizeMb;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirSetMdbStateInLock(
    MDB_state_op        op,
    DWORD               *pdwLogNum,
    DWORD               *pdwDbSizeMb,
    DWORD               *pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize)
{
    DWORD dwError = 0;
    BOOLEAN bLogEvent = FALSE;

    if (op == MDB_STATE_READONLY || op == MDB_STATE_KEEPXLOGS)
    {
        gVDirMdbStateGlobals.dwDBCopyCount++;

        if (gVDirMdbStateGlobals.dwDBCopyCount == 1)
        {
            dwError = mdb_env_set_state(
                gVdirMdbGlobals.mdbEnv,
                op,
                &gVDirMdbStateGlobals.xLogNum,
                &gVDirMdbStateGlobals.dbSizeMb,
                &gVDirMdbStateGlobals.dbMapMb,
                gVDirMdbStateGlobals.bufDBPath,
                sizeof(gVDirMdbStateGlobals.bufDBPath));
            BAIL_ON_VMDIR_ERROR(dwError);

            bLogEvent = TRUE;
        }
    }
    else if (op == MDB_STATE_CLEAR)
    {
        gVDirMdbStateGlobals.dwDBCopyCount--;

        if (gVDirMdbStateGlobals.dwDBCopyCount <= 0)
        {   // handle <= 0 to cover RPC rundown thread corner case where fail before reaching dwDBCopyCount++
            dwError = mdb_env_set_state(
                gVdirMdbGlobals.mdbEnv,
                MDB_STATE_CLEAR,
                &gVDirMdbStateGlobals.xLogNum,
                &gVDirMdbStateGlobals.dbSizeMb,
                &gVDirMdbStateGlobals.dbMapMb,
                gVDirMdbStateGlobals.bufDBPath,
                sizeof(gVDirMdbStateGlobals.bufDBPath));
            BAIL_ON_VMDIR_ERROR(dwError);

            gVDirMdbStateGlobals.dwDBCopyCount = 0;
            bLogEvent = TRUE;
        }
    }
    else if (op == MDB_STATE_GETXLOGNUM)
    {
        dwError = mdb_env_set_state(
            gVdirMdbGlobals.mdbEnv,
            MDB_STATE_GETXLOGNUM,
            &gVDirMdbStateGlobals.xLogNum,
            &gVDirMdbStateGlobals.dbSizeMb,
            &gVDirMdbStateGlobals.dbMapMb,
            gVDirMdbStateGlobals.bufDBPath,
            sizeof(gVDirMdbStateGlobals.bufDBPath));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringCpyA(pszDbPath, dwDbPathSize, gVDirMdbStateGlobals.bufDBPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwLogNum = (DWORD)gVDirMdbStateGlobals.xLogNum;
    *pdwDbSizeMb = (DWORD)gVDirMdbStateGlobals.dbSizeMb;
    *pdwDbMapSizeMb = (DWORD)gVDirMdbStateGlobals.dbMapMb;

    if (bLogEvent)
    {
        VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "%s, set MDB state to (%s), logNum (%d), DB size (%d MB), map size (%d MB)",
            __FUNCTION__, VmDirMdbStateToName(op), *pdwLogNum, *pdwDbSizeMb, *pdwDbMapSizeMb);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
