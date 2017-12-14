/*
 * Copyright ©2017 VMware, Inc.  All Rights Reserved.
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

#define MAX_DC_CONNECT_DURATION_TIME_SEC    60*10*6

#define MAX_DC_CONNECT_SLEEP_TIME_SEC       60*10

#define UNIT_DC_CONNECT_SLEEP_TIME_SEC      5

static
DWORD
_VmDirDCConnThreadFun(
    PVOID pArg
    );

static
DWORD
_VmDirConnectToDC(
    PVMDIR_DC_CONNECTION pDCConn
    );

static
DWORD
_VmDirGetDCCredsFromRegistry(
    PVMDIR_CONNECTION_CREDS pDCCreds
    );

static
BOOLEAN
_VmDirHasCredInfo(
    PVMDIR_CONNECTION_CREDS pCreds
    );

static
PCSTR
_VmDirDCConnType(
    VMDIR_DC_CONNECTION_TYPE connType
    );

VOID
VmDirFreeConnCredContent(
    PVMDIR_CONNECTION_CREDS pCreds
    )
{
    if (pCreds)
    {
        VMDIR_SAFE_FREE_STRINGA(pCreds->pszUPN);
        VMDIR_SECURE_FREE_STRINGA(pCreds->pszPassword);
        VMDIR_SECURE_FREE_STRINGA(pCreds->pszOldPassword);
    }
}

VOID
VmDirFreeDCConnContent(
    PVMDIR_DC_CONNECTION pDCConn
    )
{
    if (pDCConn)
    {
        VmDirFreeConnCredContent(&pDCConn->creds);
        VMDIR_SAFE_FREE_STRINGA(pDCConn->pszRemoteDCHostName);
        VDIR_SAFE_UNBIND_EXT_S(pDCConn->pLd);
    }
}

DWORD
VmDirInitDCConnThread(
    PVMDIR_DC_CONNECTION pDCConn
    )
{
    DWORD dwError = 0;
    VMDIR_THREAD    threadId = {0};

    if (!pDCConn)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // handle over pDCConn ownership to DCConnThread
    pDCConn->connState = DC_CONNECTION_STATE_CONNECTING;

    dwError = VmDirCreateThread(
        &threadId,
        FALSE,  // no thread join
        _VmDirDCConnThreadFun,
        pDCConn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);

    if (pDCConn)
    {
        pDCConn->connState = DC_CONNECTION_STATE_NOT_CONNECTED;
    }
    goto cleanup;
}

/*
 * Handle connection to a remote DC.
 * At the end of this function, pDCConn->connState should be either
 * 1. DC_CONNECTION_STATE_FAILED or
 * 2. DC_CONNECTION_STATE_CONNECTED
 */
static
DWORD
_VmDirDCConnThreadFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;
    DWORD dwSleepTimeSec = 0;
    DWORD dwThrStartTime = time(NULL);
    BOOLEAN bHasConnection = FALSE;
    PVMDIR_DC_CONNECTION pDCConn = (PVMDIR_DC_CONNECTION)pArg;

    assert(pDCConn->connState == DC_CONNECTION_STATE_CONNECTING);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
        "%s user (%s) connecting to (%s) started",
        __FUNCTION__,
        VDIR_SAFE_STRING(pDCConn->creds.pszUPN),
        VDIR_SAFE_STRING(pDCConn->pszRemoteDCHostName));

    while (TRUE)
    {
        if (!_VmDirHasCredInfo(&pDCConn->creds))
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);
        }

        dwError = _VmDirConnectToDC(pDCConn);
        if (dwError == 0)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                "%s user (%s) connected to (%s) done",
                __FUNCTION__,
                VDIR_SAFE_STRING(pDCConn->creds.pszUPN),
                VDIR_SAFE_STRING(pDCConn->pszRemoteDCHostName));

            bHasConnection = TRUE;
            goto cleanup;
        }

        if (pDCConn->connType == DC_CONNECTION_TYPE_BASIC)
        {
            goto error;  // no retry, bail
        }

        if (dwError == VMDIR_ERROR_USER_INVALID_CREDENTIAL)
        {
            dwSleepTimeSec = MAX_DC_CONNECT_SLEEP_TIME_SEC;
        }
        else if (dwError == VMDIR_ERROR_SERVER_DOWN || dwError == VMDIR_ERROR_NETWORK_TIMEOUT)
        {
            dwSleepTimeSec = UNIT_DC_CONNECT_SLEEP_TIME_SEC * pDCConn->dwConsecutiveFailAttempt;
        }
        else
        {
            goto error;
        }

        if (dwSleepTimeSec >= MAX_DC_CONNECT_SLEEP_TIME_SEC)
        {
            dwSleepTimeSec = MAX_DC_CONNECT_SLEEP_TIME_SEC;
        }

        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
            "%s URGENT %s (%s) connection to (%s) failed (%d) times, last error (%d), sleep (%d)",
            __FUNCTION__,
            _VmDirDCConnType(pDCConn->connType),
            VDIR_SAFE_STRING(pDCConn->creds.pszUPN),
            VDIR_SAFE_STRING(pDCConn->pszRemoteDCHostName),
            pDCConn->dwConsecutiveFailAttempt,
            pDCConn->dwlastFailedError,
            dwSleepTimeSec);

        if (time(NULL) - dwThrStartTime >= MAX_DC_CONNECT_DURATION_TIME_SEC)
        {
            // bail to allow deleted RA cache clean up
            BAIL_WITH_VMDIR_ERROR(dwError, pDCConn->dwlastFailedError);
        }

        while (dwSleepTimeSec)
        {
            VmDirSleep(1000);  // pause 1 second
            dwSleepTimeSec--;

            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }
        }
    }

cleanup:
    VmDirFreeConnCredContent(&pDCConn->creds);

    // set connState before transfer pDCConn ownership back to calling thread
    if (bHasConnection)
    {
        pDCConn->connState = DC_CONNECTION_STATE_CONNECTED;
    }
    else
    {
        pDCConn->connState = DC_CONNECTION_STATE_FAILED;
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "%s user (%s) connect to (%s) failed (%d), connection state set to failed",
        __FUNCTION__,
        VDIR_SAFE_STRING(pDCConn->creds.pszUPN),
        VDIR_SAFE_STRING(pDCConn->pszRemoteDCHostName),
        dwError);

    goto cleanup;
}

static
BOOLEAN
_VmDirHasCredInfo(
    PVMDIR_CONNECTION_CREDS    pCreds
    )
{
    BOOLEAN bRtn = TRUE;

    if (pCreds->bUseDCAccountCreds)
    {
        if (_VmDirGetDCCredsFromRegistry(pCreds))
        {
            bRtn = FALSE;   // failed to get DC creds
        }
    }
    else
    {
        if (!pCreds->pszUPN || !pCreds->pszPassword)
        {
            bRtn = FALSE;   // caller failed to supply credentials
        }
    }

    return bRtn;
}

static
DWORD
_VmDirConnectToDC(
    PVMDIR_DC_CONNECTION pDCConn
    )
{
    DWORD   dwError = 0;
    LDAP*   pLocalLd = NULL;

    VDIR_SAFE_UNBIND_EXT_S(pDCConn->pLd);

    dwError = VmDirSafeLDAPBindExt1(
            &pLocalLd,
            pDCConn->pszRemoteDCHostName,
            pDCConn->creds.pszUPN,
            pDCConn->creds.pszPassword,
            pDCConn->dwConnectTimeoutSec);

    if (dwError == VMDIR_ERROR_USER_INVALID_CREDENTIAL &&
        pDCConn->creds.pszOldPassword)
    {
        dwError = VmDirSafeLDAPBindExt1(
                &pLocalLd,
                pDCConn->pszRemoteDCHostName,
                pDCConn->creds.pszUPN,
                pDCConn->creds.pszOldPassword,
                pDCConn->dwConnectTimeoutSec);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pDCConn->dwConsecutiveFailAttempt = 0;
    pDCConn->dwlastFailedError = 0;
    pDCConn->iLastFailedTime = 0;

    pDCConn->pLd = pLocalLd;
    pLocalLd = NULL;

    // TODO
    // metric set connection duration

cleanup:
    return dwError;

error:
    pDCConn->dwlastFailedError = dwError;
    pDCConn->iLastFailedTime = time(NULL);
    pDCConn->dwConsecutiveFailAttempt++;

    // TODO
    // metric set connection failed count

    VDIR_SAFE_UNBIND_EXT_S(pLocalLd);

    goto cleanup;
}

static
DWORD
_VmDirGetDCCredsFromRegistry(
    PVMDIR_CONNECTION_CREDS pDCCreds
    )
{
    DWORD   dwError = 0;

    VmDirFreeConnCredContent(pDCCreds);

    dwError = VmDirAllocateStringA(
        gVmdirServerGlobals.dcAccountUPN.lberbv.bv_val,
        &pDCCreds->pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pDCCreds->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirReadDCAccountOldPassword(&pDCCreds->pszOldPassword);  // ignore error

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
PCSTR
_VmDirDCConnType(
    VMDIR_DC_CONNECTION_TYPE connType
    )
{
    struct
    {
        VMDIR_DC_CONNECTION_TYPE    connType;
        PCSTR                       pszConnTypeName;
    }
    static connTypeToNameTable[] =
    {
        { DC_CONNECTION_TYPE_BASIC,         "basic" },
        { DC_CONNECTION_TYPE_REPL,          "replication" },
        { DC_CONNECTION_TYPE_CLUSTER_STATE, "cluster state" },
    };

    return connType >= VMDIR_ARRAY_SIZE(connTypeToNameTable) ?
             VMDIR_PCSTR_UNKNOWN :  connTypeToNameTable[connType].pszConnTypeName;
}
