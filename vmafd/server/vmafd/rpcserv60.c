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


idl_ulong_int
Srv60_VmAfdRpcGetStatus(
    handle_t hBinding,
    VMAFD_STATUS *pStatus
    )
{
    return Srv_VmAfdRpcGetStatus(
                hBinding,
                pStatus);
}

idl_ulong_int
Srv60_VmAfdRpcGetDomainName(
    handle_t hBinding,
    wstring_t *ppwszDomain
    )
{
    return Srv_VmAfdRpcGetDomainName(
                hBinding,
                ppwszDomain);
}

idl_ulong_int
Srv60_VmAfdRpcGetDomainState(
    handle_t hBinding,
    VMAFD_DOMAIN_STATE *pDomainState
    )
{
    return Srv_VmAfdRpcGetDomainState(
                hBinding,
                pDomainState);
}

idl_ulong_int
Srv60_VmAfdRpcSetDomainName(
    handle_t hBinding,
    wstring_t pswzDomainName
    )
{
    return Srv_VmAfdRpcSetDomainName(
                hBinding,
                pswzDomainName);
}

idl_ulong_int
Srv60_VmAfdRpcGetLDU(
    handle_t hBinding,
    wstring_t *ppwszLDU
    )
{
    return Srv_VmAfdRpcGetLDU(
                hBinding,
                ppwszLDU);
}

idl_ulong_int
Srv60_VmAfdRpcSetLDU(
    handle_t hBinding,
    wstring_t pswzLDU
    )
{
    return Srv_VmAfdRpcSetLDU(
                hBinding,
                pswzLDU);
}

idl_ulong_int
Srv60_VmAfdRpcSetRHTTPProxyPort(
    handle_t hBinding,
    idl_ulong_int dwPort
    )
{
    return Srv_VmAfdRpcSetRHTTPProxyPort(
                hBinding,
                dwPort);
}

idl_ulong_int
Srv60_VmAfdRpcSetDCPort(
    handle_t hBinding,
    idl_ulong_int dwPort
    )
{
    return Srv_VmAfdRpcSetDCPort(
                hBinding,
                dwPort);
}

idl_ulong_int
Srv60_VmAfdRpcGetCMLocation(
    handle_t hBinding,
    wstring_t *ppwszCMLocation
    )
{
    return Srv_VmAfdRpcGetCMLocation(
                hBinding,
                ppwszCMLocation);
}

idl_ulong_int
Srv60_VmAfdRpcGetLSLocation(
    handle_t hBinding,
    wstring_t *ppwszLSLocation
    )
{
    return Srv_VmAfdRpcGetLSLocation(
                hBinding,
                ppwszLSLocation);
}

idl_ulong_int
Srv60_VmAfdRpcGetDCName(
    handle_t hBinding,
    wstring_t *ppwszDCName
)
{
    return Srv_VmAfdRpcGetDCName(
                hBinding,
                ppwszDCName);
}

idl_ulong_int
Srv60_VmAfdRpcSetDCName(
    handle_t hBinding,
    wstring_t pswzDCName
)
{
    return Srv_VmAfdRpcSetDCName(
                hBinding,
                pswzDCName);
}

idl_ulong_int
Srv60_VmAfdRpcGetSiteGUID(
    handle_t hBinding,
    wstring_t *ppwszGUID
)
{
    return Srv_VmAfdRpcGetSiteGUID(
                hBinding,
                ppwszGUID);
}

idl_ulong_int
Srv60_VmAfdRpcGetMachineID(
    handle_t hBinding,
    wstring_t *ppwszMachineID
)
{
    return Srv_VmAfdRpcGetMachineID(
                hBinding,
                ppwszMachineID);
}

idl_ulong_int
Srv60_VmAfdRpcSetMachineID(
    handle_t hBinding,
    wstring_t pwszMachineID
    )
{
    return Srv_VmAfdRpcSetMachineID(
                hBinding,
                pwszMachineID);
}

idl_ulong_int
Srv60_VmAfdRpcQueryAD(
    handle_t hBinding,
    wstring_t *ppwszComputer,
    wstring_t *ppwszDomain,
    wstring_t *ppwszDistinguishedName,
    wstring_t *ppwszNetbiosName
    )
{
    return Srv_VmAfdRpcQueryAD(
                hBinding,
                ppwszComputer,
                ppwszDomain,
                ppwszDistinguishedName,
                ppwszNetbiosName);
}

idl_ulong_int
Srv60_VmAfdRpcForceReplication(
    handle_t hBinding,
    wstring_t pwszServerName
    )
{
    return Srv_VmAfdRpcForceReplication(
                hBinding,
                pwszServerName);
}

idl_ulong_int
Srv60_VmAfdRpcGetPNID(
    handle_t hBinding,
    wstring_t *ppwszPNID
    )
{
    return Srv_VmAfdRpcGetPNID(
                hBinding,
                ppwszPNID);
}

idl_ulong_int
Srv60_VmAfdRpcSetPNID(
    handle_t hBinding,
    wstring_t pswzPNID
    )
{
    return Srv_VmAfdRpcSetPNID(
                hBinding,
                pswzPNID);
}

idl_ulong_int
Srv60_VmAfdRpcGetCAPath(
    handle_t hBinding,
    wstring_t *ppwszPath
    )
{
    return Srv_VmAfdRpcGetCAPath(
                hBinding,
                ppwszPath);
}

idl_ulong_int
Srv60_VmAfdRpcSetCAPath(
    handle_t hBinding,
    wstring_t pswzPath
    )
{
    return Srv_VmAfdRpcSetCAPath(
                hBinding,
                pswzPath);
}

idl_ulong_int
Srv60_VmAfdRpcTriggerRootCertsRefresh(
    handle_t hBinding
    )
{
    return Srv_VmAfdRpcTriggerRootCertsRefresh(
                hBinding);
}

idl_ulong_int
Srv60_VecsRpcCreateCertStore(
    handle_t hBinding,
    wstring_t pwszStoreName,
    wstring_t pwszPassword,
    vecs_store_handle60_t *ppStore
    )
{
    return Srv_VecsRpcCreateCertStore(
                hBinding,
                pwszStoreName,
                pwszPassword,
                ppStore);
}

idl_ulong_int
Srv60_VecsRpcOpenCertStore(
    handle_t hBinding,
    wstring_t pwszStoreName,
    wstring_t pwszPassword,
    vecs_store_handle60_t *ppStore
    )
{
    return Srv_VecsRpcOpenCertStore(
                hBinding,
                pwszStoreName,
                pwszPassword,
                ppStore);
}

idl_ulong_int
Srv60_VecsRpcCloseCertStore(
    handle_t hBinding,
    vecs_store_handle60_t *ppStore
    )
{
    return Srv_VecsRpcCloseCertStore(
                hBinding,
                ppStore);
}

idl_ulong_int
Srv60_VecsRpcEnumCertStore(
    handle_t hBinding,
    PVMAFD_CERT_STORE_ARRAY *ppCertStoreArray
    )
{
    return Srv_VecsRpcEnumCertStore(
                hBinding,
                ppCertStoreArray);
}

idl_ulong_int
Srv60_VecsRpcDeleteCertStore(
    handle_t hBinding,
    wstring_t pwszStoreName
    )
{
    return Srv_VecsRpcDeleteCertStore(
                hBinding,
                pwszStoreName);
}

idl_ulong_int
Srv60_VecsRpcBeginEnumCerts(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    idl_ulong_int dwMaxCount,
    idl_ulong_int dwInfoLevel,
    idl_ulong_int *dwLimit,
    vecs_entry_enum_handle60_t *ppEnumContext
    )
{
    return Srv_VecsRpcBeginEnumCerts(
                hBinding,
                pStore,
                dwMaxCount,
                dwInfoLevel,
                dwLimit,
                ppEnumContext);
}

idl_ulong_int
Srv60_VecsRpcEnumCerts(
    handle_t hBinding,
    vecs_entry_enum_handle60_t pEnumContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    return Srv_VecsRpcEnumCerts(
                hBinding,
                pEnumContext,
                ppCertContainer);
}

idl_ulong_int
Srv60_VecsRpcEndEnumCerts(
    handle_t hBinding,
    vecs_entry_enum_handle60_t *ppEnumContext
    )
{
    return Srv_VecsRpcEndEnumCerts(
                hBinding,
                ppEnumContext);
}

idl_ulong_int
Srv60_VecsRpcGetEntryCount(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    idl_ulong_int *pdwSize
    )
{
    return Srv_VecsRpcGetEntryCount(
                hBinding,
                pStore,
                pdwSize);
}

idl_ulong_int
Srv60_VecsRpcGetCertificateByAlias(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pAlias,
    wstring_t *pCertificate
    )
{
    return Srv_VecsRpcGetCertificateByAlias(
                hBinding,
                pStore,
                pAlias,
                pCertificate);
}

idl_ulong_int
Srv60_VecsRpcGetPrivateKeyByAlias(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pszAlias,
    wstring_t pszPassword,
    wstring_t *ppszPrivateKey
    )
{
    return Srv_VecsRpcGetPrivateKeyByAlias(
                hBinding,
                pStore,
                pszAlias,
                pszPassword,
                ppszPrivateKey);
}

idl_ulong_int
Srv60_VecsRpcAddCertificate(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    idl_ulong_int entryType,
    wstring_t pszAlias,
    wstring_t pszCertificate,
    wstring_t pszPrivateKey,
    wstring_t pszPassword,
    idl_ulong_int bAutoRefresh
    )
{
    return Srv_VecsRpcAddCertificate(
                hBinding,
                pStore,
                entryType,
                pszAlias,
                pszCertificate,
                pszPrivateKey,
                pszPassword,
                bAutoRefresh);
}

idl_ulong_int
Srv60_VecsRpcGetEntryTypeByAlias(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pszAlias,
    idl_ulong_int *pEntryType
    )
{
    return Srv_VecsRpcGetEntryTypeByAlias(
                hBinding,
                pStore,
                pszAlias,
                pEntryType);
}

idl_ulong_int
Srv60_VecsRpcGetEntryDateByAlias(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pszAlias,
    idl_ulong_int *pdwDate
    )
{
    return Srv_VecsRpcGetEntryDateByAlias(
                hBinding,
                pStore,
                pszAlias,
                pdwDate);
}

idl_ulong_int
Srv60_VecsRpcGetEntryByAlias(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pszAlias,
    idl_ulong_int dwInfoLevel,
    PVMAFD_CERT_ARRAY *ppCertArray
    )
{
    return Srv_VecsRpcGetEntryByAlias(
                hBinding,
                pStore,
                pszAlias,
                dwInfoLevel,
                ppCertArray);
}

idl_ulong_int
Srv60_VecsRpcDeleteCertificate(
    handle_t hBinding,
    vecs_store_handle60_t pStore,
    wstring_t pszAlias
    )
{
    return Srv_VecsRpcDeleteCertificate(
                hBinding,
                pStore,
                pszAlias);
}

void
vecs_store_handle60_t_rundown(void *ctx)
{
    if (ctx)
    {
        PVECS_SERV_STORE pStore = (PVECS_SERV_STORE)ctx;

        VecsSrvReleaseCertStore(pStore);
    }
}

void
vecs_entry_enum_handle60_t_rundown(void *ctx)
{
    if (ctx)
    {
        PVECS_SRV_ENUM_CONTEXT pContext = (PVECS_SRV_ENUM_CONTEXT)ctx;

        VecsSrvReleaseEnumContext(pContext);
    }
}

