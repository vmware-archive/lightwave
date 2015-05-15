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
BOOLEAN
_VmDirIsIPV4AddrFormat(
    PCSTR   pszAddr
    );

/*
 * Assumptions: tenant dn starts with "dc="
 */
PCSTR
VmDirSearchDomainDN(
    PCSTR pszNormObjectDN
    )
{
    PSTR pszDomainDn = VmDirStringCaseStrA(pszNormObjectDN, "dc=");

    if (!pszDomainDn)
    {
        VmDirLog(LDAP_DEBUG_ANY, "VmDirSearchDomainDN failed on %s.", pszNormObjectDN);
    }

    return pszDomainDn;
}

DWORD
VmDirDomainDNToName(
    PCSTR pszDomainDN,
    PSTR* ppszDomainName)
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    SIZE_T  bufferSize = VmDirStringLenA(pszDomainDN)+1;
    char*   pDN = (char*)pszDomainDN;
    char*   pTag = NULL;

    dwError = VmDirAllocateMemory(bufferSize, (PVOID*)&pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    while ((pTag = VmDirStringStrA(pDN, "dc=")))
    {
        char* pStart = pTag+3;
        char* pEnd = pStart;

        while (*pEnd!=',' && *pEnd!='\0')
        {
            pEnd++;
        }

        if (strlen(pszDomainName)!=0)
        {
            dwError = VmDirStringCatA(pszDomainName, bufferSize, ".");
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirStringNCatA(pszDomainName, bufferSize, pStart, pEnd-pStart);
        pDN = pEnd;
    }

    //if no "dc="
    if (VmDirStringLenA(pszDomainName) == 0)
    {
        dwError = VmDirStringCpyA(pszDomainName, VmDirStringLenA(pszDomainDN), pszDomainDN );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszDomainName = pszDomainName;

cleanup:
    return dwError;

error:
    *ppszDomainName = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VmDirLog(LDAP_DEBUG_TRACE, "VmDirDomainDNToName failed with error (%u)\n", dwError);
    goto cleanup;
}

DWORD
VmDirSrvCreateDomainDN(
    PCSTR pszFQDomainName,
    PSTR* ppszDomainDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainDN = NULL;
    int     fqDomainNameLen = (int) VmDirStringLenA(pszFQDomainName);
    int     domainDNBufLen = 0;
    PSTR    pszTmpFQDomainName = NULL;
    int     numDomainComps = 1;
    int     i = 0;
    int     nextDomainCompStartInd = 0;
    int     targetCurrInd = 0;

    // SJ-TBD: Validate pszFQDomainName, i.e. domain components should not contain '=', ',', ' ' etc.

    // Make a copy of passed-in FQ Domain name

    dwError = VmDirAllocateMemory( fqDomainNameLen + 1 /* \0 */, (PVOID *) &pszTmpFQDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringCpyA( pszTmpFQDomainName, fqDomainNameLen + 1, pszFQDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Count number of domain components

    for (i=0; i<fqDomainNameLen; i++)
    {
        if (pszTmpFQDomainName[i] == '.')
        {
            numDomainComps++;
        }
    }

    // 3 => for "dc=,", 1 => for string terminator ('\0')
    domainDNBufLen = fqDomainNameLen + (numDomainComps * (ATTR_DOMAIN_COMPONENT_LEN + 2)) + 1;
    dwError = VmDirAllocateMemory( domainDNBufLen, (PVOID *) &pszDomainDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create Domain DN

    nextDomainCompStartInd = 0;
    targetCurrInd = 0;
    for (i=0; i<(fqDomainNameLen + 1); i++)
    {
        if (pszTmpFQDomainName[i] == '.' || pszTmpFQDomainName[i] == '\0')
        {
            pszTmpFQDomainName[i] = '\0';
            dwError = VmDirStringPrintFA( pszDomainDN + targetCurrInd, domainDNBufLen - targetCurrInd, "%s=%s%c",
                                          ATTR_DOMAIN_COMPONENT, pszTmpFQDomainName + nextDomainCompStartInd,
                                          RDN_SEPARATOR_CHAR );
            BAIL_ON_VMDIR_ERROR(dwError);

            nextDomainCompStartInd = i + 1;
            targetCurrInd += (int) VmDirStringLenA( pszDomainDN + targetCurrInd );
        }
    }
    *(pszDomainDN + targetCurrInd - 1) = '\0'; // over-write last ','

    *ppszDomainDN = pszDomainDN;

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszTmpFQDomainName );
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY( pszDomainDN );
    *ppszDomainDN = NULL;
    goto cleanup;
}

#if defined(HAVE_DCERPC_WIN32)
static void uuid_unparse_lower(uuid_t uuid, char *str)
{
    /*
     * Convert a binary uuid into string form, with the format as
     *   c2391bd1-2ebf-11e2-ad41-000c29ec55bd
     */
    _snprintf(str, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8],
             uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}


int uuid_parse(char *str, uuid_t ret_uuid)
{
    unsigned char *p = NULL;
    char *n = NULL;
    char hex[3] = {0};
    int count = 0;
    int i = 0;

    p = str;
    while (p[0] && p[1])
    {
        hex[0] = p[0];
        hex[1] = p[1];
	ret_uuid[i++] = (unsigned char) strtoul(hex, &n, 16);
	if ((n-hex) != 2)
	{
            return -1;
	}
	p += 2;
	count++;
	if (*p && *p == '-')
	{
	    p++;
	}
	if (count > sizeof(uuid_t))
	{
            return -1;
	}
    }
    return count;
}


void
uuid_generate(uuid_t pGuid)
{
    dce_uuid_t dce_uuid;
    unsigned32 sts = 0;
    unsigned char *uuidstr = NULL;

    memset(&dce_uuid, 0, sizeof(dce_uuid_t));
    dce_uuid_create(&dce_uuid, &sts);
    if (sts)
    {
        goto error;
    }
    dce_uuid_to_string(&dce_uuid, &uuidstr, &sts);
    if (sts)
    {
        goto error;
    }

    if (uuid_parse(uuidstr, pGuid) != 16)
    {
        goto error;
    }

error:
    VMDIR_SAFE_FREE_MEMORY( uuidstr );
    return;
}
#endif

DWORD
VmDirUuidGenerate(
    uuid_t* pGuid
)
{
    DWORD dwError = ERROR_SUCCESS;

    if(pGuid == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
    uuid_generate(*pGuid);
#else
    dwError = UuidCreate( pGuid );
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

error:
    return dwError;
}

DWORD
VmDirUuidToStringLower(
    uuid_t* pGuid,
    PSTR pStr,
    DWORD sizeOfBuffer
)
{
    DWORD dwError = ERROR_SUCCESS;
#ifdef _WIN32
    PSTR pGuidStr = NULL;
#endif

    if ((pGuid == NULL) || (pStr == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)

    /*
    The uuid_unparse function converts the supplied UUID uu from the binary
    representation into a 36-byte string (plus tailing '\0')
    of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427.
    */
    if( sizeOfBuffer < VMDIR_GUID_STR_LEN )
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    uuid_unparse_lower( *pGuid, pStr );
#else
    dwError = UuidToStringA( pGuid, (RPC_CSTR*)(&pGuidStr) );
    BAIL_ON_VMDIR_ERROR(dwError);

    if( sizeOfBuffer < VmDirStringLenA( pGuidStr ) + 1 )
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringCpyA( pStr, sizeOfBuffer, pGuidStr );
    BAIL_ON_VMDIR_ERROR(dwError);

    // ensure lower case string
    if ( _strlwr_s(pStr, sizeOfBuffer) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#endif

error:

#if defined(_WIN32) && !defined(HAVE_DCERPC_WIN32)
    if( pGuidStr != NULL )
    {
        RpcStringFreeA((RPC_CSTR*)(&pGuidStr));
    }
#endif

    return dwError;
}

#ifdef _WIN32
DWORD
VmDirGetEnvironmentVariable(
    PCSTR pszVariableName,
    PSTR  pszBuffer,
    DWORD dwBufLen
)
{
    DWORD dwError = 0;

    if (0 == GetEnvironmentVariableA(
                            pszVariableName,
                            pszBuffer,
                            dwBufLen))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}
#endif

ULONG
VmDirGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    )
{
    ULONG  ulError = 0;
    struct addrinfo* pHostInfo = NULL;
    struct addrinfo* pAddrInfo = NULL;
    CHAR   szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTR   pszCanonicalHostname = NULL;
    int    IPVers[] = { AF_INET, AF_INET6 };
    int    iCnt = 0;
    BOOLEAN bHasCanonName = FALSE;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirGetCanonicalHostName, input hostnamne=(%s)",
                                           VDIR_SAFE_STRING( pszHostname) );

    ulError = VmDirGetAddrInfo(pszHostname, &pHostInfo);
    BAIL_ON_VMDIR_ERROR(ulError);

    // Since most setup (client, QE..etc.) focus on IPV4, at least for 2013 use cases,
    // we will resolve IPV4 addr first if available.
    for ( iCnt = 0;
          bHasCanonName == FALSE && iCnt < sizeof(IPVers)/sizeof(IPVers[0]);
          iCnt++
        )
    {
        for ( pAddrInfo = pHostInfo; pAddrInfo != NULL;  pAddrInfo = pAddrInfo->ai_next )
        {
            if ( pAddrInfo->ai_family == IPVers[iCnt] )
            {
                if ( pAddrInfo->ai_canonname != NULL
                     &&
                     _VmDirIsIPV4AddrFormat(pAddrInfo->ai_canonname) == FALSE
                     &&
                     VmDirIsIPV6AddrFormat(pAddrInfo->ai_canonname) == FALSE
                   )
                {   // use canonname from getaddinfo call directly
                    ulError = VmDirAllocateStringA( pAddrInfo->ai_canonname,
                                                    &pszCanonicalHostname);
                    BAIL_ON_VMDIR_ERROR(ulError);
                }
                else
                {
                    ulError = VMDirGetNameInfo( pAddrInfo->ai_addr,
                                                (socklen_t)(pAddrInfo->ai_addrlen),
                                                szCanonicalHostname,
                                                NI_MAXHOST);
                    BAIL_ON_VMDIR_ERROR(ulError);

                    if ( IsNullOrEmptyString(&szCanonicalHostname[0]) )
                    {   // no canon name, continue to next addinfo
                        continue;
                    }

                    ulError = VmDirAllocateStringA( szCanonicalHostname,
                                                    &pszCanonicalHostname);
                    BAIL_ON_VMDIR_ERROR(ulError);
                }

                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "canon name via IPV%d, ai_canonname=(%s), result name=(%s)",
                                       (IPVers[iCnt] == AF_INET) ? 4 : 6,
                                       VDIR_SAFE_STRING(pAddrInfo->ai_canonname),
                                       VDIR_SAFE_STRING(pszCanonicalHostname) );

                bHasCanonName = TRUE;
                break;
            }
        }
    }

    if (pszCanonicalHostname == NULL)
    {
        ulError = ERROR_NO_DATA;
    }
    BAIL_ON_VMDIR_ERROR(ulError);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return ulError;

error:

    *ppszCanonicalHostname = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszCanonicalHostname);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetCanonicalHostName, (%s) faild with error code (%u)",
                                         VDIR_SAFE_STRING(pszHostname), ulError );

    goto cleanup;
}

ULONG
VmDirGetHostName(
    PSTR pszHostName,
    DWORD dwBufLen
)
{
    ULONG ulError = ERROR_SUCCESS;

#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    if (gethostname(pszHostName, dwBufLen) < 0)
    {
        ulError = LwErrnoToWin32Error(errno);
    }
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ulError = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if( gethostname(pszHostName, dwBufLen) != 0 )
        {
            ulError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif

    return ulError;
}


ULONG
VmDirGetAddrInfo(
  PCSTR pszHostname,
  struct addrinfo** ppHostInfo
)
{
    ULONG ulError = ERROR_SUCCESS;
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
    ulError = getaddrinfo(pszHostname, NULL, &hints, ppHostInfo);
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ulError = WSAGetLastError();
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
            ulError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif
    return ulError;
}


ULONG
VMDirGetNameInfo(
    const struct sockaddr*     pSockaddr,
    socklen_t           sockaddrLength,
    PCHAR               pHostName,
    DWORD               dwBufferSize
)
{
    ULONG ulError = ERROR_SUCCESS;
#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    ulError = getnameinfo(
                pSockaddr,
                sockaddrLength,
                pHostName,
                dwBufferSize,
                NULL,
                0,
                0);
#else
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ulError = WSAGetLastError();
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
            ulError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif
    return ulError;
}

static
DWORD
_VmDirGetLocalInfoFromSocket(
    int fd,
    PSTR *ppszAddress
    )
{
    DWORD dwError = 0;
    int iRetVal = 0;
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    char str[INET6_ADDRSTRLEN];
    PSTR pszAddress = NULL;

    iRetVal = getsockname(fd, (struct sockaddr*)&addr, &len);
    if (iRetVal != 0)
    {
        BAIL_ON_VMDIR_ERROR(VMDIR_ERROR_GENERIC);
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in*) &addr;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) != NULL)
        {
            dwError = VmDirAllocateStringPrintf(&pszAddress, "%s:%d", str, ntohs(sin->sin_port));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin = (struct sockaddr_in6*) &addr;
        if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str)) != NULL)
        {
            dwError = VmDirAllocateStringPrintf(&pszAddress, "[%s]:%d", str, ntohs(sin->sin6_port));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszAddress = pszAddress;

cleanup:
    return dwError;
error:
    VMDIR_SAFE_FREE_MEMORY(pszAddress);
    goto cleanup;
}

static
DWORD
_VmDirGetPeerInfoFromSocket(
    int fd,
    PSTR *ppszAddress
    )
{
    DWORD dwError = 0;
    int iRetVal = 0;
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    char str[INET6_ADDRSTRLEN];
    PSTR pszAddress = NULL;

    iRetVal = getpeername(fd, (struct sockaddr*)&addr, &len);
    if (iRetVal != 0)
    {
        BAIL_ON_VMDIR_ERROR(VMDIR_ERROR_GENERIC);
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in*) &addr;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) != NULL)
        {
            dwError = VmDirAllocateStringPrintf(&pszAddress, "%s:%d", str, ntohs(sin->sin_port));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin = (struct sockaddr_in6*) &addr;
        if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str)) != NULL)
        {
            dwError = VmDirAllocateStringPrintf(&pszAddress, "[%s]:%d", str, ntohs(sin->sin6_port));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszAddress = pszAddress;

cleanup:
    return dwError;
error:
    VMDIR_SAFE_FREE_MEMORY(pszAddress);
    goto cleanup;
}

DWORD
VmDirAllocateStringFromSocket(
    int fd,
    BOOLEAN bLocalIsServer,
    PSTR *ppszInfo
    )
{
    DWORD dwError = 0;
    PSTR pszLocalAddress = NULL;
    PSTR pszPeerAddress = NULL;
    PSTR pszInfo = NULL;

    _VmDirGetLocalInfoFromSocket(fd, &pszLocalAddress);
    _VmDirGetPeerInfoFromSocket(fd, &pszPeerAddress);

    if (pszLocalAddress && pszPeerAddress)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszInfo,
                    "[%d] %s%s%s",
                    fd,
                    pszLocalAddress,
                    bLocalIsServer ? "<-" : "->",
                    pszPeerAddress);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(&pszInfo, "[%d]", fd);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszInfo = pszInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalAddress);
    VMDIR_SAFE_FREE_MEMORY(pszPeerAddress);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszInfo);
    goto cleanup;
}

DWORD
VmDirGenerateGUID(
    PSTR pszGuid
    )
{
    DWORD       dwError = 0;
    uuid_t      guid;

    dwError = VmDirUuidGenerate(&guid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidToStringLower( &guid, pszGuid, VMDIR_GUID_STR_LEN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDirLog( LDAP_DEBUG_TRACE, "VmDirGenerateGUID failed. Error(%u)", dwError);
    goto cleanup;
}

#ifdef _WIN32
/*
 * Get the directory where we should store our database, i.e.,
 * <VMDIR_REG_KEY_DATA_PATH config key value or %PROGRAMDATA%>\data\%COMPONENT%
 * %PROGRAMDATA% usually expands to
 * C:\ProgramData\VMware\CIS\data\vmdir (note: we don't have a trailing "\" here)
 */
DWORD
VmDirMDBGetHomeDir(_TCHAR *lpHomeDirBuffer)
{
    DWORD   dwError              = 0;
    const   _TCHAR dbHomeDir[]   = _T("\\VMware\\CIS\\data\\vmdird");
    size_t  dbHomeDirLen         = VmDirStringLenA(dbHomeDir);
    size_t  dbHomePrefixLen      = 0 ;

    if ((dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_DATA_PATH, lpHomeDirBuffer,
                                        MAX_PATH )) != 0)
    {
#ifdef WIN2008
        dwError =  GetEnvironmentVariable(
            _T("PROGRAMDATA"),    // __in_opt   LPCTSTR lpName,
            lpHomeDirBuffer,      // __out_opt  LPTSTR lpBuffer,
            MAX_PATH              // __in       DWORD nSize
        );
#else
        dwError =  GetEnvironmentVariable(
            _T("ALLUSERSPROFILE"),    // __in_opt   LPCTSTR lpName,
            lpHomeDirBuffer,      // __out_opt  LPTSTR lpBuffer,
            MAX_PATH              // __in       DWORD nSize
        );
#endif
        BAIL_ON_VMDIR_ERROR(0 == dwError);
        dwError = ERROR_SUCCESS;

        dbHomePrefixLen = VmDirStringLenA(lpHomeDirBuffer);

        if ( dbHomePrefixLen + dbHomeDirLen < MAX_PATH )
        {
           dwError = VmDirStringCatA(lpHomeDirBuffer, MAX_PATH, dbHomeDir);
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
    }
error:
    return dwError;
}
#endif

DWORD
VmDirGetRegKeyValue(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Reading Reg: %s", pszKey );

    dwError = RegUtilGetValue(
                NULL,
                HKEY_THIS_MACHINE,
                NULL,
                pszConfigParamKeyPath,
                pszKey,
                NULL,
                (PVOID*)&pszLocalValue,
                &dwLocalValueLen);

    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwLocalValueLen > valueLen) // in case of string values, dwLocalValueLen includes '\0' and therefore valueLen
                                    // should also include space for '\0'
    {
        dwError = ERROR_INVALID_PARAMETER; // TBD: Better error code??
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCopyMemory( pszValue, valueLen, pszLocalValue, dwLocalValueLen );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pszLocalValue)
    {
        RegFreeMemory(pszLocalValue);
    }
    return dwError;

error:
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirGetRegKeyValue failed with error (%u)(%s)\n", dwError, pszKey);

    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;
    DWORD   dwType = 0;

    if (pszValue == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Reading Reg: %s", pszKey );

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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegQueryValueExA(
                        hKey,
                        pszKey,
                        NULL,
                        &dwType,
                        pszValue,
                        (DWORD *)&valueLen);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirGetRegKeyValue failed with error (%u)(%s)\n", dwError, pszKey);

    goto cleanup;
}
#endif

#ifdef _WIN32
DWORD
VmDirGetRegKeyValueDword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PDWORD  pdwValue
    )
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    DWORD   dwValue = 0;
    DWORD   dwValueLen = sizeof(DWORD);

    if (pszConfigParamKeyPath == NULL ||
        pszKey == NULL ||
        pdwValue == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Reading Reg: %s", pszKey );

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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegQueryValueExA(
                        hKey,
                        pszKey,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwValue,
                        &dwValueLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwType != REG_DWORD)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwValue = dwValue;

cleanup:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    VMDIR_LOG_VERBOSE(
        VMDIR_LOG_MASK_ALL,
        "VmDirGetRegKeyValueDword failed with error (%u)(%s)",
        dwError,
        VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
VmDirGetRegKeyValueQword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PINT64  pi64Value
    )
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    INT64   qwValue = 0;
    DWORD   dwValueLen = sizeof(qwValue);

    if (pszConfigParamKeyPath == NULL ||
        pszKey == NULL ||
        pi64Value == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Reading Reg: %s", pszKey );

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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegQueryValueExA(
                        hKey,
                        pszKey,
                        NULL,
                        &dwType,
                        (LPBYTE) &qwValue,
                        &dwValueLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwType != REG_QWORD)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pi64Value = qwValue;

cleanup:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    VMDIR_LOG_VERBOSE(
        VMDIR_LOG_MASK_ALL,
        "VmDirGetRegKeyValueDword failed with error (%u)(%s)",
        dwError,
        VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

#endif

DWORD
VmDirReadDCAccountPassword(
    PSTR* ppszPassword)
{
    int     dwError = 0;
    PSTR    pLocalPassword = NULL;

    dwError = VmDirAllocateMemory( VMDIR_KDC_RANDOM_PWD_LEN + 1, (PVOID *)&pLocalPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_DC_ACCOUNT_PWD, pLocalPassword,
                                   VMDIR_KDC_RANDOM_PWD_LEN + 1 );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszPassword = pLocalPassword;

cleanup:
    return dwError;

error:
    VMDIR_SECURE_FREE_STRINGA(pLocalPassword);
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirReadDCAccountPassword failed with error code: %d", dwError );
    goto cleanup;
}

DWORD
VmDirReadDCAccountOldPassword(
    PSTR* ppszPassword)
{
    int     dwError = 0;
    PSTR    pLocalPassword = NULL;

    dwError = VmDirAllocateMemory( VMDIR_KDC_RANDOM_PWD_LEN + 1, (PVOID *)&pLocalPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_DC_ACCOUNT_OLD_PWD, pLocalPassword,
                                   VMDIR_KDC_RANDOM_PWD_LEN + 1 );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszPassword = pLocalPassword;

cleanup:
    return dwError;

error:
    VMDIR_SECURE_FREE_STRINGA(pLocalPassword);
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirReadDCAccountOldPassword failed with error code: %d", dwError );
    goto cleanup;
}

DWORD
VmDirRegReadDCAccount(
    PSTR* ppszDCAccount
    )
{
    int     dwError = 0;
    PSTR    pszLocal = NULL;
    char    pszBuf[512] = {0};

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszBuf,
                                   sizeof(pszBuf)-1 );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pszBuf, &pszLocal );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDCAccount = pszLocal;

cleanup:

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocal);
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRegReadDCAccount failed (%d)", dwError );

    goto cleanup;
}

DWORD
VmDirRegReadKrb5Conf(
    PSTR* ppszKrb5Conf
    )
{
    int     dwError = 0;
    PSTR    pszLocal = NULL;
    char    pszBuf[512] = {0};

    dwError = VmDirGetRegKeyValue( VMAFD_CONFIG_PARAMETER_KEY_PATH,
                                   VMAFD_REG_KEY_KRB5_CONF,
                                   pszBuf,
                                   sizeof(pszBuf)-1 );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pszBuf, &pszLocal );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszKrb5Conf = pszLocal;

cleanup:

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocal);
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRegReadKrb5Conf failed (%d)", dwError );

    goto cleanup;
}

/*
 * extract hostname portion of labeledURI
 *
 * ldap(s)://HOSTNAME:port where HOSTNAME could be
 * 1. FQDN or hostname
 * 2. IPV4 "x.x.x.x"
 * 3. IPV6 "[IPV6]"
 */
DWORD
VmDirReplURIToHostname(
    PSTR    pszRepURI,
    PSTR*   ppszPartnerHostName
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalPartnerHostName = NULL;
    PSTR    pszSlashSeperator = NULL;
    PSTR    pszPortSeperator = NULL;
    PSTR    pszStartHostName = NULL;
    PSTR    pszLocalErrorMsg = NULL;
    BOOLEAN bIPV6=FALSE;

    if ( pszRepURI == NULL || ppszPartnerHostName == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg,
                                     "VmDirReplURIToHostname: replication URI/Hostname NULL" );
    }

    if ((pszSlashSeperator = VmDirStringChrA(pszRepURI, '/')) != NULL)
    {
        pszStartHostName = pszSlashSeperator + 2; /* skip // */
        if (pszStartHostName[0] == '[') // IPV6 URI
        {
            pszStartHostName++;
            bIPV6=TRUE;
        }
    }
    else
    {
        pszStartHostName = pszRepURI;
    }

    dwError = VmDirAllocateStringA( pszStartHostName, &pszLocalPartnerHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // last ':' i.e. :port
    if ( bIPV6 )
    {   // first ']'
        if ((pszPortSeperator = VmDirStringChrA(pszLocalPartnerHostName, ']')) != NULL)
        {
            *pszPortSeperator = '\0';
        }
    }
    else if ((pszPortSeperator = VmDirStringRChrA(pszLocalPartnerHostName, ':')) != NULL)
    {   // last ':' i.e. :port
        *pszPortSeperator = '\0';
    }

    *ppszPartnerHostName = pszLocalPartnerHostName;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Replication partner: %s", *ppszPartnerHostName);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    VMDIR_SAFE_FREE_MEMORY(pszLocalPartnerHostName);

    goto cleanup;
}

DWORD
VmDirValidateDCAccountPassword(
    PSTR pszPassword)
{
    DWORD dwError = 0;
    PSTR  pLocalPassword = NULL;

    dwError = VmDirAllocateMemory( VMDIR_KDC_RANDOM_PWD_LEN + 1, (PVOID *)&pLocalPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_DC_ACCOUNT_PWD, pLocalPassword,
                                   VMDIR_KDC_RANDOM_PWD_LEN + 1 );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirIsValidSecret(pszPassword, pLocalPassword) == FALSE)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pLocalPassword);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirValidateDCAccountPassword failed with error code: %d", dwError );
    goto cleanup;
}

VOID
VmDirSleep(
    DWORD dwMilliseconds
)
{
#ifndef _WIN32
    sleep( dwMilliseconds/1000 );
#else
    Sleep( dwMilliseconds );
#endif
}

DWORD
VmDirRun(
    PCSTR pszCmd
    )
{
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "exeuting %s\n", pszCmd);
    return system(pszCmd);
}

/*
 * If pszServerName is in IP format, use it as Lotus Server Name.
 * If pszServerName is NOT "localhost" which means caller specify a name they prefer, use it as the Lotus Server Name.
 *
 * Otherwise, derive FQDN based on existing network naming configuration.
 *   i.e. Call gethostname then perform forward+reverse lookup to derive the FQDN as Lotus Server Name.
 *        The forward+reverse look up is for kerberos naming consistency between server (Lotus) and clients, which
 *        could be Lotus or open sources, e.g. openldap.
 *        However, this auto name resolution is error-prone as system could have multiple IF(s) defined and
 *        we have no idea which IF we should pick to perform reverse lookup.
 *        Thus, the best chance to get Kerberos working is - customer provides proper FQDN as Lotus Server Name.
 */
DWORD
VmDirGetLotusServerName(
    PCSTR   pszServerName,
    PSTR*   ppOutServerName
    )
{
    DWORD dwError = 0;
    PSTR  pszHostnameCanon = NULL;
    char  pszLocalHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};

    if ( !pszServerName || !ppOutServerName )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirStringCompareA( pszServerName, "localhost", FALSE ) != 0 )
    {   // caller provides preferred Lotus Server Name or IP
        dwError = VmDirAllocateStringA( pszServerName, &pszHostnameCanon );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {   // caller does NOT specify preferred Lotus Server Name, derives it ourselves.
        dwError = VmDirGetHostName(pszLocalHostName, sizeof(pszLocalHostName)-1);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetCanonicalHostName(pszLocalHostName, &pszHostnameCanon);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOutServerName = pszHostnameCanon;
    pszHostnameCanon = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Lotus server name: (%s)", *ppOutServerName);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHostnameCanon);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed (%s). Error(%u)",
                    __FUNCTION__, VDIR_SAFE_STRING(pszServerName), dwError);
    goto cleanup;
}

DWORD
VmDirAllocASCIILowerToUpper(
    PCSTR   pszInputStr,
    PSTR*   ppszOutputStr)
{
    DWORD   dwError = 0;
    size_t  iCnt=0;
    size_t  iLen = 0;
    PSTR    pszOutputStr = NULL;

    if ((pszInputStr == NULL) || (ppszOutputStr == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    iLen = VmDirStringLenA(pszInputStr);

    dwError = VmDirAllocateStringPrintf( &pszOutputStr, "%s", pszInputStr );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < iLen; iCnt++)
    {
        VMDIR_ASCII_LOWER_TO_UPPER(pszOutputStr[iCnt]);
    }

    *ppszOutputStr = pszOutputStr;

cleanup:
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirAllocASCIILowerToUpper failed. Error(%u)", dwError);

    VMDIR_SAFE_FREE_MEMORY(pszOutputStr);
    goto cleanup;
}

#ifdef _WIN32

DWORD
VmDirGetProgramDataEnvVar(
    _TCHAR *pEnvName,   // [in]
    _TCHAR **ppEnvValue // [out]
    )
{
    DWORD           dwError                 = 0;
    DWORD           dwReturn                = 0;
    DWORD           dwBufSize                = MAX_PATH; // just a initial value
    LPTSTR          pszEnvValBuf            = NULL;

#ifndef WIN2008 /* i.e., Windows server 2003 */
    const _TCHAR ProgramDataEnvName[] = _T("ALLUSERSPROFILE");
    const _TCHAR VmwareDataSuffix[]   = _T("");
#else /* i.e., Windows server 2008 */
    const _TCHAR ProgramDataEnvName[] = _T("PROGRAMDATA");
    const _TCHAR VmwareDataSuffix[]   = _T("");
#endif
    SIZE_T desiredLen = 0;

    if (VmDirStringCompareA("PROGRAMDATA", pEnvName, FALSE) == 0)
    // we currently only need "PROGRAMDATA", may add more support later
    {
        dwError = VmDirAllocateMemory(
                        dwBufSize * sizeof(TCHAR),
                        (PVOID*) &pszEnvValBuf
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwReturn = GetEnvironmentVariable(
                        ProgramDataEnvName,
                        pszEnvValBuf,
                        dwBufSize
                        );

        if (0 == dwReturn) // failed
        {
            dwError = GetLastError();
            if ( ERROR_ENVVAR_NOT_FOUND == dwError )
            {
                VmDirLog( LDAP_DEBUG_ANY, "Environment variable does not exist. %s", pEnvName);
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else // succeeded
        {
            // TODO: add support for Unicode
            desiredLen = dwReturn + 1 + VmDirStringLenA(VmwareDataSuffix);
            if (dwBufSize < desiredLen) // buffer too small, however, it should
                                       // never happen for ProgramData env.
            {
                dwError = VmDirReallocateMemory(
                                pszEnvValBuf,
                                &pszEnvValBuf,
                                desiredLen * sizeof (TCHAR)
                                );
                BAIL_ON_VMDIR_ERROR(dwError);

                dwReturn = GetEnvironmentVariable(
                            ProgramDataEnvName,
                            pszEnvValBuf,
                            (DWORD)desiredLen
                            );

                if (!dwReturn)
                {
                    dwError = GetLastError();
                    VmDirLog(
                        LDAP_DEBUG_ANY,
                        "GetEnvironmentVariable failed (%d)",
                        dwError
                        );
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }
        // Append suffix to env variable
        // Todo: add support for Unicode
        dwError = VmDirStringCatA(
                        pszEnvValBuf,
                        desiredLen,
                        VmwareDataSuffix
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
        *ppEnvValue = pszEnvValBuf;
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszEnvValBuf);
    goto cleanup;

}
#endif

DWORD
VmDirAllocASCIIUpperToLower(
    PCSTR   pszInputStr,
    PSTR*   ppszOutputStr)
{
    DWORD   dwError = 0;
    size_t  iCnt=0;
    size_t  iLen = 0;
    PSTR    pszOutputStr = NULL;

    if ((pszInputStr == NULL) || (ppszOutputStr == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    iLen = VmDirStringLenA(pszInputStr);

    dwError = VmDirAllocateStringPrintf( &pszOutputStr, "%s", pszInputStr );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < iLen; iCnt++)
    {
        VMDIR_ASCII_UPPER_TO_LOWER(pszOutputStr[iCnt]);
    }

    *ppszOutputStr = pszOutputStr;

cleanup:
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirAllocASCIIUpperToLower failed. Error(%u)", dwError);

    VMDIR_SAFE_FREE_MEMORY(pszOutputStr);
    goto cleanup;
}

#ifndef _WIN32
DWORD
VmDirFileExists(
    PCSTR       pszFileName,
    PBOOLEAN    pbFound)
{
    DWORD       dwError = 0;
    BOOLEAN     bFound = FALSE;
    struct stat statBuf = {0};
    int         iRetVal = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pszFileName, dwError);

    iRetVal = stat(pszFileName, &statBuf);
    if (iRetVal == 0 && S_ISREG(statBuf.st_mode))
    {
        bFound = TRUE;
    }

    *pbFound = bFound;

error:
    return dwError;
}
#else
DWORD
VmDirFileExists(
    PCSTR       pszFileName,
    PBOOLEAN    pbFound)
{
    DWORD               dwError = 0;
    BOOLEAN             bFound = FALSE;
    WIN32_FIND_DATAA    FindFileData = {0};
    HANDLE              handle = NULL;

    BAIL_ON_VMDIR_INVALID_POINTER(pszFileName, dwError);

    handle = FindFirstFileA(pszFileName, &FindFileData) ;
    if (handle != INVALID_HANDLE_VALUE)
    {
        bFound = TRUE;
        FindClose(handle);
    }

    *pbFound = bFound;

error:
    return dwError;
}
#endif

#ifdef _WIN32
/*
 * Get the directory where we should store our config files, i.e.,
 * <VMDIR_REG_KEY_DATA_PATH config key value or %PROGRAMDATA%>\cfg\%COMPONENT%
 * %PROGRAMDATA% usually expands to
 * C:\ProgramData\VMware\CIS\cfg\vmdir (note: we don't have a trailing "\" here)
 *
 * We assume lpHomeDirBuffer is pre-allocated,
 * and the size is MAX_PATH.
 */

const   _TCHAR  serverCertFilename[]        = _T("\\vmdircert.pem");
const   _TCHAR  serverKeyFilename[]        = _T("\\vmdirkey.pem");

static DWORD
VmDirOpensslGetConfigHomeDir(_TCHAR *lpHomeDirBuffer)
{
    DWORD   dwError              = 0;
    const   _TCHAR configHomeDir[]   = _T("\\VMware\\CIS\\cfg\\vmdird");
    size_t  configHomeDirLen         = VmDirStringLenA(configHomeDir);
    size_t  configHomePrefixLen      = 0 ;

    if ((dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_CONFIG_PATH, lpHomeDirBuffer,
                                        MAX_PATH )) != 0)
    {
#ifdef WIN2008
        dwError =  GetEnvironmentVariable(
            _T("PROGRAMDATA"),    // __in_opt   LPCTSTR lpName,
            lpHomeDirBuffer,      // __out_opt  LPTSTR lpBuffer,
            MAX_PATH              // __in       DWORD nSize
        );
#else
        dwError =  GetEnvironmentVariable(
            _T("ALLUSERSPROFILE"),    // __in_opt   LPCTSTR lpName,
            lpHomeDirBuffer,      // __out_opt  LPTSTR lpBuffer,
            MAX_PATH              // __in       DWORD nSize
        );
#endif
        BAIL_ON_VMDIR_ERROR(0 == dwError);
        dwError = ERROR_SUCCESS;

        configHomePrefixLen = VmDirStringLenA(lpHomeDirBuffer);

        if ( configHomePrefixLen + configHomeDirLen < MAX_PATH )
        {
           dwError = VmDirStringCatA(lpHomeDirBuffer, MAX_PATH, configHomeDir);
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
    }
error:
    return dwError;
}

DWORD
VmDirOpensslSetServerCertPath(_TCHAR *lpPath)
{
    DWORD   dwError                 = 0;
    size_t  serverCertFilenameLen   = VmDirStringLenA(serverCertFilename);
    size_t  serverCertPrefixLen     = 0;

    dwError = VmDirOpensslGetConfigHomeDir(lpPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ERROR_SUCCESS;

    serverCertPrefixLen = VmDirStringLenA(lpPath);

    if ( serverCertPrefixLen + serverCertFilenameLen < MAX_PATH )
    {
        dwError = VmDirStringCatA(lpPath, MAX_PATH, serverCertFilename);
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
error:
    return dwError;
}

DWORD
VmDirOpensslSetServerKeyPath(_TCHAR *lpPath)
{

    DWORD   dwError                 = 0;
    size_t  serverKeyFilenameLen    = VmDirStringLenA(serverKeyFilename);
    size_t  serverKeyPrefixLen     = 0;

    dwError = VmDirOpensslGetConfigHomeDir(lpPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ERROR_SUCCESS;

    serverKeyPrefixLen = VmDirStringLenA(lpPath);

    if ( serverKeyPrefixLen + serverKeyFilenameLen < MAX_PATH )
    {
        dwError = VmDirStringCatA(lpPath, MAX_PATH, serverKeyFilename);
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
error:
    return dwError;
}

#endif

DWORD
VmDirCertificateFileNameFromHostName(
    PCSTR   pszPartnerHostName,
    PSTR *  ppszFileName)
{
    DWORD   dwError = 0;
    PSTR    pszSlash = NULL;
    PSTR    pszLocalFileName = NULL;
    PSTR    pszLocalRsaServerCertFileName = NULL;
    PSTR    pszLocalErrMsg = NULL;

#ifdef _WIN32
    _TCHAR          RSA_SERVER_CERT[MAX_PATH];

    dwError = VmDirOpensslSetServerCertPath(RSA_SERVER_CERT);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "VmDirOpensslSetServerCertPath() failed" );
#endif

    if (VmDirStringCompareA(pszPartnerHostName, "localhost", TRUE) == 0)
    {
        dwError = VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND;  // disable server cert verification
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringAVsnprintf( &pszLocalRsaServerCertFileName, "%s", RSA_SERVER_CERT);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirAllocateStringAVsnprintf(pszLocalRsaServerCertFileName) failed" );

        pszSlash = VmDirStringRChrA(pszLocalRsaServerCertFileName, VMDIR_PATH_SEPARATOR_STR[0]);

        if (pszSlash == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "No VMDIR_PATH_SEPARATOR_STR[0] in pszLocalRsaServerCertFileName (%s) ",
                                          pszLocalRsaServerCertFileName  );
        }

        *(pszSlash + 1) = '\0';

        dwError = VmDirAllocateStringAVsnprintf( &pszLocalFileName, "%s%s.pem", pszLocalRsaServerCertFileName, pszPartnerHostName);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirAllocateStringAVsnprintf(pszLocalFileName) failed" );
    }

    *ppszFileName = pszLocalFileName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalRsaServerCertFileName);
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalFileName);
    if ( dwError != VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetPartnerCertificateFileName() failed with error message (%s), error code (%u)",
                            VDIR_SAFE_STRING(pszLocalErrMsg), dwError);
    }
    goto cleanup;
}

// Given [<ldap/ldaps>://]<partner host name>[:port number], generate partner's certificate file name

DWORD
VmDirCertficateFileNameFromLdapURI(
    PSTR    pszLdapURI,
    PSTR *  ppszCertFileName)

{
    DWORD   dwError = 0;
    PSTR    pszPartnerHostName = NULL;
    PSTR    pszSlashSeperator = NULL;
    PSTR    pszPortSeperator = NULL;
    PSTR    pszStartHostName = NULL;
    PSTR    pszLocalErrorMsg = NULL;
    PSTR    pszLocalCertFileName = NULL;

    if (pszLdapURI == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg,
                                     "VmDirCertficateFileNameFromLdapURI: LDAP URI NULL" );
    }

    if ((pszSlashSeperator = VmDirStringChrA(pszLdapURI, '/')) != NULL)
    {
        pszStartHostName = pszSlashSeperator + 2; /* skip // */
    }
    else
    {
        pszStartHostName = pszLdapURI;
    }

    dwError = VmDirAllocateStringA( pszStartHostName, &pszPartnerHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((pszPortSeperator = VmDirStringRChrA(pszPartnerHostName, ':')) != NULL)
    {
        *pszPortSeperator = '\0';
    }

    dwError = VmDirCertificateFileNameFromHostName( pszPartnerHostName, &pszLocalCertFileName );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszCertFileName = pszLocalCertFileName;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Ldap URI is: %s, certificate file name is: %s",
                      pszLdapURI, pszLocalCertFileName );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);

    return dwError;

error:

    if (dwError != VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    }

    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszLocalCertFileName);

    goto cleanup;
}

/*
 *  Quick and dirty function to verify format - 3 dots and digits only
 *
 *  called with input from struct addrinfo*->ai_canonname
 */
static
BOOLEAN
_VmDirIsIPV4AddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN     bIsIPV4 = TRUE;
    size_t      iSize = VmDirStringLenA(pszAddr);
    size_t      iCnt = 0;
    size_t      iDotCnt = 0;

    for (iCnt=0; bIsIPV4 && iCnt < iSize; iCnt++)
    {
        if ( pszAddr[iCnt] == '.' )
        {
            iDotCnt++;
        }
        else if ( VMDIR_ASCII_DIGIT( pszAddr[iCnt]) )
        {
        }
        else
        {
            bIsIPV4 = FALSE;
        }
    }

    if ( iDotCnt != 3 )
    {
        bIsIPV4 = FALSE;
    }

    return bIsIPV4;
}

/*
 *  Quick and dirty function to verify format - colons and digits/a-f/A-F only
 */
BOOLEAN
VmDirIsIPV6AddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN     bIsIPV6 = pszAddr ? TRUE : FALSE;
    size_t      iSize = 0;
    size_t      iCnt = 0;
    size_t      iColonCnt = 0;

    if ( pszAddr != NULL )
    {
        iSize = VmDirStringLenA(pszAddr);
        for (iCnt=0; bIsIPV6 && iCnt < iSize; iCnt++)
        {
            if ( pszAddr[iCnt] == ':' )
            {
                iColonCnt++;
            }
            else if ( VMDIR_ASCII_DIGIT( pszAddr[iCnt] )
                      ||
                      VMDIR_ASCII_aTof( pszAddr[iCnt] )
                      ||
                      VMDIR_ASCII_AToF( pszAddr[iCnt] )
                    )
            {
            }
            else
            {
                bIsIPV6 = FALSE;
            }
        }

        // should not count on iColonCnt == 7
        if ( iColonCnt < 2 )
        {
            bIsIPV6 = FALSE;
        }
    }

    return bIsIPV6;
}

BOOLEAN
VmDirIsIPAddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN bIsIPFormat = FALSE;

    if ( pszAddr
         &&
         (_VmDirIsIPV4AddrFormat( pszAddr ) == TRUE || VmDirIsIPV6AddrFormat( pszAddr ) == TRUE )
       )
    {
        bIsIPFormat = TRUE;
    }

    return bIsIPFormat;
}

/*
 * Compare two strings by its length
 */
int
VmDirCompareStrByLen(
    const void * pszStr1, // PSTR *
    const void * pszStr2  // PSTR *
    )
{
    size_t  len1 = VmDirStringLenA(*(PSTR *)pszStr1);
    size_t  len2 = VmDirStringLenA(*(PSTR *)pszStr2);

    return ( len1 < len2 ? -1 : len1 == len2 ? 0 : 1);
}

DWORD
VmDirReadStringFromFile(
    PCSTR pszFile,
    PSTR szString,
    int len
    )
{
    DWORD dwError = 0;
    PSTR pszNl = NULL;
    FILE *fp = NULL;

    if (pszFile == NULL || szString == NULL || len < 1)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    fp = fopen(pszFile, "r");
    if (!fp)
    {
        dwError = errno;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (fgets(szString, len, fp) == NULL)
    {
        szString[0] = '\0';
    }
    else
    {
        pszNl = VmDirStringChrA(szString, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }
    }

cleanup:
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

VOID
VmDirReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    )
{
#ifndef _WIN32
    sigset_t sig, osig;
    struct termios ts, ots;
#else
    DWORD oldMode = 0;
    HANDLE hConIn = INVALID_HANDLE_VALUE;
#endif
    PSTR pszNl = NULL;

    assert(szString != NULL);
    assert(len > 0);

    if (bHideString)
    {
#ifndef _WIN32
        sigemptyset(&sig);
        sigaddset(&sig, SIGINT);
        sigaddset(&sig, SIGTSTP);
        sigprocmask(SIG_BLOCK, &sig, &osig);

        tcgetattr(fileno(stdin), &ts);
        ots = ts;
        ts.c_lflag &= ~(ECHO);
        tcsetattr(fileno(stdin), TCSANOW, &ts);
#else
        hConIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hConIn != INVALID_HANDLE_VALUE)
        {
            if (GetConsoleMode(hConIn, &oldMode))
            {
                SetConsoleMode(hConIn, oldMode & ~ENABLE_ECHO_INPUT);
            }
        }
#endif
    }

    if (szPrompt)
    {
        fputs(szPrompt, stderr);
        fflush(stderr);
    }

    if (fgets(szString, len, stdin) == NULL)
    {
        szString[0] = '\0';
    }
    else
    {
        pszNl = VmDirStringChrA(szString, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }
    }

    if (bHideString)
    {
        fputs("\n", stderr);

#ifndef _WIN32
        tcsetattr(fileno(stdin), TCSANOW, &ots);
        sigprocmask(SIG_SETMASK, &osig, NULL);
#else
        SetConsoleMode(hConIn, oldMode);
#endif
    }
    fflush(stderr);
}

DWORD
VmDirAllocateUserCreateParamsWFromA(
    PVMDIR_USER_CREATE_PARAMS_A  pCreateParamsA,
    PVMDIR_USER_CREATE_PARAMS_W* ppCreateParamsW
    )
{
    DWORD dwError = 0;
    PVMDIR_USER_CREATE_PARAMS_W pCreateParamsW = NULL;

    dwError = VmDirAllocateMemory(
                    sizeof(*pCreateParamsW),
                    (PVOID*)&pCreateParamsW);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pCreateParamsA->pszName)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszName,
                        &pCreateParamsW->pwszName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsA->pszAccount)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszAccount,
                        &pCreateParamsW->pwszAccount);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsA->pszUPN)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszUPN,
                        &pCreateParamsW->pwszUPN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsA->pszFirstname)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszFirstname,
                        &pCreateParamsW->pwszFirstname);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsA->pszLastname)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszLastname,
                        &pCreateParamsW->pwszLastname);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsA->pszPassword)
    {
        dwError = VmDirAllocateStringWFromA(
                        pCreateParamsA->pszPassword,
                        &pCreateParamsW->pwszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppCreateParamsW = pCreateParamsW;

cleanup:

    return dwError;

error:

    *ppCreateParamsW = NULL;

    if (pCreateParamsW)
    {
        VmDirFreeUserCreateParamsW(pCreateParamsW);
    }

    goto cleanup;
}

DWORD
VmDirAllocateUserCreateParamsAFromW(
    PVMDIR_USER_CREATE_PARAMS_W  pCreateParamsW,
    PVMDIR_USER_CREATE_PARAMS_A* ppCreateParamsA
    )
{
    DWORD dwError = 0;
    PVMDIR_USER_CREATE_PARAMS_A pCreateParamsA = NULL;

    dwError = VmDirAllocateMemory(
                    sizeof(*pCreateParamsA),
                    (PVOID*)&pCreateParamsA);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pCreateParamsW->pwszName)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszName,
                        &pCreateParamsA->pszName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsW->pwszAccount)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszAccount,
                        &pCreateParamsA->pszAccount);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsW->pwszUPN)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszUPN,
                        &pCreateParamsA->pszUPN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsW->pwszFirstname)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszFirstname,
                        &pCreateParamsA->pszFirstname);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsW->pwszLastname)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszLastname,
                        &pCreateParamsA->pszLastname);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCreateParamsW->pwszPassword)
    {
        dwError = VmDirAllocateStringAFromW(
                        pCreateParamsW->pwszPassword,
                        &pCreateParamsA->pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppCreateParamsA = pCreateParamsA;

cleanup:

    return dwError;

error:

    *ppCreateParamsA = NULL;

    if (pCreateParamsA)
    {
        VmDirFreeUserCreateParamsA(pCreateParamsA);
    }

    goto cleanup;
}

VOID
VmDirFreeUserCreateParamsA(
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    )
{
    if (pCreateParams)
    {
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszName);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszAccount);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszUPN);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszFirstname);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszLastname);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pszPassword);
        VmDirFreeMemory(pCreateParams);
    }
}

VOID
VmDirFreeUserCreateParamsW(
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    )
{
    if (pCreateParams)
    {
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszName);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszAccount);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszUPN);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszFirstname);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszLastname);
        VMDIR_SAFE_FREE_MEMORY(pCreateParams->pwszPassword);
        VmDirFreeMemory(pCreateParams);
    }
}

VOID
VmDirFreeMemberships(
    PSTR *ppszMemberships,
    DWORD dwMemberships
    )
{
    if (ppszMemberships != NULL)
    {
        DWORD i = 0;

        for (i = 0; i < dwMemberships; i++)
        {
            VMDIR_SAFE_FREE_STRINGA(ppszMemberships[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszMemberships);
    }
}
