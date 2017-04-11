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
PCSTR
VmDirGetVMDIRErrorDesc(
    DWORD dwErrorCode
    )
{
    DWORD i;
    for (i=0;
         i < VMDIR_ERROR_Table_size;
         i++)
    {
        if ( dwErrorCode == VMDIR_ERROR_Table[i].code )
        {
            return VMDIR_ERROR_Table[i].desc;
        }
    }

    return NULL;
}

static
PCSTR
VmDirGetDceRpcErrorDesc(
    DWORD dwErrorCode
    )
{
    DWORD i;
    for (i=0;
         i < VMDIR_RPC_ERROR_Table_size;
         i++)
    {
        if ( dwErrorCode == VMDIR_RPC_ERROR_Table[i].code )
        {
             return VMDIR_RPC_ERROR_Table[i].desc;
        }
    }
    return NULL;
}

DWORD
VmDirGetWin32ErrorDesc(
    DWORD dwErrorCode,
    PSTR* ppszErrorMessage
    )
{
    PSTR  pszErrorMessage = NULL;
    DWORD dwError = 0;
#ifndef _WIN32
    PCSTR  pszErr = NULL;
#else
    DWORD dwNofOfBytes = 0;
    PSTR pszWin32Error = NULL;
#endif

#ifndef _WIN32
    pszErr = LwWin32ErrorToName( dwErrorCode );
    if ( !pszErr ){
        switch( dwErrorCode )
        {
            case ERROR_INVALID_OPERATION:
                pszErr = "Operation not permitted";
                break;
            case ERROR_FILE_NOT_FOUND:
                pszErr = "No such file or directory";
                break;
            case ERROR_OUTOFMEMORY:
                pszErr = "Out of memory";
                break;
            case ERROR_ACCESS_DENIED:
                pszErr = "Permission denied";
                break;
            case ERROR_FILE_EXISTS:
                pszErr = "File exists";
                break;
            case ERROR_INVALID_PARAMETER:
                pszErr = "Invalid argument";
                break;
        }
    }
    if ( pszErr )
    {
        dwError = VmDirAllocateStringA(
            pszErr,
            &pszErrorMessage);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
#else
    dwNofOfBytes = FormatMessageA(
         FORMAT_MESSAGE_FROM_SYSTEM
        |FORMAT_MESSAGE_ALLOCATE_BUFFER
        |FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL,
        SUBLANG_DEFAULT
        ),
        (PSTR)&pszWin32Error,
        0,
        NULL
        );
    if ( pszWin32Error )
    {
        dwError = VmDirAllocateStringA(
            pszWin32Error,
            &pszErrorMessage);
        LocalFree(pszWin32Error);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
#endif

    *ppszErrorMessage = pszErrorMessage;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    pszErrorMessage = NULL;
    goto cleanup;
}

DWORD
VmDirGetErrorMessage(
    DWORD dwErrorCode,
    PSTR* ppszErrorMessage
    )
{
    DWORD dwError = 0;
    PSTR  pszErrorMessage = NULL;
    PCSTR pszErr = NULL;
    PSTR  pszWin32Err = NULL;

    if ( IS_VMDIR_ERROR_SPACE(dwErrorCode) )
    {
        pszErr = VmDirGetVMDIRErrorDesc(dwErrorCode);
    }
    if ( !pszErr && IS_VMDIR_RPC_ERROR_SPACE(dwErrorCode) )
    {
        pszErr = VmDirGetDceRpcErrorDesc(dwErrorCode);
    }
    if ( !pszErr && IS_VMDIR_LDAP_ERROR_SPACE(dwErrorCode) )
    {
        pszErr = ldap_err2string(dwErrorCode);
    }
    if ( !pszErr )
    {
        dwError = VmDirGetWin32ErrorDesc(dwErrorCode, &pszWin32Err);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( pszWin32Err )
    {
        pszErrorMessage = pszWin32Err;
    }
    else
    {
        dwError = VmDirAllocateStringA(
            ( pszErr )? pszErr : "Unknown error",
            &pszErrorMessage);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszErrorMessage = pszErrorMessage;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszWin32Err);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    goto cleanup;
}

DWORD
VmDirDCEGetErrorCode(
    dcethread_exc* pDceException
    )
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus (pDceException);

    return dwError;
}
