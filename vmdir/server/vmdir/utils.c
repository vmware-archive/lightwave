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

VOID
VmDirdStateSet(
    VDIR_SERVER_STATE   state)
{
    BOOLEAN             bInLock = FALSE;
    VDIR_BACKEND_CTX    beCtx = {0};

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);
    gVmdirGlobals.vmdirdState = state;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    if (state == VMDIRD_STATE_READ_ONLY) // Wait for the pending write transactions to be over before returning
    {
        beCtx.pBE = VmDirBackendSelect(NULL);
        assert(beCtx.pBE);

        while (beCtx.pBE->pfnBEGetLeastOutstandingUSN(&beCtx, TRUE) != 0)
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirdStateSet: Waiting for the pending write transactions to be over" );
            VmDirSleep(2*1000); // sleep for 2 seconds
        }
    }

    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDir State (%u)", state);

    return;
}

VDIR_SERVER_STATE
VmDirdState(
    VOID
    )
{
    VDIR_SERVER_STATE rtnState;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);
    rtnState = gVmdirGlobals.vmdirdState;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    return rtnState;
}

BOOLEAN
VmDirdGetRestoreMode(
    VOID
    )
{
    BOOLEAN bRestoreMode;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);
    bRestoreMode = gVmdirRunmodeGlobals.mode == VMDIR_RUNMODE_RESTORE;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);

    return bRestoreMode;
}

VMDIR_RUNMODE
VmDirdGetRunMode(
    VOID
    )
{
    VMDIR_RUNMODE runMode;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);
    runMode = gVmdirRunmodeGlobals.mode;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);

    return runMode;
}

VOID
VmDirdSetRunMode(
    VMDIR_RUNMODE mode
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);
    gVmdirRunmodeGlobals.mode = mode;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirRunmodeGlobals.pMutex);
}

VOID
VmDirdSetReplNow(
    BOOLEAN bReplNow)
{
    BOOLEAN             bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);
    gVmdirGlobals.bReplNow = bReplNow;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    return;
}

BOOLEAN
VmDirdGetReplNow(
    VOID
    )
{
    BOOLEAN     bReplNow = FALSE;
    BOOLEAN     bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);
    bReplNow = gVmdirGlobals.bReplNow;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    return bReplNow;
}

BOOLEAN
VmDirdGetAllowInsecureAuth(
    VOID
    )
{
    return gVmdirGlobals.bAllowInsecureAuth;
}

VOID
VmDirGetLdapListenPorts(
    PDWORD* ppdwLdapListenPorts,
    PDWORD  pdwLdapListenPorts
    )
{
    *ppdwLdapListenPorts = gVmdirGlobals.pdwLdapListenPorts;
    *pdwLdapListenPorts = gVmdirGlobals.dwLdapListenPorts;
}

VOID
VmDirGetLdapsListenPorts(
    PDWORD* ppdwLdapsListenPorts,
    PDWORD  pdwLdapsListenPorts
    )
{
    *ppdwLdapsListenPorts = gVmdirGlobals.pdwLdapsListenPorts;
    *pdwLdapsListenPorts = gVmdirGlobals.dwLdapsListenPorts;
}

VOID
VmDirGetLdapConnectPorts(
    PDWORD* ppdwLdapConnectPorts,
    PDWORD  pdwLdapConnectPorts
    )
{
    *ppdwLdapConnectPorts = gVmdirGlobals.pdwLdapConnectPorts;
    *pdwLdapConnectPorts = gVmdirGlobals.dwLdapConnectPorts;
}

VOID
VmDirGetLdapsConnectPorts(
    PDWORD* ppdwLdapsConnectPorts,
    PDWORD  pdwLdapsConnectPorts
    )
{
    *ppdwLdapsConnectPorts = gVmdirGlobals.pdwLdapsConnectPorts;
    *pdwLdapsConnectPorts = gVmdirGlobals.dwLdapsConnectPorts;
}

DWORD
VmDirGetAllLdapPortsCount(
    VOID
)
{
    return gVmdirGlobals.dwLdapConnectPorts + gVmdirGlobals.dwLdapsConnectPorts;
}

DWORD
VmDirServerStatusEntry(
    PVDIR_ENTRY*    ppEntry
    )
{
#define SUPPORTED_STATUS_COUNT    6

    DWORD           dwError = 0;
    int             iNumAttrs = 1 + 1 + SUPPORTED_STATUS_COUNT;     // cn/oc/6 ops
    PSTR            pszAry[SUPPORTED_STATUS_COUNT + 1] = {0};
    int             iCnt = 0;
    int             iTmp = 0;
    PSTR*           ppszAttrList = NULL;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    assert( ppEntry );

    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * (iNumAttrs * 2 + 1), // add 1 for VmDirFreeStringArrayA call later
            (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ATTR_CN, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( "ServerStatus", &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ATTR_OBJECT_CLASS, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( OC_SERVER_STATUS, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszAry[0] = VmDirOPStatistic(LDAP_REQ_BIND);        // pszAry owns PSTR
    pszAry[1] = VmDirOPStatistic(LDAP_REQ_SEARCH);
    pszAry[2] = VmDirOPStatistic(LDAP_REQ_ADD);
    pszAry[3] = VmDirOPStatistic(LDAP_REQ_MODIFY);
    pszAry[4] = VmDirOPStatistic(LDAP_REQ_DELETE);
    pszAry[5] = VmDirOPStatistic(LDAP_REQ_UNBIND);

    for (iTmp = 0; iTmp < SUPPORTED_STATUS_COUNT; iTmp++)
    {
        dwError = VmDirAllocateStringA( ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA( pszAry[iTmp] ? pszAry[iTmp] : "NONE", &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAttrListToNewEntry( pSchemaCtx,
                                       SERVER_STATUS_DN,
                                       ppszAttrList,
                                       &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:

    VmDirFreeStringArrayA(pszAry);

    if (ppszAttrList != NULL)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    if (pSchemaCtx != NULL)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    if (pEntry != NULL)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

DWORD
VmDirSrvValidateUserCreateParams(
    PVMDIR_USER_CREATE_PARAMS_RPC pCreateParams
    )
{
    DWORD dwError = 0;

    if (!pCreateParams ||
        IsNullOrEmptyString(pCreateParams->pwszAccount) ||
        IsNullOrEmptyString(pCreateParams->pwszFirstname) ||
        IsNullOrEmptyString(pCreateParams->pwszLastname) ||
        IsNullOrEmptyString(pCreateParams->pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

#ifdef _WIN32

DWORD
VmDirAppendStringToEnvVar(
    _TCHAR *pEnvName, // in
    _TCHAR *pStrIn,   // in
    _TCHAR *pStrOut   // out
)
{
    DWORD   dwError            = 0;
    _TCHAR* localBuf           = NULL;
    size_t  strInLen           = VmDirStringLenA(pStrIn);
    size_t  envValueLen        = 0 ;

    dwError =  VmDirGetProgramDataEnvVar(
                    pEnvName,
                    &localBuf
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    envValueLen = VmDirStringLenA(localBuf);

    if ( envValueLen + strInLen + 1 < MAX_PATH )
    {
        dwError = VmDirStringCatA(
                        localBuf,
                        MAX_PATH,
                        pStrIn
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else // path too long
    {
        dwError = ERROR_BUFFER_OVERFLOW;    // The file name is too long.
                                            // In WinError.h, this error message maps to
                                            // ERROR_BUFFER_OVERFLOW. Not very
                                            // straight forward, though.
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringCpyA(
                    pStrOut,
                    VmDirStringLenA(localBuf) + 1,
                    localBuf
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(localBuf);
    return dwError;

error:
    goto cleanup;

}

/*
 * Get the directory where we should store our schema config file, i.e.,
 * %PROGRAMDATA%\cfg\%COMPONENT%\vmdirschema.ldif
 * which usually expands to
 * C:\ProgramData\VMware\CIS\cfg\vmdir\vmdirschema.ldif
 */
DWORD
VmDirGetBootStrapSchemaFilePath(
    _TCHAR *pBootStrapSchemaFile
)
{
    DWORD dwError = 0;

    if ((dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_CONFIG_PATH, pBootStrapSchemaFile,
                                        MAX_PATH )) != 0)
    {
       dwError = VmDirAppendStringToEnvVar(
                        TEXT("PROGRAMDATA"),
                        TEXT("\\VMware\\CIS\\cfg\\vmdird\\vmdirschema.ldif"),
                        pBootStrapSchemaFile
                        );
    }
    else
    {
       dwError = VmDirStringCatA(pBootStrapSchemaFile, MAX_PATH, "\\vmdirschema.ldif");
    }
    return dwError;
}

DWORD
VmDirGetLogFilePath(
    _TCHAR *pServerLogFile
)
{
    DWORD dwError = 0;

    if ((dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_LOG_PATH, pServerLogFile,
                                        MAX_PATH )) != 0)
    {
       dwError = VmDirAppendStringToEnvVar(
                        TEXT("PROGRAMDATA"),
                        TEXT("\\VMware\\CIS\\logs\\vmdird\\vmdir.log"),
                        pServerLogFile
                        );
    }
    else
    {
       dwError = VmDirStringCatA(pServerLogFile, MAX_PATH, "\\vmdir.log");
    }

    return dwError;
}

void
VmDirGetLogMaximumOldLogs(
    PDWORD pdwMaximumOldLogs
    )
{
    if (VmDirGetRegKeyValueDword(VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_MAXIMUM_OLD_LOGS, pdwMaximumOldLogs) != 0)
    {
        *pdwMaximumOldLogs = VMDIR_LOG_MAX_OLD_FILES;
    }
}

void
VmDirGetLogMaximumLogSize(
    PINT64 pI64MaximumLogSize
    )
{
    if (VmDirGetRegKeyValueQword(VMDIR_CONFIG_SOFTWARE_KEY_PATH,
        VMDIR_REG_KEY_MAXIMUM_LOG_SIZE, pI64MaximumLogSize) != 0 ||
        *pI64MaximumLogSize == 0)
    {
        *pI64MaximumLogSize = VMDIR_LOG_MAX_SIZE_BYTES;
    }
}

#endif
