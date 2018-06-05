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
#ifdef _WIN32
#define wchar16_t wchar_t
#define fileno(F) _fileno((F))
#endif

static
DWORD
_VmDirRPCCheckAccess(
    handle_t                 IDL_handle,
    DWORD                    dwRpcFlags,
    PSTR                     pszDomainDn,
    PVMDIR_SRV_ACCESS_TOKEN* ppAccessToken
    );

static
DWORD
_VmDirRemoteDBCopyWhiteList(
    PCSTR   pszTargetFile
    );

DWORD
VmDirSrvInitializeHost(
    PWSTR    pwszDomainName,
    PWSTR    pwszUsername,  // We ignore this parameter and use hard code value for system Administrator.
    PWSTR    pwszPassword,
    PWSTR    pwszSiteName,
    PWSTR    pwszReplURI,
    UINT32   firstReplCycleMode,
    PVMDIR_TRUST_INFO_W pTrustInfoW
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainName = NULL;
    PSTR  pszUsername = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszSiteName = NULL;
    PSTR  pszReplURI = NULL;
    PSTR  pszFQDomainName = NULL;
    PSTR  pszSystemDomainAdminName = "Administrator";  // For system domain, we always use "Administrator" name.
    PVMDIR_TRUST_INFO_A pTrustInfoA = NULL;

    if (IsNullOrEmptyString(pwszDomainName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( ! IsNullOrEmptyString(pwszUsername) )
    {
        dwError = VmDirAllocateStringAFromW(pwszUsername, &pszUsername);
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( VmDirStringCompareA( pszUsername, pszSystemDomainAdminName, FALSE) != 0 )
        {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "VmDirSrvInitializeHost overrides username (%s) using (%s)",
                                   VDIR_SAFE_STRING(pszUsername),
                                   VDIR_SAFE_STRING(pszSystemDomainAdminName) );
        }
    }

    dwError = VmDirAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pwszSiteName)
    {
        dwError = VmDirAllocateStringAFromW(pwszSiteName, &pszSiteName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pwszReplURI)
    {
        dwError = VmDirAllocateStringAFromW(pwszReplURI, &pszReplURI);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszDomainName, &pszFQDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pTrustInfoW)
    {
        dwError = VmDirAllocateTrustInfoAFromW(
                    pTrustInfoW,
                    &pTrustInfoA);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvSetupHostInstance(
                    pszFQDomainName,
                    pszSystemDomainAdminName,
                    pszPassword,
                    pszSiteName,
                    pszReplURI,
                    firstReplCycleMode,
                    pTrustInfoA );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSrvInitializeHost success: (%s)(%s)(%s)(%s)",
                                     VDIR_SAFE_STRING(pszFQDomainName),
                                     VDIR_SAFE_STRING(pszSystemDomainAdminName),
                                     VDIR_SAFE_STRING(pszSiteName),
                                     VDIR_SAFE_STRING(pszReplURI));

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszUsername);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);
    VMDIR_SAFE_FREE_MEMORY(pszSiteName);
    VMDIR_SAFE_FREE_MEMORY(pszReplURI);
    VMDIR_SAFE_FREE_MEMORY(pszFQDomainName);
    VMDIR_SAFE_FREE_TRUST_INFO_A(pTrustInfoA);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSrvInitializeHost failed (%u)(%s)(%s)(%s)(%s)(%u)", dwError,
                                     VDIR_SAFE_STRING(pszFQDomainName),
                                     VDIR_SAFE_STRING(pszSystemDomainAdminName),
                                     VDIR_SAFE_STRING(pszSiteName),
                                     VDIR_SAFE_STRING(pszReplURI),
                                     firstReplCycleMode);
    goto cleanup;
}

DWORD
VmDirSrvForceResetPassword(
    PWSTR                   pwszTargetUPN,  // [in] UPN
    VMDIR_DATA_CONTAINER*   pContainer      // [out]
    )
{
    DWORD   dwError = 0;
    DWORD   dwAPIError = 0;
    PSTR    pszTargetUPN = NULL;
    PSTR    pLocalPassword = NULL;
    DWORD   dwAPIErrorMap[] = { VMDIR_ERROR_INVALID_PARAMETER,
                                VMDIR_ERROR_ENTRY_NOT_FOUND,
                                VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND
                              };
    if ( !pwszTargetUPN
         || !pContainer )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW( pwszTargetUPN, &pszTargetUPN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // BUGBUG, only handle default policy now.  should to be per domain
    dwError = VmDirGenerateRandomPasswordByDefaultPolicy(&pLocalPassword );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirResetPassword( pszTargetUPN, pLocalPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    pContainer->dwCount = (DWORD)VmDirStringLenA(pLocalPassword);
    pContainer->data    = pLocalPassword;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirSrvForceResetPassword (%s)", VDIR_SAFE_STRING(pszTargetUPN) );

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszTargetUPN );

    return dwAPIError;

error:

    VMDIR_SAFE_FREE_MEMORY( pLocalPassword );
    VMDIR_API_ERROR_MAP( dwError, dwAPIError, dwAPIErrorMap);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSrvForceResetPassword failed (%u)(%u)(%s)",
                                      dwError, dwAPIError, VDIR_SAFE_STRING(pszTargetUPN) );

    goto cleanup;
}

DWORD
VmDirSrvGetServerState(
    PDWORD   pdwServerState      // [out]
    )
{
    DWORD   dwError = 0;
    DWORD   dwState = 0;

    if ( !pdwServerState )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwState = (DWORD)VmDirdState();

    *pdwServerState = dwState;

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
		     "VmDirSrvGetServerState failed (%u)",
		     dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirGeneratePassword(
    handle_t    hBinding,
    VMDIR_DATA_CONTAINER* pContainer        // [out]
    )
{
    DWORD dwError = 0;
    DWORD dwAPIError = 0;
    PBYTE pLocalByte = NULL;
    PBYTE pContainerBlob = NULL;
    DWORD dwAPIErrorMap[] = {  VMDIR_ERROR_INVALID_PARAMETER   //TODO, not complete
                            };
    DWORD   dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    int pwdLen = 0;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    if ( !pContainer)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateRandomPasswordByDefaultPolicy( (PSTR*)&pLocalByte );
    BAIL_ON_VMDIR_ERROR( dwError );
    pwdLen = (int)VmDirStringLenA((PSTR)pLocalByte);

    //clone data into container
    dwError = VmDirRpcAllocateMemory(
                    pwdLen,
                    (PVOID*)&pContainerBlob
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory (
                    pContainerBlob,
                    pwdLen,
                    pLocalByte,
                    pwdLen
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    pContainer->dwCount = pwdLen;
    pContainer->data    = pContainerBlob;
    pContainerBlob      = NULL;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "RpcVmDirGeneratePassword passed");

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_GENERATEPASSWORD, uiStartTime, uiEndTime);

    return dwAPIError;

error:

    VmDirRpcFreeMemory( pContainerBlob );
    VMDIR_API_ERROR_MAP( dwError, dwAPIError, dwAPIErrorMap);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirGeneratePassword failed (%u)(%u)", dwError, dwAPIError);

    goto cleanup;
}

UINT32
Srv_RpcVmDirGetKeyTabRecBlob(
    handle_t    hBinding,
    PWSTR       pwszUPN,                // [in] FQDN
    VMDIR_DATA_CONTAINER* pContainer    // [out]
    )
{
    DWORD dwError = 0;
    DWORD dwAPIError = 0;
    PSTR  pszUPN = NULL;
    PBYTE pLocalByte = NULL;
    DWORD dwByteSize = 0;
    PBYTE pContainerBlob = NULL;
    DWORD dwAPIErrorMap[] = {  VMDIR_ERROR_INVALID_PARAMETER   //TODO, not complete
                            };
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                     | VMDIR_RPC_FLAG_ALLOW_TCPIP
                     | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                     | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    if (IsNullOrEmptyString(pwszUPN)
        || !pContainer )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                    pwszUPN,
                    &pszUPN
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetKeyTabRecBlob(
                    pszUPN,
                    &pLocalByte,
                    &dwByteSize
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    //clone data into container
    dwError = VmDirRpcAllocateMemory(
                    dwByteSize,
                    (PVOID*)&pContainerBlob
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory (
                    pContainerBlob,
                    dwByteSize,
                    pLocalByte,
                    dwByteSize
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    pContainer->dwCount = dwByteSize;
    pContainer->data    = pContainerBlob;
    pContainerBlob      = NULL;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "RpcVmDirGetKeyTabRecBlob (%s)", VDIR_SAFE_STRING(pszUPN) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_GETKEYTABRECBLOB, uiStartTime, uiEndTime);

    return dwAPIError;

error:

    VmDirRpcFreeMemory( pContainerBlob );
    VMDIR_API_ERROR_MAP( dwError, dwAPIError, dwAPIErrorMap);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirGetKeyTabRecBlob failed (%u)(%u)(%s)",
                                      dwError, dwAPIError, VDIR_SAFE_STRING(pszUPN) );

    goto cleanup;
}

UINT32
Srv_RpcVmDirGetKrbMasterKey(
    handle_t    hBinding,
    PWSTR       pwszDomainName,      // [in] FQDN
    VMDIR_DATA_CONTAINER* pContainer // [out]
    )
{
    DWORD   dwError = 0;
    DWORD   dwAPIError = 0;
    PBYTE   pLocalByte = NULL;
    PBYTE   pLocalRPCByte = NULL;
    DWORD   dwKeySize = 0;
    PSTR    pszDomainName = NULL;
    DWORD   dwAPIErrorMap[] = {  VMDIR_ERROR_INVALID_PARAMETER,
                                 VMDIR_ERROR_INVALID_REALM
                              };
    DWORD   dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pwszDomainName)
        || !pContainer
       )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW( pwszDomainName,
                                         &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetKrbMasterKey( pszDomainName,
                                    &pLocalByte,
                                    &dwKeySize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateMemory( dwKeySize,
                                      (PVOID*)&pLocalRPCByte);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory ( pLocalRPCByte,
                                dwKeySize,
                                pLocalByte,
                                dwKeySize);
    BAIL_ON_VMDIR_ERROR(dwError);

    pContainer->data    = pLocalRPCByte;
    pContainer->dwCount = dwKeySize;
    pLocalRPCByte = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "RpcVmDirGetKrbMasterKey (%s)", VDIR_SAFE_STRING(pszDomainName) );

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );

    return dwAPIError;

error:

    VmDirRpcFreeMemory( pLocalRPCByte );
    VMDIR_API_ERROR_MAP( dwError, dwAPIError, dwAPIErrorMap);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirGetKrbMasterKey failed (%u)(%u)(%s)",
                                      dwError, dwAPIError, VDIR_SAFE_STRING(pszDomainName) );

    goto cleanup;
}

DWORD
VmDirSrvSetSRPSecret(
    PWSTR       pwszUPN,             // [in] account UPN
    PWSTR       pwszSecret           // [in] secret
    )
{
    DWORD   dwError = 0;
    PSTR    pszUPN = NULL;
    PSTR    pszSecret = NULL;

    if ( IsNullOrEmptyString(pwszUPN)
         || IsNullOrEmptyString(pwszSecret)
       )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW( pwszUPN,
                                         &pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW( pwszSecret,
                                         &pszSecret);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSRPSetIdentityData( pszUPN,
                                       pszSecret);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "VmDirSrvSetSRPSecret (%s)", VDIR_SAFE_STRING(pszUPN) );

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    VMDIR_SAFE_FREE_STRINGA(pszSecret);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSrvSetSRPSecret failed (%u)(%s)",
                                      dwError, VDIR_SAFE_STRING(pszUPN) );

    goto cleanup;
}

UINT32
Srv_RpcVmDirGetKrbUPNKey(
    handle_t    hBinding,
    PWSTR       pwszUpnName,
    VMDIR_DATA_CONTAINER *pContainer
    )
{
    DWORD   dwError = 0;
    PBYTE   pLocalByte = NULL;
    PBYTE   pLocalRPCByte = NULL;
    DWORD   dwKeySize = 0;
    PSTR    pszUpnName = NULL;
    DWORD   dwAPIError = 0;
    DWORD   dwAPIErrorMap[] = {  VMDIR_ERROR_INVALID_PARAMETER,
                                 VMDIR_ERROR_NO_SUCH_ATTRIBUTE,
                                 VMDIR_ERROR_ENTRY_NOT_FOUND
                              };
    DWORD   dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pwszUpnName)
        || !pContainer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW( pwszUpnName,
                                         &pszUpnName);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirGetKrbUPNKey( pszUpnName,
                                 &pLocalByte,
                                 &dwKeySize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRpcAllocateMemory( dwKeySize,
                                      (PVOID*)&pLocalRPCByte);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory ( pLocalRPCByte,
                                dwKeySize,
                                pLocalByte,
                                dwKeySize);
    BAIL_ON_VMDIR_ERROR(dwError);

    pContainer->data    = pLocalRPCByte;
    pContainer->dwCount = dwKeySize;
    pLocalRPCByte = NULL;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "RpcVmDirGetKrbUPNKey (%s)", VDIR_SAFE_STRING(pszUpnName) );

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    VMDIR_SAFE_FREE_MEMORY(pszUpnName);
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );

    return dwAPIError;

error:

    VmDirRpcFreeMemory( pLocalRPCByte );
    VMDIR_API_ERROR_MAP( dwError, dwAPIError, dwAPIErrorMap);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirGetKrbUPNKey failed (%u)(%u)(%s)",
                                     dwError, dwAPIError, VDIR_SAFE_STRING(pszUpnName) );
    goto cleanup;
}

static
DWORD
_RpcVmDirCreateUserInternal(
    handle_t    hBinding,
    PWSTR pwszUserName,
    PWSTR pwszPassword,
    PWSTR pwszUPNName,
    unsigned char bRandKey
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD j = 0;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUPNName  = NULL;
    CHAR pszHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
    PSTR pszDomainName = NULL;   /* This is an alias, do not free */
    PSTR pszDnUsers = NULL;
    PSTR pszDnDomain = NULL;
    PSTR pszDnUpn = NULL;

    if ( IsNullOrEmptyString(pwszUserName)
     ||  IsNullOrEmptyString(pwszUPNName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( bRandKey )
    {
        dwError = VmDirGenerateRandomPasswordByDefaultPolicy( &pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        if ( IsNullOrEmptyString(pwszPassword) )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        dwError = VmDirAllocateStringAFromW(
                        pwszPassword,
                        &pszPassword
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW(
                    pwszUserName,
                    &pszUserName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                    pwszUPNName,
                    &pszUPNName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    /* vdcpromo sets this key. */
    dwError = VmDirGetRegKeyValue(VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                  VMDIR_REG_KEY_DC_ACCOUNT,
                                  pszHostName,
                                  sizeof(pszHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Skip over the host name part of the FQDN */
    for (i=0; pszHostName[i] && pszHostName[i] != '.'; i++)
        ;

    /* Remainder is domain name. Convert to lower case */
    if (pszHostName[i])
    {
        i++;
        for (j=i; pszHostName[j]; j++)
        {
            VMDIR_ASCII_UPPER_TO_LOWER(pszHostName[j]);
        }
        pszDomainName = &pszHostName[i];
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirFQDNToDN(pszDomainName, &pszDnDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDN("users", pszDnDomain, &pszDnUsers);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDN(pszUserName, pszDnUsers, &pszDnUpn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccount(
                pszUPNName,
                pszUserName,
                pszPassword,
                pszDnUpn
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "_RpcVmDirCreateUserInternal (%s)", VDIR_SAFE_STRING(pszUPNName) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDnUsers);
    VMDIR_SAFE_FREE_MEMORY(pszDnDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDnUpn);
    VMDIR_SAFE_FREE_MEMORY(pszUserName);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);
    VMDIR_SAFE_FREE_MEMORY(pszUPNName);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_RpcVmDirCreateUserInternal failed (%u)(%s)",
                                      dwError, VDIR_SAFE_STRING(pszUPNName) );
    goto cleanup;

}

UINT32
Srv_RpcVmDirCreateUser(
    handle_t    hBinding,
    PWSTR pwszUserName,
    PWSTR pwszPassword,
    PWSTR pwszUPNName,
    unsigned char bRandKey
    )
{
    DWORD dwError = 0;

    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _RpcVmDirCreateUserInternal(
                  hBinding,
                  pwszUserName,
                  pwszPassword,
                  pwszUPNName,
                  bRandKey);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_CREATEUSER, uiStartTime, uiEndTime);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirCreateUser failed (%u)",
                                      dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirCreateUserEx(
    handle_t                      hBinding,
    PVMDIR_USER_CREATE_PARAMS_RPC pCreateParams
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    if (!hBinding || !pCreateParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccountEx(pAccessToken, pCreateParams);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_CREATEUSEREX, uiStartTime, uiEndTime);

    return dwError;

error:

    goto cleanup;
}

UINT32
Srv_RpcVmDirReplNow(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirReplNow failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSetLogLevel(
    handle_t        hBinding,
    VMDIR_LOG_LEVEL logLevel
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLogSetLevel( logLevel );

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SETLOGLEVEL, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "RpcVmDirSetLogLevel failed (%u)",
                    dwError);
    goto cleanup;
}

UINT32
Srv_RpcVmDirSetLogMask(
    handle_t    hBinding,
    UINT32      iVmDirLogMask
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLogSetMask( iVmDirLogMask );

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SETLOGMASK, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirSetLogMask failed (%u)(%u)", dwError, iVmDirLogMask );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSetState(
    handle_t hBinding,
    UINT32   dwState)
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    VDIR_SERVER_STATE currentState = VMDIRD_STATE_UNDEFINED;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Only valid target states
    if (dwState != VMDIRD_STATE_READ_ONLY &&
        dwState != VMDIRD_STATE_READ_ONLY_DEMOTE &&
        dwState != VMDIRD_STATE_NORMAL &&
        dwState != VMDIRD_STATE_STANDALONE)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    currentState = VmDirdState();

    if (currentState == VMDIRD_STATE_SHUTDOWN ||
        currentState == VMDIRD_STATE_FAILURE ||
        currentState == VMDIRD_STATE_RESTORE ||
        currentState == VMDIRD_STATE_STARTUP)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                        "%s: State (%u) cannot be changed from state (%u)",
                        __FUNCTION__,
                        dwState,
                        currentState);
        dwError = VMDIR_ERROR_OPERATION_NOT_PERMITTED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // no return value
    VmDirdStateSet(dwState);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirSetState: VmDir State (%u)", dwState );

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirSetState failed (%u)(%u)", dwError, dwState );
    goto cleanup;
}

UINT32
Srv_RpcVmDirOpenDBFile(
    handle_t    hBinding,
    PWSTR       pwszDBFileName,
    vmdir_ftp_handle_t  *   ppFileHandle)
{
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirOpenDBFile is obsolete.");
    return LDAP_OPERATIONS_ERROR;
}

UINT32
Srv_RpcVmDirReadDBFile(
    handle_t    hBinding,
    vmdir_ftp_handle_t pFileHandle,
    UINT32      dwCount,
    VMDIR_FTP_DATA_CONTAINER  * pReadBufferContainer)
{
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirReadDBFile is obsolete.");
    return LDAP_OPERATIONS_ERROR;
}

UINT32
Srv_RpcVmDirCloseDBFile(
    handle_t    hBinding,
    vmdir_ftp_handle_t pFileHandle)
{
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirCloseDBFile is obsolete.");
    return LDAP_OPERATIONS_ERROR;
}


/*
 * Rundown function for vmdir_ftp_handle data. Handle the case where the
 * client/server connection is lost and the existing connection handle (FILE *)
 * is still open. Close the file to prevent a fd leak. However, must protect
 * against calling fclose() twice on the same open FILE *.
 */
void vmdir_ftp_handle_t_rundown(void *ctx)
{
    if (ctx && fileno((FILE *) ctx) != -1)
    {
        fclose((FILE *) ctx);
    }
}

static
DWORD
_VmDirRPCCheckAccess(
    handle_t                 IDL_handle,
    DWORD                    dwRpcFlags,
    PSTR                     pszDomainDn,
    PVMDIR_SRV_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD   dwError = 0;
    rpc_authz_cred_handle_t hPriv = { 0 };
    DWORD   dwProtectLevel = 0;
    ULONG   rpc_status = rpc_s_ok;
    unsigned char *authPrinc = NULL;
    PSTR    pszRpcHandle = NULL;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;

    if (IsNullOrEmptyString(pszDomainDn))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    rpc_binding_to_string_binding(
         IDL_handle,
         (unsigned_char_p_t *) &pszRpcHandle,
         &rpc_status);
    if (rpc_status != rpc_s_ok)
    {
        VMDIR_LOG_VERBOSE( LDAP_DEBUG_RPC, "_VmDirRPCCheckAccess: rpc_binding_to_string_binding failed");
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_VERBOSE(LDAP_DEBUG_RPC,
             "_VmDirRPCCheckAccess: request from %s", pszRpcHandle);

    if (strncmp(pszRpcHandle, "ncalrpc:", 8) == 0)
    {
        if ( !(dwRpcFlags & VMDIR_RPC_FLAG_ALLOW_NCALRPC) )
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        if ( !(dwRpcFlags & VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC) )
        {
            goto cleanup;
        }
    }
    else
    {
        if ( !(dwRpcFlags & VMDIR_RPC_FLAG_ALLOW_TCPIP) )
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        if ( !(dwRpcFlags & VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP) )
        {
            goto cleanup;
        }
    }

    rpc_binding_inq_auth_caller(
        IDL_handle,
        &hPriv,
        &authPrinc,
        &dwProtectLevel,
        NULL, /* unsigned32 *authn_svc, */
        NULL, /* unsigned32 *authz_svc, */
        &rpc_status);
    if (rpc_status != rpc_s_ok)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Deny if connection is not encrypted */
    if (dwProtectLevel < rpc_c_protect_level_pkt_privacy)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Deny if no auth identity is provided.  */
    if (rpc_status == rpc_s_binding_has_no_auth || !authPrinc || !*authPrinc)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirRPCCheckAccess: Authenticated user %s", authPrinc);

    if ( dwRpcFlags & VMDIR_RPC_FLAG_REQUIRE_AUTHZ )
    {
        dwError = VmDirAdministratorAccessCheck(authPrinc, (PCSTR) pszDomainDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirRPCCheckAccess: Authorized user %s", authPrinc);
    }

    dwError = VmDirSrvCreateAccessToken((PCSTR)authPrinc, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAccessToken = pAccessToken;

cleanup:
    if (authPrinc)
    {
        rpc_string_free((unsigned_char_p_t *)&authPrinc, &rpc_status);
    }
    if (pszRpcHandle)
    {
        rpc_string_free((unsigned_char_p_t *)&pszRpcHandle, &rpc_status);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirRPCCheckAccess: Authenticated user (%s), handle (%s), failed code (%u)(%d)",
                     VDIR_SAFE_STRING( (PSTR)authPrinc ),
                     VDIR_SAFE_STRING(pszRpcHandle),
                     rpc_status, dwError);

    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    goto cleanup;
}

/*
 * Only allow two mdb files - data.mdb, lock.mdb, under vmdir datapath.
 */
static
DWORD
_VmDirRemoteDBCopyWhiteList(
    PCSTR   pszTargetFile
    )
{
    DWORD   dwError             = 0;
    int     i                   = 0;
    BOOLEAN bAccessAllowed      = FALSE;
    PSTR    pszDBFileNames[]    = {VMDIR_MDB_DATA_FILE_NAME, VMDIR_MDB_LOCK_FILE_NAME, VMDIR_MDB_XLOGS_DIR_NAME};
    PSTR    pszFullPathName     = NULL;
#ifdef _WIN32
    CHAR    pszFilePath[VMDIR_MAX_PATH_LEN] = {0};
#else
    CHAR    pszFilePath[VMDIR_MAX_PATH_LEN] = VMDIR_LINUX_DB_PATH;
#endif

#ifdef _WIN32
    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH,
                                   VMDIR_REG_KEY_DATA_PATH,
                                   pszFilePath,
                                   sizeof(pszFilePath)-1);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    for (i=0; i < sizeof(pszDBFileNames)/sizeof(pszDBFileNames[0]); i++)
    {
        if (VmDirStringStrA( pszTargetFile, pszDBFileNames[i]) != NULL &&
            VmDirStringStrA( pszTargetFile, pszFilePath) != NULL)
        {
            bAccessAllowed = TRUE;
            break;
        }
    }

    if ( bAccessAllowed == FALSE )
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "Remote DB transfer (%s) denied.", pszTargetFile);
        dwError = VMDIR_ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }
    else
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,"Remote DB transfer (%s) granted.", pszTargetFile);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFullPathName);

    return dwError;

error:
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogQueryServerData(
    handle_t    hBinding,
    PVMDIR_SUPERLOG_SERVER_DATA *ppServerData
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSuperLogQueryServerState(ppServerData);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGQUERYSERVERDATA, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogQueryServerData failed (%u)", dwError);
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogEnable(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEnableSuperLogging(gVmdirGlobals.pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGENABLE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogEnable failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogDisable(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDisableSuperLogging(gVmdirGlobals.pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGDISABLE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogDisable failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirIsSuperLogEnabled(
    handle_t    hBinding,
    PBOOLEAN    pbEnabled
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    BOOLEAN bEnabled = FALSE;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    bEnabled = VmDirIsSuperLoggingEnabled(gVmdirGlobals.pLogger);
    *pbEnabled = bEnabled;

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_ISSUPERLOGENABLED, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirIsSuperLogEnabled failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogFlush(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFlushSuperLogging(gVmdirGlobals.pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGFLUSH, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogFlush failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogSetSize(
    handle_t    hBinding,
    UINT32      iSize
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSuperLoggingSize(gVmdirGlobals.pLogger, iSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGSETSIZE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogSetSize failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirSuperLogGetSize(
    handle_t    hBinding,
    UINT32      *piSize
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSuperLoggingSize(gVmdirGlobals.pLogger, piSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGGETSIZE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogGetSize failed (%u)", dwError );
    goto cleanup;
}

//
// If this client doesn't have a cookie structure yet allocate one.
//
DWORD
_VmdirSuperLoggingInitializeCookie(
    vmdir_superlog_cookie_t *pEnumerationCookie
    )
{
    DWORD dwError = 0;

    if (*pEnumerationCookie == NULL)
    {
        dwError = VmDirAllocateMemory(sizeof(ULONG64), (PVOID*)pEnumerationCookie);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

//
// Rundown callback that will free the tracking information we use for paged
// log retrieval.
//
void vmdir_superlog_cookie_t_rundown(void *ctx)
{
    if (ctx)
    {
        VmDirFreeMemory(ctx);
    }
}

UINT32
Srv_RpcVmDirSuperLogGetEntriesLdapOperation(
    handle_t  hBinding,
    vmdir_superlog_cookie_t *pEnumerationCookie,
    UINT32 dwCountRequested,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppRpcEntries
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmdirSuperLoggingInitializeCookie(pEnumerationCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSuperLoggingGetEntries(gVmdirGlobals.pLogger, (UINT64 *)*pEnumerationCookie, dwCountRequested, ppRpcEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SUPERLOGGETENTRIESLDAPOPERATION, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "RpcVmDirSuperLogGetEntriesLdapOperation failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirOpenDatabaseFile(
    handle_t    hBinding,
    PWSTR       pwszDBFileName,
    vmdir_dbcp_handle_t  *   ppFileHandle)
{
    DWORD              dwError = 0;
    FILE              *pFileHandle = NULL;
    PSTR               pszDBFileName = NULL;
    PSTR               pszLocalErrMsg = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pwszDBFileName)
        || ppFileHandle == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW( pwszDBFileName, &pszDBFileName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRemoteDBCopyWhiteList(pszDBFileName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE( LDAP_DEBUG_RPC, "Srv_RpcVmDirOpenDatabaseFile: Request to open the DB file: %s", pszDBFileName );

    if ((pFileHandle = fopen(pszDBFileName, "rb")) == NULL)
    {
#ifdef _WIN32
        errno = GetLastError();
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "Srv_RpcVmDirOpenDatabaseFile: fopen() call failed, DB file: (%s), error: (%d)",
                                      pszDBFileName, errno );
#else
        if (errno == ENOENT)
        {
            dwError = ERROR_NO_SUCH_FILE_OR_DIRECTORY;
        }
        else
        {
            dwError = ERROR_IO;
        }
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "Srv_RpcVmDirOpenDatabaseFile: fopen() call failed, DB file: (%s), error: (%s)",
                                      pszDBFileName, strerror(errno) );
#endif
    }
    *ppFileHandle = pFileHandle;
    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "Srv_RpcVmDirOpenDatabaseFile: opened DB file (%s)", pszDBFileName);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    VMDIR_SAFE_FREE_MEMORY(pszDBFileName);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_OPENDATABASEFILE, uiStartTime, uiEndTime);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirOpenDatabaseFile failed  (%u)(%s)",
                                      dwError, VDIR_SAFE_STRING(pszLocalErrMsg)  );
    if (pFileHandle != NULL)
    {
        fclose(pFileHandle);
    }
    goto cleanup;
}

UINT32
Srv_RpcVmDirReadDatabaseFile(
    handle_t    hBinding,
    vmdir_dbcp_handle_t pFileHandle,
    UINT32      dwCount,
    VMDIR_DBCP_DATA_CONTAINER  * pReadBufferContainer)
{
    DWORD   dwError = 0;
    PBYTE   pData = NULL;
    PSTR    pszLocalErrMsg = NULL;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pReadBufferContainer == NULL
        || pFileHandle == 0
        || dwCount == 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_VERBOSE( LDAP_DEBUG_RPC, "Srv_RpcVmDirReadDatabaseFile: file handle (%x), read (%d) bytes to transfer",
                                        pFileHandle, dwCount );

    dwError = VmDirRpcAllocateMemory( dwCount, (PVOID*)&(pData) );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((pReadBufferContainer->dwCount = (UINT32)fread(pData, 1, dwCount, (FILE *)pFileHandle)) == 0)
    {
        if (ferror((FILE *)pFileHandle))
        {
#ifdef _WIN32
            dwError = GetLastError();
#else
            dwError = ERROR_IO;
#endif
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "Srv_RpcVmDirReadDatabaseFile: read() call failed, error: %d.", dwError);
        }
    }

    pReadBufferContainer->data = pData;

cleanup:

    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_READDATABASEFILE, uiStartTime, uiEndTime);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirReadDatabaseFile failed (%u)(%s)",
                                      dwError, VDIR_SAFE_STRING(pszLocalErrMsg) );
    VmDirRpcFreeMemory(pData);
    pReadBufferContainer->dwCount = 0;
    goto cleanup;
}

UINT32
Srv_RpcVmDirCloseDatabaseFile(
    handle_t    hBinding,
    vmdir_dbcp_handle_t *ppFileHandle)
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( ppFileHandle == NULL || *ppFileHandle == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC,  "Srv_RpcVmDirCloseDatabaseFile: DB file fd (%d)", fileno(*ppFileHandle));

    if (fclose((FILE *)*ppFileHandle) != 0)
    {
        dwError = VMDIR_ERROR_IO;
        BAIL_ON_VMDIR_ERROR( dwError );
    }
    *ppFileHandle = NULL;

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_CLOSEDATABASEFILE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirCloseDatabaseFile failed (%u)", dwError);
    goto cleanup;
}

UINT32
Srv_RpcVmDirSetBackendState(
    handle_t    hBinding,
    UINT32      fileTransferState,
    UINT32      *pdwXlogNum,
    UINT32      *pdwDbSizeMb,
    UINT32      *pdwDbMapSizeMb,
    VMDIR_DBCP_DATA_CONTAINER  * pDbPath,
    UINT32      dwDbPathSize
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    DWORD dwXlogNum = 0;
    DWORD dwDbSizeMb = 0;
    DWORD dwDbMapSizeMb = 0;
    PBYTE pData = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pDbPath == NULL || dwDbPathSize <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRpcAllocateMemory( dwDbPathSize, (PVOID*)&(pData) );
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirSetMdbBackendState(fileTransferState, &dwXlogNum, &dwDbSizeMb, &dwDbMapSizeMb, pData, dwDbPathSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwXlogNum = dwXlogNum;
    *pdwDbSizeMb = dwDbSizeMb;
    *pdwDbMapSizeMb = dwDbMapSizeMb;
    pDbPath->data = pData;
    pDbPath->dwCount = dwDbPathSize;

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_SETBACKENDSTATE, uiStartTime, uiEndTime);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Srv_RpcVmDirSetBackendState failed (%u)", dwError );
    VmDirRpcFreeMemory(pData);
    pDbPath->dwCount = 0;
    goto cleanup;
}

UINT32
Srv_RpcVmDirGetState(
    handle_t    hBinding,
    UINT32*     pdwState
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pdwState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwState = VmDirdState();

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_GETSTATE, uiStartTime, uiEndTime);

    return dwError;

error:
    goto cleanup;
}

UINT32
Srv_RpcVmDirGetLogLevel(
    handle_t        hBinding,
    VMDIR_LOG_LEVEL* pLogLevel
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pLogLevel)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pLogLevel = VmDirLogGetLevel();

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_GETLOGLEVEL, uiStartTime, uiEndTime);

    return dwError;

error:
    goto cleanup;
}

UINT32
Srv_RpcVmDirGetLogMask(
    handle_t    hBinding,
    UINT32*     pLogMask
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pLogMask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pLogMask = VmDirLogGetMask();

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    uiEndTime = VmDirGetTimeInMilliSec();

    VmDirRpcMetricsUpdate(METRICS_RPC_OP_GETLOGMASK, uiStartTime, uiEndTime);

    return dwError;

error:
    goto cleanup;
}

UINT32
Srv_RpcVmDirBackupDB(
    handle_t    hBinding,
    PWSTR       pwszBackupPath
    )
{
    DWORD  dwError = 0;
    DWORD dwRpcFlags =   VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    uint64_t uiStartTime = 0;
    uint64_t uiEndTime = 0;
    PSTR    pszBackupPath = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    uiStartTime = VmDirGetTimeInMilliSec();

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pwszBackupPath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringAFromW(pwszBackupPath, &pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvBackupDB(pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s done", __FUNCTION__);

cleanup:
    uiEndTime = VmDirGetTimeInMilliSec();
    VmDirRpcMetricsUpdate(METRICS_RPC_OP_BACKUPDB, uiStartTime, uiEndTime);

    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    VMDIR_SAFE_FREE_MEMORY(pszBackupPath);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed (%u)", __FUNCTION__, dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirGetComputerAccountInfo(
    handle_t                  hBinding,
    PWSTR                     pszDomainName,
    PWSTR                     pszPassword,
    PWSTR                     pszMachineName,
    PVMDIR_MACHINE_INFO_W     *ppMachineInfo,
    PVMDIR_KRB_INFO           *ppKrbInfo
    )
{
    DWORD dwError = 0;
    PSTR paszDomainName = NULL;
    PSTR paszMachineName = NULL;
    PSTR pszMachineUPN = NULL;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PVMDIR_SRV_ACCESS_TOKEN pAdminCheckAccessToken = NULL;
    PVDIR_CONNECTION pConnection = NULL;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;
    PVMDIR_MACHINE_INFO_W pRpcMachineInfo = NULL;
    PVMDIR_KRB_INFO pKrbInfo = NULL;
    PVMDIR_KRB_INFO pRpcKrbInfo = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    if (IsNullOrEmptyString(pszMachineName) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszPassword) ||
        !ppMachineInfo || !ppKrbInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                        pszMachineName,
                        &paszMachineName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                        pszDomainName,
                        &paszDomainName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                            &pszMachineUPN,
                            "%s@%s",
                            paszMachineName,
                            paszDomainName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(pAccessToken->pszUPN, pszMachineUPN, FALSE))
    {
        dwError = _VmDirRPCCheckAccess(
                                hBinding,
                                dwRpcFlags | VMDIR_RPC_FLAG_REQUIRE_AUTHZ,
                                pszDomainDn,
                                &pAdminCheckAccessToken);
        BAIL_ON_VMDIR_ERROR(dwError);
    }



    dwError = VmDirAllocateMemory(
                            sizeof(VMDIR_MACHINE_INFO_A),
                            (PVOID*)&pMachineInfo
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetConnectionObj(
                                pAccessToken->pszUPN,
                                &pConnection
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetComputerAccountInfo(
                                         pConnection,
                                         paszDomainName,
                                         paszMachineName,
                                         &pMachineInfo->pszComputerDN,
                                         &pMachineInfo->pszMachineGUID,
                                         &pMachineInfo->pszSiteName
                                         );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                                pszPassword,
                                &pMachineInfo->pszPassword
                                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetKeyTabInfoClient(
                                  pConnection,
                                  paszDomainName,
                                  paszMachineName,
                                  &pKrbInfo
                                  );
    if (!dwError)
    {
        dwError = VmDirSrvAllocateRpcKrbInfo(
                                 pKrbInfo,
                                 &pRpcKrbInfo
                                 );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvAllocateRpcMachineInfoWFromA(
                                            pMachineInfo,
                                            &pRpcMachineInfo
                                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pRpcMachineInfo;
    *ppKrbInfo = pRpcKrbInfo;

cleanup:

    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    if (pKrbInfo)
    {
        VmDirFreeKrbInfo(pKrbInfo);
    }
    if (pConnection)
    {
        VmDirDeleteConnection(&pConnection);
    }
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    if (pAdminCheckAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAdminCheckAccessToken);
    }
    VMDIR_SAFE_FREE_STRINGA(paszDomainName);
    VMDIR_SAFE_FREE_STRINGA(paszMachineName);
    VMDIR_SAFE_FREE_STRINGA(pszMachineUPN);
    return dwError;

error:
    if (ppMachineInfo)
    {
        *ppMachineInfo = NULL;
    }
    if (pRpcMachineInfo)
    {
        VmDirSrvRpcFreeMachineInfoW(pRpcMachineInfo);
    }
    if (ppKrbInfo)
    {
        *ppKrbInfo = NULL;
    }
    if (pRpcKrbInfo)
    {
        VmDirSrvRpcFreeKrbInfo(pKrbInfo);
    }
    goto cleanup;
}

UINT32
Srv_RpcVmDirClientJoin(
    handle_t                hBinding,
    PWSTR                   pszDomainName,
    PWSTR                   pszMachineName,
    PWSTR                   pszOrgUnit,
    PVMDIR_MACHINE_INFO_W   *ppMachineInfo,
    PVMDIR_KRB_INFO         *ppKrbInfo
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwServiceTableLen = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;
    PVMDIR_MACHINE_INFO_W  pRpcMachineInfo = NULL;
    PVMDIR_KRB_INFO pKrbInfo = NULL;
    PVMDIR_KRB_INFO pRpcKrbInfo = NULL;
    PSTR paszDomainName = NULL;
    PSTR paszMachineName = NULL;
    PSTR paszOrgUnit = NULL;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PCSTR   pszClientServiceTable[] = VMDIR_CLIENT_SERVICE_PRINCIPAL_INITIALIZER;
    PVDIR_CONNECTION pConnection = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    if (IsNullOrEmptyString(pszMachineName) ||
        IsNullOrEmptyString(pszDomainName) ||
        !ppMachineInfo ||
        !ppKrbInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwServiceTableLen = VMDIR_ARRAY_SIZE(pszClientServiceTable);;

    dwError = VmDirAllocateStringAFromW(
                        pszDomainName,
                        &paszDomainName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pszOrgUnit))
    {
        dwError = VmDirAllocateStringA(
                              VMDIR_COMPUTERS_RDN_VAL,
                              &paszOrgUnit
                              );
    }
    else
    {
        dwError = VmDirAllocateStringAFromW(
                              pszOrgUnit,
                              &paszOrgUnit
                              );
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                              pszMachineName,
                              &paszMachineName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetConnectionObj(
                              pAccessToken->pszUPN,
                              &pConnection
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateComputerOUContainer(
                                          pConnection,
                                          paszDomainName,
                                          paszOrgUnit
                                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvSetupComputerAccount(
                                  pConnection,
                                  paszDomainName,
                                  paszOrgUnit,
                                  paszMachineName,      // Self host name
                                  &pMachineInfo
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < dwServiceTableLen; ++dwIndex)
    {
            dwError = VmDirSrvSetupServiceAccount(
                            pConnection,
                            paszDomainName,
                            pszClientServiceTable[dwIndex],
                            paszMachineName
                            );
            if (dwError == LDAP_ALREADY_EXISTS)
            {
                dwError = LDAP_SUCCESS; // ignore if entry already exists (maybe due to prior client join)
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirSetupServiceAccount (%s) return LDAP_ALREADY_EXISTS",
                                                        pszClientServiceTable[dwIndex] );
            }
            BAIL_ON_VMDIR_ERROR(dwError);
    }


    dwError = VmDirSrvGetKeyTabInfoClient(
                                  pConnection,
                                  paszDomainName,
                                  paszMachineName,
                                  &pKrbInfo
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvAllocateRpcKrbInfo(
                                 pKrbInfo,
                                 &pRpcKrbInfo
                                 );
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirSrvAllocateRpcMachineInfoWFromA(
                            pMachineInfo,
                            &pRpcMachineInfo
                            );
    BAIL_ON_VMDIR_ERROR(dwError);


    *ppMachineInfo = pRpcMachineInfo;
    *ppKrbInfo = pRpcKrbInfo;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(paszDomainName);
    VMDIR_SAFE_FREE_MEMORY(paszMachineName);
    VMDIR_SAFE_FREE_MEMORY(paszOrgUnit);
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    if (pKrbInfo)
    {
        VmDirFreeKrbInfo(pKrbInfo);
    }
    if (pConnection)
    {
        VmDirDeleteConnection(&pConnection);
    }
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    return dwError;
error:

    if (ppMachineInfo)
    {
      *ppMachineInfo = NULL;
    }
    if (ppKrbInfo)
    {
      *ppKrbInfo = NULL;
    }
    if (pRpcMachineInfo)
    {
        VmDirSrvRpcFreeMachineInfoW(pRpcMachineInfo);
    }
    if (pRpcKrbInfo)
    {
        VmDirSrvRpcFreeKrbInfo(pKrbInfo);
    }
    goto cleanup;
}

UINT32
Srv_RpcVmDirCreateComputerAccount(
    handle_t                hBinding,
    PWSTR                   pszDomainName,
    PWSTR                   pszMachineName,
    PWSTR                   pszOrgUnit,
    PVMDIR_MACHINE_INFO_W   *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;
    PVMDIR_MACHINE_INFO_W  pRpcMachineInfo = NULL;
    PSTR paszDomainName = NULL;
    PSTR paszMachineName = NULL;
    PSTR paszOrgUnit = NULL;
    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PVDIR_CONNECTION pConnection = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    if (IsNullOrEmptyString(pszMachineName) ||
        IsNullOrEmptyString(pszDomainName) ||
        !ppMachineInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirAllocateStringAFromW(
                        pszDomainName,
                        &paszDomainName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pszOrgUnit))
    {
        dwError = VmDirAllocateStringA(
                              VMDIR_COMPUTERS_RDN_VAL,
                              &paszOrgUnit
                              );
    }
    else
    {
        dwError = VmDirAllocateStringAFromW(
                              pszOrgUnit,
                              &paszOrgUnit
                              );
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                              pszMachineName,
                              &paszMachineName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetConnectionObj(
                              pAccessToken->pszUPN,
                              &pConnection
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateComputerOUContainer(
                                          pConnection,
                                          paszDomainName,
                                          paszOrgUnit
                                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvSetupComputerAccount(
                                  pConnection,
                                  paszDomainName,
                                  paszOrgUnit,
                                  paszMachineName,      // Self host name
                                  &pMachineInfo
                                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvAllocateRpcMachineInfoWFromA(
                            pMachineInfo,
                            &pRpcMachineInfo
                            );
    BAIL_ON_VMDIR_ERROR(dwError);


    *ppMachineInfo = pRpcMachineInfo;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(paszDomainName);
    VMDIR_SAFE_FREE_MEMORY(paszMachineName);
    VMDIR_SAFE_FREE_MEMORY(paszOrgUnit);
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    if (pConnection)
    {
        VmDirDeleteConnection(&pConnection);
    }
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }
    return dwError;
error:

    if (ppMachineInfo)
    {
      *ppMachineInfo = NULL;
    }
    if (pRpcMachineInfo)
    {
        VmDirSrvRpcFreeMachineInfoW(pRpcMachineInfo);
    }
    goto cleanup;
}



/*
 * Rundown function for vmdir_dbcp_handle data. Handle the case where the
 * client/server connection is lost and the existing connection handle (FILE *)
 * is still open. Close the file to prevent a fd leak. However, must protect
 * against calling fclose() twice on the same open FILE *.
 */
void vmdir_dbcp_handle_t_rundown(void *ctx)
{
    FILE *pFileHandle = (FILE *)ctx;

    if (pFileHandle)
    {
        if (fileno(pFileHandle) != -1)
        {
            VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "vmdir_dbcp_handle_t_rundown closing file with fd: %d", fileno(pFileHandle));
            fclose(pFileHandle);
        }
    }
    // Clear backend READ-ONLY/KeeXlog mode when dbcp connection interrupted.
    VmDirSrvSetMDBStateClear();
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "vmdir_dbcp_handle_t_rundown: MdbBackendState cleared,");
}

UINT32
Srv_RpcVmDirUrgentReplicationRequest(
    handle_t    hBinding,
    PWSTR       pwszServerName
    )
{
    return VMDIR_ERROR_DEPRECATED_FUNCTION;
}

UINT32
Srv_RpcVmDirUrgentReplicationResponse(
    handle_t  hBinding,
    PWSTR     pwszInvocationId,
    PWSTR     pwszUtdVector,
    PWSTR     pwszHostName
    )
{
    return VMDIR_ERROR_DEPRECATED_FUNCTION;
}

static
DWORD
_RpcVmDirCreateDomainTrustInternal(
    handle_t hBinding,
    PWSTR    pwszTrustName,
    PWSTR    pwszDomainName,
    PWSTR    pwszTrustPasswdIn,
    PWSTR    pwszTrustPasswdOut
    )
{
    DWORD dwError = 0;
    PSTR pszTrustName = NULL;
    PSTR pszTrustPasswdIn = NULL;
    PSTR pszTrustPasswdOut = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszDnTrusts = NULL;
    PCSTR pszSystemContainerName = "System";
    PCSTR pszDnDomain = gVmdirKrbGlobals.pszDomainDN;
    PSTR pszDnUpn = NULL;

    if ( IsNullOrEmptyString(pwszTrustName)
     ||  IsNullOrEmptyString(pwszDomainName)
     ||  IsNullOrEmptyString(pwszTrustPasswdIn)
     ||  IsNullOrEmptyString(pwszTrustPasswdOut)
     ||  IsNullOrEmptyString(pszDnDomain)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW(
                    pwszTrustName,
                    &pszTrustName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                    pwszDomainName,
                    &pszDomainName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                    pwszTrustPasswdIn,
                    &pszTrustPasswdIn
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                    pwszTrustPasswdOut,
                    &pszTrustPasswdOut
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDN(
                    pszSystemContainerName,
                    pszDnDomain,
                    &pszDnTrusts);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDN(pszTrustName, pszDnTrusts, &pszDnUpn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainTrust(
                    pszTrustName,
                    pszDomainName,
                    pszTrustPasswdIn,
                    pszTrustPasswdOut,
                    pszDnUpn
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_RPC, "%s (%s)", __FUNCTION__, VDIR_SAFE_STRING(pszTrustName) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDnTrusts);
    VMDIR_SAFE_FREE_MEMORY(pszDnUpn);
    VMDIR_SAFE_FREE_MEMORY(pszTrustName);
    VMDIR_SAFE_FREE_MEMORY(pszTrustPasswdIn);
    VMDIR_SAFE_FREE_MEMORY(pszTrustPasswdOut);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed (%u)(%s)",
                     __FUNCTION__,
                     dwError,
                     VDIR_SAFE_STRING(pszTrustName) );
    goto cleanup;
}

UINT32
Srv_RpcVmDirCreateDomainTrust(
    handle_t hBinding,
    PWSTR    pwszTrustName,
    PWSTR    pwszDomainName,
    PWSTR    pwszTrustPasswdIn,
    PWSTR    pwszTrustPasswdOut
    )
{
    DWORD dwError = 0;

    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _RpcVmDirCreateDomainTrustInternal(
                  hBinding,
                  pwszTrustName,
                  pwszDomainName,
                  pwszTrustPasswdIn,
                  pwszTrustPasswdOut
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed (%u)",
                     __FUNCTION__,
                     dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirInitializeTenant(
    handle_t hBinding,
    PWSTR    pwszDomainName,
    PWSTR    pwszUsername,
    PWSTR    pwszPassword
    )
{
    DWORD dwError = 0;

    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;
    PSTR pszDomainDn = NULL;

    VMDIR_GET_SYSTEM_DOMAIN_DN(pszDomainDn, dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvInitializeTenant(
                    pwszDomainName,
                    pwszUsername,
                    pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed (%u)",
                     __FUNCTION__,
                     dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmDirDeleteTenant(
    handle_t hBinding,
    PWSTR    pwszDomainName
    )
{
    DWORD dwError = 0;
    PSTR pszDomainName = NULL;
    PSTR pszDomainDn = NULL;

    DWORD dwRpcFlags = VMDIR_RPC_FLAG_ALLOW_NCALRPC
                       | VMDIR_RPC_FLAG_ALLOW_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_NCALRPC
                       | VMDIR_RPC_FLAG_REQUIRE_AUTH_TCPIP
                       | VMDIR_RPC_FLAG_REQUIRE_AUTHZ;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;

    dwError = VmDirAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFQDNToDN(pszDomainName, &pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRPCCheckAccess(hBinding, dwRpcFlags, pszDomainDn, &pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvDeleteTenant((PCSTR) pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDn);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed (%u)",
                     __FUNCTION__,
                     dwError );
    goto cleanup;
}
