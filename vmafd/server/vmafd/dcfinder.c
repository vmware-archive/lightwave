/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : dcfinder.c
 *
 */

#include "includes.h"

#ifdef _WIN32

DWORD
VmAfdGetDomainController(
    PCSTR pszDomain,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PSTR* ppszHostname,
    PSTR* ppszNetworkAddress
    )
{
    return ERROR_NOT_SUPPORTED;
}

DWORD
VmAfdGetDomainControllerList(
    PCSTR pszDomain,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList,
    PDWORD pdCount
    )
{
    return ERROR_NOT_SUPPORTED;
}

#else

static
DWORD
VmAfSrvCheckDC(
    PDNS_SERVER_INFO pServerInfo,
    PCSTR pszDomain,
    PCSTR pszUPN,
    PCSTR pszPassword,
    PSTR* ppszHostname,
    PSTR* ppszNetworkAddress
    );

static
DWORD
VmAfSrvGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    );

DWORD
VmAfdGetDomainController(
    PCSTR pszDomain,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PSTR* ppszHostname,
    PSTR* ppszNetworkAddress
    )
{
    DWORD dwError = 0;
    PDNS_SERVER_INFO pServerArray = NULL;
    DWORD dwServerCount = 0;
    PSTR  pszQuestion = NULL;
    DWORD dwDsFlags = 0;
    PSTR  pszNetworkAddress = NULL;
    PSTR  pszHostname = NULL;
    PSTR  pszUPN = NULL;
    PSTR  pszDomainDN = NULL;

    if (IsNullOrEmptyString(pszDomain) || !ppszHostname || !ppszNetworkAddress)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszQuestion,
                    "_ldap._tcp.%s",
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LWNetDnsSrvQueryByQuestion(
                    pszQuestion,
                    NULL,
                    dwDsFlags,
                    &pServerArray,
                    &dwServerCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwServerCount > 0)
    {
        DWORD iServer = 0;

        dwError = VmAfdAllocateStringPrintf(
                      &pszUPN,
                      "%s@%s",
                      pszUserName,
                      pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvGetDomainDN(pszDomain, &pszDomainDN);
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; iServer < dwServerCount; iServer++)
        {
            DWORD dwError2 = 0;
            PDNS_SERVER_INFO pServerInfo = &pServerArray[iServer];

            dwError2 = VmAfSrvCheckDC(
                            pServerInfo,
                            pszDomainDN,
                            pszUPN,
                            pszPassword,
                            &pszHostname,
                            &pszNetworkAddress);
            if (dwError2 == ERROR_SUCCESS)
            {
                break;
            }
        }
    }

    if (!pszNetworkAddress || !pszHostname)
    {
        dwError = ERROR_NO_SUCH_DOMAIN;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszHostname = pszHostname;
    *ppszNetworkAddress = pszNetworkAddress;

cleanup:

    if (pServerArray)
    {
        LWNetFreeMemory(pServerArray);
    }
    VMAFD_SAFE_FREE_MEMORY(pszQuestion);
    VMAFD_SAFE_FREE_MEMORY(pszUPN);
    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    if (ppszNetworkAddress)
    {
        *ppszNetworkAddress = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHostname);
    VMAFD_SAFE_FREE_MEMORY(pszNetworkAddress);

    goto cleanup;
}

DWORD
VmAfdGetDomainControllerList(
    PCSTR pszDomain,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList,
    PDWORD pdCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PDNS_SERVER_INFO pServerArray = NULL;
    DWORD dwServerCount = 0;
    PSTR  pszQuestion = NULL;
    DWORD dwDsFlags = 0;
    PSTR  pszUPN = NULL;
    PSTR  pszDomainDN = NULL;
    PVMAFD_DC_INFO_W pVmAfdDCEntries = NULL;

    if (IsNullOrEmptyString(pszDomain) || !ppVmAfdDCInfoList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszQuestion,
                    "_ldap._tcp.%s",
                    pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LWNetDnsSrvQueryByQuestion(
                    pszQuestion,
                    NULL,
                    dwDsFlags,
                    &pServerArray,
                    &dwServerCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwServerCount > 0 )
    {
        dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_DC_INFO_W)*dwServerCount,
                            (PVOID *)&pVmAfdDCEntries
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

       for (; dwIndex < dwServerCount; dwIndex++)
       {
           PDNS_SERVER_INFO pServerInfo = &pServerArray[dwIndex];
           PVMAFD_DC_INFO_W pDcInfo = &pVmAfdDCEntries[dwIndex];

           dwError = VmAfdAllocateStringWFromA(
                                       pServerInfo->pszName,
                                       &pDcInfo->pwszHostName
                                       );
           BAIL_ON_VMAFD_ERROR(dwError);

           dwError = VmAfdAllocateStringWFromA(
                                        pServerInfo->pszAddress,
                                        &pDcInfo->pwszAddress
                                        );
           BAIL_ON_VMAFD_ERROR(dwError);
       }
    }
    *pdCount = dwServerCount;
    *ppVmAfdDCInfoList = pVmAfdDCEntries;
cleanup:

    if (pServerArray)
    {
        LWNetFreeMemory(pServerArray);
    }
    VMAFD_SAFE_FREE_MEMORY(pszQuestion);
    VMAFD_SAFE_FREE_MEMORY(pszUPN);
    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:
   if (dwServerCount > 0 && pVmAfdDCEntries )
   {
      for (dwIndex = 0; dwIndex < dwServerCount ; dwIndex++)
      {
          PVMAFD_DC_INFO_W pDcInfo = &pVmAfdDCEntries[dwIndex];
          if (pDcInfo)
          {
              VMAFD_SAFE_FREE_MEMORY(pDcInfo->pwszHostName);
              VMAFD_SAFE_FREE_MEMORY(pDcInfo->pwszAddress);
          }
      }
   }
   VMAFD_SAFE_FREE_MEMORY(pVmAfdDCEntries);
   goto cleanup;

}
static
DWORD
VmAfSrvCheckDC(
    PDNS_SERVER_INFO pServerInfo,
    PCSTR pszDomain,
    PCSTR pszUPN,
    PCSTR pszPassword,
    PSTR* ppszHostname,
    PSTR* ppszNetworkAddress
    )
{
    DWORD dwError = 0;
    PSTR  pszHostname = NULL;
    PSTR  pszNetworkAddress = NULL;
    PSTR  pszDomainOther = NULL;
    LDAP* pLd = NULL;

    dwError = VmAfdLDAPConnect(
                    pServerInfo->pszAddress,
                    0,
                    pszUPN,
                    pszPassword,
                    &pLd);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetDefaultDomainName(pLd, &pszDomainOther);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (VmAfdStringCompareA(pszDomain, pszDomainOther, FALSE) != 0)
    {
        dwError = ERROR_DC_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pServerInfo->pszName, &pszHostname);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringA(pServerInfo->pszAddress, &pszNetworkAddress);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHostname = pszHostname;
    *ppszNetworkAddress = pszNetworkAddress;

cleanup:

    if (pLd)
    {
        VmAfdLdapClose(pLd);
    }

    VMAFD_SAFE_FREE_MEMORY(pszDomainOther);

    return dwError;

error:

    *ppszNetworkAddress = NULL;
    *ppszHostname = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszHostname);
    VMAFD_SAFE_FREE_MEMORY(pszNetworkAddress);

    goto cleanup;
}

static
DWORD
VmAfSrvGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    )
{
    DWORD dwError = 0;
    const char* pszDelim = ".";
    const char* pszPrefix = "DC=";
    size_t len_prefix = sizeof("DC=")-1;
    const char* pszCommaDelim = ",";
    size_t len_comma_delim = sizeof(",")-1;
    size_t len = 0;
    PSTR  pszDomainDN = NULL;
    PCSTR pszReadCursor = pszDomain;
    PSTR  pszWriteCursor = NULL;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (len > 0)
            {
                len += len_comma_delim;
            }
            len += len_prefix;
            len += len_name;
        }

        pszReadCursor += len_name;

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    if (!len)
    {
        dwError = ERROR_INVALID_DOMAINNAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(len+1, (PVOID*)&pszDomainDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    pszReadCursor  = pszDomain;
    pszWriteCursor = pszDomainDN;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (pszWriteCursor > pszDomainDN)
            {
                memcpy(pszWriteCursor, pszCommaDelim, len_comma_delim);
                pszWriteCursor += len_comma_delim;
            }

            memcpy(pszWriteCursor, pszPrefix, len_prefix);
            pszWriteCursor += len_prefix;

            memcpy(pszWriteCursor, pszReadCursor, len_name);

            pszReadCursor += len_name;
            pszWriteCursor += len_name;
        }

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    *ppszDomainDN = pszDomainDN;

cleanup:

    return dwError;

error:

    *ppszDomainDN = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszDomainDN);

    goto cleanup;
}

#endif /* ifndef _WIN32 */

