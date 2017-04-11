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


/*
 * Return all partners for the localhost using the machine account credentials.
 */
DWORD
getReplicationPartnersViaMachineAccount(
    PVMDIR_REPL_PARTNER_INFO* ppReplPartnerInfo,
    DWORD* pdwNumReplPartner
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszDCAccount = NULL;
    PSTR pszDCAccountPassword = NULL;
    PVMDIR_REPL_PARTNER_INFO  pReplPartnerInfo = NULL;
    DWORD dwNumReplPartner = 0;

    if (pdwNumReplPartner == NULL ||
        ppReplPartnerInfo == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Support for local backup only.
    dwError = VmDirGetServerName( "localhost", &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Get machine account name
    dwError = VmDirRegReadDCAccount( &pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Get machine account password
    dwError = VmDirReadDCAccountPassword( &pszDCAccountPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetReplicationPartners(
                                pszServerName,
                                pszDCAccount,
                                pszDCAccountPassword,
                                &pReplPartnerInfo,
                                &dwNumReplPartner
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReplPartnerInfo = pReplPartnerInfo;
    *pdwNumReplPartner = dwNumReplPartner;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SECURE_FREE_STRINGA(pszDCAccountPassword);

    return dwError;

error:
    goto cleanup;
}

/*
 * Write partner hostnames to file in specified directory.
 * Filename is 'partnerlist'
 */
DWORD
writePartnerList(
    PCSTR pszTgtDir
    )
{
    DWORD dwError = 0;
    PVMDIR_REPL_PARTNER_INFO pReplPartnerInfo = NULL;
    DWORD dwNumReplPartner = 0;
    DWORD dwCnt = 0;
    DWORD dwWritten = 0;
    PSTR pszPartnerHostname = NULL;
    FILE* fPartnerList = NULL;
    PSTR pszFileName = NULL;
#ifdef _WIN32
    PCSTR pszSeparator = "\\";
#else
    PCSTR pszSeparator = "/";
#endif

    if (pszTgtDir == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Get partners
    dwError = getReplicationPartnersViaMachineAccount(
                                &pReplPartnerInfo,
                                &dwNumReplPartner
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Only write file if server has partners
    if ( dwNumReplPartner > 0 )
    {

        // Build the filename, i.e., /tmp/backup/partnerlist
        dwError = VmDirAllocateStringPrintf(
                                        &pszFileName,
                                        "%s%spartnerlist",
                                        pszTgtDir,
                                        pszSeparator
                                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Open file
        fPartnerList =fopen(pszFileName, "w+");
        if (fPartnerList == NULL)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
        }

        // For each partner
        for ( dwCnt=0; dwCnt < dwNumReplPartner; dwCnt++ )
        {

            VMDIR_SAFE_FREE_MEMORY(pszPartnerHostname);

            dwError = VmDirReplURIToHostname(
                                pReplPartnerInfo[dwCnt].pszURI,
                                &pszPartnerHostname
                                );
            BAIL_ON_VMDIR_ERROR(dwError);

            // Write partner hostname to file.
            dwWritten = (DWORD)fwrite(
                                pszPartnerHostname,
                                sizeof(char),
                                VmDirStringLenA(pszPartnerHostname),
                                fPartnerList);
            if (dwWritten != VmDirStringLenA(pszPartnerHostname))
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
            }

            // Add newline to file.
            dwWritten = (DWORD)fwrite(
                                    "\n",
                                    sizeof(char),
                                    VmDirStringLenA("\n"),
                                    fPartnerList
                                    );
            if (dwWritten != VmDirStringLenA("\n"))
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_IO);
            }
        }
    }

cleanup:
    if (fPartnerList)
    {
        fclose(fPartnerList);
    }

    for (dwCnt=0; dwCnt < dwNumReplPartner; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo[dwCnt].pszURI);
    }

    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);
    VMDIR_SAFE_FREE_MEMORY(pszFileName);
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostname);
    return dwError;

error:
    goto cleanup;
}

DWORD
Cleanup(VOID)
{
    DWORD   dwError = 0;

    printf( "Cleanup: Setting vmdir state to VMDIRD_NORMAL \n" );

    if ((dwError = VmDirSetState( NULL, VMDIRD_STATE_NORMAL )) != 0)
    {
        fprintf(stderr, "Cleanup: Setting vmdir state to VMDIRD_NORMAL failed, error (%d) \n", dwError);
    }

    return dwError;
}

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;

    if (argc == 2 && VmDirStringCompareA("-c", argv[1], TRUE) == 0)
    {
        dwError = Cleanup();
    }
    else if (argc == 3)
    {
        dwError = writePartnerList(argv[2]);
        if (dwError != 0)
        {
            fprintf(stderr, "Warning: could not create partner list");
        }

        dwError = BackupDB(argv[1], argv[2]);

    }
    else
    {
        dwError = EINVAL;
        fprintf(stderr, "usage: %s srcpath dstpath\n"
                        "       %s -c\n",
                        argv[0], argv[0]);
    }

    return dwError;
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
