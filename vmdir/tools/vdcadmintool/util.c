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
#ifdef _WIN32
#define strdup _strdup
#endif


/*
 * SRP parameters
 */
static PSTR pszNetworkAddress;
static PSTR pszUserName;
static PSTR pszDomain;
static PSTR pszPassword;
static PVMDIR_SERVER_CONTEXT g_hServer;

VOID
VdcadminForceResetPassword(
    VOID
    )
{
    char        pszUPN[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwByteSize = 0;

    VmDirReadString(
        "  Please enter account UPN : ",
        pszUPN,
        SIZE_256,
        FALSE);

    dwError = VmDirForceResetPassword( pszUPN, &pLocalByte, &dwByteSize );
    if (dwError == 0)
    {
        DWORD dwCnt = 0;
        printf("New password is -\n");
        for (dwCnt=0; dwCnt<dwByteSize; dwCnt++)
        {
            printf("%c", pLocalByte[dwCnt]);
        }
        printf("\n");
    }
    else
    {
        printf("VmDirForceResetPassword failed (%u)\n", dwError);
    }

    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}

VOID
VdcadminSetLogParameters(
    VOID
    )
{
    DWORD       dwError = 0;
    char        pszLogLevel[SIZE_256] = {0};
    char        pszLogMask[SIZE_256] = {0};
    int         iMask = 0;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    hServer = g_hServer;
    VmDirReadString(
        "  Log level (ERROR|WARNING|INFO|VERBOSE|DEBUG): ",
        pszLogLevel,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "  Log mask : ",
        pszLogMask,
        SIZE_256,
        FALSE);

    iMask = atoi(pszLogMask);

    dwError = VmDirSetLogLevelH(hServer, pszLogLevel);
    if (dwError)
    {
        printf( "VmDirSetLogLevel failed (%u)\n", dwError);
            goto error;
    }

    dwError = VmDirSetLogMaskH(hServer, iMask);
    if (dwError)
    {
        printf( "VmDirSetLogMask failed (%u)\n", dwError);
        goto error;
    }
error:
    return;
}

VOID
VdcadminGetLogParameters(
    VOID
    )
{
    DWORD       dwError = 0;
    UINT32      iMask = 0;
    VMDIR_LOG_LEVEL logLevel = 0;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    PCSTR       pszLevel = NULL;

    hServer = g_hServer;

    dwError = VmDirGetLogLevelH(hServer, &logLevel);
    if (dwError)
    {
        printf( "VmDirGetLogLevel failed (%u)\n", dwError);
        goto error;
    }
    printf( "Log level ");
    switch (logLevel)
    {
        case VMDIR_LOG_ERROR:   pszLevel = "ERROR";     break;
        case VMDIR_LOG_WARNING: pszLevel = "WARNING";   break;
        case VMDIR_LOG_INFO:    pszLevel = "INFO";      break;
        case VMDIR_LOG_VERBOSE: pszLevel = "VERBOSE";   break;
        case VMDIR_LOG_DEBUG:   pszLevel = "DEBUG";     break;
        default:                pszLevel = "UNKNOWN";   break;
    }
    printf( "%s\n\n", pszLevel);

    dwError = VmDirGetLogMaskH(hServer, &iMask);
    if (dwError)
    {
        printf( "VmDirGetLogMask failed (%u)\n", dwError);
        goto error;
    }
    printf( "Log mask (%u)\n\n", iMask);

error:
    return;
}

VOID
VdcadminSetVmdirState(
    VOID
    )
{
    DWORD       dwError = 0;
    char        pszState[SIZE_256] = {0};
    PSTR        pszLocalErrorMsg = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    VDIR_SERVER_STATE vmdirState = VMDIRD_STATE_NORMAL;

    VmDirReadString(
        "Enter state (NORMAL|READ_ONLY): ",
        pszState,
        SIZE_256,
        FALSE);

    if ( VmDirStringCompareA( pszState, "NORMAL", FALSE) == 0 )
    {
        vmdirState = VMDIRD_STATE_NORMAL;
    }
    else if ( VmDirStringCompareA( pszState, "READ_ONLY", FALSE) == 0 )
    {
        vmdirState = VMDIRD_STATE_READ_ONLY;
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (g_hServer)
    {
        hServer = g_hServer;
    }

    dwError = VmDirSetState(hServer, vmdirState );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "VmDirSetState() failed. error(%u)", dwError );

    printf("\n\n State of Vmdir set to %s\n\n", pszState);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return;

error:

    printf("\n SetVmDirState failed: %s\n", VDIR_SAFE_STRING(pszLocalErrorMsg));

    goto cleanup;
}

VOID
VdcadminGetVmdirState(
    VOID
    )
{
    DWORD       dwError = 0;
    PCSTR       pszState = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    UINT32      vmdirState = 0;


    if (g_hServer)
    {
        hServer = g_hServer;
    }

    dwError = VmDirGetState(hServer, &vmdirState );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "VmDirGetState() failed. error(%u)", dwError );

    switch (vmdirState)
    {
        case VMDIRD_STATE_UNDEFINED: pszState = "Undefined"; break;
        case VMDIRD_STATE_STARTUP:   pszState = "Startup"; break;
        case VMDIRD_STATE_READ_ONLY: pszState = "Read only"; break;
        case VMDIRD_STATE_NORMAL:    pszState = "Normal" ; break;
        case VMDIRD_STATE_SHUTDOWN:  pszState = "Shutdown"; break;
    }
    printf("\n\n VmDir State is - %s\n\n", pszState);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return;

error:

    printf("\n GetVmDirState failed: %s\n", VDIR_SAFE_STRING(pszLocalErrorMsg));

    goto cleanup;
}

VOID
VdcadminReplNow(
    VOID
    )
{

    DWORD       dwError = 0;
    char        pszServerName[SIZE_256];
    PSTR        pszLocalErrorMsg = NULL;

    VmDirReadString(
        " Enter hostname to start replication cycle: ",
        pszServerName,
        SIZE_256,
        FALSE);

    dwError = VmDirReplNow( pszServerName );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "VdcadminReplNow: VmDirReplNow() call failed with error: %d", dwError  );

    printf( "VmDirReplNow passed\n");

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return;

error:

    printf( "%s\n", pszLocalErrorMsg ? pszLocalErrorMsg : "Hmmm ... no local error message."  );
    goto cleanup;
}

VOID
VdcadminSetSRPAuthData(
    VOID
    )
{
    CHAR inBuf[512] = {0};
    DWORD dwError = 0;

    if (pszUserName)
    {
        free(pszUserName);
    }
    if (pszDomain)
    {
        free(pszDomain);
    }
    if (pszPassword)
    {
        free(pszPassword);
    }
    VmDirReadString(
        "VMDIR IP address: ",
        inBuf,
        sizeof(inBuf),
        FALSE);
    if (inBuf[0])
    {
        pszNetworkAddress = strdup(inBuf);
    }

    VmDirReadString(
        "SRP Username: ",
        inBuf,
        sizeof(inBuf),
        FALSE);
    if (inBuf[0])
    {
        pszUserName = strdup(inBuf);
    }

    VmDirReadString(
        "SRP Domain: ",
        inBuf,
        sizeof(inBuf),
        FALSE);
    if (inBuf[0])
    {
        pszDomain = strdup(inBuf);
    }

    VmDirReadString(
        "SRP Password: ",
        inBuf,
        sizeof(inBuf),
        TRUE);
    if (inBuf[0])
    {
        pszPassword = strdup(inBuf);
    }

    if (g_hServer)
    {
        VmDirCloseServer(g_hServer);
    }
    if (dwError)
    {
        printf("VmDirCreateBindingHandleAuthA: failed %x\n", dwError);
    }

    dwError = VmDirOpenServerA(
                  pszNetworkAddress,
                  pszUserName,
                  pszDomain,
                  pszPassword,
                  0,
                  NULL,
                  &g_hServer);
    if (dwError)
    {
        printf("VmDirOpenServerA: failed %x\n", dwError);
    }
}

