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




#ifdef __cplusplus
extern "C" {
#endif

/* config.c */

DWORD
VmAfSrvGetDomainName(
    PWSTR*   ppwszDomain        /*    OUT */
    );

DWORD
VmAfSrvGetDomainNameA(
    PSTR* ppszDomain        /*    OUT */
    );

DWORD
VmAfSrvSetDomainName(
    PWSTR    pwszDomain        /* IN     */
    );

DWORD
VmAfSrvSetDomainNameA(
    PSTR    pszDomain        /* IN     */
    );

DWORD
VmAfSrvSetSiteName(
    PWSTR    pwszSiteName        /* IN     */
    );

DWORD
VmAfSrvGetSiteName(
    PWSTR*   ppwszSiteName        /*    OUT */
    );

DWORD
VmAfSrvGetDomainState(
    PVMAFD_DOMAIN_STATE pDomainState        /*    OUT */
    );

DWORD
VmAfSrvSetDomainState(
    VMAFD_DOMAIN_STATE domainState       /* IN     */
    );

DWORD
VmAfSrvGetDomainState(
    VMAFD_DOMAIN_STATE*  pDomainState  /*    OUT */
    );

DWORD
VmAfSrvGetLDU(
    PWSTR*   ppwszLDU       /*    OUT */
    );

DWORD
VmAfSrvSetLDU(
    PWSTR    pwszLDU           /* IN     */
    );

DWORD
VmAfSrvGetRHTTPProxyPort(
    PDWORD   pdwPort          /* OUT     */
    );

DWORD
VmAfSrvSetRHTTPProxyPort(
    DWORD    dwPort           /* IN     */
    );

DWORD
VmAfSrvGetDCPort(
    PDWORD   pdwPort          /* OUT     */
    );

DWORD
VmAfSrvSetDCPort(
    DWORD    dwPort           /* IN     */
    );

DWORD
VmAfSrvGetCMLocation(
    PWSTR*   ppwszCMLocation  /*    OUT */
    );

DWORD
VmAfSrvGetLSLocation(
    PWSTR*   ppwszLSLocation  /*    OUT */
    );

DWORD
VmAfSrvGetDCName(
    PWSTR*   ppwszDCName    /*    OUT */
    );

DWORD
VmAfSrvSetDCName(
    PWSTR    pwszDCName     /* IN     */
    );

DWORD
VmAfSrvGetMachineAccountInfo(
    PWSTR*   ppwszAccount,    /*    OUT */
    PWSTR*   ppwszPassword,   /*    OUT */
    PWSTR*   ppwszAccountDN,  /*    OPTIONAL  */
    PWSTR*   ppwszMachineGUID /*    OPTIONAL  */
    );

DWORD
VecsSrvGetDBBasePath(
    PSTR *ppszDbPath
    );

DWORD
VmAfSrvSetMachineSSLCert(
    PWSTR    pwszPrivateKey,    /* IN      */
    DWORD    dwKeyLength,       /* IN     */
    PBYTE    pPublicKey         /* IN     */
    );

DWORD
VmAfSrvGetMachineSSLCert(
    PWSTR*   ppwszPrivateKey,  /*    OUT */
    PBYTE*   ppPublicKey,      /*    OUT */
    PDWORD   pdwKeyLength      /* IN OUT */
    );

DWORD
VmAfSrvGetSiteGUID(
    PWSTR*   ppwszGUID        /*    OUT          */
    );

DWORD
VmAfSrvPromoteVmDir(
    PWSTR    pwszLotusServerName,/* IN              */
    PWSTR    pwszDomainName,     /* IN     OPTIONAL */
    PWSTR    pwszUserName,       /* IN              */
    PWSTR    pwszPassword,       /* IN              */
    PWSTR    pwszSiteName,       /* IN     OPTIONAL */
    PWSTR    pwszPartnerHostName /* IN     OPTIONAL */
    );

DWORD
VmAfSrvDemoteVmDir(
    PWSTR    pwszServerName,    /* IN              */
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    );

DWORD
VmAfSrvJoinVmDir(
    PWSTR    pwszServerName,     /* IN              */
    PWSTR    pwszUserName,       /* IN              */
    PWSTR    pwszPassword,       /* IN              */
    PWSTR    pwszMachineName,    /* IN              */
    PWSTR    pwszDomainName,     /* IN              */
    PWSTR    pwszOrgUnit         /* IN              */
    );

DWORD
VmAfSrvJoinVmDir2(
    PWSTR            pwszDomainName,     /* IN              */
    PWSTR            pwszUserName,       /* IN              */
    PWSTR            pwszPassword,       /* IN              */
    PWSTR            pwszMachineName,    /* IN     OPTIONAL */
    PWSTR            pwszOrgUnit,        /* IN     OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags             /* IN              */
    );

DWORD
VmAfSrvLeaveVmDir(
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword,      /* IN              */
    DWORD    dwLeaveFlags       /* IN              */
    );

DWORD
VmAfSrvCreateComputerAccount(
    PCWSTR   pwszUserName,      /* IN            */
    PCWSTR   pwszPassword,      /* IN            */
    PCWSTR   pwszMachineName,   /* IN            */
    PCWSTR   pwszOrgUnit,       /* IN   OPTIONAL */
    PWSTR*   pszOutPassword     /* OUT           */
    );

DWORD
VmAfSrvJoinAD(
    PWSTR    pwszUserName,       /* IN              */
    PWSTR    pwszPassword,       /* IN              */
    PWSTR    pwszDomainName,     /* IN              */
    PWSTR    pwszOrgUnit         /* IN              */
    );

DWORD
VmAfSrvLeaveAD(
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    );

DWORD
VmAfSrvQueryAD(
    PWSTR   *ppwszComputer,          /*    OUT          */
    PWSTR   *ppwszDomain,            /*    OUT          */
    PWSTR   *ppwszDistinguishedName, /*    OUT          */
    PWSTR   *ppwszNetbiosName        /*    OUT          */
    );

DWORD
VmAfSrvForceReplication(
    PWSTR pwszServerName         /* IN               */
    );

UINT32
VmAfSrvOpenCertStore(
    DWORD    type,
    DWORD *  hStore
    );

UINT32
VmAfSrvCloseCertStore(
    DWORD    hStore
    );

UINT32
VmAfSrvAddCertificate(
    UINT32   hStore,
    PWSTR    pszAlias,
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey,
    UINT32   uAutoRefresh
    );

UINT32
VmAfSrvDeleteCertificate(
    DWORD    hStore,
    PWSTR    pszCertificate
    );

UINT32
VmAfSrvEnumCertificates(
    DWORD                   hStore,
    DWORD                   dwStartIndex,
    DWORD                   dwNumCertificates,
    PVMAFD_CERT_CONTAINER** ppCertContainer
    );

UINT32
VmAfSrvVerifyCertificateTrust(
    PWSTR    pszCertificate
    );

UINT32
VmAfSrvGetCertificateChain(
    PWSTR                  pszCertificate,
    PVMAFD_CERT_CONTAINER* ppCertContainer
    );

UINT32
VmAfSrvGetCertificateByAlias(
    DWORD                  hStore,
    PWSTR                  pszAlias,
    PVMAFD_CERT_CONTAINER* ppCertContainer
    );

UINT32
VmAfSrvSetPassword(
    PWSTR    pszAliasName,
    PWSTR    pszPassword
    );

UINT32
VmAfSrvGetPassword(
    PWSTR    pszAliasName,
    PWSTR *  ppszPassword
    );

DWORD
VmAfSrvSetSSLCertificate(
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey
    );

DWORD
VmAfSrvGetSSLCertificate(
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey
    );

DWORD
VmAfSrvCfgSetMachineID(
    PCWSTR pszMachineID
    );

DWORD
VmAfSrvCfgGetMachineID(
    PWSTR *ppszMachineID
    );

DWORD
VmAfSrvChangePNID(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    );

/* dcfinder.c */

DWORD
VmAfdGetDomainController(
    PCSTR pszDomain,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PSTR* ppszHostname,
    PSTR* ppszDCAddress
    );

DWORD
VmAfdGetDomainControllerList(
    PCSTR pszDomain,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList,
    PDWORD pdCount
    );

/* dns.c */

DWORD
VmAfSrvConfigureDNSW(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    );

DWORD
VmAfSrvConfigureDNSA(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword
    );

DWORD
VmAfSrvUnconfigureDNSW(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    );

DWORD
VmAfSrvUnconfigureDNSA(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword
    );

DWORD
VmAfSrvGetIPAddressesWrap(
    VMDNS_IP4_ADDRESS** ppV4Addresses,
    PDWORD              pdwNumV4Address,
    VMDNS_IP6_ADDRESS** ppV6Addresses,
    PDWORD              pdwNumV6Address
    );

/* init.c */

DWORD
VmAfdInit(
    VOID
    );

DWORD
VmAfdInitLoop(
    PVMAFD_GLOBALS pGlobals
    );

int
LoadServerGlobals();

/* regconfig.c */

DWORD
VmAfdSrvUpdateConfig(
    PVMAFD_GLOBALS pGlobals
    );

/* rpc.c */

DWORD
VmAfdRpcServerStartListen(
    VOID
);

DWORD
VmAfdRpcServerStopListen(
    VOID
);

DWORD
VmAfdRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
);

DWORD
VmAfdRpcServerUseProtSeq(
    PCSTR pszProtSeq
);

DWORD
VmAfdRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
);

DWORD
VmAfdRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
);

DWORD
VmAfdRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
);

DWORD
VmAfdRpcServerRegisterAuthInfo(
    VOID
);

DWORD
VmAfdRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
);

DWORD
VmAfdRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
);

DWORD
VmAfdRpcServerAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmAfdRpcServerFreeMemory(
    PVOID pMemory
    );

DWORD
VmAfdRpcServerAllocateStringW(
    PCWSTR  pwszSrc,
    PWSTR* ppwszDst
    );

DWORD
VmAfdRpcServerAllocateStringArrayW(
    DWORD dwCount,
    PCWSTR *pwszSrc,
    PWSTR **ppwszDst
    );

DWORD
VmAfdRpcClientAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmAfdRpcClientFreeMemory(
    PVOID pMemory
    );

VOID
VmAfdRpcServerFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    );

VOID
VmAfdRpcServerFreeStringArrayW(
    PWSTR *ppwszStrArray,
    DWORD dwCount
    );

DWORD
CdcRpcServerAllocateDCInfoW(
    PCDC_DC_INFO_W  pCdcInfo,
    PCDC_DC_INFO_W  *ppRpcCdcInfo
    );

DWORD
CdcRpcServerFreeDCInfoW(
    PCDC_DC_INFO_W  pCdcInfo
    );

VOID
VmAfdRpcServerFreeStringA(
    PSTR pszStr
    );

VOID
VmAfdRpcClientFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    );

VOID
VmAfdRpcClientFreeStringA(
    PSTR pszStr
    );

DWORD
VmAfdRpcGetErrorCode(
    dcethread_exc* pException
    );

DWORD
VmAfdRpcServerCheckAccess(
    rpc_binding_handle_t hBinding,
    DWORD dwRpcFlags
    );

/* service.c */

DWORD
VmAfdRpcServerInit(
    VOID
    );

VOID
VmAfdRpcServerShutdown(
    VOID
    );

DWORD
VmAfdRpcAuthCallback(
    PVOID Context
);

#ifndef _WIN32

/* signals. c*/

VOID
VmAfdBlockSelectedSignals(
    VOID
    );

DWORD
VmAfdHandleSignals(
    VOID
    );

DWORD
VmAfdInitSignalThread(
    PVMAFD_GLOBALS pGlobals
    );

#endif

/* parseargs.c */

DWORD
VmAfdParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableConsole
);

VOID
ShowUsage(
    PSTR pName
);

/* shutdown.c */

VOID
VmAfdServerShutdown(
    VOID
      );

/* utils.c */

VOID
VmAfdSrvSetStatus(
    VMAFD_STATUS status
    );

VMAFD_STATUS
VmAfdSrvGetStatus(
    VOID
    );

DWORD
VmAfdSrvGetDomainState(
    PVMAFD_DOMAIN_STATE
    );

DWORD
VmAfdGetMachineInfo(
    PVMAFD_REG_ARG *ppArgs
    );

VOID
VmAfdFreeRegArgs(
    PVMAFD_REG_ARG pArgs
    );

DWORD
VmAfdConnectLdapWithMachineAccount(
    LDAP** ppLotus
    );

DWORD
VmAfdCheckDomainFunctionalLevel(
    int nMinMajor,
    int nMinMinor
    );

/* krbconfig.c */

DWORD
VmAfdInitKrbConfig(
    PCSTR pszFileName,
    PVMAFD_KRB_CONFIG *ppKrbConfig
    );

DWORD
VmAfdDestroyKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig
    );

DWORD
VmAfdBackupKrbConfig(
    PVMAFD_KRB_CONFIG
    );

DWORD
VmAfdBackupCopyKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig
    );

DWORD
VmAfdAddKdcKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszKdc
    );

DWORD
VmAfdSetDefaultRealmKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszDefaultRealm
    );

DWORD
VmAfdSetDefaultKeytabNameKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszDefaultKeytabName
    );

DWORD
VmAfdWriteKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig
    );

// certutil.c

DWORD
VecsSrvRpcAllocateCertStoreArray(
    PWSTR* ppwszStoreNames,
    DWORD  dwCount,
    PVMAFD_CERT_STORE_ARRAY* ppCertStoreArray
    );

VOID
VecsSrvRpcFreeCertStoreArray(
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray
    );

DWORD
VecsRpcAllocateCertArray(
    PVMAFD_CERT_ARRAY  pSrc,
    PVMAFD_CERT_ARRAY* ppDst
    );

VOID
VecsSrvRpcFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    );

VOID
VecsSrvFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    );

VOID
VecsSrvFreeCertContainer(
    PVMAFD_CERT_CONTAINER pContainer
    );

VOID
VecsSrvRpcFreeCRLData(
    PVMAFD_CRL_DATA pCRLData
    );

VOID
VecsSrvFreeCRLMetaDataArray(
    PVMAFD_CRL_FILE_METADATA pMetaData,
    DWORD                    dwCount
    );

VOID
VecsSrvFreeCRLData(
    PVMAFD_CRL_DATA pCRLData
    );

DWORD
VmAfdSrvRpcAllocateCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER  pSrcData,
    PVMAFD_CRL_METADATA_CONTAINER* ppDstData
    );

VOID
VecsSrvRpcFreeCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER pContainer
    );

VOID
VecsSrvFreeCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER pContainer
    );

VOID
VecsSrvFreeCRLArray(
    PVMAFD_CRL_METADATA_CONTAINER pCRLArray,
    DWORD dwSize
    );

DWORD
VmAfSrvGetPNID(
    PWSTR* ppwszPNID
    );

DWORD
VmAfSrvSetPNID(
    PWSTR pwszPNID
    );

DWORD
VmAfSrvGetCAPath(
    PWSTR* ppwszPath
    );

DWORD
VmAfSrvSetCAPath(
    PWSTR pwszPath
    );

DWORD
VmAfSrvSetDCActPassword(
    PWSTR    pwszPassword   /* IN     */
    );

DWORD
VmAfSrvSetMachineID(
    PWSTR pwszGUID
    );

DWORD
VmAfdLDAPConnect(
    PSTR pszHostName,
    DWORD  Port,
    PCSTR pszUpn,
    PCSTR pszPassword,
    LDAP** ppLotus
    );

DWORD
VmAfdInitCertificateThread(
    PVMAFD_THREAD* ppThread
    );

VOID
VmAfdShutdownCertificateThread(
    PVMAFD_THREAD pThread
    );

VOID
VmAfdLdapClose(
    LDAP* pHandle
    );

// rootfetch.c
DWORD
VmAfdWakeupCertificateUpdatesThr(
    PVMAFD_THREAD pCertThread,
    BOOLEAN forceFlush
    );

DWORD
VmAfdRootFetchTask(
    BOOLEAN bLogOnDuplicate
    );


// rootflush.c

DWORD
VmAfdInitRootFlushThread(
    PVMAFD_THREAD* ppThread
    );

VOID
VmAfdShutdownRootFlushThread(
    PVMAFD_THREAD pThread
    );

// passrefresh.c

DWORD
VmAfdInitPassRefreshThread(
    PVMAFD_THREAD* ppThread
    );

VOID
VmAfdShutdownPassRefreshThread(
    PVMAFD_THREAD pThread
    );

//IPC

typedef struct _IDM_USER_IDENTITY_
{
    unsigned long clientProcessID;//TODO change to ULONG
} IDM_USER_IDENTITY, *PSECURITY_IDENTITY;


//ipcserver.c

DWORD
VmAfdIpcServerInit(
    VOID
    );

VOID
VmAfdIpcServerShutDown(
    VOID
    );

//ipcapihandler.c

DWORD
VmAfdLocalAPIHandler(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    DWORD * pdwResponseSize
    );

//ipclocalapi.c

DWORD
VecsIpcCreateCertStore(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcOpenCertStore (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcAddEntry(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcDeleteEntry(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcDeleteCertStore(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcSetPermission(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcRevokePermission(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcGetPermissions(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcChangeOwner (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcBeginEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcGetEntryByAlias (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcGetKeyByAlias (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcEnumStores (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcGetEntryCount (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcCloseCertStore (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VecsIpcEndEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetDomainName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetDomainName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetDomainState(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetLDU(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetLDU(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetRHTTPProxyPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetRHTTPProxyPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetDCPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetCMLocation(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetLSLocation(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetMachineAccountInfo(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetSiteGUID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetSiteName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetMachineID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetMachineID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcPromoteVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcDemoteVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcJoinValidateCredentials(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfSrvJoinValidateCredentials(
    PWSTR pwszDomainName,       /* IN            */
    PWSTR pwszUserName,         /* IN            */
    PWSTR pwszPassword          /* IN            */
    );

DWORD
VmAfdIpcJoinVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcJoinVmDir2(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcLeaveVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcCreateComputerAccount(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcJoinAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcLeaveAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcQueryAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcForceReplication(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetPNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetPNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetCAPath(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcSetCAPath(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcTriggerRootCertsRefresh(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcRefreshSiteName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcConfigureDNS(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcGetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcGetCurrentState(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcForceRefreshCache(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcEnumDCEntries(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcEnableDefaultHA(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcEnableLegacyModeHA(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
CdcIpcGetDCStatusInfo(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcPostHeartbeatStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetHeartbeatStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcChangePNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmAfdIpcGetDCList(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

//rpcserv_internal.c

DWORD
VmAfdAddCertificateInternal(
    UINT32 dwStoreType,
    PWSTR pwszAlias,
    PWSTR pwszCertificate,
    PWSTR pwszPrivateKey,
    UINT32 uAutoRefresh
);


//vecsserviceapi.c

PVECS_SERV_STORE
VecsSrvAcquireCertStore(
    PVECS_SERV_STORE pStore
    );

VOID
VecsSrvReleaseCertStore(
    PVECS_SERV_STORE pStore
    );

PVECS_SRV_ENUM_CONTEXT
VecsSrvAcquireEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    );

VOID
VecsSrvReleaseEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    );

DWORD
VecsSrvCreateCertStore(
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_SERV_STORE *ppStore
        );

DWORD
VecsSrvOpenCertStore(
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_SERV_STORE *ppStore
        );

DWORD
VecsSrvCloseCertStore(
        PVECS_SERV_STORE pStore
        );

DWORD
VecsSrvEnumCertStore(
    PWSTR ** ppszStoreNameArray,
    PDWORD pdwCount
    );

DWORD
VecsSrvDeleteCertStore(
    PCWSTR pwszStoreName
    );

DWORD
VecsSrvAllocateCertEnumContext(
    PVECS_SERV_STORE        pStore,
    DWORD                   dwMaxCount,
    ENTRY_INFO_LEVEL        infoLevel,
    PVECS_SRV_ENUM_CONTEXT* ppContext
    );

DWORD
VecsSrvEnumCertsInternal(
    PVECS_SRV_ENUM_CONTEXT pContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsSrvEnumCerts(
    PVECS_SRV_ENUM_CONTEXT pContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsSrvGetEntryCount(
    PVECS_SERV_STORE pStore,
    PDWORD pdwSize
    );

DWORD
VecsSrvGetCertificateByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PWSTR *ppszCertificate
    );

DWORD
VecsSrvGetPrivateKeyByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PWSTR pszPassword,
    PWSTR *ppszPrivateKey
    );

DWORD
VecsSrvValidateEntryType(
    UINT32 uInputEntryType,
    CERT_ENTRY_TYPE *pOutputEntryType
    );

DWORD
VecsSrvAddCertificate(
    PVECS_SERV_STORE pStore,
    CERT_ENTRY_TYPE cEntryType,
    PWSTR pszAliasName,
    PWSTR pszCertificate,
    PWSTR pszPrivateKey,
    PWSTR pszPassword,
    BOOLEAN bAutoRefresh
    );

DWORD
VecsSrvGetEntryTypeByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    CERT_ENTRY_TYPE *pEntryType
    );

DWORD
VecsSrvGetEntryDateByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PDWORD pdwDate
    );

DWORD
VecsSrvGetEntryByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    ENTRY_INFO_LEVEL infoLevel,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsSrvDeleteCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName
    );

DWORD
VecsSrvValidateInfoLevel(
    UINT32 uInfoLevel,
    ENTRY_INFO_LEVEL *pInfoLevel
    );

DWORD
VmAfSrvGetComputerAccountID(
    PWSTR    pwszMachineName, /*    IN           */
    PWSTR*   ppwszGUID        /*    OUT          */
    );

DWORD
VecsSrvValidateAddEntryInput(
    CERT_ENTRY_TYPE entryType,
    PCWSTR pwszCertificate,
    PCWSTR pwszPrivateKey
    );

DWORD
VecsSrvFlushRootCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    BOOLEAN bLogOnDuplicate
    );

DWORD
VecsSrvFlushMachineSslCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    PWSTR pszCanonicalKeyPEM,
    BOOL  bLogOnError
    );

DWORD
VecsSrvFlushCrl(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    BOOLEAN bLogOnDuplicate
    );

DWORD
VecsFillVacantFileSlot(
    PCSTR pszPath
    );

/*DWORD
VecsSrvComputeCertAlias(
     PWSTR pszCertificate,
     PWSTR *ppszHash
    );*/

DWORD
VecsSrvFlushCertsToDisk(
    VOID
    );

DWORD
VecsSrvFlushSSLCertFromDB(
    BOOL bLogOnError
    );


//ipcmarshalhelper.c
//
DWORD
VmAfdEncodeVecsStore (
        PVECS_SERV_STORE pStore,
        PBYTE *ppStoreBlob
        );

DWORD
VmAfdDecodeVecsStore (
    PBYTE pStoreBlob,
    DWORD dwSizeOfBlob,
    PVECS_SERV_STORE *ppStore
    );

DWORD
VmAfdEncodeEnumContext(
    PVECS_SRV_ENUM_CONTEXT pEnumContext,
    PBYTE *ppEnumContextBlob
    );

DWORD
VmAfdDecodeEnumContext (
    PBYTE pEnumContextBlob,
    DWORD dwSizeOfBlob,
    PVECS_SRV_ENUM_CONTEXT *ppEnumContext
    );

DWORD
VmAfdEncodeVecsStoreHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PBYTE *ppStoreBlob
                           );

DWORD
VmAfdDecodeVecsStoreHandle (
                            PBYTE pStoreBlob,
                            DWORD dwSizeOfBlob,
                            PVECS_SRV_STORE_HANDLE *ppStore
                           );
DWORD
VmAfdEncodeEnumContextHandle(
                    PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext,
                    PBYTE *ppEnumContextBlob
                    );

DWORD
VmAfdDecodeEnumContextHandle(
                      PBYTE pEnumContextBlob,
                      DWORD dwBlobSize,
                      PVECS_SRV_ENUM_CONTEXT_HANDLE *ppEnumContext
                      );


//authservice.c
DWORD
VecsSrvCreateCertStoreWithAuth (
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PVECS_SRV_STORE_HANDLE *ppStore
    );

DWORD
VecsSrvDeleteCertStoreWithAuth (
    PCWSTR pszStoreName,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VecsSrvSetPermission (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    UINT32 accessMask,
    VMAFD_ACE_TYPE aceType,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VecsSrvRevokePermission (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    UINT32 accessMask,
    VMAFD_ACE_TYPE aceType,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );


DWORD
VecsSrvChangeOwner (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VecsSrvEnumFilteredStores (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PWSTR **ppwszStoreNames,
    PDWORD pdwCount
    );

DWORD
VecsSrvGetPermissions (
    PVECS_SRV_STORE_HANDLE pStore,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PWSTR *ppszOwnerName,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppPermissions
    );


// ldap.c

DWORD
VmAfdGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    );

DWORD
VmAfdQueryCACerts(
    LDAP* pLotus,
    PCSTR pszCACN,
    BOOL  bDetail,
    PVMAFD_CA_CERT_ARRAY* ppCACertificates
    );

DWORD
VmAfdGetDomainFunctionLevel(
    LDAP* pLotus,
    PSTR* ppDomainFunctionLevel
    );

DWORD
VecsSrvOpenCertStoreWithAuth(
                             PCWSTR pszStoreName,
                             PCWSTR pszPassword,
                             PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
                             PVECS_SRV_STORE_HANDLE *ppStore
                            );

DWORD
VecsSrvCloseCertStoreHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PVM_AFD_CONNECTION_CONTEXT pConnectionContext
                            );

DWORD
VecsSrvEnumCertsHandle(
    PVECS_SRV_ENUM_CONTEXT_HANDLE pContext,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );



DWORD
VecsSrvAllocateCertEnumContextHandle(
    PVECS_SRV_STORE_HANDLE  pStore,
    DWORD                   dwMaxCount,
    ENTRY_INFO_LEVEL        infoLevel,
    PVECS_SRV_ENUM_CONTEXT_HANDLE* ppContext
    );

DWORD
VecsSrvEndEnumContextHandle (
    PVECS_SRV_ENUM_CONTEXT_HANDLE  pEnumContext,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VecsSrvValidateAceType(
    DWORD dwAceType,
    VMAFD_ACE_TYPE *pAceType
    );

DWORD
VmAfdGetDefaultDomainName(
    LDAP* pLotus,
    PSTR* ppDomainName
    );

//storehash_util.c
//
DWORD
VmAfdGetStoreHandle (
                      PWSTR pszStoreName,
                      PWSTR pszPassword,
                      PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                      PVECS_SRV_STORE_HANDLE *ppStore
                    );

PVECS_SRV_STORE_HANDLE
VmAfdAcquireStoreHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle
                        );

VOID
VmAfdReleaseStoreHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle
                        );

BOOL
VmAfdIsValidStoreHandle (
                PVECS_SRV_STORE_HANDLE pStore,
                PVM_AFD_SECURITY_CONTEXT pSecurityContext
              );

DWORD
VmAfdGetStoreStatus(
                    PVECS_SRV_STORE_HANDLE pStore,
                    STORE_MAP_ENTRY_STATUS *pStoreStatus
                   );

DWORD
VmAfdGetStoreFromHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle,
                          PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                          PVECS_SERV_STORE *ppStore
                        );
DWORD
VmAfdGetSecurityDescriptorFromHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
                           );


DWORD
VmAfdSetSecurityDescriptorForHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
                             );

VOID
VmAfdTearDownStoreHashMap (
                            VOID
                          );

VOID
VmAfdDeleteStoreEntry (
                        PVECS_SRV_STORE_HANDLE pStoreHandle
                      );

BOOL
VmAfdCanDeleteStore (
                      PVECS_SRV_STORE_HANDLE pStoreHandle
                    );

VOID
VmAfdCloseStoreHandle (
                        PVECS_SRV_STORE_HANDLE pStoreHandle,
                        PVM_AFD_SECURITY_CONTEXT pSecurityContext
                      );


// utils.c

BOOLEAN
VmAfdSrvIsValidGUID(
    PCWSTR pwszGUID
    );

DWORD
_VmAfdConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
    );

DWORD
_VmAfdConfigGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
    );


DWORD
_VmAfdConfigSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue        /* IN */
    );

VMAFD_STATUS
VmAfdStatus(
    VOID
    );

UINT64
VmAfdGetTimeInMilliSec(
    VOID
    );

DWORD
VmAfdCircularBufferCreate(
    DWORD dwCapacity,
    DWORD dwElementSize,
    PVMAFD_CIRCULAR_BUFFER *ppCircularBuffer
    );

VOID VmAfdCircularBufferFree(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmAfdCircularBufferReset(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmAfdCircularBufferGetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pdwCapacity
    );

DWORD
VmAfdCircularBufferSetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCapacity
    );

PVOID
VmAfdCircularBufferGetNextEntry(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmAfdSuperLoggingGetEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *pEnumerationCookie,
    DWORD dwCount,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    );

DWORD
VmAfdCircularBufferSelectElements(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCount, // 0 implies that we want all elements.
    CIRCULAR_BUFFER_SELECT_CALLBACK Callback,
    PVOID pContext
    );

DWORD
VmAfdCircularBufferGetSize(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pSize
    );

// vmdir.c


DWORD
VmAfSrvDirGetMachineId(
    VMAFD_DOMAIN_STATE domainState, /* IN              */
    PWSTR*             ppwszGUID    /*    OUT          */
    );

DWORD
VmAfSrvDirSetMachineId(
    VMAFD_DOMAIN_STATE domainState,
    PCWSTR   pwszGUID        /* IN */
    );

DWORD
VmAfSrvDirGetMemberships(
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    );

BOOLEAN
VmAfSrvDirIsMember(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    );

VOID
VmAfSrvDirFreeMemberships(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VmAfSrvRefreshSiteName(
    VOID
    );

DWORD
VmAfSrvGetSiteNameForDC(
    PWSTR pwszDCName,
    PWSTR* ppwszSiteName
    );

//cdcthread.c

DWORD
CdcInitThreadContext(
    PCDC_THREAD_CONTEXT *ppThrContext
    );

VOID
CdcShutdownThread(
    PCDC_THREAD_CONTEXT pThrContext,
    PSTR pszThreadName
    );

DWORD
CdcWakeupThread(
    PCDC_THREAD_CONTEXT pThrContext
    );

BOOLEAN
CdcToShutdownThread(
    PCDC_THREAD_CONTEXT pThrContext
    );

VOID
CdcSetShutdownFlagThread(
    PCDC_THREAD_CONTEXT pThrContext
    );

//cdcservice.c

DWORD
CdcInitCdcService(
    PCDC_CONTEXT *ppContext
    );

VOID
CdcShutdownCdcService(
    PCDC_CONTEXT pCdcContext
    );

DWORD
CdcSrvInitDefaultHAMode(
    PCDC_CONTEXT pCdcContext
    );

DWORD
CdcSrvShutdownDefaultHAMode(
    PCDC_CONTEXT pCdcContext
    );

DWORD
CdcSrvEnableDefaultHA(
    PCDC_CONTEXT pContext
    );

DWORD
CdcSrvEnableLegacyModeHA(
    PCDC_CONTEXT pContext
    );

DWORD
CdcSrvForceRefreshCache(
    PCDC_CONTEXT pContext
    );

DWORD
CdcSrvGetDCName(
    PCWSTR pszDomain,
    DWORD  dwFlags,
    PCDC_DC_INFO_W *ppAffinitizedDC
    );

DWORD
CdcSrvGetCurrentState(
    PCDC_DC_STATE pCdcState
    );

DWORD
VmAfSrvGetAffinitizedDC(
    PWSTR* ppwszDCName
    );


DWORD
CdcSrvEnumDCEntries(
    PWSTR **pppszEntryNames,
    PDWORD pdwCount
    );

DWORD
CdcSrvGetDCStatusInfo(
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PCDC_DC_STATUS_INFO_W *ppCdcStatusInfo,
    PVMAFD_HB_STATUS_W    *ppHeartbeatStatus
    );

DWORD
CdcSrvWakeupStateMachine(
    PCDC_CONTEXT pCdcContext
    );

DWORD
CdcRpcAllocateDCStatusInfo(
     PCDC_DC_STATUS_INFO_W pDCStatusInfo,
     PCDC_DC_STATUS_INFO_W *ppRpcDCStatusInfo
     );

VOID
CdcRpcFreeDCStatuInfo(
    PCDC_DC_STATUS_INFO_W  pRpcDCStatusInfo
    );

DWORD
VmAfSrvGetRegKeySecurity(
    PCSTR    pszSubKey,
    PSTR*    ppszSecurity
    );

//cdcstatemachine.c


DWORD
CdcInitStateMachine(
      PCDC_STATE_MACHINE_CONTEXT *ppStateMachine
      );

DWORD
CdcRunStateMachine(
      PCDC_STATE_MACHINE_CONTEXT pStateMachine,
      PCDC_DC_STATE              pCdcEndingState
      );

DWORD
CdcEnableStateMachine(
     PCDC_STATE_MACHINE_CONTEXT pStateMachine
     );

DWORD
CdcDisableStateMachine(
     PCDC_STATE_MACHINE_CONTEXT pStateMachine
     );

VOID
CdcShutdownStateMachine(
      PCDC_STATE_MACHINE_CONTEXT pStateMachine
      );

DWORD
CdcWakeupStateMachine(
      PCDC_STATE_MACHINE_CONTEXT pStateMachine,
      BOOLEAN                    bWaitForCompletion
      );


//cdcupdate.c

DWORD
CdcInitCdcCacheUpdate(
      PCDC_CACHE_UPDATE_CONTEXT *ppUpdateContext
      );

VOID
CdcShutdownCdcCacheUpdate(
      PCDC_CACHE_UPDATE_CONTEXT pUpdateContext
      );

DWORD
CdcWakeupCdcCacheUpdate(
      PCDC_CACHE_UPDATE_CONTEXT pDCCaching,
      BOOLEAN                   bPurgeRefresh,
      BOOLEAN                   bWaitForRefresh
      );

//heartbeat.c

DWORD
VmAfSrvInitHeartbeatTable(
    VOID
    );

DWORD
VmAfSrvPostHeartbeat(
    PCWSTR pwszServiceName,
    DWORD  dwPort
    );

DWORD
VmAfSrvGetHeartbeatStatus(
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
    );

VOID
VmAfdFreeHbNode(
    PVMAFD_HB_NODE pNode
    );

VOID
VmAfdHeartbeatCleanup(
    VOID
    );

DWORD
VmAfdRpcAllocateHeartbeatStatus(
    PVMAFD_HB_STATUS_W  pHeartbeatStatus,
    PVMAFD_HB_STATUS_W  *ppHeartbeatStatus
    );

VOID
VmAfdRpcFreeHeartbeatStatus(
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    );


//ddns.c

DWORD
VmDdnsInitThread(
        PDDNS_CONTEXT* ppDdnsContext
        );

VOID
VmDdnsShutdown(
        PDDNS_CONTEXT pDdnsContext
        );

VOID
VmDdnsExit(
          PDDNS_CONTEXT pDdnsContext
          );

DWORD
VmDdnsGetSourceIp(
        VMDNS_IP4_ADDRESS** ppSourceIp4,
        VMDNS_IP6_ADDRESS** ppSourceIp6
        );

DWORD
VmDdnsUpdateMakePacket(
          PSTR pszZone,
          PSTR pszHostname,
          PSTR pszName,
          PSTR* ppDnsPacket,
          DWORD* ppacketSize,
          DWORD hederId,
          DWORD dwFlag
          );

//sourceip.c

VOID
VmAfdShutdownSrcIpThread(
    PSOURCE_IP_CONTEXT pSourceIpContext
    );

DWORD
VmAfdInitSourceIpThread(
    PSOURCE_IP_CONTEXT* ppSourceIpContext
    );

DWORD
VmAfdIpcCreateComputerAccount(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

#ifdef __cplusplus
}
#endif

