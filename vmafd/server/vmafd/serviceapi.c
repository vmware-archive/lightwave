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

DWORD
VmAfdGetStatusInternal(
    handle_t      hBinding, /* IN     */
    PVMAFD_STATUS pStatus   /* IN OUT */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    *pStatus = VmAfdSrvGetStatus();

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetStatus failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetDomainNameInternal(
    rpc_binding_handle_t hBinding,          /* IN     */
    PWSTR*   ppwszDomain        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDomain_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDomain, dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszDomain, &pwszDomain_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDomain = pwszDomain_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    return dwError;

error:

    if (ppwszDomain)
    {
        *ppwszDomain = NULL;
    }

    if (pwszDomain_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszDomain_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDomainName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdSetDomainNameInternal(
    rpc_binding_handle_t hBinding,         /* IN     */
    PWSTR    pwszDomain        /* IN     */
    )
{
    DWORD dwError = 0;

    if IsNullOrEmptyString(pwszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : Authorization check

    dwError = VmAfSrvSetDomainName(pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetDomainName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetDomainStateInternal(
    rpc_binding_handle_t hBinding,         /* IN     */
    PVMAFD_DOMAIN_STATE  pDomainState      /*    OUT */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pDomainState, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pDomainState = domainState;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDomainState failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetLDUInternal(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR*   ppwszLDU       /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszLDU = NULL;
    PWSTR pwszLDU_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszLDU, dwError);

    dwError = VmAfSrvGetLDU(&pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszLDU, &pwszLDU_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszLDU = pwszLDU_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);

    return dwError;

error:

    if (ppwszLDU)
    {
        *ppwszLDU = NULL;
    }

    if (pwszLDU_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszLDU_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetLDU failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdSetLDUInternal(
    rpc_binding_handle_t hBinding,         /* IN     */
    PWSTR    pwszLDU           /* IN     */
    )
{
    DWORD dwError = 0;

    if IsNullOrEmptyString(pwszLDU)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : Authorization check

    dwError = VmAfSrvSetLDU(pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetLDU failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetCMLocationInternal(
    rpc_binding_handle_t hBinding,        /* IN     */
    PWSTR*   ppwszCMLocation  /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszCMLocation = NULL;
    PWSTR pwszCMLocation_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszCMLocation, dwError);

    dwError = VmAfSrvGetCMLocation(&pwszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszCMLocation, &pwszCMLocation_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszCMLocation = pwszCMLocation_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszCMLocation);

    return dwError;

error:

    if (ppwszCMLocation)
    {
        *ppwszCMLocation = NULL;
    }

    if (pwszCMLocation_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszCMLocation_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetCMLocation failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetDCNameInternal(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR*   ppwszDCName    /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszDCName_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDCName, dwError);

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszDCName, &pwszDCName_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDCName = pwszDCName_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);

    return dwError;

error:

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }

    if (pwszDCName_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszDCName_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDCName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdSetDCNameInternal(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR    pwszDCName     /* IN     */
    )
{
    DWORD dwError = 0;

    if IsNullOrEmptyString(pwszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : Authorization check

    dwError = VmAfSrvSetDCName(pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetDCName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetPNIDInternal(
    rpc_binding_handle_t hBinding,
    PWSTR* ppwszPNID
    )
{
    DWORD dwError = 0;
    PWSTR pwszPNID = NULL;
    PWSTR pwszPNID_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPNID, dwError);

    dwError = VmAfSrvGetPNID(&pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszPNID, &pwszPNID_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszPNID = pwszPNID_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    if (ppwszPNID)
    {
        *ppwszPNID = NULL;
    }

    if (pwszPNID_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszPNID_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetPNID failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdSetPNIDInternal(
    rpc_binding_handle_t hBinding,
    PWSTR pwszPNID
    )
{
    DWORD dwError = 0;

    if IsNullOrEmptyString(pwszPNID)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : Authorization check

    dwError = VmAfSrvSetPNID(pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetPNID failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdGetMachineAccountInfoInternal(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR*   ppwszAccount,  /*    OUT */
    PWSTR*   ppwszPassword  /*    OUT */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszAccount, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPassword, dwError);

    /* TBD */

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetMachineAccountInfo failed. Error(%u)",
                              dwError);
    goto cleanup;
}

UINT32
VmAfdGetSiteGUIDInternal(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR*               ppwszGUID /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszGUID = NULL;
    PWSTR pwszGUID_rpc = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszGUID, dwError);

    dwError = VmAfSrvGetSiteGUID(&pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszGUID, &pwszGUID_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszGUID = pwszGUID_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszGUID);

    return dwError;

error:

    if (ppwszGUID)
    {
        *ppwszGUID = NULL;
    }
    if (pwszGUID_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszGUID_rpc);
    }

    goto cleanup;
}

DWORD
VmAfdPromoteVmDirInternal(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR pwszServerName,          /* IN              */
    PWSTR pwszDomainName,          /* IN     OPTIONAL */
    PWSTR pwszUserName,            /* IN              */
    PWSTR pwszPassword,            /* IN              */
    PWSTR pwszSiteName,            /* IN     OPTIONAL */
    PWSTR pwszPartnerHostName      /* IN     OPTIONAL */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_CANNOT_PROMOTE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvPromoteVmDir(
                      pwszServerName,
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword,
                      pwszSiteName,
                      pwszPartnerHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CONTROLLER);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcPromoteVmDir failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdDemoteVmDirInternal(
    rpc_binding_handle_t hBinding,         /* IN              */
    PWSTR   pwszServerName,    /* IN              */
    PWSTR   pwszUserName,      /* IN              */
    PWSTR   pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_CANNOT_DEMOTE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

  dwError = VmAfSrvDemoteVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcDemoteVmDir failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdJoinVmDirInternal(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR pwszServerName,          /* IN              */
    PWSTR pwszUserName,            /* IN              */
    PWSTR pwszPassword,            /* IN              */
    PWSTR pwszMachineName,         /* IN              */
    PWSTR pwszDomainName,          /* IN              */
    PWSTR pwszOrgUnit              /* IN              */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszMachineName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (domainState != VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = ERROR_CANNOT_JOIN_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvJoinVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszDomainName,
                      pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CLIENT);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcJoinVmDir failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdLeaveVmDirInternal(
    rpc_binding_handle_t hBinding,         /* IN              */
    PWSTR   pwszServerName,    /* IN              */
    PWSTR   pwszUserName,      /* IN              */
    PWSTR   pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = ERROR_CANNOT_LEAVE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvLeaveVmDir(
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcLeaveVmDir failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdJoinADInternal(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR pwszUserName,            /* IN              */
    PWSTR pwszPassword,            /* IN              */
    PWSTR pwszDomainName,          /* IN              */
    PWSTR pwszOrgUnit              /* IN              */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    dwError = VmAfSrvJoinAD(
                      pwszUserName,
                      pwszPassword,
                      pwszDomainName,
                      pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcJoinAD failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdLeaveADInternal(
    rpc_binding_handle_t hBinding,         /* IN              */
    PWSTR   pwszUserName,      /* IN              */
    PWSTR   pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvLeaveAD(
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcLeaveAD failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdForceReplicationInternal(
    rpc_binding_handle_t hBindings, /* IN              */
    PWSTR   pwszServerName          /* IN              */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);

    dwError = VmAfSrvForceReplication(pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcForceReplication failed. Error(%u)",
                              dwError);
    goto cleanup;
}


/*
    Add Certificate takes Certificate allows the user
    to put an alias to the Certificate.
*/
DWORD
VmAfdAddCertificateInternal(
    UINT32 dwStoreType,
    PWSTR pwszAlias,
    PWSTR pwszCertificate,
    PWSTR pwszPrivateKey,
    UINT32 uAutoRefresh)
{
    DWORD dwError = 0;
    VECS_DB_CERTIFICATE_ENTRY Entry = {0};
    PSTR pszMachineAlias = NULL;
    DWORD dwCertCount = 0;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;

    size_t nCertLen = 0;
    size_t nKeyLen = 0;

    // TODO : Verify that private key belongs to certificate
    // Private Key can be null when we add Trusted Roots

    ENTER_LOG();
    if(IsNullOrEmptyString(pwszAlias) ||
       (IsNullOrEmptyString(pwszCertificate) &&
        IsNullOrEmptyString(pwszPrivateKey)))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // If it is not trusted Roots, we better have Private key,
    // Otherwise reject the Add Cert Call.
    if (dwStoreType != CERTIFICATE_STORE_TYPE_TRUSTED_ROOTS)
    {
        if (IsNullOrEmptyString(pwszPrivateKey))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = VmAfdAllocateStringAFromW(pwszAlias,&pszMachineAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    // if user tries to give us a certificate called
    // __MACHINE_CERT reject it
    if(VmAfdStringCompareA( pszMachineAlias,
                                VECS_MACHINE_CERT_ALIAS, FALSE) == 0)
    {
        dwError = VECS_UNIQUE_ALIAS_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Favoring Java, and being more restrictive here
    // We have different kind of stores, however
    // allow different aliases inside it, then we will
    // not be able to show all of that us in single KeyStore.
    // I might change this in future, and NGC has fixed the
    // bug where they were using SAME Alias for multiple
    // certs

    dwError = VecsGetCertificatebyAlias(pwszAlias,
                                        &dwCertCount,
                                        &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCertCount != 0) {
        dwError = VECS_UNIQUE_ALIAS_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pwszCertificate))
    {
        dwError = VmAfdGetStringLengthW((PCWSTR)pwszCertificate, &nCertLen);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pwszPrivateKey))
    {
        dwError = VmAfdGetStringLengthW((PCWSTR)pwszPrivateKey, &nKeyLen);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    Entry.pwszAlias = pwszAlias;
    Entry.pCertBlob = (PBYTE) pwszCertificate;
    Entry.dwCertSize = nCertLen * sizeof(wchar16_t);
    Entry.dwStoreType = dwStoreType;
    Entry.pPrivateKey = (PBYTE) pwszPrivateKey;
    Entry.dwKeySize = nKeyLen  * sizeof(wchar16_t);

    dwError = VecsInsertCertificate(&Entry);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMachineAlias);
    if (pCertContainer)
    {
        VecsSrvFreeCertContainer(pCertContainer);
    }
    EXIT_LOG();
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcAddCertificate failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
VmAfdSetSSLCertificateInternal(
    rpc_binding_handle_t hBinding,
    PWSTR pwszCertificate,
    PWSTR pwszPrivateKey)
{
    DWORD dwError = 0;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;
    VECS_DB_CERTIFICATE_ENTRY Entry = {0};
    PWSTR pwszMachineCertAlias = NULL;
    DWORD dwCertCount = 0;
    size_t nCertLen = 0;
    size_t nKeyLen = 0;
    ENTER_LOG();

    if(IsNullOrEmptyString(pwszCertificate) ||
       IsNullOrEmptyString(pwszPrivateKey)){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(VECS_MACHINE_CERT_ALIAS,
                        &pwszMachineCertAlias);
    BAIL_ON_VMAFD_ERROR(dwError);
 

    dwError = VecsGetCertificatebyAlias(pwszMachineCertAlias,
                                        &dwCertCount,
                                        &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCertCount != 0)
    {
        // Let us delete the existing SSL certificate
        // So that we can over write with the new one.
        dwError = VecsDeleteCertificateByAlias(pwszMachineCertAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW((PCWSTR)pwszCertificate, &nCertLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetStringLengthW((PCWSTR)pwszPrivateKey, &nKeyLen);
    BAIL_ON_VMAFD_ERROR(dwError);
 
 
    Entry.pwszAlias = pwszMachineCertAlias;
    Entry.pCertBlob = (PBYTE) pwszCertificate;
    Entry.dwCertSize = nCertLen * sizeof(wchar16_t);
    Entry.dwStoreType = CERTIFICATE_STORE_TYPE_TRUSTED;
    Entry.pPrivateKey = (PBYTE) pwszPrivateKey;
    Entry.dwKeySize = nKeyLen  * sizeof(wchar16_t);
    dwError = VecsInsertCertificate(&Entry);
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszMachineCertAlias);
    if (pCertContainer)
    {
        VecsSrvFreeCertContainer(pCertContainer);
    }
    EXIT_LOG();
    return dwError;
error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetSSLCertificate failed. Error(%u)",
                              dwError);
    goto cleanup;
}

/*
  Get Machine SSL Cert
*/
DWORD
VmAfdGetSSLCertificateInternal(
    rpc_binding_handle_t hBinding,
    PWSTR*   ppwszCertificate,
    PWSTR*   ppwszPrivateKey)
{

    DWORD dwError = 0;
    PWSTR pwszMachineCertAlias = NULL;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;
    DWORD dwCertCount = 0;
    PWSTR pwszRpcCert = NULL;
    PWSTR pwszRpcPrivateKey = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszCertificate, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPrivateKey, dwError);

    dwError = VmAfdAllocateStringWFromA(VECS_MACHINE_CERT_ALIAS,
                                &pwszMachineCertAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsGetCertificatebyAlias(
                    pwszMachineCertAlias,
                    &dwCertCount,
                    &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCertCount == 0) {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pCertContainer->pCert)
    {
        dwError = VmAfdRpcServerAllocateStringW(pCertContainer->pCert,
                                                &pwszRpcCert);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pCertContainer->pPrivateKey)
    {
        dwError = VmAfdRpcServerAllocateStringW(pCertContainer->pPrivateKey,
                                                &pwszRpcPrivateKey);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszCertificate = pwszRpcCert;
    *ppwszPrivateKey = pwszRpcPrivateKey;
 
cleanup:
 
    if (pCertContainer)
    {
        VecsSrvFreeCertContainer(pCertContainer);
    }

    VMAFD_SAFE_FREE_MEMORY(pwszMachineCertAlias);

    return dwError;

error:

    if (ppwszCertificate)
    {
        *ppwszCertificate = NULL;
    }
    if (ppwszPrivateKey)
    {
        *ppwszPrivateKey = NULL;
    }
    if ( pwszRpcPrivateKey != NULL) {
        VmAfdRpcServerFreeMemory(pwszRpcPrivateKey);
    }

    if ( pwszRpcCert != NULL) {
        VmAfdRpcServerFreeMemory(pwszRpcCert);
    }
    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetSSLCertificate failed. Error(%u)",
                              dwError);
    goto cleanup;
}
