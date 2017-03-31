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
VmAfSrvGetIPAddresses(
    VMDNS_IP4_ADDRESS* ppV4Addresses,
    PDWORD             pdwNumV4Address,
    VMDNS_IP6_ADDRESS* ppV6Addresses,
    PDWORD             pdwNumV6Address
    );

static
DWORD
VmAfSrvGetLotusServerName(
    PCSTR   pszServerName,
    PSTR*   ppOutServerName
    );

static
DWORD
VmAfdAppendDomain(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PSTR*   pszServerFQDN
    );

DWORD
VmAfSrvConfigureDNSW(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    dwError = VmAfdAllocateStringAFromW(
                  pwszServerName,
                  &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszDomainName,
                  &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszUserName,
                  &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszPassword,
                  &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvConfigureDNSA(
                  pszServerName,
                  pszDomainName,
                  pszUserName,
                  pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    VMAFD_SAFE_FREE_MEMORY(pszServerName);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);

    return dwError;
}

DWORD
VmAfSrvConfigureDNSA(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszCanonicalServerName = NULL;
    PVMDNS_SERVER_CONTEXT   pServerContext = NULL;
    VMDNS_IP4_ADDRESS*      pV4Addresses = NULL;
    DWORD                   numV4Address = 0;
    PVMDNS_IP6_ADDRESS      pV6Addresses = NULL;
    DWORD                   numV6Address = 0;
    VMDNS_INIT_INFO         initInfo = {0};
    CHAR                    szDomainFQDN[260] = {0};
    PSTR                    pszServerFQDN = NULL;

    if (IsNullOrEmptyString(pszDomainName) || strlen(pszDomainName) > 255)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetIPAddressesWrap(
                  &pV4Addresses,
                  &numV4Address,
                  &pV6Addresses,
                  &numV6Address);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetLotusServerName(
                    pszServerName,
                    &pszCanonicalServerName);

    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringCpyA(szDomainFQDN, 260, pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (szDomainFQDN[strlen(szDomainFQDN) - 1] != '.')
    {
        szDomainFQDN[strlen(szDomainFQDN)] = '.';
        szDomainFQDN[strlen(szDomainFQDN)+1] = 0;
    }

    if (!VmAfdCheckIfIPV4AddressA(pszServerName) &&
        !VmAfdCheckIfIPV6AddressA(pszServerName))
    {
        dwError = VmAfdAppendDomain(pszServerName, pszDomainName, &pszServerFQDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s Server name for dns initialize: %s",
            __FUNCTION__,
           pszCanonicalServerName);
    }
    else
    {
       VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s",pszServerName);
    }
    initInfo.IpV4Addrs.Addrs = pV4Addresses;
    initInfo.IpV4Addrs.dwCount = numV4Address;
    initInfo.IpV6Addrs.Addrs = pV6Addresses;
    initInfo.IpV6Addrs.dwCount = numV6Address;
    initInfo.pszDcSrvName = pszServerFQDN;
    initInfo.pszDomain = szDomainFQDN;
    initInfo.wPort = VMDNS_DEFAULT_LDAP_PORT;

    dwError = VmDnsOpenServerA(
                "localhost", /* always promote with local DNS */
                pszUserName,
                pszDomainName,
                pszPassword,
                0,
                NULL,
                &pServerContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDnsInitializeA(pServerContext, &initInfo);
    VmAfdLog( VMAFD_DEBUG_ERROR, "%s DnsInitialize : Error:%d,ServerName : %s", __FUNCTION__,dwError,pszServerFQDN);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }
    VMAFD_SAFE_FREE_MEMORY(pV4Addresses);
    VMAFD_SAFE_FREE_MEMORY(pV6Addresses);
    VMAFD_SAFE_FREE_STRINGA(pszCanonicalServerName);

    return dwError;

error:

    VmAfdLog(
        VMAFD_DEBUG_ANY,
        "Failed to initialize DNS. Error(%u)",
        dwError);
    goto cleanup;
}

DWORD
VmAfSrvUnconfigureDNSW(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    dwError = VmAfdAllocateStringAFromW(
                  pwszServerName,
                  &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszDomainName,
                  &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszUserName,
                  &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszPassword,
                  &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvUnconfigureDNSA(
                  pszServerName,
                  pszDomainName,
                  pszUserName,
                  pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    VMAFD_SAFE_FREE_MEMORY(pszServerName);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);

    return dwError;
}

DWORD
VmAfSrvUnconfigureDNSA(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszCanonicalServerName = NULL;
    PVMDNS_SERVER_CONTEXT   pServerContext = NULL;
    VMDNS_IP4_ADDRESS*      pV4Addresses = NULL;
    DWORD                   numV4Address = 0;
    PVMDNS_IP6_ADDRESS      pV6Addresses = NULL;
    DWORD                   numV6Address = 0;
    VMDNS_INIT_INFO         initInfo = {0};

    dwError = VmAfSrvGetIPAddressesWrap(
                  &pV4Addresses,
                  &numV4Address,
                  &pV6Addresses,
                  &numV6Address);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetLotusServerName(
                    pszServerName,
                    &pszCanonicalServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    initInfo.IpV4Addrs.Addrs = pV4Addresses;
    initInfo.IpV4Addrs.dwCount = numV4Address;
    initInfo.IpV6Addrs.Addrs = pV6Addresses;
    initInfo.IpV6Addrs.dwCount = numV6Address;
    initInfo.pszDcSrvName = pszCanonicalServerName;
    initInfo.pszDomain = (PSTR)pszDomainName;
    initInfo.wPort = VMDNS_DEFAULT_LDAP_PORT;

    dwError = VmDnsOpenServerA(
                "localhost", /* always promote with local DNS */
                pszUserName,
                pszDomainName,
                pszPassword,
                0,
                NULL,
                &pServerContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDnsUninitializeA(pServerContext, &initInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }
    VMAFD_SAFE_FREE_MEMORY(pV4Addresses);
    VMAFD_SAFE_FREE_MEMORY(pV6Addresses);
    VMAFD_SAFE_FREE_STRINGA(pszCanonicalServerName);

    return dwError;

error:

    VmAfdLog(
        VMAFD_DEBUG_ANY,
        "Failed to Uninitialize DNS. Error(%u)",
        dwError);
    goto cleanup;
}

DWORD
VmAfSrvGetIPAddressesWrap(
    VMDNS_IP4_ADDRESS** ppV4Addresses,
    PDWORD              pdwNumV4Address,
    VMDNS_IP6_ADDRESS** ppV6Addresses,
    PDWORD              pdwNumV6Address
    )
{
    DWORD                   dwError = 0;
    VMDNS_IP4_ADDRESS*      pV4Addresses = NULL;
    DWORD                   numV4Address = 0;
    PVMDNS_IP6_ADDRESS      pV6Addresses = NULL;
    DWORD                   numV6Address = 0;

    dwError = VmAfSrvGetIPAddresses(NULL, &numV4Address, NULL, &numV6Address);
    if (dwError == ERROR_INSUFFICIENT_BUFFER)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (numV4Address)
    {
        dwError = VmAfdAllocateMemory(
                        sizeof(VMDNS_IP4_ADDRESS)*numV4Address,
                        (PVOID*)&pV4Addresses);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (numV6Address)
    {
        dwError = VmAfdAllocateMemory(
                        sizeof(VMDNS_IP6_ADDRESS)*numV6Address,
                        (PVOID*)&pV6Addresses);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pV4Addresses || pV6Addresses)
    {
        dwError = VmAfSrvGetIPAddresses(
                    pV4Addresses,
                    &numV4Address,
                    pV6Addresses,
                    &numV6Address);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppV4Addresses = pV4Addresses;
    *pdwNumV4Address = numV4Address;
    *ppV6Addresses = pV6Addresses;
    *pdwNumV6Address = numV6Address;

cleanup:

    return dwError;

error:

    *ppV4Addresses = NULL;
    *pdwNumV4Address = 0;
    *ppV6Addresses = NULL;
    *pdwNumV6Address = 0;

    VMAFD_SAFE_FREE_MEMORY(pV4Addresses);
    VMAFD_SAFE_FREE_MEMORY(pV6Addresses);

    goto cleanup;
}

static
DWORD
VmAfSrvGetIPAddresses(
    VMDNS_IP4_ADDRESS*  pIpV4Addrs,
    PDWORD              pdwV4Addrs,
    VMDNS_IP6_ADDRESS*  pIpV6Addrs,
    PDWORD              pdwV6Addrs
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumV4Addrs = 0;
    DWORD   dwNumV6Addrs = 0;
    struct sockaddr_in  *pIp4Addr = NULL;
    struct sockaddr_in6 *pIp6Addr = NULL;

#ifndef _WIN32
    struct ifaddrs *    addrList = NULL;
    struct ifaddrs *    ifa = NULL;
#else
    PADDRINFOA          addrList = NULL;
    PADDRINFOA          ifa = NULL;
    unsigned long       loopbackAddr = 0;
    struct addrinfo     hints = {0};
    BYTE                loopbackAddr6[16] =
                            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
#endif

    if (!pdwV4Addrs || !pdwV6Addrs)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    dwError = getifaddrs(&addrList);
#else
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    loopbackAddr = inet_addr("127.0.0.1");

    if (getaddrinfo( "", NULL, &hints, &addrList ) != 0 )
    {
        dwError = WSAGetLastError();
    }
#endif
    BAIL_ON_VMAFD_ERROR(dwError);

    for (ifa = addrList; ifa != NULL; ifa = VMAFD_ADDR_INFO_NEXT(ifa))
    {
        if ((VMAFD_ADDR_INFO_ADDR(ifa) == NULL)
#ifndef _WIN32 // because getaddrinfo() does NOT set ai_flags in the returned address info structures.
            || ((VMAFD_ADDR_INFO_FLAGS(ifa) & IFF_UP) == 0)
            || ((VMAFD_ADDR_INFO_FLAGS(ifa) & IFF_LOOPBACK) != 0)
#endif
        )
        {
            continue;
        }

        if (VMAFD_ADDR_INFO_ADDR(ifa)->sa_family == AF_INET)
        {
            pIp4Addr = (struct sockaddr_in *) VMAFD_ADDR_INFO_ADDR(ifa);
#ifdef _WIN32
            if (memcmp(&pIp4Addr->sin_addr.s_addr,
                    &loopbackAddr,
                    sizeof(loopbackAddr)) == 0)
            {
                continue;
            }
#endif
            if (IS_IPV4_LINKLOCAL((unsigned char*)&pIp4Addr->sin_addr.s_addr))
            {
                continue;
            }
            if (pIpV4Addrs)
            {
                if (dwNumV4Addrs < *pdwV4Addrs)
                {
                    pIpV4Addrs[dwNumV4Addrs] = ntohl(pIp4Addr->sin_addr.s_addr);
                }
                else
                {
                    dwError = ERROR_INSUFFICIENT_BUFFER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
            }
            ++dwNumV4Addrs;
        }
        else if (VMAFD_ADDR_INFO_ADDR(ifa)->sa_family == AF_INET6)
        {
            pIp6Addr = (struct sockaddr_in6 *) VMAFD_ADDR_INFO_ADDR(ifa);
#ifdef _WIN32
            if (memcmp(&pIp6Addr->sin6_addr.s6_addr,
                    &loopbackAddr6,
                    sizeof(loopbackAddr6)) == 0)
            {
                continue;
            }
#endif
            if (IS_IPV6_LINKLOCAL(pIp6Addr->sin6_addr.s6_addr) ||
                IS_IPV6_ULA(pIp6Addr->sin6_addr.s6_addr))
            {
                continue;
            }
            if (pIpV6Addrs)
            {
                if (dwNumV6Addrs < *pdwV6Addrs)
                {
                    memcpy(
                        pIpV6Addrs[dwNumV6Addrs].IP6Byte,
                        pIp6Addr->sin6_addr.s6_addr,
                        sizeof(pIp6Addr->sin6_addr.s6_addr));
                }
                else
                {
                    dwError = ERROR_INSUFFICIENT_BUFFER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
            }
            ++dwNumV6Addrs;
        }
    }

    *pdwV4Addrs = dwNumV4Addrs;
    *pdwV6Addrs = dwNumV6Addrs;

    if (!pIpV4Addrs && !pIpV6Addrs)
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    if (addrList)
    {
#ifndef _WIN32
        freeifaddrs(addrList);
#else
        freeaddrinfo(addrList);
#endif
    }
    return dwError;

error:
    goto cleanup;
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
static
DWORD
VmAfSrvGetLotusServerName(
    PCSTR   pszServerName,
    PSTR*   ppOutServerName
    )
{
    DWORD dwError = 0;
    PSTR  pszHostnameCanon = NULL;
    PSTR  pszFQDN = NULL;
    PWSTR pwszPNID = NULL;

    if ( !pszServerName || !ppOutServerName )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if ( VmAfdStringCompareA( pszServerName, "localhost", FALSE ) != 0 )
    {   // caller provides preferred Lotus Server Name or IP
        dwError = VmAfdAllocateStringA( pszServerName, &pszHostnameCanon );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {   // caller does NOT specify preferred Lotus Server Name, derives it ourselves.
        dwError = VmAfSrvGetPNID(&pwszPNID);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(
                         pwszPNID,
                         &pszHostnameCanon);
    }

    BAIL_ON_VMAFD_EMPTY_STRING(pszHostnameCanon, dwError);

    if (!VmAfdCheckIfIPV4AddressA(pszHostnameCanon) &&
        !VmAfdCheckIfIPV6AddressA(pszHostnameCanon) &&
        pszHostnameCanon[VmAfdStringLenA(pszHostnameCanon) - 1] != '.')
    {
        dwError = VmAfdAllocateStringPrintf(
                    &pszFQDN,
                    "%s.",
                    pszHostnameCanon);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        pszFQDN = pszHostnameCanon;
        pszHostnameCanon = NULL;
    }

    *ppOutServerName = pszFQDN;

    VmAfdLog(VMAFD_DEBUG_DEBUG, "Lotus server name: (%s)", *ppOutServerName);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszHostnameCanon);
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszFQDN);
    VmAfdLog(VMAFD_DEBUG_DEBUG, "%s failed (%s). Error(%u)",
                    __FUNCTION__, pszServerName, dwError);
    goto cleanup;
}


static
DWORD
VmAfdAppendDomain(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PSTR*   ppszServerFQDN
    )
{

    DWORD dwError = 0;
    PSTR  pszServerFQDN = NULL;
    DWORD dwServerStrLen = 0;
    DWORD dwDomainStrLen = 0;
    DWORD dwCursor = 0 ;

    if (!pszServerName || !pszDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    dwServerStrLen = strlen (pszServerName);
    dwDomainStrLen = strlen (pszDomainName);


    if (dwDomainStrLen  > dwServerStrLen)
    {
        VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s.%s.",pszServerName,pszDomainName);
    }
    else
    {
       dwCursor = dwServerStrLen - dwDomainStrLen;
       if (VmAfdStringCompareA(
                  &pszServerName[dwCursor],
                  pszDomainName,
                  FALSE) != 0)
       {
            if (pszServerName[dwServerStrLen - 1] != '.')
            {
                VmAfdAllocateStringPrintf(
                          &pszServerFQDN,
                          "%s.%s.",
                          pszServerName,
                          pszDomainName);
            }
            else
            {
                VmAfdAllocateStringPrintf(
                          &pszServerFQDN,
                          "%s%s.",
                          pszServerName,
                          pszDomainName);
            }
       }
       else
       {
           VmAfdAllocateStringPrintf(
                          &pszServerFQDN,
                          "%s.",
                          pszServerName);
       }
    }
    *ppszServerFQDN = pszServerFQDN;
cleanup:
    return dwError;

error:
    goto cleanup;

}
