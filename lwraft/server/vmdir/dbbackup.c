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


#include "includes.h"

static
DWORD
_VmDirCopyDBFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath
    );

static
DWORD
_VmDirCopyXlogFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath,
    DWORD   dwLowNum,
    DWORD   dwHighNum
    );

static
DWORD
_VmDirValidateBackupPath(
    PCSTR       pszBackupPath
    );

static
DWORD
_VmDirCopyFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath,
    PCSTR   pszFileName
    );

VOID
VmDirSrvSetMDBStateClear(
    VOID
    )
{
    DWORD   dwXlogNum = 0;
    DWORD   dwDbSizeMb = 0;
    DWORD   dwDbMapSizeMb = 0;
    CHAR    bufDBPath[VMDIR_SIZE_256] = {0};

    VmDirSetMdbBackendState(
        MDB_STATE_CLEAR,
        &dwXlogNum,
        &dwDbSizeMb,
        &dwDbMapSizeMb,
        bufDBPath,
        VMDIR_SIZE_256);
}

DWORD
VmDirSrvBackupDB(
    PCSTR       pszBackupPath
    )
{
    DWORD   dwError = 0;
    DWORD   low_xlognum = 0;
    DWORD   high_xlognum = 0;
    DWORD   remoteDbSizeMb = 0;
    DWORD   remoteDbMapSizeMb = 0;
    CHAR    bufCurrentDBPath[VMDIR_MAX_FILE_NAME_LEN] = {0};
    DWORD   dwMdbWalEnable = 1;
    BOOLEAN bClearMDBState = FALSE;
    MDB_state_op    mdbMode = MDB_STATE_KEEPXLOGS;

    if (IsNullOrEmptyString(pszBackupPath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError  = _VmDirValidateBackupPath(pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    // default mode is to enable WAL
    dwError = VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_MDB_ENABLE_WAL,
        &dwMdbWalEnable,
        1);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwMdbWalEnable)
    {
        mdbMode = MDB_STATE_KEEPXLOGS;
    }
    else
    {
        mdbMode = MDB_STATE_READONLY;
    }

    dwError = VmDirSetMdbBackendState(
        mdbMode,
        &low_xlognum,
        &remoteDbSizeMb,
        &remoteDbMapSizeMb,
        bufCurrentDBPath,
        VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR(dwError);

    bClearMDBState = TRUE;

    dwError = _VmDirCopyDBFile(bufCurrentDBPath, pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (low_xlognum > 0)
    {
        dwError = VmDirSetMdbBackendState(
            MDB_STATE_GETXLOGNUM,
            &high_xlognum,
            &remoteDbSizeMb,
            &remoteDbMapSizeMb,
            bufCurrentDBPath,
            VMDIR_MAX_FILE_NAME_LEN);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (low_xlognum > high_xlognum)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
        }

        dwError = _VmDirCopyXlogFile(bufCurrentDBPath, pszBackupPath, low_xlognum, high_xlognum);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    if (bClearMDBState)
    {
        VmDirSrvSetMDBStateClear();
    }

    return dwError;

error:
    goto cleanup;
}

/*
 * Either not exists and can create
 * or
 * exists but empty
 */
static
DWORD
_VmDirValidateBackupPath(
    PCSTR       pszBackupPath
    )
{
    DWORD   dwError = 0;
    BOOLEAN bExists = FALSE;
    DWORD   dwCnt = 0;
    struct dirent * pDirEntry = NULL;
    DIR *           pDir = NULL;

    dwError = VmDirPathExists(pszBackupPath, &bExists);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bExists)
    {
        if ((pDir = opendir(pszBackupPath)) == NULL)
        {   // Not a directory
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PATH);
        }

        while ((pDirEntry = readdir(pDir)) != NULL)
        {
          if(++dwCnt > 2)
          {   // NOT an empty directory
              BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PATH);
          }
        }
    }
    else
    {
        dwError = mkdir(pszBackupPath, S_IRUSR | S_IWUSR | S_IXUSR);
        if (dwError)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }
    }

cleanup:
    if (pDir)
    {
        closedir(pDir);
    }
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirCopyFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath,
    PCSTR   pszFileName
    )
{
    DWORD   dwError = 0;
    CHAR    bufSrcFile[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    bufDstFile[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR    pszCpCmd = NULL;

    dwError = VmDirStringPrintFA(bufSrcFile, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", pszSourcePath, pszFileName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(bufDstFile, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", pszBackupPath, pszFileName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszCpCmd, "cp %s %s", bufSrcFile, bufDstFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRun(pszCpCmd);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszCpCmd);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirCopyDBFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath
    )
{
    DWORD   dwError = 0;

    // copy data.mdb
    dwError = _VmDirCopyFile(pszSourcePath, pszBackupPath, VMDIR_MDB_DATA_FILE_NAME);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirCopyXlogFile(
    PCSTR   pszSourcePath,
    PCSTR   pszBackupPath,
    DWORD   dwLowNum,
    DWORD   dwHighNum
    )
{
    DWORD   dwError = 0;
    DWORD   dwIdx = 0;
    CHAR    bufSrcPath[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    bufDstPath[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    bufFileName[VMDIR_SIZE_32] = {0};

    dwError = VmDirStringPrintFA(bufSrcPath, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", pszSourcePath, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(bufDstPath, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", pszBackupPath, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirValidateBackupPath(bufDstPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIdx = dwLowNum; dwIdx <= dwHighNum; dwIdx++)
    {
        dwError = VmDirStringPrintFA(bufFileName, VMDIR_SIZE_32, "%u", dwIdx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirCopyFile(bufSrcPath, bufDstPath, bufFileName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
