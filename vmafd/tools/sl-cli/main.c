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

    dwError = VmAfdParseBaseArgs(
            argc,
            argv,
            &pszNetworkAddress,
            &pszDomain,
            &pszUserName,
            &pszPassword
            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdValidateBaseArgs(
            pszNetworkAddress,
            pszDomain,
            pszUserName,
            pszPassword
            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CreateSuperlogWrapper(
            pszNetworkAddress,
            pszDomain,
            pszUserName,
            pszPassword,
            &pSuperlogWrapper
            );
    BAIL_ON_VMAFD_ERROR(dwError);

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
    BOOLEAN bEnable     = FALSE;
    BOOLEAN bIsEnabled  = FALSE;
    BOOLEAN bDisable    = FALSE;
    BOOLEAN bSetSize    = FALSE;
    BOOLEAN bGetSize    = FALSE;
    BOOLEAN bRetrieve   = FALSE;
    BOOLEAN bFlush      = FALSE;
    BOOLEAN bAggregate  = FALSE;

    dwError = VmAfdParseOperationArgs(
                argc,
                argv,
                &bEnable,
                &bIsEnabled,
                &bDisable,
                &bSetSize,
                &bGetSize,
                &bRetrieve,
                &bFlush,
                &bAggregate
                );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdValidateOperationArgs(
                bEnable,
                bIsEnabled,
                bDisable,
                bSetSize,
                bGetSize,
                bRetrieve,
                bFlush,
                bAggregate
                );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bEnable)
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
        dwError = ClearSuperlogBuffer(pSuperlogWrapper);
    }
    else if (bAggregate)
    {
        dwError = _performAggregationAndPrint(argc, argv, pSuperlogWrapper);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    printf("%s %d", "Failed to execute operation. Error: ", dwError);
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

    dwError = VmAfdParseSetSizeArgs(argc, argv, &dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdValidateSetSizeArgs(dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = SetSuperlogBufferSize(pSuperlogWrapper, dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

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
    BAIL_ON_VMAFD_ERROR(dwError);

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
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Super Logger buffer size: %d\n", dwSize);

cleanup:
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
    PVMAFD_SUPERLOG_ENTRY_ARRAY pEntries = NULL;
    DWORD dwCDCState = 0;
    DWORD dwErrorCode = 0;
    UINT32 dwHBLastPing = 0;
    UINT32 dwIsAlve = 0;
    UINT32 time       = 0;
    UINT32 *pEnumerationCookie = NULL;

    const char *szHeaders[] = { "DC_NAME", "DC_ADDRESS", "CDC_STATE", "ERROR_CODE", "HB_LAST_PING", "IS_ALIVE", "EXEC_TIME" };
    const int iIndents[] = { 4, 4, 4, 4, 4, 4, 4 };

    unsigned int i;

    dwError = RetrieveSuperlogEntries(
            pSuperlogWrapper,
            &pEntries,
            &pEnumerationCookie,
            0,
            FALSE
            );
    BAIL_ON_VMAFD_ERROR(dwError);


    for (i = 0; i < pEntries->dwCount; i++)
    {
        dwCDCState = pEntries->entries[i].dwState;
        dwErrorCode = pEntries->entries[i].dwErrorCode;
        dwHBLastPing = pEntries->entries[i].dwCDCLastPing;
        dwIsAlve = pEntries->entries[i].bHBIsAlive;
        time = pEntries->entries[i].iEndTime - pEntries->entries[i].iStartTime;

        printf("--------------------\n");
        printf("%-*s%s: %s\n", iIndents[0], "", szHeaders[0], pEntries->entries[i].pszDCName);
        if(IsNullOrEmptyString(pEntries->entries[i].pszDCAddress))
        {
            printf("%-*s%s: %s\n", iIndents[1], "", szHeaders[1], "");
        }
        else
        {
            printf("%-*s%s: %s\n", iIndents[1], "", szHeaders[1], pEntries->entries[i].pszDCAddress);
        }
        printf("%-*s%s: %d\n", iIndents[2], "", szHeaders[2], dwCDCState);
        printf("%-*s%s: %d\n", iIndents[3], "", szHeaders[3], dwErrorCode);
        printf("%-*s%s: %d ms\n", iIndents[4], "", szHeaders[4], dwHBLastPing);
        printf("%-*s%s: %d\n", iIndents[5], "", szHeaders[5], dwIsAlve);
        printf("%-*s%s: %d ms\n", iIndents[6], "", szHeaders[6], time);
    }

cleanup:
    return dwError;

error:
    goto cleanup;

    return dwError;
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
    return dwError;
}




static
int
VmAfdMain(int argc, char* argv[])
{
    DWORD dwError = 0;
    PSUPERLOG_WRAPPER pSuperlogWrapper = NULL;
    PSTR pszErrorMessage = NULL;

    dwError = _createSuperlogWrapper(argc, argv, &pSuperlogWrapper);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _parseAndPerformOperation(argc, argv, pSuperlogWrapper);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("SUCCESS\n");

cleanup:
    FreeSuperlogWrapper(pSuperlogWrapper);
    VMAFD_SAFE_FREE_MEMORY(pszErrorMessage);
    return dwError;

error:
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

    dwError = VmAfdAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmAfdAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdMain(argc, ppszArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMAFD_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VmAfdFreeMemory(ppszArgs);
    }
    return dwError;
}

#else

int main(int argc, char* argv[])
{
    return VmAfdMain(argc, argv);
}

#endif

