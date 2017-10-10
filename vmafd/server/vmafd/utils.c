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
#include "cdcclient.h"

#define PTR_NAME_SUFFIX_IP4 ".in-addr.arpa"
#define PTR_NAME_SUFFIX_IP6 ".ip6.arpa"
#define LOW_HEX(byte) ((byte) & 0xF)
#define HIGH_HEX(byte) (((byte) & 0xF0) >> 4)

VOID
VmAfdSrvSetStatus(
    VMAFD_STATUS state
    )
{
    pthread_mutex_lock(&gVmafdGlobals.mutex);

    gVmafdGlobals.status = state;
    pthread_cond_signal(&gVmafdGlobals.statusCond);

    pthread_mutex_unlock(&gVmafdGlobals.mutex);
}

VMAFD_STATUS
VmAfdSrvGetStatus(
    VOID
    )
{
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    pthread_mutex_lock(&gVmafdGlobals.mutex);

    status = gVmafdGlobals.status;

    pthread_mutex_unlock(&gVmafdGlobals.mutex);

    return status;
}

DWORD
VmAfdGetMachineInfo(
    PVMAFD_REG_ARG *ppArgs
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccountName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszAccount = NULL;
    PVMAFD_REG_ARG pArgs = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE ;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError =  VmAfdAllocateMemory(
                    sizeof(VMAFD_REG_ARG),
                    (PVOID*)&pArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszAccountDN,
                    &pArgs->pszAccountDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszPassword,
                    &pArgs->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszAccount,
                    &pArgs->pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszDomain,
                    &pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf( &(pArgs->pszAccountUPN), "%s@%s",
                                         pArgs->pszAccount, pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pArgs->pszAccountDN) ||
        IsNullOrEmptyString(pArgs->pszPassword))
    {
        dwError = VECS_MISSING_CREDS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppArgs = pArgs;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAccountName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);

    return dwError;

error :

    *ppArgs = NULL;

    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }

    switch (dwError)
    {
        case VECS_MISSING_CREDS:

            VmAfdLog(VMAFD_DEBUG_ANY, "Account DN / Password missing");

            break;

        case VECS_MISSING_DC_NAME:

            VmAfdLog(VMAFD_DEBUG_ANY, "Invalid domain controller name");

            break;

        default:

            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Error [%d] getting machine Info",
                dwError);

            break;
    }

    goto cleanup;
}

VOID
VmAfdFreeRegArgs(
    PVMAFD_REG_ARG pArgs
    )
{
	if ( pArgs )
	{
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccount);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccountDN);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszPassword);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszDomain);
		VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccountUPN);
		VmAfdFreeMemory(pArgs);
	}
}

BOOLEAN
VmAfdSrvIsValidGUID(
    PCWSTR pwszGUID
    )
{
    DWORD dwError = 0;
    BOOLEAN bResult = FALSE;
    PSTR    pszGUID = NULL;

    if (!IsNullOrEmptyString(pwszGUID))
    {
        unsigned32 status = 0;
        dce_uuid_t uuid;

        dwError = VmAfdAllocateStringAFromW(pwszGUID, &pszGUID);
        BAIL_ON_VMAFD_ERROR(dwError);

        dce_uuid_from_string(pszGUID, &uuid, &status);

        bResult =  (status == uuid_s_ok);
    }

error:

    VMAFD_SAFE_FREE_STRINGA(pszGUID);

    return bResult;
}

DWORD
_VmAfdConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PSTR  pszValue = NULL;
    PWSTR pwszValue = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszValue, dwError);

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &pszValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfdAllocateStringWFromA(pszValue, &pwszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszValue = pwszValue;

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    VMAFD_SAFE_FREE_STRINGA(pszValue);

    return dwError;

error:

    if (ppwszValue)
    {
        *ppwszValue = NULL;
    }

    goto cleanup;
}


DWORD
_VmAfdConfigGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    DWORD dwValue = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwValue, dwError);

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadDWORDValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &dwValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwValue = dwValue;

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}


DWORD
_VmAfdConfigSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue         /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;

    if (IsNullOrEmptyString(pszValueName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pParamsKey,
                    pszValueName,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD));
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}


VMAFD_STATUS
VmAfdStatus(
    VOID
    )
{
    VMAFD_STATUS rtnStatus;

    pthread_mutex_lock(&gVmafdGlobals.mutex);
    rtnStatus = gVmafdGlobals.status;
    pthread_mutex_unlock(&gVmafdGlobals.mutex);

    return rtnStatus;
}

UINT64
VmAfdGetTimeInMilliSec(
    VOID
    )
{
    UINT64 iTimeInMSec = 0;

#ifdef _WIN32

    FILETIME        currentFileTime = {0};
    ULARGE_INTEGER  currentTime = {0};

    GetSystemTimeAsFileTime(&currentFileTime);

    currentTime.LowPart  = currentFileTime.dwLowDateTime;
    currentTime.HighPart = currentFileTime.dwHighDateTime;

    iTimeInMSec = (currentTime.QuadPart * 100) / NSECS_PER_MSEC;

#else

    struct timespec     timeValue = {0};

    if (clock_gettime(CLOCK_MONOTONIC, &timeValue) == 0)
    {
        iTimeInMSec = (timeValue.tv_sec * NSECS_PER_SEC + timeValue.tv_nsec ) / NSECS_PER_MSEC;
    }

#endif

    return  iTimeInMSec;
}

DWORD
VmAfdConnectLdapWithMachineAccount(
    LDAP** ppLotus
    )
{
    DWORD dwError = 0;
    LDAP* pLotus = NULL;
    PSTR pszUpn = NULL;
    PVMAFD_REG_ARG pArgs = NULL;
    PWSTR pwszDCName = NULL;
    PSTR pszDCName = NULL;

    dwError = VmAfdGetMachineInfo(&pArgs);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pszUpn,
                "%s@%s",
                pArgs->pszAccount,
                pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLDAPConnect(
                pszDCName,
                LDAP_PORT,
                pszUpn,
                pArgs->pszPassword,
                &pLotus);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppLotus = pLotus;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszUpn);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGW(pwszDCName);
    VmAfdFreeRegArgs(pArgs);
    return dwError;
error:
    if (pLotus)
    {
        VmAfdLdapClose(pLotus);
    }
    goto cleanup;

}

DWORD
VmAfdCheckDomainFunctionalLevel(
    int nMinMajor,
    int nMinMinor
    )
{
    DWORD dwError = 0;
    LDAP* pLotus = NULL;
    PSTR pszDomainFunctionalLevel = NULL;
    int nMajor = 0, nMinor = 0;
    int cFields = 0;

    dwError = VmAfdConnectLdapWithMachineAccount(&pLotus);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfdGetDomainFunctionLevel(pLotus, &pszDomainFunctionalLevel);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    if (IsNullOrEmptyString(pszDomainFunctionalLevel))
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    cFields = sscanf(pszDomainFunctionalLevel, "%u.%u", &nMajor, &nMinor);
    if (cFields <= 0)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if ((nMajor < nMinMajor) ||
        (nMajor == nMinMajor && nMinor < nMinMinor))
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

cleanup:
    if (pLotus)
    {
        VmAfdLdapClose(pLotus);
    }
    VMAFD_SAFE_FREE_MEMORY(pszDomainFunctionalLevel);
    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdGeneratePtrNameFromIp(
        PCSTR pszIPAddress,
        PSTR* ppszPtrName
    )
{
    DWORD dwError = 0;
    DWORD dwAddr = 0;
    PSTR pszPtrName = NULL;
    BYTE* pByte = NULL;
    DWORD ret = 0;
    int af = AF_INET;
    unsigned char buf[sizeof(struct in6_addr)];

    BAIL_ON_VMAFD_EMPTY_STRING(pszIPAddress, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppszPtrName, dwError);

    if (VmAfdStringChrA(pszIPAddress, ':'))
    {
        af = AF_INET6;
    }

    ret = inet_pton(af, pszIPAddress, buf);
    if (ret <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (af == AF_INET)
    {
        dwAddr = ((struct in_addr*)buf)->s_addr;

        // See RFC 1035 for name format
        // In short, record name is octets in reverse order appened with "in-addr.arpa".
        // Example: 11.1.193.128.in-addr.arpa
        dwError = VmAfdAllocateStringPrintf(
                    &pszPtrName,
                    "%d.%d.%d.%d%s.",
                    (dwAddr & 0xFF000000) >> 24,
                    (dwAddr & 0xFF0000) >> 16,
                    (dwAddr & 0xFF00) >> 8,
                    (dwAddr & 0xFF),
                    PTR_NAME_SUFFIX_IP4
                    );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
#ifdef _WIN32
        pByte = ((struct in6_addr*)buf)->u.Byte;
#else
        pByte = ((struct in6_addr*)buf)->s6_addr;
#endif
        // See RFC 1886 for ipv6 ptr name format
        // In short, record name is address presented in nibbles separated by dots,
        // in reverse order appened with "ip6.arpa".
        // Example: 4.1.2.2.0.3.e.f.f.f.6.5.0.5.2.0.7.9.0.0.8.1.1.0.0.1.0.0.0.0.c.f.ip6.arpa
        dwError = VmAfdAllocateStringPrintf(
                    &pszPtrName,
                    "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x."
                    "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x"
                    "%s.",
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszPtrName = pszPtrName;

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszPtrName);
    goto cleanup;
}
