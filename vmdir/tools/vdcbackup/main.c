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



/*
 * Module Name: vdcbackup
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcbackup main module entry point
 *
 */

#include "includes.h"

DWORD
BackupDB(PCSTR srcDir, PCSTR tgtDir)
{
#define VMDIR_MDB_DATA_FILE_NAME "data.mdb"
#define VMDIR_LOCK_DATA_FILE_NAME "lock.mdb"

    DWORD       dwError = 0;
    char        dbLocalFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR        pszLocalErrorMsg = NULL;
    char        cpFileCmdLine[4 /* max of "cp" and "copy" */ + 1 + VMDIR_MAX_FILE_NAME_LEN + 1 +
                              VMDIR_MAX_FILE_NAME_LEN] = {0};

#ifndef _WIN32
    const char * cpFileCmd = "cp";
    const char   fileSeperator = '/';
#else
    const char * cpFileCmd = "copy";
    const char   fileSeperator = '\\';
#endif

    printf( "BackupDB: Setting vmdir state to VMDIRD_READ_ONLY \n" );
    dwError = VmDirSetState( NULL, VMDIRD_STATE_READ_ONLY );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirSetState() call failed with error: %d", dwError  );

    // Backup data.mdb

    dwError = VmDirStringPrintFA( dbLocalFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", srcDir, fileSeperator,
                                  VMDIR_MDB_DATA_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirStringPrintFA() call failed with error: %d", dwError );

    dwError = VmDirStringPrintFA( cpFileCmdLine, sizeof(cpFileCmdLine), "%s %s %s", cpFileCmd, dbLocalFilename, tgtDir  );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirStringPrintFA() call failed with error: %d", dwError );

    printf( "BackupDB: Backing up: %s \n", dbLocalFilename );

    dwError = VmDirRun(cpFileCmdLine);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirRun() call failed, cmd: %s", cpFileCmdLine );

    // Backup lock.mdb

    dwError = VmDirStringPrintFA( dbLocalFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", srcDir, fileSeperator,
                                  VMDIR_LOCK_DATA_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirStringPrintFA() call failed with error: %d", dwError );

    dwError = VmDirStringPrintFA( cpFileCmdLine, sizeof(cpFileCmdLine), "%s %s %s", cpFileCmd, dbLocalFilename, tgtDir  );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirStringPrintFA() call failed with error: %d", dwError );

    printf( "BackupDB: Backing up: %s \n", dbLocalFilename );

    dwError = VmDirRun(cpFileCmdLine);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "BackupDB: VmDirRun() call failed, cmd: %s", cpFileCmdLine );

cleanup:
    printf( "BackupDB: Setting vmdir state to VMDIRD_NORMAL \n" );

    if ((dwError = VmDirSetState( NULL, VMDIRD_STATE_NORMAL )) != 0)
    {
        fprintf(stderr, "BackupDB: Setting vmdir state to VMDIRD_NORMAL failed, error (%d) \n", dwError);
    }

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:
    fprintf( stderr, "%s\n", pszLocalErrorMsg ? pszLocalErrorMsg : "Hmmm ... no local error message."  );
    goto cleanup;
}

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s srcpath dstpath\n", argv[0]);
        exit(1);
    }

    dwError = BackupDB(argv[1], argv[2]);
    exit(dwError);
}

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VmDirFreeMemory(ppszArgs);
    }

    return dwError;
}
#else

int main(int argc, char* argv[])
{
    return VmDirMain(argc, argv);
}

#endif
