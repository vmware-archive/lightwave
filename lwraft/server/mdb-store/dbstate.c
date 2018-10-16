/*
 * Copyright © 2108 VMware, Inc.  All Rights Reserved.
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
    PVDIR_MDB_STATE         pDbState,
    MDB_state_op            op,
    DWORD*                  pdwLogNum,
    DWORD*                  pdwDbSizeMb,
    DWORD*                  pdwDbMapSizeMb,
    PSTR                    pszDbPath,
    DWORD                   dwDbPathSize
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

    dwError = LwRtlCreateHashMap(
            &gVDirMdbStateGlobals.pDbPathToStateMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDirFreeMdbStateGlobals();
    goto cleanup;
}

static
VOID
_VmDirEmptyFreePair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
}

static
VOID
_VmDirFreeMdbState(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
    PVDIR_MDB_STATE pDbState = (PVDIR_MDB_STATE)pPair->pValue;
    if (pDbState)
    {
        if (pDbState->pActiveFileHandleMap)
        {
            LwRtlHashMapClear(pDbState->pActiveFileHandleMap, _VmDirEmptyFreePair, NULL);
            LwRtlFreeHashMap(&pDbState->pActiveFileHandleMap);
        }
        VmDirFreeMemory(pDbState);
    }
}

static
VOID
_VmDirClearActiveFileHandlesInLock(
    PVDIR_MDB_STATE pDbState
    )
{
    if (pDbState)
    {
        if (pDbState->pActiveFileHandleMap)
        {
            LwRtlHashMapClear(pDbState->pActiveFileHandleMap, _VmDirEmptyFreePair, NULL);
        }
    }
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

    if (gVDirMdbStateGlobals.pDbPathToStateMap)
    {
        LwRtlHashMapClear(
            gVDirMdbStateGlobals.pDbPathToStateMap,
            _VmDirFreeMdbState,
            NULL);
        LwRtlFreeHashMap(&gVDirMdbStateGlobals.pDbPathToStateMap);
    }

    memset(&gVDirMdbStateGlobals, 0, sizeof(gVDirMdbStateGlobals));
}

/*
 * make sure a state entry exists for this db
 * if not, create it
*/
DWORD
_VmDirGetDbEntryByPathInLock(
    PCSTR pszDbPath,
    PVDIR_MDB_STATE *ppDbState
    )
{
    DWORD dwError = 0;
    PVDIR_MDB_STATE pDbState = NULL;

    if (!ppDbState || IsNullOrEmptyString(pszDbPath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlHashMapFindKey(
        gVDirMdbStateGlobals.pDbPathToStateMap,
        (PVOID *)&pDbState,
        pszDbPath);

    if (dwError)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_MDB_STATE), (PVOID *)&pDbState);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlCreateHashMap(
                &pDbState->pActiveFileHandleMap,
                LwRtlHashDigestPointer,
                LwRtlHashEqualPointer,
                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                      gVDirMdbStateGlobals.pDbPathToStateMap,
                      (PVOID)pszDbPath,
                      pDbState,
                      NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppDbState = pDbState;

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * lookup a db state entry by file handle.
 * this is needed to do handle based operations
 * like rpc file close and rpc handle rundown
*/
static
DWORD
_VmDirGetDbEntryByFileHandleInLock(
    FILE *pFileHandle,
    PVDIR_MDB_STATE *ppDbState
    )
{
    DWORD dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_MDB_STATE pDbState = NULL;
    PVDIR_MDB_STATE pDbStateIter = NULL;

    if (!pFileHandle || !ppDbState)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (LwRtlHashMapIterate(gVDirMdbStateGlobals.pDbPathToStateMap, &iter, &pair))
    {
        pDbStateIter = (PVDIR_MDB_STATE)pair.pValue;
        if (pDbStateIter && pDbStateIter->pActiveFileHandleMap)
        {
            if (LwRtlHashMapFindKey(
                    pDbStateIter->pActiveFileHandleMap,
                    NULL,
                    pFileHandle) == 0)
            {
                pDbState = pDbStateIter;
                break;
            }
        }
    }

    if (!pDbState)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    *ppDbState = pDbState;
cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * change MDB state to allow hot database copy
 */
DWORD
VmDirSetMdbBackendState(
    MDB_state_op            op,
    DWORD*                  pdwLogNum,
    DWORD*                  pdwDbSizeMb,
    DWORD*                  pdwDbMapSizeMb,
    PSTR                    pszDbPath,
    DWORD                   dwDbPathSize)
{
    DWORD dwError = 0;
    DWORD lognum = 0L;
    DWORD dbSizeMb = 0L;
    DWORD dbMapSizeMb = 0L;
    BOOLEAN bInLock = FALSE;
    PVDIR_MDB_STATE pDbState = NULL;

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

    dwError = _VmDirGetDbEntryByPathInLock(pszDbPath, &pDbState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSetMdbStateInLock(pDbState, op, &lognum, &dbSizeMb, &dbMapSizeMb, pszDbPath, dwDbPathSize);
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

/*
 * add active file handle to db state
*/
DWORD
VmDirAddActiveHandleToMdbState(
    FILE *pFileHandle,
    PCSTR pszFilePath
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_MDB_STATE pDbState = NULL;
    PCSTR pszDbPath = LWRAFT_DB_DIR;

    if (!pFileHandle || IsNullOrEmptyString(pszFilePath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);

    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
        "%s: %p, %s", __FUNCTION__, pFileHandle, pszFilePath);

    if (VmDirStringStartsWith(pszFilePath, LOG1_DB_PATH, FALSE))
    {
        pszDbPath = LOG1_DB_PATH;
    }

    dwError = _VmDirGetDbEntryByPathInLock(pszDbPath, &pDbState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(
            pDbState->pActiveFileHandleMap,
            pFileHandle,
            NULL,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * remove active handle from db state.
*/
DWORD
VmDirRemoveActiveHandleFromMdbState(
    FILE *pFileHandle
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_MDB_STATE pDbState = NULL;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pFileHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: %p", __FUNCTION__, pFileHandle);

    dwError = _VmDirGetDbEntryByFileHandleInLock(pFileHandle, &pDbState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapRemove(pDbState->pActiveFileHandleMap, pFileHandle, &pair);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * lookup db state by file handle, then clear keep logs state
*/
DWORD
VmDirClearMdbStateByFileHandle(
    FILE *pFileHandle
    )
{
    DWORD   dwError = 0;
    DWORD   lognum = 0L;
    DWORD   dbSizeMb = 0L;
    DWORD   dbMapSizeMb = 0L;
    size_t  nLength = 0;
    CHAR    bufDBPath[VMDIR_MAX_FILE_NAME_LEN] = {0};
    BOOLEAN bInLock = FALSE;
    PVDIR_MDB_STATE pDbState = NULL;

    if (!pFileHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVDirMdbStateGlobals.pMutex);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: %p", __FUNCTION__, pFileHandle);

    dwError = _VmDirGetDbEntryByFileHandleInLock(pFileHandle, &pDbState);
    BAIL_ON_VMDIR_ERROR(dwError);

    nLength = VmDirStringLenA(pDbState->bufDBPath);
    dwError = VmDirCopyMemory(
                 bufDBPath,
                 VMDIR_MAX_FILE_NAME_LEN,
                 pDbState->bufDBPath,
                 nLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSetMdbStateInLock(
                  pDbState,
                  MDB_STATE_CLEAR,
                  &lognum,
                  &dbSizeMb,
                  &dbMapSizeMb,
                  bufDBPath,
                  nLength);
    BAIL_ON_VMDIR_ERROR(dwError);

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
    PVDIR_MDB_STATE         pDbState,
    MDB_state_op            op,
    DWORD                   *pdwLogNum,
    DWORD                   *pdwDbSizeMb,
    DWORD                   *pdwDbMapSizeMb,
    PSTR                    pszDbPath,
    DWORD                   dwDbPathSize)
{
    DWORD dwError = 0;
    BOOLEAN bLogEvent = FALSE;

    PVDIR_MDB_DB pDB = (PVDIR_MDB_DB)VmDirSafeDBFromPath(pszDbPath);

    /*
     * it is possible a log enabled peer joins to a log disabled server
     * allow not found.
    */
    if (!pDB)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_LDAP_ERROR_NO_SUCH_DB);
    }

    if (op == MDB_STATE_READONLY || op == MDB_STATE_KEEPXLOGS)
    {
        pDbState->nDBCopyCount++;

        if (pDbState->nDBCopyCount == 1)
        {
            dwError = mdb_env_set_state(
                pDB->mdbEnv,
                op,
                &pDbState->xLogNum,
                &pDbState->dbSizeMb,
                &pDbState->dbMapMb,
                pDbState->bufDBPath,
                sizeof(pDbState->bufDBPath));
            BAIL_ON_VMDIR_ERROR(dwError);

            bLogEvent = TRUE;
        }
    }
    else if (op == MDB_STATE_CLEAR)
    {
        pDbState->nDBCopyCount--;

        if (pDbState->nDBCopyCount <= 0)
        {   // handle <= 0 to cover RPC rundown thread corner case where fail before reaching dwDBCopyCount++

            _VmDirClearActiveFileHandlesInLock(pDbState);

            dwError = mdb_env_set_state(
                pDB->mdbEnv,
                MDB_STATE_CLEAR,
                &pDbState->xLogNum,
                &pDbState->dbSizeMb,
                &pDbState->dbMapMb,
                pDbState->bufDBPath,
                sizeof(pDbState->bufDBPath));
            BAIL_ON_VMDIR_ERROR(dwError);

            pDbState->nDBCopyCount = 0;
            bLogEvent = TRUE;
        }
    }
    else if (op == MDB_STATE_GETXLOGNUM)
    {
        dwError = mdb_env_set_state(
            pDB->mdbEnv,
            MDB_STATE_GETXLOGNUM,
            &pDbState->xLogNum,
            &pDbState->dbSizeMb,
            &pDbState->dbMapMb,
            pDbState->bufDBPath,
            sizeof(pDbState->bufDBPath));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringCpyA(pszDbPath, dwDbPathSize, pDbState->bufDBPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwLogNum = (DWORD)pDbState->xLogNum;
    *pdwDbSizeMb = (DWORD)pDbState->dbSizeMb;
    *pdwDbMapSizeMb = (DWORD)pDbState->dbMapMb;

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
