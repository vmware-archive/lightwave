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

DWORD
LwCAStringToLower(
    PSTR pszString,
    PSTR *ppszNewString
    )
{
    DWORD dwError = 0;
    size_t nStrlen = 0;
    PSTR pTempString = NULL;
    int nNdx = 0;

    if ( IsNullOrEmptyString(pszString)) {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (ppszNewString == NULL) {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    nStrlen = strlen(pszString);

    dwError = LwCAAllocateStringA(pszString, &pTempString);
    BAIL_ON_LWCA_ERROR(dwError);
    for (nNdx = 0; nNdx < nStrlen; nNdx++){
        pTempString[nNdx] = tolower((int)pTempString[nNdx]);
    }
    *ppszNewString = pTempString;

cleanup :
    return dwError;
error :

    LWCA_SAFE_FREE_MEMORY(pTempString);
    goto cleanup;
}

DWORD
LwCAGetNameInfo(
    const struct sockaddr*     pSockaddr,
    socklen_t           sockaddrLength,
    PCHAR               pHostName,
    DWORD               dwBufferSize
    )
{
    DWORD dwError = LWCA_SUCCESS;

    dwError = getnameinfo(
                pSockaddr,
                sockaddrLength,
                pHostName,
                dwBufferSize,
                NULL,
                0,
                0);
    if( dwError != 0) {
        dwError = LWCA_GET_NAME_INFO_FAIL;
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
LwCAGetAddrInfo(
    PCSTR pszHostname,
    struct addrinfo** ppHostInfo
    )
{
    DWORD dwError = LWCA_SUCCESS;
    struct addrinfo hints = {0};

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = 0;
    hints.ai_protocol   = 0;
    hints.ai_flags      = AI_CANONNAME;

    dwError = getaddrinfo(pszHostname, NULL, &hints, ppHostInfo);
    if ( dwError != 0 ){
        dwError = LWCA_GET_ADDR_INFO_FAIL;
    }

    return dwError;
}



DWORD
LwCAGetHostName(
    PSTR *ppszHostName
    )
{
    DWORD dwError = LWCA_SUCCESS;
    char hostBuf[NI_MAXHOST+1];
    DWORD dwBufLen = sizeof(hostBuf) - 1;
    PSTR pszHostName = NULL;

    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
    }

    dwError = LwCAAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszHostName = pszHostName;
error :
    return dwError;
}



DWORD
LwCAGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    )
{
    DWORD  dwError = 0;
    struct addrinfo* pHostInfo = NULL;
    CHAR   szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTR   pszCanonicalHostname = NULL;

    dwError = LwCAGetAddrInfo(pszHostname, &pHostInfo);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetNameInfo(
                    pHostInfo->ai_addr,
                    (socklen_t)(pHostInfo->ai_addrlen),
                    szCanonicalHostname,
                    NI_MAXHOST);
    BAIL_ON_LWCA_ERROR(dwError);

    if (szCanonicalHostname[0] != 0)
    {
        dwError = LwCAAllocateStringA(
                    szCanonicalHostname,
                    &pszCanonicalHostname);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LWCA_ERROR_INVALID_DATA;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return dwError;

error:

    *ppszCanonicalHostname = NULL;

    LWCA_SAFE_FREE_MEMORY(pszCanonicalHostname);

    goto cleanup;
}


DWORD
LwCASetRegKeyValue(
   PCSTR   pszConfigParamKeyPath,
   PCSTR   pszKey,
   PSTR    pszValue,
   size_t  valueLen
   )
{
    DWORD   dwError = 0;

    if (IsNullOrEmptyString(pszConfigParamKeyPath) || IsNullOrEmptyString(pszKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
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
    BAIL_ON_LWCA_ERROR(dwError);


cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
LwCAGetRegKeyValue(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PSTR    pszValue,
    size_t  valueLen
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalValue = NULL;
    DWORD   dwLocalValueLen = 0;

    if (pszValue == NULL)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
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
    BAIL_ON_LWCA_ERROR(dwError);

    if (dwLocalValueLen > valueLen) // in case of string values, dwLocalValueLen includes '\0' and therefore valueLen
        // should also include space for '\0'
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER; // TBD: Better error code??
        BAIL_ON_LWCA_ERROR(dwError);
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

DWORD
LwCAGetRegKeyValueDword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;
    REG_DATA_TYPE RegType = 0;

    if (pszConfigParamKeyPath == NULL || pszKey == NULL || pdwValue == NULL)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOG_VERBOSE("Reading Reg: %s", pszKey);

    *pdwValue = dwDefaultValue;

    dwError = RegUtilGetValue(
                NULL,
                HKEY_THIS_MACHINE,
                NULL,
                pszConfigParamKeyPath,
                pszKey,
                &RegType,
                (PVOID*)&dwValue,
                NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    if (RegType != REG_DWORD)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pdwValue = dwValue;

cleanup:
    return dwError;

error:
    LWCA_LOG_VERBOSE(
         "VmDirGetRegKeyValueDword failed with error (%u)(%s)",
          dwError,
          LWCA_SAFE_STRING(pszKey));
    goto cleanup;
}

DWORD
LwCACommonInit(
    VOID
    )
{
    DWORD dwError = LWCA_SUCCESS;
    dwError = LwCAOpenSSLInitialize();
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;
error:
    LwCAOpenSSLCleanup();
    goto cleanup;
}

DWORD
LwCACommonShutdown(
    VOID
    )
{
    DWORD dwError = LWCA_SUCCESS;
    dwError = LwCAOpenSSLCleanup();
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;
error:
   goto cleanup;
}

DWORD
LwCALoadLibrary(
    PCSTR           pszLibPath,
    LWCA_LIB_HANDLE* ppLibHandle
    )
{
    DWORD   dwError = 0;
    LWCA_LIB_HANDLE pLibHandle = NULL;

    if (ppLibHandle == NULL)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    pLibHandle = dlopen(pszLibPath, RTLD_LAZY);
    if (pLibHandle == NULL)
    {
        LWCA_LOG_VERBOSE(
             "dlopen %s library failed, error msg (%s)",
             pszLibPath,
             LWCA_SAFE_STRING(dlerror()));
         dlerror();    /* Clear any existing error */
         dwError = LWCA_ERROR_CANNOT_LOAD_LIBRARY;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    *ppLibHandle = pLibHandle;

cleanup:
    return dwError;

error:
    *ppLibHandle = NULL;
    goto cleanup;
}

VOID
LwCACloseLibrary(
    LWCA_LIB_HANDLE  pLibHandle
    )
{
    if (pLibHandle)
    {
        dlclose(pLibHandle);
    }
}

VOID*
LwCAGetLibSym(
    LWCA_LIB_HANDLE  pLibHandle,
    PCSTR           pszFunctionName
    )
{
    return dlsym(pLibHandle, pszFunctionName);
}

DWORD
LwCABytesToHexString(
    PUCHAR  pData,
    DWORD   length,
    PSTR*   pszHexString,
    BOOLEAN bLowerCase
    )
{
    DWORD dwError = LWCA_SUCCESS;
    char* pszOut = NULL;
    DWORD i = 0;

    dwError = LwCAAllocateMemory(length * 2 + 1, (PVOID*)&pszOut);
    BAIL_ON_LWCA_ERROR(dwError);

    for (; i < length; ++i)
    {
        sprintf(pszOut + i * 2, bLowerCase ? "%02x" : "%02X", pData[i]);
    }
    pszOut[length * 2] = '\0';

    *pszHexString = pszOut;

error:
    return dwError;
}

DWORD
LwCAHexStringToBytes(
    PSTR    pszHexStr,
    PUCHAR* ppData,
    size_t* pLength
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    size_t  hexlen = 0;
    size_t  datalen = 0;
    PBYTE   pData = NULL;
    char    twoHexChars[3] = {'\0'};

    if (!pszHexStr || !ppData || !pLength)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    hexlen = LwCAStringLenA(pszHexStr);
    if (hexlen % 2)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    datalen = hexlen / 2;
    dwError = LwCAAllocateMemory(datalen, (PVOID*)&pData);
    BAIL_ON_LWCA_ERROR(dwError);

    for (i = 0; i < datalen; i++)
    {
        strncpy(twoHexChars, &pszHexStr[i*2], 2);
        twoHexChars[2] = '\0';
        pData[i] = (unsigned char)strtoul(twoHexChars, NULL, 16);
    }

    *ppData = pData;
    *pLength = datalen;

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    LWCA_SAFE_FREE_MEMORY(pData);
    goto cleanup;
}
