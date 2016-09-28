/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef _WIN32

typedef struct _tagVmcaErrorMap
{
    DWORD dwUnixErrno;
    DWORD dwWin32Error;
    PCSTR szDesc;
} VmcaErrorMap;

VmcaErrorMap gWin32ErrMap[] = 
{
    {EPERM,  ERROR_INVALID_OPERATION, "Operation not permitted" },
    {ENOENT, ERROR_FILE_NOT_FOUND, "No such file or directory "},
    {ENOMEM, ERROR_OUTOFMEMORY, "Out of memory "},
    {EACCES, ERROR_ACCESS_DENIED, "Permission denied "},
    {EEXIST, ERROR_FILE_EXISTS, "File exists "},
    {EINVAL, ERROR_INVALID_PARAMETER, "Invalid argument "}
};

DWORD
VMCAGetWin32ErrorCode(DWORD dwUnixError)
{
    unsigned int i;
    DWORD dwError = LwErrnoToWin32Error(dwUnixError);
    if (dwError == (DWORD)-1)
    {
        // these error are not mapped now. We will be moving this to likewise later
        for (i = 0; i < sizeof(gWin32ErrMap)/sizeof(gWin32ErrMap[0]); i++)
        {
            if (gWin32ErrMap[i].dwUnixErrno == dwUnixError)
            {
                dwError = gWin32ErrMap[i].dwWin32Error;
                break;
            }
        }
    }

    if (dwError == (DWORD)-1)
    {
        dwError = dwUnixError;
    }

    return dwError;
}

PCSTR
VMCAGetWin32ErrorDesc(DWORD dwWin32Error)
{
    unsigned int i;
    PCSTR szError = LwWin32ErrorToName(dwWin32Error);

    if (!szError)
    {
        // these error are not mapped now. We will be moving this to likewise later
        for (i = 0; i < sizeof(gWin32ErrMap)/sizeof(gWin32ErrMap[0]); i++)
        {
            if (gWin32ErrMap[i].dwWin32Error == dwWin32Error)
            {
                szError = gWin32ErrMap[i].szDesc;
                break;
            }
        }
    }

    if (!szError)
    {
        szError = "ERROR_UNKNOWN";
    }

    return szError;
}


#endif

DWORD
VMCAStringToLower(
    PSTR pszString,
    PSTR *ppszNewString
)
{
    DWORD dwError = 0;
    size_t nStrlen = 0;
    PSTR pTempString = NULL;
    int nNdx = 0;

    if ( IsNullOrEmptyString(pszString)) {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (ppszNewString == NULL) {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    nStrlen = strlen(pszString);

    dwError = VMCAAllocateStringA(pszString, &pTempString);
    BAIL_ON_VMCA_ERROR(dwError);
    for (nNdx = 0; nNdx < nStrlen; nNdx++){
        pTempString[nNdx] = tolower((int)pTempString[nNdx]);
    }
    *ppszNewString = pTempString;

cleanup :
    return dwError;
error :

    VMCA_SAFE_FREE_MEMORY(pTempString);
    goto cleanup;
}

DWORD
VMCAGetNameInfo(
    const struct sockaddr*     pSockaddr,
    socklen_t           sockaddrLength,
    PCHAR               pHostName,
    DWORD               dwBufferSize
)
{
    DWORD dwError = ERROR_SUCCESS;
#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    dwError = getnameinfo(
                pSockaddr,
                sockaddrLength,
                pHostName,
                dwBufferSize,
                NULL,
                0,
                0);
    if( dwError != 0) {
        dwError = VMCA_GET_NAME_INFO_FAIL;
        BAIL_ON_VMCA_ERROR(dwError);
    }
error:
#else
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        dwError = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if( getnameinfo(
             pSockaddr, sockaddrLength, pHostName, dwBufferSize,
             NULL, 0, 0 ) != 0 )
        {
            dwError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif
    return dwError;
}

DWORD
VMCAGetAddrInfo(
  PCSTR pszHostname,
  struct addrinfo** ppHostInfo
)
{
    DWORD dwError = ERROR_SUCCESS;
    struct addrinfo hints = {0};

#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = 0;
    hints.ai_protocol   = 0;
    hints.ai_flags      = AI_CANONNAME;

#ifndef _WIN32
    dwError = getaddrinfo(pszHostname, NULL, &hints, ppHostInfo);
    if ( dwError != 0 ){
        dwError = VMCA_GET_ADDR_INFO_FAIL;
    }
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        dwError = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if( getaddrinfo(pszHostname, NULL, &hints, ppHostInfo) != 0 )
        {
            dwError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif
    return dwError;
}



DWORD
VMCAGetHostName(
    PSTR *ppszHostName
)
{
    DWORD dwError = ERROR_SUCCESS;
    char hostBuf[NI_MAXHOST+1];
    DWORD dwBufLen = sizeof(hostBuf) - 1;
    PSTR pszHostName = NULL;

#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        dwError = VMCAGetWin32ErrorCode(errno);
    }
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        dwError = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if( gethostname(hostBuf, dwBufLen) != 0 )
        {
            dwError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif

    dwError = VMCAAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszHostName = pszHostName;
error :
    return dwError;
}



DWORD
VMCAGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    )
{
    DWORD  dwError = 0;
    struct addrinfo* pHostInfo = NULL;
    CHAR   szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTR   pszCanonicalHostname = NULL;

    dwError = VMCAGetAddrInfo(pszHostname, &pHostInfo);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetNameInfo(
                    pHostInfo->ai_addr,
                    (socklen_t)(pHostInfo->ai_addrlen),
                    szCanonicalHostname,
                    NI_MAXHOST);
    BAIL_ON_VMCA_ERROR(dwError);

    if (szCanonicalHostname[0] != 0)
    {
        dwError = VMCAAllocateStringA(
                    szCanonicalHostname,
                    &pszCanonicalHostname);
        BAIL_ON_VMCA_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NO_DATA;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return dwError;

error:

    *ppszCanonicalHostname = NULL;

    VMCA_SAFE_FREE_MEMORY(pszCanonicalHostname);

    goto cleanup;
}


DWORD
VMCASetRegKeyValue(
                   PCSTR   pszConfigParamKeyPath,
                   PCSTR   pszKey,
                   PSTR    pszValue,
                   size_t  valueLen
                   )
#ifndef _WIN32
{
    DWORD   dwError = 0;

    if (IsNullOrEmptyString(pszConfigParamKeyPath) || IsNullOrEmptyString(pszKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegUtilSetValue(
        NULL,
        HKEY_THIS_MACHINE,
        pszConfigParamKeyPath,
        NULL,
        pszKey,
        REG_SZ,
        (PVOID)pszValue,
        valueLen);
    BAIL_ON_VMCA_ERROR(dwError);


cleanup:
    return dwError;
error:
    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;

    if (IsNullOrEmptyString(pszConfigParamKeyPath) || IsNullOrEmptyString(pszKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE,
        pszConfigParamKeyPath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = RegSetValueExA(
        hKey,
        pszKey,
        0,
        REG_SZ,
        (BYTE*)pszValue,
        (DWORD)valueLen);
    BAIL_ON_VMCA_ERROR(dwError);


cleanup:
    if ( hKey != NULL) {
        RegCloseKey(hKey);
    }
    return dwError;
error:
    goto cleanup;
}
#endif

DWORD
VMCAGetRegKeyValue(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PSTR    pszValue,
    size_t  valueLen
    )
#ifndef _WIN32
{
    DWORD   dwError = 0;
    PSTR    pszLocalValue = NULL;
    DWORD   dwLocalValueLen = 0;

    if (pszValue == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegUtilGetValue(
        NULL,
        HKEY_THIS_MACHINE,
        NULL,
        pszConfigParamKeyPath,
        pszKey,
        NULL,
        (PVOID*)&pszLocalValue,
        &dwLocalValueLen);
    BAIL_ON_VMCA_ERROR(dwError);

    if (dwLocalValueLen > valueLen) // in case of string values, dwLocalValueLen includes '\0' and therefore valueLen
        // should also include space for '\0'
    {
        dwError = ERROR_INVALID_PARAMETER; // TBD: Better error code??
        BAIL_ON_VMCA_ERROR(dwError);
    }


    memcpy( pszValue, pszLocalValue, dwLocalValueLen );

cleanup:
    if (pszLocalValue)
    {
        RegFreeMemory(pszLocalValue);
    }
    return dwError;

error:

    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey;
    DWORD   dwType = 0;

    if (pszValue == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE,
        pszConfigParamKeyPath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = RegQueryValueExA(
        hKey,
        pszKey,
        NULL,
        &dwType,
        pszValue,
        (DWORD *)&valueLen);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:

    goto cleanup;
}
#endif

DWORD
VMCAGetSharedSecret(
    PWSTR* ppszSharedSecret
    )
{
    int     dwError = 0;
    PSTR    pLocalPassword = NULL;

    *ppszSharedSecret = NULL;

    dwError = VMCAAllocateMemory( VMCA_SHARED_SECRET_LEN + 1, (PVOID *)&pLocalPassword );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetRegKeyValue( VMCA_SHARED_SECRET_KEY, VMCA_SHARED_ROOT_KEY, pLocalPassword,
        VMCA_SHARED_SECRET_LEN + 1 );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pLocalPassword, ppszSharedSecret);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pLocalPassword);
    return dwError;

error:
    VMCA_SAFE_FREE_MEMORY(*ppszSharedSecret);
    goto cleanup;
}

DWORD
VMCACommonInit()
{
    DWORD dwError = ERROR_SUCCESS;
    dwError = VMCAIntializeOpenSSL();
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;
error:
    VMCACleanupOpenSSL();
    goto cleanup;
}

DWORD
VMCACommonShutdown()
{
    DWORD dwError = ERROR_SUCCESS;
    dwError = VMCACleanupOpenSSL();
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;
error:
   goto cleanup;
}

