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

DWORD
VmDirGetNetworkInfoFromSocket(
    ber_socket_t fd,
    PSTR pszAddress,
    DWORD dwAddressLen,
    PDWORD pdwPort,
    BOOLEAN bClientInfo // Client or Server side of socket?
    )
{
    DWORD dwError = 0;
    int iRetVal = 0;
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (bClientInfo)
    {
        iRetVal = getpeername((int)fd, (struct sockaddr*)&addr, &len);
    }
    else
    {
        iRetVal = getsockname((int)fd, (struct sockaddr*)&addr, &len);
    }

    if (iRetVal != 0)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in*) &addr;
        if (inet_ntop(AF_INET, &sin->sin_addr, pszAddress, dwAddressLen) != NULL)
        {
            *pdwPort = ntohs(sin->sin_port);
        }
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin = (struct sockaddr_in6*) &addr;
        if (inet_ntop(AF_INET6, &sin->sin6_addr, pszAddress, dwAddressLen) != NULL)
        {
            *pdwPort = ntohs(sin->sin6_port);
        }
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s. Error(%u) iRetVal(%d)", __FUNCTION__, dwError, iRetVal);

    goto cleanup;
}

DWORD
VmDirGenerateGUID(
    PSTR*   ppszGuid
    )
{
    DWORD   dwError = 0;
    uuid_t  guid;
    PSTR    pszGuid = NULL;

    if (!ppszGuid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(VMDIR_GUID_STR_LEN, (PVOID*)&pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidGenerate(&guid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidToStringLower( &guid, pszGuid, VMDIR_GUID_STR_LEN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;

cleanup:
    return dwError;
error:
    VmDirLog( LDAP_DEBUG_TRACE, "VmDirGenerateGUID failed. Error(%u)", dwError);
    VMDIR_SAFE_FREE_MEMORY(pszGuid);
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
VmDirGetRegGuid(
    PCSTR pszKey,
    PSTR  pszGuid
    )
{
    return VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH, pszKey, pszGuid, VMDIR_GUID_STR_LEN );
}

DWORD
VmDirGetRegKeyTabFile(
    PSTR  pszKeyTabFile
    )
{
    return VmDirGetRegKeyValue( VMAFD_CONFIG_PARAMETER_KEY_PATH, VMDIR_REG_KEY_KEYTAB_FILE, pszKeyTabFile,
                                VMDIR_MAX_FILE_NAME_LEN );
}

DWORD
VmDirGetLocalLduGuid(
    PSTR pszLduGuid
    )
{
    return VmDirGetRegGuid(VMDIR_REG_KEY_LDU_GUID, pszLduGuid);
}

DWORD
VmDirGetLocalSiteGuid(
    PSTR pszSiteGuid
    )
{
    return VmDirGetRegGuid(VMDIR_REG_KEY_SITE_GUID, pszSiteGuid);
}

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

DWORD
VmDirGetRegKeyValueDword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue
    )
#ifdef _WIN32
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

cleanup:
    *pdwValue = dwValue;

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    dwValue = dwDefaultValue;

    VMDIR_LOG_VERBOSE(
        VMDIR_LOG_MASK_ALL,
        "VmDirGetRegKeyValueDword failed with error (%u)(%s)",
        dwError,
        pszKey);

    goto cleanup;
}
#else
{
    DWORD dwError = 0;
    DWORD dwValue = 0;
    REG_DATA_TYPE RegType = 0;

    if (pszConfigParamKeyPath == NULL || pszKey == NULL || pdwValue == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Reading Reg: %s", pszKey);

    dwError = RegUtilGetValue(
                NULL,
                HKEY_THIS_MACHINE,
                NULL,
                pszConfigParamKeyPath,
                pszKey,
                &RegType,
                (PVOID*)&dwValue,
                NULL);

    BAIL_ON_VMDIR_ERROR(dwError);

    if (RegType != REG_DWORD)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    *pdwValue = dwValue;
    return dwError;

error:
    dwValue = dwDefaultValue;

    VMDIR_LOG_VERBOSE(
        VMDIR_LOG_MASK_ALL,
        "VmDirGetRegKeyValueDword failed with error (%u)(%s)",
        dwError,
        pszKey);

    goto cleanup;
}
#endif

DWORD
VmDirSetRegKeyValueString(
    PCSTR pszConfigParamKeyPath,
    PCSTR pszKey,
    PCSTR pszValue,
    DWORD dwLength /* Should not include +1 for terminating null */
    )
#ifdef _WIN32
{
    DWORD dwError = 0;
    HKEY hKey = NULL;

    dwError = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                              pszConfigParamKeyPath,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(hKey,
                             pszKey,
                             0,
                             REG_SZ,
                             (BYTE*)pszValue,
                             dwLength + 1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    goto cleanup;
}
#else
{
    DWORD dwError;

    dwError = RegUtilSetValue(NULL,
                              HKEY_THIS_MACHINE,
                              pszConfigParamKeyPath,
                              NULL,
                              pszKey,
                              REG_SZ,
                              (BYTE*)pszValue,
                              dwLength + 1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}
#endif

DWORD
VmDirWriteDCAccountOldPassword(
    PCSTR pszOldPassword,
    DWORD dwLength /* Length of the string, not including null */
    )
{
    DWORD dwError = 0;

    dwError = VmDirSetRegKeyValueString(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                VMDIR_REG_KEY_DC_ACCOUNT_OLD_PWD,
                pszOldPassword,
                dwLength);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirWriteDCAccountOldPassword failed with error code: %d", dwError );
    goto cleanup;
}

DWORD
VmDirWriteDCAccountPassword(
    PCSTR pszPassword,
    DWORD dwLength /* Length of the string, not including null */
    )
{
    DWORD dwError = 0;

    dwError = VmDirSetRegKeyValueString(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                VMDIR_REG_KEY_DC_ACCOUNT_PWD,
                pszPassword,
                dwLength);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirWriteDCAccountPassword failed with error code: %d", dwError );
    goto cleanup;
}

DWORD
VmDirSetRegKeyValueDword(
    PCSTR pszConfigParamKeyPath,
    PCSTR pszKey,
    DWORD dwValue
    )
#ifdef _WIN32
{
    DWORD dwError = 0;
    HKEY hKey = NULL;

    dwError = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                              pszConfigParamKeyPath,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(hKey,
                             pszKey,
                             0,
                             REG_DWORD,
                             (BYTE*)&dwValue,
                             sizeof(dwValue));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwError;

error:
    goto cleanup;
}
#else
{
    DWORD dwError;

    dwError = RegUtilSetValue(NULL,
                              HKEY_THIS_MACHINE,
                              pszConfigParamKeyPath,
                              NULL,
                              pszKey,
                              REG_DWORD,
                              &dwValue,
                              sizeof(dwValue));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}
#endif

#ifdef _WIN32
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
        "VmDirGetRegKeyValueQword failed with error (%u)(%s)",
        dwError,
        VDIR_SAFE_STRING(pszKey));

    goto cleanup;
}

#endif

DWORD
VmDirLoadLibrary(
    PCSTR           pszLibPath,
    VMDIR_LIB_HANDLE* ppLibHandle
    )
{
    DWORD   dwError = 0;
    VMDIR_LIB_HANDLE pLibHandle = NULL;

#ifdef _WIN32
    pLibHandle = LoadLibrary(pszLibPath);
    if (pLibHandle == NULL)
    {
        VMDIR_LOG_VERBOSE(
            VMDIR_LOG_MASK_ALL,
            "LoadLibrary %s failed, error code %d",
            pszLibPath,
            WSAGetLastError());
        dwError = VMDIR_ERROR_CANNOT_LOAD_LIBRARY;
    }
#else
    pLibHandle = dlopen(pszLibPath, RTLD_LAZY);
    if (pLibHandle == NULL)
    {
        VMDIR_LOG_VERBOSE(
             VMDIR_LOG_MASK_ALL,
             "dlopen %s library failed, error msg (%s)",
             pszLibPath,
             VDIR_SAFE_STRING(dlerror()));
         dlerror();    /* Clear any existing error */
         dwError = VMDIR_ERROR_CANNOT_LOAD_LIBRARY;
    }
#endif
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLibHandle = pLibHandle;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirCloseLibrary(
    VMDIR_LIB_HANDLE  pLibHandle
    )
{
    if (pLibHandle)
    {
#ifdef _WIN32
        FreeLibrary(pLibHandle);
#else
        dlclose(pLibHandle);
#endif
    }
}

#ifdef _WIN32
FARPROC WINAPI
#else
VOID*
#endif
VmDirGetLibSym(
    VMDIR_LIB_HANDLE  pLibHandle,
    PCSTR           pszFunctionName
    )
{
#ifdef _WIN32
    return GetProcAddress(pLibHandle, pszFunctionName);
#else
    return dlsym(pLibHandle, pszFunctionName);
#endif
}

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
VmDirRegReadDCAccountDn(
    PSTR* ppszDCAccount
    )
{
    int     dwError = 0;
    PSTR    pszLocal = NULL;
    char    pszBuf[512] = {0};

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT_DN,
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
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRegReadDCAccountDn failed (%d)", dwError );

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

VOID
VmDirSleep(
    DWORD dwMilliseconds
)
{
#ifndef _WIN32
    struct timespec req={0};
    DWORD   dwSec = dwMilliseconds/1000;
    DWORD   dwMS  = dwMilliseconds%1000;

    req.tv_sec  = dwSec;
    req.tv_nsec = dwMS*1000000;

    nanosleep( &req, NULL ); // ignore error
#else
    Sleep( dwMilliseconds );
#endif
}

DWORD
VmDirRun(
    PCSTR pszCmd
    )
{
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "executing %s\n", pszCmd);
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
        dwError = VmDirAllocateStringPrintf( &pszLocalRsaServerCertFileName, "%s", RSA_SERVER_CERT);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirAllocateStringPrintf(pszLocalRsaServerCertFileName) failed" );

        pszSlash = VmDirStringRChrA(pszLocalRsaServerCertFileName, VMDIR_PATH_SEPARATOR_STR[0]);

        if (pszSlash == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "No VMDIR_PATH_SEPARATOR_STR[0] in pszLocalRsaServerCertFileName (%s) ",
                                          pszLocalRsaServerCertFileName  );
        }

        *(pszSlash + 1) = '\0';

        dwError = VmDirAllocateStringPrintf( &pszLocalFileName, "%s%s.pem", pszLocalRsaServerCertFileName, pszPartnerHostName);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirAllocateStringPrintf(pszLocalFileName) failed" );
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

/*
 * len should be the entire length of the string, including the space for the
 * null terminator.
 */
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

/*
 * Get the host portion of LDAP URI ldap(s)://host:port
 */
DWORD
VmDirLdapURI2Host(
    PCSTR   pszURI,
    PSTR*   ppszHost
    )
{
    DWORD   dwError = 0;
    PCSTR   pszSlashSeperator = NULL;
    PCSTR   pszPortSeperator  = NULL;
    PCSTR   pszStartHostName  = NULL;
    PSTR    pszLocalHostName  = NULL;
    size_t  iSize = 0;

    if ( !pszURI || !ppszHost )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // skip "ldap(s)://" part in pszURI
    if ((pszSlashSeperator = strchr( pszURI, '/')) != NULL)
    {
        pszStartHostName = pszSlashSeperator + 2; // skip
    }
    else
    {
        pszStartHostName =  pszURI;
    }

    pszPortSeperator = strrchr(pszStartHostName, ':');

    iSize = pszPortSeperator ? (pszPortSeperator - pszStartHostName) :
                                VmDirStringLenA( pszStartHostName);

    dwError = VmDirAllocateMemory(  iSize + 1, (PVOID*) &pszLocalHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory( pszLocalHostName,
                               iSize,
                               (const PVOID)pszStartHostName,
                               iSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszHost = pszLocalHostName;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY( pszLocalHostName );

    goto cleanup;
}

/*given the UPN eg. <username>@vsphere.local returns the UserName */
DWORD
VmDirUPNToNameAndDomain(
    PCSTR   pszUPN,
    PSTR*   ppszName,
    PSTR*   ppszDomain
    )
{
    DWORD   dwError = 0;
    PSTR    pszTmp = NULL;
    PSTR    pszName = NULL;
    PSTR    pszDomain = NULL;

    if (pszUPN == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszTmp = VmDirStringChrA(pszUPN, '@');
    if (pszTmp)
    {
        dwError = VmDirAllocateStringOfLenA( pszUPN, pszTmp-pszUPN, &pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA( pszTmp+1, &pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
    }

    if (ppszName)
    {
        *ppszName = pszName;
        pszName = NULL;
    }
    if (ppszDomain)
    {
        *ppszDomain = pszDomain;
        pszDomain = NULL;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszName);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "[%s][%d] for (%s) failed (%u)",
                     __FUNCTION__,__LINE__, VDIR_SAFE_STRING(pszUPN), dwError);

    goto cleanup;
}

/*
 * Qsort comparison function
 */
int
VmDirQsortCaseExactCompareString(
    const void* ppStr1,
    const void* ppStr2
    )
{
    if ((ppStr1 == NULL || *(char * const *)ppStr1 == NULL) &&
            (ppStr2 == NULL || *(char * const *)ppStr2 == NULL))
    {
        return 0;
    }

    if (ppStr1 == NULL || *(char * const *)ppStr1 == NULL)
    {
        return -1;
    }

    if (ppStr2 == NULL || *(char * const *)ppStr2 == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(* (char * const *) ppStr1, * (char * const *) ppStr2, TRUE);
}

int
VmDirQsortCaseIgnoreCompareString(
    const void* ppStr1,
    const void* ppStr2
    )
{
    if ((ppStr1 == NULL || *(char * const *)ppStr1 == NULL) &&
        (ppStr2 == NULL || *(char * const *)ppStr2 == NULL))
    {
        return 0;
    }

    if (ppStr1 == NULL || *(char * const *)ppStr1 == NULL)
    {
        return -1;
    }

    if (ppStr2 == NULL || *(char * const *)ppStr2 == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(* (char * const *) ppStr1, * (char * const *) ppStr2, FALSE);
}

DWORD
VmDirCopyStrArray(
    PSTR*   ppszOrgArray,
    PSTR**  pppszCopyArray
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszCopyArray = NULL;
    int     iSize = 0, iCnt = 0;

    if (!pppszCopyArray)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ppszOrgArray)
    {
        for (iSize=0; ppszOrgArray[iSize]; iSize++);

        dwError = VmDirAllocateMemory(
                sizeof(PSTR)*(iSize+1), (PVOID*)&ppszCopyArray);
        BAIL_ON_VMDIR_ERROR(dwError);


        for (iCnt=0; ppszOrgArray[iCnt]; iCnt++)
        {
            dwError = VmDirAllocateStringA(
                    ppszOrgArray[iCnt], &ppszCopyArray[iCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszCopyArray = ppszCopyArray;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszCopyArray);
    goto cleanup;
}

/*
 *  ppszArray1 and ppszArray2 are NULL terminated PSTR arrays.
 *  pppszOutArray is ppszArray1 UNION ppszArray2.
 */
DWORD
VmDirMergeStrArray(
    PSTR*   ppszArray1,
    PSTR*   ppszArray2,
    PSTR**  pppszOutArray
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszArray = NULL;
    int     iIdx = 0;
    int     iSize1 = 0, iSize2 = 0, iCnt1 = 0, iCnt2 = 0;

    if (!pppszOutArray)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (iSize1=0; ppszArray1 && ppszArray1[iSize1]; iSize1++);
    for (iSize2=0; ppszArray2 && ppszArray2[iSize2]; iSize2++);

    if (iSize1 == 0 && iSize2 == 0)
    {
        goto cleanup;
    }

    if (iSize1 > 0)
    {
        qsort(ppszArray1, iSize1, sizeof(*ppszArray1), VmDirQsortCaseIgnoreCompareString);
    }
    if (iSize2 > 0)
    {
        qsort(ppszArray2, iSize2, sizeof(*ppszArray2), VmDirQsortCaseIgnoreCompareString);
    }

    dwError = VmDirAllocateMemory(sizeof(PSTR)*(iSize1+iSize2+1), (PVOID*)&ppszArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt1=0, iCnt2=0, iIdx=0; iCnt1<iSize1 || iCnt2<iSize2; iIdx++)
    {
        PSTR pszTmp = NULL;

        if (iCnt1<iSize1 && iCnt2<iSize2)
        {
            LONG i = VmDirStringCompareA(ppszArray1[iCnt1], ppszArray2[iCnt2], FALSE);

            if (i == 0)
            {
                pszTmp = ppszArray1[iCnt1];
                iCnt1++;
                iCnt2++;
            }
            else if (i < 0)
            {
                pszTmp = ppszArray1[iCnt1];
                iCnt1++;
            }
            else
            {
                pszTmp = ppszArray2[iCnt2];
                iCnt2++;
            }
        }
        else if (iCnt1<iSize1 && iCnt2>=iSize2)
        {
            pszTmp = ppszArray1[iCnt1];
            iCnt1++;
        }
        else if (iCnt1>=iSize1 && iCnt2<iSize2)
        {
            pszTmp = ppszArray2[iCnt2];
            iCnt2++;
        }

        dwError = VmDirAllocateStringA(pszTmp, (ppszArray+iIdx));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppszOutArray = ppszArray;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszArray);
    goto cleanup;
}

DWORD
VmDirGetStrArrayDiffs(
    PSTR*   ppszArray1,
    PSTR*   ppszArray2,
    PSTR**  pppszNotArray1,
    PSTR**  pppszNotArray2
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszNotArray1 = NULL;
    PSTR*   ppszNotArray2 = NULL;
    int     iSize1 = 0, iSize2 = 0, i = 0, j = 0, x = 0, y = 0;

    for (iSize1=0; ppszArray1 && ppszArray1[iSize1]; iSize1++);
    for (iSize2=0; ppszArray2 && ppszArray2[iSize2]; iSize2++);

    if (iSize1 > 0)
    {
        qsort(ppszArray1, iSize1, sizeof(*ppszArray1),
                VmDirQsortCaseIgnoreCompareString);

        if (pppszNotArray2)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(PSTR) * (iSize1 + 1),
                    (PVOID*)&ppszNotArray2);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (iSize2 > 0)
    {
        qsort(ppszArray2, iSize2, sizeof(*ppszArray2),
                VmDirQsortCaseIgnoreCompareString);

        if (pppszNotArray1)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(PSTR) * (iSize2 + 1),
                    (PVOID*)&ppszNotArray1);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    while (i < iSize1 || j < iSize2)
    {
        LONG cmp = 0;
        if (i < iSize1 && j < iSize2)
        {
            cmp = VmDirStringCompareA(ppszArray1[i], ppszArray2[j], FALSE);
        }

        if (cmp > 0 || i == iSize1)
        {
            if (ppszNotArray1)
            {
                dwError = VmDirAllocateStringA(
                        ppszArray2[j], &ppszNotArray1[x++]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            j++;
        }
        else if (cmp < 0 || j == iSize2)
        {
            if (ppszNotArray2)
            {
                dwError = VmDirAllocateStringA(
                        ppszArray1[i], &ppszNotArray2[y++]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            i++;
        }
        else
        {
            i++;
            j++;
        }
    }

cleanup:
    if (x == 0)
    {
        VmDirFreeStrArray(ppszNotArray1);
        ppszNotArray1 = NULL;
    }
    if (y == 0)
    {
        VmDirFreeStrArray(ppszNotArray2);
        ppszNotArray2 = NULL;
    }
    if (pppszNotArray1)
    {
        *pppszNotArray1 = ppszNotArray1;
    }
    if (pppszNotArray2)
    {
        *pppszNotArray2 = ppszNotArray2;
    }
    return dwError;

error:
    x = y = 0;
    goto cleanup;
}

/*
 * valid super/sub set relationship.
 * if super >= sub, return true; otherwise, return false;
 *
 * ppszSuper/ppszSub set could be NULL.
 */
BOOLEAN
VmDirIsStrArraySuperSet(
    PSTR*   ppszSuper,
    PSTR*   ppszSub
    )
{
    BOOLEAN bRtn = FALSE;
    PSTR*   ppszNotSuper = NULL;

    VmDirGetStrArrayDiffs(ppszSuper, ppszSub, &ppszNotSuper, NULL);
    bRtn = ppszNotSuper == NULL;

    VmDirFreeStrArray(ppszNotSuper);
    return bRtn;
}

/*
 * ppszArray could be NULL
 */
BOOLEAN
VmDirIsStrArrayIdentical(
    PSTR*   ppszArray1,
    PSTR*   ppszArray2
    )
{
    BOOLEAN bRtn = FALSE;
    PSTR*   ppszNotArray1 = NULL;
    PSTR*   ppszNotArray2 = NULL;

    VmDirGetStrArrayDiffs(ppszArray1, ppszArray2,
            &ppszNotArray1, &ppszNotArray2);
    bRtn = ppszNotArray1 == NULL && ppszNotArray2 == NULL;

    VmDirFreeStrArray(ppszNotArray1);
    VmDirFreeStrArray(ppszNotArray2);
    return bRtn;
}

VOID
VmDirFreeStrArray(
    PSTR*   ppszArray
    )
{
    int i = 0;

    if (ppszArray)
    {
        for (i = 0; ppszArray[i]; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArray[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszArray);
    }
}

/*
 * when hash map does not own key and value pair.
 */
VOID
VmDirNoopHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    )
{
    return;
}

/*
 * when hash map can use simple free function for both key and value.
 */
VOID
VmDirSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
}

/*
 * remove heading/trailing spaces
 * compact consecutive spaces into a single one
 */
VOID
VmdDirNormalizeString(
    PSTR    pszString
    )
{
    size_t  iSize = 0;
    size_t  iCnt = 0;
    size_t  iLast = 0;

    if (pszString)
    {
        iSize = VmDirStringLenA(pszString);

        for (iCnt = 0, iLast = 0; iCnt < iSize; iCnt++)
        {
            if (pszString[iCnt] == ' ' &&
                    (iCnt == 0 || pszString[iCnt-1] == ' '))
            {   // skip leading/trailing spaces and multiple spaces
                continue;
            }

            pszString[iLast] = pszString[iCnt];
            iLast++;
        }

        // handle last space if exists
        if (iLast > 0 && pszString[ iLast - 1 ] == ' ')
        {
            pszString[iLast - 1] = '\0';
        }
        else
        {
            pszString[iLast] = '\0';
        }
    }

    return;
}

#ifdef _WIN32

static
DWORD
_VmDirGetGenericPath(
    PCSTR   pszName,
    PSTR*   ppszPath
)
{
    DWORD   dwError = 0;
    CHAR    pbuf[MAX_PATH+1] = {0};
    PSTR    pszPath = NULL;

    if (pszName==NULL || ppszPath== NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetRegKeyValue(
                VMDIR_CONFIG_SOFTWARE_KEY_PATH,
                pszName,
                pbuf,
                MAX_PATH );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pbuf, &pszPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszPath = pszPath;

cleanup:
    return dwError;
error:
    VMDIR_SAFE_FREE_MEMORY(pszPath);
    goto cleanup;
}

DWORD
VmDirGetCfgPath(
    PSTR*   ppszCfgPath
    )
{
    return _VmDirGetGenericPath(VMDIR_REG_KEY_CONFIG_PATH,ppszCfgPath);
}

#endif

DWORD
VmDirGetSingleAttributeFromEntry(
    LDAP*        pLd,
    LDAPMessage* pEntry,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
)
{
    DWORD   dwError = 0;
    struct berval** ppValues = NULL;
    PSTR   pszOut = NULL;

    if( !pLd || !pEntry || !pszAttribute )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(
                                pLd,
                                pEntry,
                                pszAttribute);

    // only single value is expected
    if (ldap_count_values_len(ppValues) > 1)
    {

        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);

    }
    else if (ppValues && ppValues[0])
    {
        dwError = VmDirAllocateMemory(
                        sizeof(CHAR) * ppValues[0]->bv_len + 1,
                        (PVOID)&pszOut);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringNCpyA(
                    pszOut,
                    ppValues[0]->bv_len + 1,
                    ppValues[0]->bv_val,
                    ppValues[0]->bv_len);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!bOptional)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszOut = pszOut;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }
    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszOut);
    if (ppszOut)
    {
        *ppszOut = NULL;
    }
    goto cleanup;
}

/*
 * assume pszDN : cn=xxx,.....
 * return *ppszCN=xxx
 */
DWORD
VmDirDnLastRDNToCn(
    PCSTR   pszDN,
    PSTR*   ppszCN
    )
{
    DWORD   dwError = 0;
    PSTR    pszCN = NULL;
    int     i = 0;
    DWORD   offset = strlen("cn=");

    while (pszDN[i] !=',' && pszDN[i] != '\0')
    {
        i++;
    }

    dwError = VmDirAllocateMemory(i+1, (VOID*)&pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(pszCN, i, pszDN+offset, i-offset);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszCN = pszCN;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszCN);
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s, (%s) failed with error code (%u)",
                     __FUNCTION__,
                     VDIR_SAFE_STRING(pszDN),
                     dwError );
    goto cleanup;
}

DWORD
VmDirGetDCDNList(
    LDAP* pLd,
    PCSTR pszDomainDN,
    PVMDIR_STRING_LIST*  ppDCList
    )
{
    DWORD               dwError = 0;
    PCSTR               ppszAttrs[2] = {ATTR_DN, NULL};
    LDAPMessage*        pResult = NULL;
    PSTR                pszAttrVal = NULL;
    PSTR                pszDCDN = NULL;
    PVMDIR_STRING_LIST  pDCList = NULL;
    LDAPMessage*        pEntry = NULL;


    if (pLd == NULL || pszDomainDN == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pDCList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirAllocateStringPrintf(&pszDCDN,
                                            "%s=%s,%s",
                                            ATTR_OU,
                                            VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pLd,
                pszDCDN,
                LDAP_SCOPE_ONELEVEL,
                NULL,
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        for (pEntry = ldap_first_entry(pLd, pResult);
             pEntry != NULL;
             pEntry = ldap_next_entry(pLd,pEntry))
        {
            dwError = VmDirGetSingleAttributeFromEntry(pLd,
                                                       pEntry,
                                                       ATTR_DN,
                                                       FALSE,
                                                       &pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pDCList, pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszAttrVal = NULL;

        }
    }

    *ppDCList = pDCList;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCDN);
    VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VmDirStringListFree(pDCList);

    goto cleanup;
}

DWORD
VmDirStrToNameAndNumber(
    PCSTR   pszStr,
    CHAR    del,
    PSTR*   ppszName,
    USN*    pUSN
    )
{
    DWORD   dwError = 0;
    PCSTR   pszTmp = NULL;
    PSTR    pszName = NULL;
    USN     localUSN = 0;

    pszTmp = VmDirStringChrA(pszStr, del);

    if (pszTmp)
    {
        dwError = VmDirAllocateStringOfLenA( pszStr, (DWORD)(pszTmp-pszStr), &pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        localUSN = atol( pszTmp+1 );
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszName = pszName;
    *pUSN = localUSN;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszName);
    goto cleanup;
}

DWORD
VmDirUTDVectorToStruct(
    PCSTR   pszStr,
    PVMDIR_REPL_UTDVECTOR*  ppVector
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_STRING_LIST      pStrList = NULL;
    PVMDIR_REPL_UTDVECTOR   pVector = NULL;
    PVMDIR_REPL_UTDVECTOR   pTmpVector = NULL;
    PCSTR                   pDelimiter = ",";
    // No UTD Vector.
    if (VmDirStringCompareA(pszStr, "Unknown", FALSE) == 0)
    {
        *ppVector = pVector;
        goto cleanup;
    }

    dwError = VmDirStringToTokenList(pszStr, pDelimiter, &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < pStrList->dwCount; dwCnt++)
    {
        if (pStrList->pStringList[dwCnt][0] == '\0')
        {
            continue;
        }

        dwError = VmDirAllocateMemory(sizeof(*pTmpVector), (PVOID*)&pTmpVector);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTmpVector->next = pVector;
        pVector          = pTmpVector;
        pTmpVector       = NULL;

        dwError = VmDirStrToNameAndNumber( pStrList->pStringList[dwCnt], ':',
                                            &pVector->pszPartnerInvocationId,
                                            &pVector->maxOriginatingUSN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppVector = pVector;

cleanup:
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VmDirFreeReplVector(pVector);
    goto cleanup;
}


VOID
VmDirFreeReplVector(
    PVMDIR_REPL_UTDVECTOR  pVector
    )
{
    while (pVector)
    {
        PVMDIR_REPL_UTDVECTOR pNext = pVector->next;

        VMDIR_SAFE_FREE_MEMORY(pVector->pszPartnerInvocationId);
        VMDIR_SAFE_FREE_MEMORY(pVector);
        pVector = pNext;
    }

    return;
}

uint64_t
VmDirGetTimeInMilliSec(
    VOID
    )
{
    uint64_t            iTimeInMSec = 0;

#ifdef _WIN32

    FILETIME        currentFileTime = {0};
    ULARGE_INTEGER  currentTime = {0};

    GetSystemTimeAsFileTime(&currentFileTime);

    currentTime.LowPart  = currentFileTime.dwLowDateTime;
    currentTime.HighPart = currentFileTime.dwHighDateTime;

    /*
     * FILETIME.QuadPart represents the number of 100-nanosecond intervals
     * since January 1, 1601 (UTC).
     *
     * First, convert it to the number of 100-nanosecond intervals since
     * January 1, 1970 (UTC).
     *
     * Then, convert it to milliseconds since January 1, 1970 (UTC).
     */
    currentTime.QuadPart -= WIN_EPOCH;
    iTimeInMSec = (currentTime.QuadPart * 100) / NSECS_PER_MSEC;

#elif !defined(__APPLE__)

    struct timespec     timeValue = {0};

    if (clock_gettime(CLOCK_REALTIME, &timeValue) == 0)
    {
        iTimeInMSec = timeValue.tv_sec * MSECS_PER_SEC + timeValue.tv_nsec / NSECS_PER_MSEC;
    }

#endif

    return  iTimeInMSec;
}

/*
 * Compares two version strings. If one has more dots than the other,
 * it will compare up to the end of shorter one.
 *
 * For example:
 * 6.5 vs 6.6 => -1
 * 6.6 vs 6.6 => 0
 * 6.7 vs 6.6 => 1
 * 6.6 vs 6.6.1 => 0
 * 6.6.1 vs 6.6.1 => 0
 * 6.6.3 vs 6.6.1 => 1
 */
int
VmDirCompareVersion(
    PSTR    pszVerA,
    PSTR    pszVerB
    )
{
    char    bufA[128] = {0};
    char    bufB[128] = {0};
    PSTR    pszTokA = NULL;
    PSTR    pszTokB = NULL;
    int     iTokA = 0;
    int     iTokB = 0;
    char*   saveA = NULL;
    char*   saveB = NULL;

    if (IsNullOrEmptyString(pszVerA) && IsNullOrEmptyString(pszVerB))
    {
        return 0;
    }
    else if (IsNullOrEmptyString(pszVerA))
    {
        return -1;
    }
    else if (IsNullOrEmptyString(pszVerB))
    {
        return 1;
    }

    VmDirStringCpyA(bufA, 128, pszVerA);
    VmDirStringCpyA(bufB, 128, pszVerB);

    pszTokA = VmDirStringTokA(bufA, ".", &saveA);
    pszTokB = VmDirStringTokA(bufB, ".", &saveB);

    while (pszTokA && pszTokB)
    {
        iTokA = VmDirStringToIA(pszTokA);
        iTokB = VmDirStringToIA(pszTokB);

        if (iTokA < iTokB)
        {
            return -1;
        }
        else if (iTokA > iTokB)
        {
            return 1;
        }

        pszTokA = VmDirStringTokA(NULL, ".", &saveA);
        pszTokB = VmDirStringTokA(NULL, ".", &saveB);
    }

    return 0;
}

/**
 * Get maximum domain functional level for given version.
 * If not known for version, return the default (lowest) dfl.
 *
 * The mapping is succesful when the given version begins with a version
 * in the table. Versions in the table are typically in the form of "major.minor"
 * as DFL should avoid being changed except for major and minor releases.
 *
 * Example 1: pszVersion of "7.0.1-a" will match "7.0" and return a DFL of '3'
 * Example 2: pszVersion of "7" will not match any DFL version and return '1'
 * Example 3: pszVersion of "6.5" will match "6.5" and return a DFL of '2'
 */
DWORD
VmDirMapVersionToMaxDFL(
    PCSTR	pszVersion,
    PDWORD	pdwDFL
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    BOOLEAN matched = FALSE;
    VMDIR_DFL_VERSION_MAP dflVersionTable[] = VMDIR_DFL_VERSION_INITIALIZER;
    DWORD dwTableSize = sizeof(dflVersionTable)/sizeof(dflVersionTable[0]);

    if ( !pdwDFL)
    {
	dwError = ERROR_INVALID_PARAMETER;
	BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Search table
    for(i = 0; i < dwTableSize; i++)
    {
	if (VmDirStringNCompareA(
                        pszVersion,
                        dflVersionTable[i].version,
                        VmDirStringLenA(dflVersionTable[i].version),
                        FALSE) == 0)
        {
            *pdwDFL = dflVersionTable[i].dfl;
            matched = TRUE;
            break;
        }
    }

    if (!matched)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "DFL not found for version %s, default to %d",
                          pszVersion, VMDIR_DFL_DEFAULT);
	*pdwDFL = VMDIR_DFL_DEFAULT;
    }

cleanup:
    return dwError;

error:
    goto cleanup;

}

DWORD
VmDirGetDomainFuncLvlInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PDWORD pdwFuncLvl
    )
{
    DWORD dwError = 0;
    DWORD dwFuncLvl = 0;
    LDAPMessage *pResult = NULL;
    PCSTR  pszAttrFuncLvl = ATTR_DOMAIN_FUNCTIONAL_LEVEL;
    PCSTR  ppszAttrs[] = { pszAttrFuncLvl, NULL };
    PCSTR  pszFilter = "objectclass=*";
    PSTR pszDomainDN = NULL;
    struct berval** ppValues = NULL;

    if (!pLd || !pszDomain || !pdwFuncLvl)
    {
	dwError = ERROR_INVALID_PARAMETER;
	BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Get the domain DN from the domain name.
    dwError = VmDirSrvCreateDomainDN(
                  pszDomain,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
		pLd,
		pszDomainDN,
		LDAP_SCOPE_BASE,
		pszFilter,
		(PSTR*)ppszAttrs,
		FALSE, /* get values also */
		NULL,  /* server controls */
		NULL,  /* client controls */
		NULL,  /* timeout         */
		0,     /* size limit      */
		&pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
	LDAPMessage *pEntry = ldap_first_entry(pLd, pResult);

	if (pEntry)
	{
	    ppValues = ldap_get_values_len(pLd,
					   pEntry,
					   pszAttrFuncLvl);

	    if (ppValues && ldap_count_values_len(ppValues) > 0)
	    {

		dwFuncLvl = atoi(ppValues[0]->bv_val);

	    }
	}
    }

    *pdwFuncLvl = dwFuncLvl;

cleanup:

    if (ppValues)
    {
	ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
	ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGetServerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszServerDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszServerDN = NULL;

    if (IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppszServerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszServerDN,
                    "CN=%s,OU=%s,%s",
                    pszMachineName,
                    VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszServerDN = pszServerDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    if (ppszServerDN)
    {
        *ppszServerDN = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszServerDN);

    goto cleanup;
}

/*
 * query single attribute value of a DN via ldap
 * "*ppByte" is NULL terminated.
 */
DWORD
VmDirLdapGetSingleAttribute(
    LDAP*   pLD,
    PCSTR   pszDN,
    PCSTR   pszAttr,
    PBYTE*  ppByte,
    DWORD*  pdwLen
    )
{
    DWORD           dwError=0;
    PBYTE           pLocalByte = NULL;
    BerValue**      ppBerValues = NULL;

    if ( !pLD || !pszDN || !pszAttr || !ppByte || !pdwLen )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapGetAttributeValues(
                                        pLD,
                                        pszDN,
                                        pszAttr,
                                        NULL,
                                        &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( ppBerValues[0] == NULL || ppBerValues[0]->bv_val == NULL )
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ppBerValues[1] != NULL )   // more than one attribute value
    {
        dwError = VMDIR_ERROR_INVALID_RESULT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateAndCopyMemory(
                            ppBerValues[0]->bv_val,
                            ppBerValues[0]->bv_len + 1,
                            (PVOID*)&pLocalByte);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppByte = pLocalByte;
    *pdwLen = (DWORD)ppBerValues[0]->bv_len;
    pLocalByte = NULL;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirLdapGetAttributeValues(
    LDAP* pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    LDAPControl** ppSrvCtrls,
    BerValue*** pppBerValues
    )
{
    DWORD           dwError = 0;
    PCSTR           ppszAttrs[] = {pszAttribute, NULL};
    LDAPMessage*    pSearchRes = NULL;
    LDAPMessage*    pEntry = NULL;
    BerValue**      ppBerValues = NULL;

    dwError = ldap_search_ext_s(
                            pLd,
                            pszDN,
                            LDAP_SCOPE_BASE,
                            NULL,       /* filter */
                            (PSTR*)ppszAttrs,
                            FALSE,      /* attrsonly */
                            ppSrvCtrls, /* serverctrls */
                            NULL,       /* clientctrls */
                            NULL,       /* timeout */
                            0,         /* sizelimit */
                            &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
    if (!ppBerValues)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppBerValues = ppBerValues;
cleanup:
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    return dwError;
error:
    *pppBerValues = NULL;
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLdapGetAttributeValues failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirGetServerName(
    PCSTR pszHostName,
    PSTR* ppszServerName
    )
{
    DWORD       dwError = 0;
    PSTR        pszHostURI = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszServerName = NULL;
    BerValue**  ppBerValues = NULL;

    if (IsNullOrEmptyString(pszHostName) || ppszServerName == NULL)
    {
        dwError =  LDAP_INVALID_SYNTAX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirIsIPV6AddrFormat( pszHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszHostURI, "%s://[%s]:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszHostURI, "%s://%s:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAnonymousLDAPBind( &pLd, pszHostURI );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapGetAttributeValues(
                                pLd,
                                "",
                                ATTR_SERVER_NAME,
                                NULL,
                                &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDnLastRDNToCn(ppBerValues[0]->bv_val, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszServerName = pszServerName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHostURI);

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;
error:
    *ppszServerName = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetServerName failed with error (%u)", dwError);
    goto cleanup;
}
