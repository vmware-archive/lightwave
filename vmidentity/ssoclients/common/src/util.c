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

static
SSOERROR
SSOGetCanonicalHostName(
    PCSTRING    pszHostname,
    PSTRING*    ppszCanonicalHostName
    );

static
SSOERROR
SSOGetHostName(
    PSTRING*    pszHostName
    );

static
SSOERROR
SSOGetAddrInfo(
    PCSTRING            pszHostname,
    struct addrinfo**   ppResult
    );

static
SSOERROR
SSOGetNameInfo(
    const struct sockaddr*  pSockaddr,
    socklen_t               sockaddrLength,
    PSTRING                 pHostName,
    long                    lBufferSize
    );


bool
SSOIsLocalHost(
   PCSTRING     pszHostname
   )
{
    SSOERROR    e = SSOERROR_NONE;
    bool        bResult = false;
    PSTRING     localHostName = NULL;
    PSTRING     pszLocalHostnameCanon = NULL;
    PSTRING     pszInputHostnameCanon = NULL;
    PCSTRING    pszLocalName = NULL;
    PCSTRING    pszInputName = NULL;
#ifdef _WIN32
    WSADATA     wsaData = { 0 };
    bool        bWsaStartup = false;
#endif

    if (SSOStringEqual(pszHostname, "localhost") ||
        SSOStringEqual(pszHostname, "127.0.0.1") ||
        SSOStringEqual(pszHostname, "::1"))
    {
        bResult = true;
    }
    else
    {
#ifdef _WIN32
        e = WSAStartup(MAKEWORD(2, 2), &wsaData);
        BAIL_ON_ERROR(e);
        bWsaStartup = true;
#endif
        // Convert the local host name to its canonical form and check against
        // the canonical form of the input host name
        e = SSOGetHostName(&localHostName);
        BAIL_ON_ERROR(e);

        e = SSOGetCanonicalHostName(localHostName, &pszLocalHostnameCanon);

        pszLocalName = e ? &localHostName[0] : pszLocalHostnameCanon;

        e = SSOGetCanonicalHostName(
                            pszHostname,
                            &pszInputHostnameCanon);

        pszInputName = e ? pszHostname : pszInputHostnameCanon;

        bResult = SSOStringEqual(pszLocalName, pszInputName);
    }

cleanup:

    SSOStringFree(pszLocalHostnameCanon);
    SSOStringFree(pszInputHostnameCanon);
    SSOStringFree(localHostName);

#ifdef _WIN32
    if (bWsaStartup != false)
    {
        WSACleanup();
    }
#endif

    return bResult;

error:

    bResult = false;

    goto cleanup;
}


static
SSOERROR
SSOGetCanonicalHostName(
    PCSTRING    pszHostname,
    PSTRING*    ppszCanonicalHostname
    )
{
    SSOERROR    e = SSOERROR_NONE;
    struct      addrinfo* pHostInfo = NULL;
    char        szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTRING     pszCanonicalHostname = NULL;

    e = SSOGetAddrInfo(pszHostname, &pHostInfo);
    BAIL_ON_ERROR(e);

    e = SSOGetNameInfo(
                    pHostInfo->ai_addr,
                    (socklen_t)(pHostInfo->ai_addrlen),
                    szCanonicalHostname,
                    NI_MAXHOST);
    BAIL_ON_ERROR(e);

    if (!IS_NULL_OR_EMPTY_STRING(&szCanonicalHostname[0]))
    {
        e = SSOStringAllocate(
                    &szCanonicalHostname[0],
                    &pszCanonicalHostname);
    }
    else
    {
        e = SSOERROR_INVALID_ARGUMENT;
    }
    BAIL_ON_ERROR(e);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return e;

error:

    *ppszCanonicalHostname = NULL;

    SSOStringFree(pszCanonicalHostname);

    goto cleanup;
}

static
SSOERROR
SSOGetHostName(
    PSTRING     *ppszHostName
    )
{
    SSOERROR    e = SSOERROR_NONE;
    char        hostBuf[NI_MAXHOST + 1];
    long        dwBufLen = sizeof(hostBuf)-1;
    PSTRING     pszHostName = NULL;

#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = false;
#endif

#ifndef _WIN32
    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        e = SSOERROR_INVALID_ARGUMENT;
    }
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        e = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if (gethostname(hostBuf, dwBufLen) != 0)
        {
            e = WSAGetLastError();
        }

        WSACleanup();
    }

#endif

    e = SSOStringAllocate(hostBuf, &pszHostName);
    BAIL_ON_ERROR(e);

    *ppszHostName = pszHostName;
error:
    return e;
}

static
SSOERROR
SSOGetAddrInfo(
    PCSTRING            pszHostname,
    struct addrinfo**   ppHostInfo
)
{
    SSOERROR e = SSOERROR_NONE;
#ifndef _WIN32
    e = getaddrinfo(pszHostname, NULL, NULL, ppHostInfo);
#else
    /*
    MSDN:
    Success returns zero. Failure returns a nonzero Windows Sockets error code,
    as found in the Windows Sockets Error Codes.

    It is recommended that the WSA error codes be used, as they offer familiar
    and comprehensive error information for Winsock programmers.
    */
    if( getaddrinfo(pszHostname, NULL, NULL, ppHostInfo) != 0 )
    {
        e = WSAGetLastError();
    }
#endif
    return e;
}

static
SSOERROR
SSOGetNameInfo(
    const struct sockaddr*  pSockaddr,
    socklen_t               sockaddrLength,
    PSTRING                 pHostName,
    long                    lBufferSize
)
{
    SSOERROR e = SSOERROR_NONE;
#ifndef _WIN32
    e = getnameinfo(
                pSockaddr,
                sockaddrLength,
                pHostName,
                lBufferSize,
                NULL,
                0,
                0);
#else
    /*
    MSDN:
    On success, getnameinfo returns zero. Any nonzero return value indicates
    failure and a specific error code can be retrieved
    by calling WSAGetLastError.
    */
    if ( getnameinfo(
             pSockaddr, sockaddrLength, pHostName, lBufferSize,
             NULL, 0,0 ) != 0 )
    {
        e = WSAGetLastError();
    }
#endif
    return e;
}

