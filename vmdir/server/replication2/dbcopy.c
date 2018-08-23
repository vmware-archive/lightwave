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
#define VMDIR_MDB_COPY_SAFE_MARGIN 0

ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

VOID
VmDirFreeBindingHandle(
    handle_t *ppBinding
    );

DWORD
VmDirReadDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hServer,
    FILE *                  pFileHandle,
    UINT32 *                pdwCount,
    PBYTE                   pReadBuffer,
    UINT32                  bufferSize
    );

DWORD
VmDirCloseDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hServer,
    FILE **                 ppFileHandle
    );

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       dbRemoteFilename,
    PCSTR       localFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDBMapSizeMb
    );

static
int
_VmDirGetRemoteDBFileUsingLDAP(
    PCSTR       pszHostname,
    PSTR        pszDcAccountPwd,
    PCSTR       dbRemoteFilename,
    PCSTR       pszLocalFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDbMapSizeMb
    );

static
DWORD
_VmDirMkdir(
    PCSTR path,
    int mode
    );

static
int
_VmDirDoDbCopyLdapSearch(
    LDAP    *pLd,
    PVDIR_DB_COPY_CONTROL_VALUE pDbCopyCtrlVal
    );

int
VmDirCopyRemoteDB(
    PCSTR   pszHostname,
    PCSTR   dbHomeDir
    )
{
    DWORD       retVal = 0;
    PSTR        pszLocalErrorMsg = NULL;
    char        dbRemoteFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localXlogDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR        pszDcAccountPwd = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    DWORD       low_xlognum = 0;
    DWORD       high_xlognum = 0;
    DWORD       xlognum = 0;
    DWORD       remoteDbSizeMb = 0;
    DWORD       remoteDbMapSizeMb = 0;
    PBYTE       pDbPath = NULL;
    BOOLEAN     bMdbWalEnable = FALSE;
    BOOLEAN     bLdapCopyEnable = FALSE;

#ifndef _WIN32
    const char   fileSeperator = '/';
#else
    const char   fileSeperator = '\\';
#endif

    retVal = VmDirAllocateMemory(VMDIR_MAX_FILE_NAME_LEN, (PVOID)&pDbPath );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirReadDCAccountPassword(&pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirOpenServerA(pszHostname, gVmdirServerGlobals.dcAccountUPN.lberbv_val, NULL, pszDcAccountPwd, 0, NULL, &hServer);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirOpenServerA() call failed with error: %d, host name = %s",
            retVal, pszHostname  );
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: Connected to the replication partner (%s).", pszHostname );

    VmDirGetMdbWalEnable(&bMdbWalEnable);

    if (bMdbWalEnable)
    {
        //Set remote server backend to KEEPXLOGS  mode
        retVal = VmDirSetBackendState (hServer, MDB_STATE_KEEPXLOGS, &low_xlognum, &remoteDbSizeMb,
                                       &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    } else
    {
        //Set remote server backend to ReadOnly mode
        retVal = VmDirSetBackendState (hServer, MDB_STATE_READONLY, &low_xlognum, &remoteDbSizeMb,
                                       &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    }
    BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirSetBackendState failed, WalEnabled: %d, error: %d", bMdbWalEnable, retVal);

    retVal = VmDirStringPrintFA( localDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = _VmDirMkdir(localDir, 0700);
    BAIL_ON_VMDIR_ERROR( retVal );

    if (low_xlognum > 0)
    {
        retVal = VmDirStringPrintFA( localXlogDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", localDir, fileSeperator, VMDIR_MDB_XLOGS_DIR_NAME);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

        retVal = _VmDirMkdir(localXlogDir, 0700);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBUsingRPC: _VmDirMkdir() call failed with error: %d %s", retVal );
    }

    retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", (char *)pDbPath,
                                 VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( localFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir,
                                 fileSeperator, LOCAL_PARTNER_DIR, fileSeperator, VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: copying remote file %s with data size %ld MB with Map size %ld MB ...",
                    dbRemoteFilename, remoteDbSizeMb, remoteDbMapSizeMb );

    VmDirGetLdapCopyEnable(&bLdapCopyEnable);

    if (bLdapCopyEnable)
    {
        retVal = _VmDirGetRemoteDBFileUsingLDAP( pszHostname, pszDcAccountPwd, dbRemoteFilename, localFilename, remoteDbSizeMb, remoteDbMapSizeMb );
    }
    else
    {
        retVal = _VmDirGetRemoteDBFileUsingRPC( hServer, dbRemoteFilename, localFilename, remoteDbSizeMb, remoteDbMapSizeMb );
    }
    BAIL_ON_VMDIR_ERROR( retVal );

    if (low_xlognum == 0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirGetRemoteDBUsingRPC: complete MDB cold copy - WAL not supported by remote");
        goto cleanup;
    }

    //Query current xlog number
    retVal = VmDirSetBackendState (hServer, MDB_STATE_GETXLOGNUM, &high_xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirSetBackendState failed to get current xlog: %d", retVal  );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: start transfering XLOGS from %d to %d", low_xlognum, high_xlognum);
    for (xlognum = low_xlognum; xlognum <= high_xlognum; xlognum++)
    {
        retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%lu", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_XLOGS_DIR_NAME, fileSeperator, xlognum );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

        retVal = VmDirStringPrintFA( localFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%lu", localXlogDir, fileSeperator, xlognum);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

        if (bLdapCopyEnable)
        {
            retVal = _VmDirGetRemoteDBFileUsingLDAP( pszHostname, pszDcAccountPwd, dbRemoteFilename, localFilename, 0, 0 );
        }
        else
        {
            retVal = _VmDirGetRemoteDBFileUsingRPC( hServer, dbRemoteFilename, localFilename, 0, 0);
        }
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: _VmDirGetRemoteDBFileUsingRPC() call failed with error: %d", retVal );
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: complete transfering XLOGS from %d to %d", low_xlognum, high_xlognum);

cleanup:
    if (hServer)
    {
        //clear backend transfering xlog files mode.
        VmDirSetBackendState (hServer, MDB_STATE_CLEAR, &xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
        VmDirCloseServer( hServer);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(pDbPath);
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);

    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirDoDbCopyLdapSearch(
    LDAP    *pLd,
    PVDIR_DB_COPY_CONTROL_VALUE pDbCopyCtrlVal)
{
    DWORD           dwError = 0;
    LDAPControl     dbCopyCtl = {0};
    LDAPControl*    pSrvCtrls[2] = {&dbCopyCtl, NULL};
    LDAPMessage*    pResult = NULL;
    LDAPControl**   ppSearchResCtrls = NULL;

    dwError = VmDirCreateDbCopyControlContent(
                                pDbCopyCtrlVal,
                                &dbCopyCtl);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*Do dummy LDAP search that returns no entries*/
    dwError = ldap_search_ext_s(
                pLd,
                gVmdirServerGlobals.systemDomainDN.lberbv_val,
                LDAP_SCOPE_BASE,
                "objectclass=user",
                NULL,
                0,
                pSrvCtrls,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_parse_result(pLd, pResult, NULL, NULL, NULL, NULL, &ppSearchResCtrls, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppSearchResCtrls[0] == NULL ||
        VmDirStringCompareA(ppSearchResCtrls[0]->ldctl_oid, LDAP_DB_COPY_CONTROL, TRUE) != 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    dwError = VmDirParseDBCopyReplyControlContent(ppSearchResCtrls[0],
                                                  pDbCopyCtrlVal);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VmDirFreeCtrlContent(&dbCopyCtl);

    if (ppSearchResCtrls)
    {
        ldap_controls_free(ppSearchResCtrls);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirDoDbCopyLdapSearch: Error %d", dwError);
    goto cleanup;
}

static
int
_VmDirGetRemoteDBFileUsingLDAP(
    PCSTR       pszHostname,
    PSTR        pszDcAccountPwd,
    PCSTR       dbRemoteFilename,
    PCSTR       pszLocalFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDbMapSizeMb
    )
{
    DWORD                       dwError = 0;
    LDAP*                       pLd = NULL;
    int                         localFileFd = -1;
    PSTR                        pszLocalErrorMsg = NULL;
    DWORD                       dwWriteSize = 0;
    VDIR_DB_COPY_CONTROL_VALUE  localDbCopyCtrlValue = {0};
#define VMDIR_LDAP_DB_READ_BLOCK_SIZE     (1<<23)

    dwError = VmDirAllocateStringA(dbRemoteFilename, &localDbCopyCtrlValue.pszPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pszHostname,
                gVmdirServerGlobals.dcAccountUPN.lberbv_val,
                pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((localFileFd = creat(pszLocalFilename, S_IRUSR|S_IWUSR)) < 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingLDAP: creat local file %s failed, error %d", pszLocalFilename, errno);
    }

    localDbCopyCtrlValue.fd = -1;
    localDbCopyCtrlValue.dwBlockSize = VMDIR_LDAP_DB_READ_BLOCK_SIZE;

    for (;;)
    {
        dwError = _VmDirDoDbCopyLdapSearch(pLd, &localDbCopyCtrlValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (localDbCopyCtrlValue.dwDataLen > 0)
        {
            dwWriteSize = (UINT32)write(localFileFd, localDbCopyCtrlValue.pszData, localDbCopyCtrlValue.dwDataLen);
            if (dwWriteSize == -1 || dwWriteSize < localDbCopyCtrlValue.dwDataLen)
            {
                dwError = VMDIR_ERROR_IO;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                    "_VmDirGetRemoteDBFileUsingLDAP: write() call failed, recvSize: %d, writeSize: %d",
                    localDbCopyCtrlValue.dwDataLen, dwWriteSize );
            }

            if (localDbCopyCtrlValue.dwDataLen < VMDIR_LDAP_DB_READ_BLOCK_SIZE)
            {
                break;
            }
        }
        else
        {
            break;
        }

        localDbCopyCtrlValue.pszPath[0] = '\0';
        VMDIR_SAFE_FREE_MEMORY(localDbCopyCtrlValue.pszData);
        localDbCopyCtrlValue.dwDataLen = 0;
    }

cleanup:

    if (localFileFd != -1)
    {
        close(localFileFd);
    }

    VMDIR_SAFE_FREE_STRINGA(localDbCopyCtrlValue.pszPath);
    VMDIR_SAFE_FREE_MEMORY(localDbCopyCtrlValue.pszData);
    VDIR_SAFE_UNBIND_EXT_S(pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingLDAP: Error %d", dwError);
    goto cleanup;
}

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       dbRemoteFilename,
    PCSTR       localFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDbMapSizeMb
    )
{
//read block size of one MB.
#define VMDIR_DB_READ_BLOCK_SIZE     (1<<23)

    DWORD       retVal = 0;
    int         localFileFd = 0;

    FILE *      pRemoteFile = 0;
    UINT32      dwCount = 0;
    PBYTE       pReadBuffer = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    DWORD       writeSize = 0;
    long long   readSizeTotal = 0;
    unsigned long readInc = 0;
    unsigned long totalWriteMb = 0;

    retVal = VmDirOpenDatabaseFile( hServer, dbRemoteFilename, &pRemoteFile);

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirOpenDatabaseFile failed on remote file %s with error: %d", dbRemoteFilename, retVal );

    if ((localFileFd = creat(localFilename, S_IRUSR|S_IWUSR)) < 0)
    {

        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: creat local file %s failed, error %d",dbRemoteFilename, errno);
    }

    retVal = VmDirAllocateMemory(VMDIR_DB_READ_BLOCK_SIZE, (PVOID)&pReadBuffer );
    BAIL_ON_VMDIR_ERROR(retVal);

    for (;;)
    {
        retVal = VmDirReadDatabaseFile( hServer, pRemoteFile, &dwCount, pReadBuffer, VMDIR_DB_READ_BLOCK_SIZE);

        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirReadDatabaseFile() failed on remote file %s with error: %d", dbRemoteFilename, retVal );

        if (dwCount==0)
        {
            break;
        }

        writeSize = (UINT32)write(localFileFd, pReadBuffer, dwCount);

        if (writeSize < dwCount)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                    "_VmDirGetRemoteDBFileUsingRPC: write() call failed, recvSize: %d, writeSize: %d.",
                    dwCount, writeSize );
        }
        readSizeTotal += (long long)dwCount;
        readInc += dwCount;
        if (readInc >= (1L<<30))
        {
            // Log progress every 1 GB read.
            readInc = 0;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingRPC: remote file %s %lu MB copied ...",
                   dbRemoteFilename, readSizeTotal/(1L<<20));
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

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Complete copying remote file %s with actual size %lu MB", dbRemoteFilename, totalWriteMb);

cleanup:
    if (pRemoteFile != NULL)
    {
        DWORD       localRetVal = 0;
        if ((localRetVal = VmDirCloseDatabaseFile( hServer, &pRemoteFile )) != 0)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirCloseDatabaseFile() call failed with error: %d",
                      localRetVal );
        }
        retVal = (retVal != 0) ? retVal : localRetVal;
    }

    if (localFileFd >= 0)
    {
        close(localFileFd);
    }

    VMDIR_SAFE_FREE_MEMORY(pReadBuffer);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
DWORD
_VmDirMkdir(
    PCSTR path,
    int mode
    )
{
    DWORD   dwError = 0;

    if(mkdir(path, mode)!=0)
    {
        dwError = VMDIR_ERROR_IO;
        goto error;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirMkdir on dir %s failed (%u) errno: (%d)", path, dwError, errno);
    goto cleanup;
}
