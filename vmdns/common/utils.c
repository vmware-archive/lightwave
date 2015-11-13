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

/*
 * Module   : utils.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 */

#include "includes.h"

#define PTR_NAME_SUFFIX_IP4 ".in-addr.arpa"
#define PTR_NAME_SUFFIX_IP6 ".ip6.arpa"
#define LOW_HEX(byte) ((byte) & 0xF)
#define HIGH_HEX(byte) (((byte) & 0xF0) >> 4)

VOID
VmDnsClearZoneInfo(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszName);
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszPrimaryDnsSrvName);
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszRName);
    }
}

VOID
VmDnsClearZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (;idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsClearZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
    }
}

DWORD
VmDnsRpcCopyRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_RECORD_ARRAY),
                                     (PVOID*)&pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_RECORD)*pRecordArray->dwCount,
                                     (PVOID*)&pRecordArrayTemp->Records);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pRecordArray->dwCount; ++idx)
    {
        dwError = VmDnsRpcCopyRecord(&pRecordArray->Records[idx],
                                    &pRecordArrayTemp->Records[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pRecordArrayTemp->dwCount = pRecordArray->dwCount;

    *ppRecordArray = pRecordArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeRecordArray(pRecordArrayTemp);
    if (ppRecordArray)
    {
        *ppRecordArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcFreeRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    DWORD idx = 0;
    if (pRecordArray)
    {
        for (; idx < pRecordArray->dwCount; ++idx)
        {
            VmDnsRpcClearRecord(&pRecordArray->Records[idx]);
        }
        VmDnsRpcFreeMemory(pRecordArray);
    }
}

DWORD
VmDnsRpcCopyZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfoSrc,
    PVMDNS_ZONE_INFO pZoneInfoDest
    )
{
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfoSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfoDest, dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszName,
                                    &pZoneInfoDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszPrimaryDnsSrvName,
                                    &pZoneInfoDest->pszPrimaryDnsSrvName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszRName,
                                    &pZoneInfoDest->pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneInfoDest->dwFlags = pZoneInfoSrc->dwFlags;
    pZoneInfoDest->dwZoneType = pZoneInfoSrc->dwZoneType;
    pZoneInfoDest->expire = pZoneInfoSrc->expire;
    pZoneInfoDest->minimum = pZoneInfoSrc->minimum;
    pZoneInfoDest->refreshInterval = pZoneInfoSrc->refreshInterval;
    pZoneInfoDest->retryInterval = pZoneInfoSrc->retryInterval;
    pZoneInfoDest->serial = pZoneInfoSrc->serial;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeMemory(pZoneInfoDest->pszName);
    VmDnsRpcFreeMemory(pZoneInfoDest->pszPrimaryDnsSrvName);
    VmDnsRpcFreeMemory(pZoneInfoDest->pszRName);
    VmDnsRpcFreeMemory(pZoneInfoDest);
    goto cleanup;
}

DWORD
VmDnsRpcCopyZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArrayTemp = NULL;
    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_ZONE_INFO_ARRAY),
                                     (PVOID*)&pZoneInfoArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_ZONE_INFO)*pZoneInfoArray->dwCount,
                                     (PVOID*)&pZoneInfoArrayTemp->ZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pZoneInfoArray->dwCount; ++idx)
    {
        dwError = VmDnsRpcCopyZoneInfo(&pZoneInfoArray->ZoneInfos[idx],
                                        &pZoneInfoArrayTemp->ZoneInfos[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pZoneInfoArrayTemp->dwCount = pZoneInfoArray->dwCount;

    *ppZoneInfoArray = pZoneInfoArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeZoneInfoArray(pZoneInfoArrayTemp);
    if (*ppZoneInfoArray)
    {
        *ppZoneInfoArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcFreeZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VmDnsRpcFreeMemory(pZoneInfo->pszName);
        VmDnsRpcFreeMemory(pZoneInfo->pszPrimaryDnsSrvName);
        VmDnsRpcFreeMemory(pZoneInfo->pszRName);
        VmDnsRpcFreeMemory(pZoneInfo);
    }
}

VOID
VmDnsRpcFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (; idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsRpcFreeZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
        VmDnsFreeMemory(pZoneInfoArray);
    }
}

VOID
VmDnsTrimDomainNameSuffix(
    PSTR    pszName,
    PCSTR   pcszDomainName
    )
{
    PSTR pszTail = NULL;

    if (IsNullOrEmptyString(pszName) || IsNullOrEmptyString(pcszDomainName))
    {
        return;
    }

    if (VmDnsStringLenA(pszName) <= VmDnsStringLenA(pcszDomainName))
    {
        return;
    }

    if (!VmDnsAllocateStringPrintfA(&pszTail, ".%s.", pcszDomainName))
    {
        VmDnsStringTrimA(pszName, pszTail, FALSE);
        VMDNS_SAFE_FREE_STRINGA(pszTail);
    }

    if (!VmDnsAllocateStringPrintfA(&pszTail, ".%s", pcszDomainName))
    {
        VmDnsStringTrimA(pszName, pszTail, FALSE);
        VMDNS_SAFE_FREE_STRINGA(pszTail);
    }
}

DWORD
VmDnsGenerateReversZoneNameFromNetworkId(
    PCSTR pszNetworkId,
    PSTR* ppszZone
    )
{
    DWORD dwError = 0;
    PSTR  pszPtrName = NULL;
    PSTR  pszZone = NULL;
    PCHAR pLength = NULL;
    int   length = 0;
    int   family = AF_INET;

    BAIL_ON_VMDNS_INVALID_POINTER(pszNetworkId, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszZone, dwError);

    pLength = VmDnsStringChrA(pszNetworkId, '/');
    if (!pLength || !*(pLength + 1))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pLength = '\0';
    ++pLength;

    length = atoi(pLength);

    dwError = VmDnsGenerateRtrNameFromIp(pszNetworkId, &family, &pszPtrName);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (family != AF_INET && family != AF_INET6)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (family == AF_INET)
    {
        if (length != 8 && length != 16 && length != 24)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        pszZone = pszPtrName;
        while (length < 32 && *pszZone)
        {
            pszZone = VmDnsStringChrA(pszZone, '.');
            if (!pszZone)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            ++pszZone;
            length += 8;
        }
        if (!*pszZone)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
    else
    {
        if (length >= 128 || length <= 0)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        pszZone = pszPtrName + 64 - length/2;
    }

    dwError = VmDnsAllocateStringA(pszZone, ppszZone);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsGenerateRtrNameFromIp(
    PCSTR pszIPAddress,
    int*  pnFamily,
    PSTR* ppszPtrName
    )
{
    DWORD dwError = 0;
    DWORD dwAddr = 0;
    PSTR pszPtrName = NULL;
    struct addrinfo *pAddrInfo = NULL;
    BYTE* pByte = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pszIPAddress, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszPtrName, dwError);

    if (getaddrinfo(pszIPAddress, NULL, NULL, &pAddrInfo))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pAddrInfo->ai_family == AF_INET)
    {
        dwAddr = (DWORD)((struct sockaddr_in *)pAddrInfo->ai_addr)->sin_addr.s_addr;

        // See RFC 1035 for name format
        // In short, record name is octets in reverse order appened with "in-addr.arpa".
        // Example: 11.1.193.128.in-addr.arpa
        dwError = VmDnsAllocateStringPrintfA(
                    &pszPtrName,
                    "%d.%d.%d.%d%s",
                    (dwAddr & 0xFF000000) >> 24,
                    (dwAddr & 0xFF0000) >> 16,
                    (dwAddr & 0xFF00) >> 8,
                    (dwAddr & 0xFF),
                    PTR_NAME_SUFFIX_IP4
                    );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (pAddrInfo->ai_family == AF_INET6)
    {
        pByte = ((struct sockaddr_in6 *)pAddrInfo->ai_addr)->sin6_addr.s6_addr;
        // See RFC 1886 for ipv6 ptr name format
        // In short, record name is address presented in nibbles separated by dots,
        // in reverse order appened with "ip6.arpa".
        // Example: 4.1.2.2.0.3.e.f.f.f.6.5.0.5.2.0.7.9.0.0.8.1.1.0.0.1.0.0.0.0.c.f.ip6.arpa
        dwError = VmDnsAllocateStringPrintfA(
                    &pszPtrName,
                    "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x."
                    "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x"
                    "%s",
                    LOW_HEX(pByte[15]), HIGH_HEX(pByte[15]),
                    LOW_HEX(pByte[14]), HIGH_HEX(pByte[14]),
                    LOW_HEX(pByte[13]), HIGH_HEX(pByte[13]),
                    LOW_HEX(pByte[12]), HIGH_HEX(pByte[12]),
                    LOW_HEX(pByte[11]), HIGH_HEX(pByte[11]),
                    LOW_HEX(pByte[10]), HIGH_HEX(pByte[10]),
                    LOW_HEX(pByte[9]),  HIGH_HEX(pByte[9]),
                    LOW_HEX(pByte[8]),  HIGH_HEX(pByte[8]),
                    LOW_HEX(pByte[7]),  HIGH_HEX(pByte[7]),
                    LOW_HEX(pByte[6]),  HIGH_HEX(pByte[6]),
                    LOW_HEX(pByte[5]),  HIGH_HEX(pByte[5]),
                    LOW_HEX(pByte[4]),  HIGH_HEX(pByte[4]),
                    LOW_HEX(pByte[3]),  HIGH_HEX(pByte[3]),
                    LOW_HEX(pByte[2]),  HIGH_HEX(pByte[2]),
                    LOW_HEX(pByte[1]),  HIGH_HEX(pByte[1]),
                    LOW_HEX(pByte[0]),  HIGH_HEX(pByte[0]),
                    PTR_NAME_SUFFIX_IP6
                    );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *ppszPtrName = pszPtrName;
    if (pnFamily)
    {
        *pnFamily = pAddrInfo->ai_family;
    }

cleanup:
    if (pAddrInfo)
    {
        freeaddrinfo(pAddrInfo);
    }
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pszPtrName);
    goto cleanup;
}

VOID
VmDnsStringTrimA(
    PSTR pszStr,
    PCSTR pszSearch,
    BOOLEAN bCaseSensitive
    )
{
    ULONG ulStrLength = 0, ulSearchLength = 0;
    PSTR pszTail = NULL;

    if (!IsNullOrEmptyString(pszStr) &&
        !IsNullOrEmptyString(pszSearch))
    {
        ulStrLength = VmDnsStringLenA(pszStr);
        ulSearchLength = VmDnsStringLenA(pszSearch);

        if (ulStrLength >= ulSearchLength)
        {
            pszTail = pszStr + ulStrLength - ulSearchLength;
            if (VmDnsStringCompareA(
                                pszTail,
                                pszSearch,
                                bCaseSensitive) == 0)
            {
                *pszTail = '\0';
            }
        }
    }
}

BOOLEAN
VmDnsCheckIfIPV4AddressA(
    PCSTR pszNetworkAddress
    )
{
    BOOLEAN bResult = FALSE;

    if (!IsNullOrEmptyString(pszNetworkAddress))
    {
        unsigned char buf[sizeof(struct in_addr)];

        bResult = (inet_pton(AF_INET, pszNetworkAddress, &buf[0]) == 1);
    }

    return bResult;
}

BOOLEAN
VmDnsCheckIfIPV6AddressA(
    PCSTR pszNetworkAddress
    )
{
    BOOLEAN bResult = FALSE;

    if (!IsNullOrEmptyString(pszNetworkAddress))
    {
        unsigned char buf[sizeof(struct in6_addr)];

        bResult = (inet_pton(AF_INET6, pszNetworkAddress, &buf[0]) == 1);
    }

    return bResult;
}
