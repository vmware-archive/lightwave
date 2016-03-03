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
_initializeLog(
        VOID
        );

static
DWORD
_createSuperlogWrapper(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER *ppSuperlogWrapper
        );

static
DWORD
_parseAndPerformOperation(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
DWORD
_performGetServerDataAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
DWORD
_performSetSize(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
DWORD
_performQueryStatusAndPrint(
    PSUPERLOG_WRAPPER pSuperlogWrapper
    );

static
DWORD
_performGetSizeAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
DWORD
_performRetrieveAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
DWORD
_performAggregationAndPrint(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

static
VOID
_printAggregatedTableInTerminal(
        PVMDIR_SUPERLOG_TABLE pSuperlogTable
        );

static
VOID
_getColumnWidths(
        int numCols,
        const int* fixedColWidths,
        const int* dynamicColWidthPercents,
        const unsigned int* enabledCols,
        const int rightPadding,
        int* colWidths
        );


static
DWORD
_initializeLog(
        VOID
        )
{
    DWORD dwError = 0;
    CHAR pszPath[MAX_PATH];
    dwError = VmDirGetVmDirLogPath(pszPath, "vdcmetric.log");
    if (!dwError)
    {
        dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL);
    }
    return dwError;
}

static
DWORD
_createSuperlogWrapper(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER *ppSuperlogWrapper
        )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszDomain = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSUPERLOG_WRAPPER pSuperlogWrapper = NULL;

    dwError = VmDirParseBaseArgs(
            argc,
            argv,
            &pszNetworkAddress,
            &pszDomain,
            &pszUserName,
            &pszPassword
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirValidateBaseArgs(
            pszNetworkAddress,
            pszDomain,
            pszUserName,
            pszPassword
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = CreateSuperlogWrapper(
            pszNetworkAddress,
            pszDomain,
            pszUserName,
            pszPassword,
            &pSuperlogWrapper
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSuperlogWrapper = pSuperlogWrapper;

cleanup:
    return dwError;

error:
    FreeSuperlogWrapper(pSuperlogWrapper);
    goto cleanup;
}

static
DWORD
_parseAndPerformOperation(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    BOOLEAN bNodeData   = FALSE;
    BOOLEAN bEnable     = FALSE;
    BOOLEAN bIsEnabled  = FALSE;
    BOOLEAN bDisable    = FALSE;
    BOOLEAN bSetSize    = FALSE;
    BOOLEAN bGetSize    = FALSE;
    BOOLEAN bRetrieve   = FALSE;
    BOOLEAN bFlush      = FALSE;
    BOOLEAN bAggregate  = FALSE;

    dwError = VmDirParseOperationArgs(
                argc,
                argv,
                &bNodeData,
                &bEnable,
                &bIsEnabled,
                &bDisable,
                &bSetSize,
                &bGetSize,
                &bRetrieve,
                &bFlush,
                &bAggregate
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirValidateOperationArgs(
                bNodeData,
                bEnable,
                bIsEnabled,
                bDisable,
                bSetSize,
                bGetSize,
                bRetrieve,
                bFlush,
                bAggregate
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bNodeData)
    {
        dwError = _performGetServerDataAndPrint(pSuperlogWrapper);
    }
    else if (bEnable)
    {
        dwError = EnableSuperlog(pSuperlogWrapper);
    }
    else if (bIsEnabled)
    {
        dwError = _performQueryStatusAndPrint(pSuperlogWrapper);
    }
    else if (bDisable)
    {
        dwError = DisableSuperlog(pSuperlogWrapper);
    }
    else if (bSetSize)
    {
        dwError = _performSetSize(argc, argv, pSuperlogWrapper);
    }
    else if (bGetSize)
    {
        dwError = _performGetSizeAndPrint(pSuperlogWrapper);
    }
    else if (bRetrieve)
    {
        dwError = _performRetrieveAndPrint(pSuperlogWrapper);
    }
    else if (bFlush)
    {
        dwError = FlushSuperlogBuffer(pSuperlogWrapper);
    }
    else if (bAggregate)
    {
        dwError = _performAggregationAndPrint(argc, argv, pSuperlogWrapper);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_performGetServerDataAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_SERVER_DATA pServerData = NULL;

    dwError = GetSuperlogServerData(pSuperlogWrapper, &pServerData);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("ServerStartupTime=%lu\n", pServerData->iServerStartupTime);
    printf("TotalAddCount=%lu\n", pServerData->iAddCount);
    printf("TotalBindCount=%lu\n", pServerData->iBindCount);
    printf("TotalDeleteCount=%lu\n", pServerData->iDeleteCount);
    printf("TotalModifyCount=%lu\n", pServerData->iModifyCount);
    printf("TotalSearchCount=%lu\n", pServerData->iSearchCount);
    printf("TotalUnbindCount=%lu\n", pServerData->iUnbindCount);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pServerData);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_performSetSize(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;

    dwError = VmDirParseSetSizeArgs(argc, argv, &dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirValidateSetSizeArgs(dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SetSuperlogBufferSize(pSuperlogWrapper, dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_performQueryStatusAndPrint(
    PSUPERLOG_WRAPPER pSuperlogWrapper
    )
{
    DWORD dwError;
    BOOLEAN bEnabled;

    dwError = IsSuperlogEnabled(pSuperlogWrapper, &bEnabled);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Super logging is currently %s\n", bEnabled ? "enabled" : "disabled");

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_performGetSizeAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;

    dwError = GetSuperlogBufferSize(pSuperlogWrapper, &dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("%d\n", dwSize);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_printSearchInformation(
        LDAP_SEARCH_INFO searchInfo
        )
{
    DWORD dwError       = 0;
    PSTR pszAttributes  = NULL;
    PSTR pszBaseDN      = NULL;
    PSTR pszScope       = NULL;
    PSTR pszFilters     = NULL;
    DWORD dwScanned     = 0;
    DWORD dwReturned    = 0;

    const char *szHeaders[] = { "ATTRIBUTE(S)", "BASE_DN", "SCOPE", "FILTER(S)", "SCANNED", "RETURNED" };
    const int iIndents[] = { 8, 8, 8, 8, 8, 8 };

    dwError = VmDirAllocateStringAFromW(searchInfo.pwszAttributes, &pszAttributes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(searchInfo.pwszBaseDN, &pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(searchInfo.pwszScope, &pszScope);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(searchInfo.pwszIndexResults, &pszFilters);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwScanned = searchInfo.dwScanned;
    dwReturned = searchInfo.dwReturned;

    printf("%-*s%s: %s\n", iIndents[0], "", szHeaders[0], pszAttributes);
    printf("%-*s%s: %s\n", iIndents[1], "", szHeaders[1], pszBaseDN);
    printf("%-*s%s: %s\n", iIndents[2], "", szHeaders[2], pszScope);
    printf("%-*s%s: %s\n", iIndents[3], "", szHeaders[3], pszFilters);
    printf("%-*s%s: %d\n", iIndents[4], "", szHeaders[4], dwScanned);
    printf("%-*s%s: %d\n", iIndents[5], "", szHeaders[5], dwReturned);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAttributes);
    VMDIR_SAFE_FREE_MEMORY(pszBaseDN);
    VMDIR_SAFE_FREE_MEMORY(pszScope);
    VMDIR_SAFE_FREE_MEMORY(pszFilters);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_performRetrieveAndPrint(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries = NULL;
    PSTR pszLoginDN     = NULL;
    PSTR pszIP          = NULL;
    PSTR pszOperation   = NULL;
    PSTR pszString      = NULL;
    DWORD dwPort        = 0;
    DWORD dwErrorCode   = 0;
    UINT64 time         = 0;
    UINT64 *pEnumerationCookie = NULL;

    const char *szHeaders[] = { "LOGIN_DN", "IP", "PORT", "OPERATION", "STRING", "ERROR_CODE", "TIME" };
    const int iIndents[] = { 0, 4, 4, 4, 8, 4, 4 };

    unsigned int i;

    dwError = RetrieveSuperlogEntries(
            pSuperlogWrapper,
            &pEntries,
            &pEnumerationCookie,
            0,
            FALSE
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pEntries->dwCount; i++)
    {
        dwError = VmDirAllocateStringAFromW(pEntries->entries[i].pwszLoginDN, &pszLoginDN);
        BAIL_ON_VMDIR_ERROR(dwError);
        dwError = VmDirAllocateStringAFromW(pEntries->entries[i].pwszClientIP, &pszIP);
        BAIL_ON_VMDIR_ERROR(dwError);
        dwError = VmDirAllocateStringAFromW(pEntries->entries[i].pwszOperation, &pszOperation);
        BAIL_ON_VMDIR_ERROR(dwError);
        dwError = VmDirAllocateStringAFromW(pEntries->entries[i].pwszString, &pszString);
        BAIL_ON_VMDIR_ERROR(dwError);
        dwPort = pEntries->entries[i].dwClientPort;
        dwErrorCode = pEntries->entries[i].dwErrorCode;
        time = pEntries->entries[i].iEndTime - pEntries->entries[i].iStartTime;

        printf("--------------------\n");
        printf("%-*s%s: %s\n", iIndents[0], "", szHeaders[0], pszLoginDN);
        printf("%-*s%s: %s\n", iIndents[1], "", szHeaders[1], pszIP);
        printf("%-*s%s: %d\n", iIndents[2], "", szHeaders[2], dwPort);
        printf("%-*s%s: %s\n", iIndents[3], "", szHeaders[3], pszOperation);

        if (!IsNullOrEmptyString(pszString))
        {
            printf("%-*s%s: %s\n", iIndents[4], "", szHeaders[4], pszString);
        }

        switch (pEntries->entries[i].opType)
        {
        case LDAP_REQ_SEARCH:
            dwError = _printSearchInformation(pEntries->entries[i].opInfo.searchInfo);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        default:
            break;
        }

        printf("%-*s%s: %d\n", iIndents[5], "", szHeaders[5], dwErrorCode);
        printf("%-*s%s: %lu ms\n", iIndents[6], "", szHeaders[6], time);

        VMDIR_SAFE_FREE_MEMORY(pszLoginDN);
        VMDIR_SAFE_FREE_MEMORY(pszIP);
        VMDIR_SAFE_FREE_MEMORY(pszOperation);
        VMDIR_SAFE_FREE_MEMORY(pszString);
    }

cleanup:
    VmDirFreeSuperLogEntryLdapOperationArray(pEntries);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLoginDN);
    VMDIR_SAFE_FREE_MEMORY(pszIP);
    VMDIR_SAFE_FREE_MEMORY(pszOperation);
    VMDIR_SAFE_FREE_MEMORY(pszString);
    goto cleanup;
}

static
DWORD
_performAggregationAndPrint(
        int argc,
        char* argv[],
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    BOOLEAN bLoginDN    = FALSE;
    BOOLEAN bIP         = FALSE;
    BOOLEAN bPort       = FALSE;
    BOOLEAN bOperation  = FALSE;
    BOOLEAN bString     = FALSE;
    BOOLEAN bErrorCode  = FALSE;
    BOOLEAN bTime       = FALSE;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries = NULL;
    UINT64 *pEnumerationCookie = NULL;
    PVMDIR_SUPERLOG_TABLE pSuperlogTable = NULL;
    VMDIR_SUPERLOG_TABLE_COLUMN_SET columnSet;

    dwError = VmDirParseAggregateArgs(
            argc,
            argv,
            &bLoginDN,
            &bIP,
            &bPort,
            &bOperation,
            &bString,
            &bErrorCode,
            &bTime
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    columnSet.isColumnSet[LOGIN_DN] = bLoginDN;
    columnSet.isColumnSet[IP] = bIP;
    columnSet.isColumnSet[PORT] = bPort;
    columnSet.isColumnSet[OPERATION] = bOperation;
    columnSet.isColumnSet[STRING] = bString;
    columnSet.isColumnSet[ERROR_CODE] = bErrorCode;
    columnSet.isColumnSet[AVG_TIME] = bTime;

    // Default columns if none is set
    if (!(bLoginDN || bIP || bPort || bOperation || bString || bErrorCode || bTime))
    {
        columnSet.isColumnSet[LOGIN_DN] = 1;
        columnSet.isColumnSet[OPERATION] = 1;
        columnSet.isColumnSet[STRING] = 1;
    }

    dwError = RetrieveSuperlogEntries(
            pSuperlogWrapper,
            &pEntries,
            &pEnumerationCookie,
            0,
            FALSE
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSuperLogGetTable(
            pEntries,
            &columnSet,
            &pSuperlogTable
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    _printAggregatedTableInTerminal(pSuperlogTable);

cleanup:
    VmDirFreeSuperLogEntryLdapOperationArray(pEntries);
    VmDirFreeSuperLogTable(pSuperlogTable);
    return dwError;

error:
    goto cleanup;
}

static
VOID
_printAggregatedTableInTerminal(
        PVMDIR_SUPERLOG_TABLE pSuperlogTable
        )
{
    const char *colStrings[VMDIR_SUPERLOG_TABLE_COL_NUM] =
    {
            "LOGIN_DN", "IP", "PORT", "OPERATION", "STRING", "ERROR_CODE", "AVG_TIME"
    };

    const int fixedColWidths[VMDIR_SUPERLOG_TABLE_COL_NUM] =
    {
            0,  20, 10, 15, 0,  15, 15
    };

    const int dynamicColWidthPercents[VMDIR_SUPERLOG_TABLE_COL_NUM] =
    {
            40, 0,  0,  0,  60, 0,  0
    };

    const char *countColString = "COUNT";

    const int countColWidth = 10;

    int colWidths[VMDIR_SUPERLOG_TABLE_COL_NUM] = {0};

    unsigned int i, j;

    _getColumnWidths(
            VMDIR_SUPERLOG_TABLE_COL_NUM,
            fixedColWidths,
            dynamicColWidthPercents,
            pSuperlogTable->cols->isColumnSet,
            countColWidth,
            colWidths);

    for (i = 0; i < VMDIR_SUPERLOG_TABLE_COL_NUM; i++)
    {
        if (pSuperlogTable->cols->isColumnSet[i])
        {
            printf("%-*.*s", colWidths[i], colWidths[i], colStrings[i]);
        }
    }
    printf("%-*.*s\n", countColWidth, countColWidth, countColString);

    for (i = 0; i< pSuperlogTable->numRows; i++)
    {
        PVMDIR_SUPERLOG_TABLE_ROW row = &pSuperlogTable->rows[i];
        for (j = 0; j < VMDIR_SUPERLOG_TABLE_COL_NUM; j++)
        {
            if (pSuperlogTable->cols->isColumnSet[j])
            {
                printf("%-*.*s", colWidths[j], colWidths[j], row->colVals[j]);
            }
        }
        printf("%-*d\n", countColWidth, row->count);
    }
}

static
VOID
_getColumnWidths(
        int numCols,
        const int* fixedColWidths,
        const int* dynamicColWidthPercents,
        const unsigned int* enabledCols,
        const int rightPadding,
        int* colWidths
        )
{
    DWORD dwError = 0;
    int windowWidth = 0;
    int remainingWindowWidth = 0;
    int percentDenom = 100;
    const int defaultWindowWidth = 512;
    int i = 0;

#ifndef _WIN32
    struct winsize ws = {0};
    dwError = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    windowWidth = dwError ? defaultWindowWidth : ws.ws_col;
#else
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    windowWidth = csbi.srWindow.Right - csbi.srWindow.Left;
#endif

    remainingWindowWidth = windowWidth - rightPadding;
    for (i = 0; i < numCols; i++)
    {
        if (enabledCols[i])
        {
            remainingWindowWidth -= fixedColWidths[i];
        }
        else if (dynamicColWidthPercents[i])
        {
            percentDenom -= dynamicColWidthPercents[i];
        }
    }

    for (i = 0; i < numCols; i++)
    {
        if (fixedColWidths[i])
        {
            colWidths[i] = fixedColWidths[i];
        }
        else if (percentDenom)
        {
            colWidths[i] = remainingWindowWidth * dynamicColWidthPercents[i] / percentDenom;
        }
        else
        {
            colWidths[i] = 0;
        }
    }
}

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD dwError = 0;
    PSUPERLOG_WRAPPER pSuperlogWrapper = NULL;
    PSTR pszErrorMessage = NULL;

    dwError = _initializeLog();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _createSuperlogWrapper(argc, argv, &pSuperlogWrapper);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _parseAndPerformOperation(argc, argv, pSuperlogWrapper);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("SUCCESS\n");

cleanup:
    FreeSuperlogWrapper(pSuperlogWrapper);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Error %d - %s\n", dwError, pszErrorMessage);

    if (dwError == ERROR_INVALID_PARAMETER)
    {
        ShowUsage();
    }

    goto cleanup;
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
