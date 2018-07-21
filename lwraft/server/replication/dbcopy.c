/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Replication
 *
 * Filename: dbcopy.c
 *
 * Abstract: hot copy remote partner DB
 *
 */

#include "includes.h"

//Read data.mdb up to last_pgno + the safe margin (256MB). The margin is for the latency
// between calling mdb_env_set_state to obtain the dbSizeMb and the meta page(first block) is read,
// i.e. the provision at the remote server wouldn't increase DB size by 256MB during this period.
// For cold copy (remote database at read-only mode), VMDIR_MDB_COPY_SAFE_MARGIN is set to 0.
#define VMDIR_MDB_COPY_SAFE_MARGIN (256 << 20)

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       pszRemoteDBDir,
    PCSTR       pszLocalDBDir,
    PCSTR       pszFileName,
    UINT32      remoteFileSizeMb
    );

static
int
_VmDirGetRemoteDBUsingRPC(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR   pszSrcDir,
    PCSTR   pszDstDir
    );

static
int
_VmDirDBNameToPath(
    PCSTR   pszDBName,
    PSTR*   ppszDBPath,
    PSTR*   ppszDBPartnerPath
    );

static
int
_VmDirCopyPartnerDB(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR   pszDBName
    );

int
VmDirCopyRemoteDB(
    PCSTR   pszHostname
    )
{
    DWORD       retVal = 0;
    PSTR        pszDcAccountPwd = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    if (IsNullOrEmptyString(pszHostname))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    retVal = VmDirReadDCAccountPassword(&pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirOpenServerA(
                 pszHostname,
                 gVmdirServerGlobals.dcAccountUPN.lberbv_val,
                 NULL,
                 pszDcAccountPwd,
                 0,
                 NULL,
                 &hServer);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "%s: Connected to the replication partner (%s).",
            __FUNCTION__, pszHostname);

    // copy main/default DB
    retVal = _VmDirCopyPartnerDB(hServer, "");
    BAIL_ON_VMDIR_ERROR(retVal);

    if (gVmdirGlobals.bUseLogDB)
    {
        /*
         * copy log database. tolerate error when remote server
         * does not have a log db.
         */
        retVal = _VmDirCopyPartnerDB(hServer, LOG1_DB_DIR);
        if (retVal == VMDIR_ERROR_NO_SUCH_DB)
        {
            retVal = 0;
            VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "%s: Remote server does not have log db",
                __FUNCTION__);
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    if (hServer)
    {
        VmDirCloseServer( hServer);
    }
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, retVal);
    goto cleanup;
}

static
int
_VmDirCopyPartnerDB(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR                   pszDBName
    )
{
    int retVal = 0;
    PSTR    pszDBPath = NULL;
    PSTR    pszDBPartnerPath = NULL;

    retVal = _VmDirDBNameToPath(pszDBName, &pszDBPath, &pszDBPartnerPath);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirMkdir(pszDBPartnerPath, 0700);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = _VmDirGetRemoteDBUsingRPC(hServer, pszDBPath, pszDBPartnerPath);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDBPath);
    VMDIR_SAFE_FREE_MEMORY(pszDBPartnerPath);
    return  retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DBName (%s) error (%d)", __FUNCTION__, VDIR_SAFE_STRING(pszDBName), retVal);
    goto cleanup;
}

/*
 * pszDBName == "",
 *  return /var/lib/vmware/post/  and /var/lib/vmware/post/partner
 * pszDBName == "postlog1"
 *  return /var/lib/vmware/post/postlog1 and /var/lib/vmware/post/partner/postlog1
 */
static
int
_VmDirDBNameToPath(
    PCSTR   pszDBName,
    PSTR*   ppszDBPath,
    PSTR*   ppszDBPartnerPath
    )
{
    int retVal = 0;
    PSTR    pszLocalDBPath = NULL;
    PSTR    pszLocalDBPartnerPath = NULL;

    if (VmDirStringLenA(pszDBName) > 0)
    {
        retVal = VmDirAllocateStringPrintf(&pszLocalDBPath, "%s/%s", LWRAFT_DB_DIR, pszDBName);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirAllocateStringPrintf(&pszLocalDBPartnerPath, "%s/%s/%s", LWRAFT_DB_DIR, LOCAL_PARTNER_DIR, pszDBName);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        retVal = VmDirAllocateStringPrintf(&pszLocalDBPath, "%s", LWRAFT_DB_DIR);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirAllocateStringPrintf(&pszLocalDBPartnerPath, "%s/%s", LWRAFT_DB_DIR, LOCAL_PARTNER_DIR);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    *ppszDBPath = pszLocalDBPath;
    *ppszDBPartnerPath = pszLocalDBPartnerPath;

cleanup:
    return retVal;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalDBPath);
    VMDIR_SAFE_FREE_MEMORY(pszLocalDBPartnerPath);
    goto cleanup;
}

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       pszRemoteDBDir,
    PCSTR       pszLocalDBDir,
    PCSTR       pszFileName,
    UINT32      remoteFileSizeMb
    )
{
//read block size of eight MB.
#define VMDIR_DB_READ_BLOCK_SIZE     (1<<23)

    DWORD       retVal = 0;
    int         localFileFd = 0;
    FILE *      pRemoteFile = 0;
    UINT32      dwCount = 0;
    PBYTE       pReadBuffer = NULL;
    DWORD       writeSize = 0;
    long long   readSizeTotal = 0;
    unsigned long readInc = 0;
    unsigned long totalWriteMb = 0;
    PSTR        pszRemtoeDBFile = NULL;
    PSTR        pszLocalDBFile = NULL;

    retVal = VmDirAllocateStringPrintf(&pszRemtoeDBFile, "%s/%s", pszRemoteDBDir, pszFileName);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAllocateStringPrintf(&pszLocalDBFile, "%s/%s", pszLocalDBDir, pszFileName);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirOpenDatabaseFile( hServer, pszRemtoeDBFile, &pRemoteFile);
    BAIL_ON_VMDIR_ERROR(retVal);

    if ((localFileFd = creat(pszLocalDBFile, S_IRUSR|S_IWUSR)) < 0)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
    }

    retVal = VmDirAllocateMemory(VMDIR_DB_READ_BLOCK_SIZE, (PVOID)&pReadBuffer );
    BAIL_ON_VMDIR_ERROR(retVal);

    for (;;)
    {
        retVal = VmDirReadDatabaseFile( hServer, pRemoteFile, &dwCount, pReadBuffer, VMDIR_DB_READ_BLOCK_SIZE);
        BAIL_ON_VMDIR_ERROR(retVal);

        if (dwCount==0)
        {
            break;
        }

        writeSize = (UINT32)write(localFileFd, pReadBuffer, dwCount);

        if (writeSize < dwCount)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: write() call failed, recvSize: %d, writeSize: %d.",
                    __FUNCTION__, dwCount, writeSize );
            BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_IO);
        }
        readSizeTotal += (long long)dwCount;
        readInc += dwCount;
        if (readInc >= (1L<<30))
        {
            // Log progress every 1 GB read.
            readInc = 0;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                "%s: remote file %s %lu MB copied ...",
                __FUNCTION__, pszRemtoeDBFile, readSizeTotal/(1L<<20));

            // We saw DCERPC calls timeout when transferring file larger than 100GB without this pause.
            // Should address this with PR 1827212
            VmDirSleep(2000);
        }

        if (dwCount < VMDIR_DB_READ_BLOCK_SIZE || (remoteFileSizeMb > 0 &&
            readSizeTotal > (((long long)remoteFileSizeMb << 20) + (long long)VMDIR_MDB_COPY_SAFE_MARGIN)))
        {
            //data.mdb may grow (during hot copy) when there is writing activity at the remote server.
            //But there is no need to copy the growning part beyond a safe margin.
            break;
        }
    }
    totalWriteMb = (unsigned long)(readSizeTotal/(1L<<20));

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Complete copying remote file %s with actual size %lu MB", pszRemtoeDBFile, totalWriteMb);

cleanup:
    if (pRemoteFile != NULL)
    {
        DWORD       localRetVal = 0;
        if ((localRetVal = VmDirCloseDatabaseFile( hServer, &pRemoteFile )) != 0)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s: VmDirCloseDatabaseFile() call failed with error: %d",
                __FUNCTION__, localRetVal );
        }
        retVal = (retVal != 0) ? retVal : localRetVal;
    }

    if (localFileFd >= 0)
    {
        close(localFileFd);
    }

    VMDIR_SAFE_FREE_MEMORY(pReadBuffer);

    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, retVal);
    goto cleanup;
}

static
int
_VmDirGetRemoteDBUsingRPC(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR                   pszSrcDir,
    PCSTR                   pszDstDir
    )
{
    int     retVal = 0;
    size_t  nLength = 0;
    DWORD   low_xlognum = 0;
    DWORD   high_xlognum = 0;
    DWORD   xlognum = 0;
    DWORD   remoteDbSizeMb = 0;
    DWORD   remoteDbMapSizeMb = 0;
    char    localDir[VMDIR_MAX_FILE_NAME_LEN+1] = {0};
    PSTR    pszXlogSrcDir = NULL;
    PSTR    pszXlogDstDir = NULL;

    nLength = VmDirStringLenA(pszSrcDir);

    retVal = VmDirCopyMemory(localDir, VMDIR_MAX_FILE_NAME_LEN, pszSrcDir, nLength);
    BAIL_ON_VMDIR_ERROR(retVal);

    //Set backend to KEEPXLOGS  mode
    retVal = VmDirSetBackendState(
                 hServer,
                 MDB_STATE_KEEPXLOGS,
                 &low_xlognum,
                 &remoteDbSizeMb,
                 &remoteDbMapSizeMb,
                 localDir,
                 VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR(retVal);

    /*
     * if we did not get back the db we asked for, we could be asking a log db
     * to a server that does not have a log db. do not proceed.
    */
    if (VmDirStringCompareA(localDir, pszSrcDir, FALSE))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_SUCH_DB);
    }

    retVal = _VmDirGetRemoteDBFileUsingRPC(
        hServer, pszSrcDir, pszDstDir, VMDIR_MDB_DATA_FILE_NAME, remoteDbSizeMb);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (low_xlognum == 0)
    {
        goto cleanup;
    }

    //Query current xlog number
    retVal = VmDirSetBackendState(
                 hServer,
                 MDB_STATE_GETXLOGNUM,
                 &high_xlognum,
                 &remoteDbSizeMb,
                 &remoteDbMapSizeMb,
                 localDir,
                 VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: copying XLOGS from %d to %d", __FUNCTION__, low_xlognum, high_xlognum);

    retVal = VmDirAllocateStringPrintf(&pszXlogSrcDir, "%s/%s", pszSrcDir, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAllocateStringPrintf(&pszXlogDstDir, "%s/%s", pszDstDir, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirMkdir(pszXlogDstDir, 0700);
    BAIL_ON_VMDIR_ERROR( retVal );

    for (xlognum = low_xlognum; xlognum <= high_xlognum; xlognum++)
    {
        char xlogNameBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};

        retVal = VmDirStringPrintFA(xlogNameBuf, VMDIR_MAX_FILE_NAME_LEN, "%lu", xlognum);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = _VmDirGetRemoteDBFileUsingRPC(
            hServer, pszXlogSrcDir, pszXlogDstDir, xlogNameBuf, 0);
        BAIL_ON_VMDIR_ERROR(retVal);

        //This is a workaround for the DCERPC blocking issue, see PR 1827212
        VmDirSleep(300);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: complete copy db (%s)", __FUNCTION__, pszSrcDir);

cleanup:
    if (hServer)
    {
        //clear backend transfering xlog files mode.
        VmDirSetBackendState(
            hServer,
            MDB_STATE_CLEAR,
            &xlognum,
            &remoteDbSizeMb,
            &remoteDbMapSizeMb,
            localDir,
            VMDIR_MAX_FILE_NAME_LEN);
    }

    VMDIR_SAFE_FREE_MEMORY(pszXlogSrcDir);
    VMDIR_SAFE_FREE_MEMORY(pszXlogDstDir);

    return  retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DBName (%s) error (%d)", __FUNCTION__, VDIR_SAFE_STRING(pszSrcDir), retVal);
    goto cleanup;
}
