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
 * Module Name: Directory main
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Directory main module prototypes
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

// accountmgmt.c

DWORD
VmDirCreateAccount(
    PCSTR   pszUPNName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,            // optional
    PCSTR   pszEntryDN
    );

DWORD
VmDirUPNToAccountDN(
    PCSTR       pszUPNName,
    PCSTR       pszAccountRDNAttr,
    PCSTR       pszAccountRDNValue,
    PSTR*       ppszContainerDN
    );

DWORD
VmDirCreateAccountEx(
    PVMDIR_SRV_ACCESS_TOKEN       pAccessToken,
    PVMDIR_USER_CREATE_PARAMS_RPC pCreateParams
    );

// auth.c

DWORD
VmDirSrvCreateAccessToken(
    PCSTR pszUPN,
    PVMDIR_SRV_ACCESS_TOKEN* ppAccessToken
    );

PVMDIR_SRV_ACCESS_TOKEN
VmDirSrvAcquireAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    );

VOID
VmDirSrvReleaseAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    );

BOOL
VmDirIsRpcOperationAllowed(
    handle_t pBinding,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    ULONG    ulAccessDesired
    );

DWORD
VmDirAdministratorAccessCheck(
    PCSTR pszUpn
    );

BOOLEAN
VmDirIsMemberOf(
    PSTR *ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupDn
    );

DWORD
VmDirGetUPNMemberships(
    PCSTR pszUpnName,
    PSTR **pppszMemberships,
    PDWORD pdwMemberships
    );

VOID
VmDirFreeMemberships(
    PSTR *ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VmDirLoadIndex(
    BOOLEAN bFirstboot
    );

// init.c
DWORD
VmDirInit(
    VOID
    );

DWORD
VmDirAllocateBerValueAVsnprintf(
    PVDIR_BERVALUE pbvValue,
    PCSTR pszFormat,
    ...
    );

DWORD
VmDirGetHostsInternal(
    PSTR **pppszServerInfo,
    size_t *pdwInfoCount
    );

VOID
VmDirFreeCountedStringArray(
    PSTR *ppszStrings,
    size_t iCount
    );

// instance.c

DWORD
VmDirSrvSetupDomainInstance(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    BOOLEAN          bFirstNodeBootstrap,
    PCSTR            pszFQDomainName,
    PCSTR            pszDomainDN,
    PCSTR            pszUsername,
    PCSTR            pszPassword,
    PVMDIR_SECURITY_DESCRIPTOR pSecDescServicesOut, // OPTIONAL
    PVMDIR_SECURITY_DESCRIPTOR pSecDescAnonymousRead, // OPTIONAL
    PVMDIR_SECURITY_DESCRIPTOR pSecDescDeletedObjectsOut // OPTIONAL
    );

DWORD
VmDirSrvSetupHostInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszSiteName,
    PCSTR pszReplURI,
    UINT32  firstReplicationCycleMode
    );

// regconfig.c

DWORD
VmDirSrvUpdateConfig(
    VOID
    );

VOID
VmDirSrvFreeConfig(
    VOID
    );

DWORD
VmDirRegGetMultiSZ(
    PCSTR   pszKeyPath,
    PCSTR   pszKeyName,
    PVMDIR_STRING_LIST* ppStrList
    );

// rpcstring.c

ULONG
VmDirRpcAllocateStringW(
    PWSTR  pwszSrc,
    PWSTR* ppwszDst
    );

// schema.c

DWORD
VmDirLoadSchema(
    PBOOLEAN    pbWriteSchemaEntry,
    PBOOLEAN    pbLegacyDataLoaded
    );

DWORD
InitializeSchemaEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

DWORD
VmDirSchemaPatchViaFile(
    PCSTR       pszSchemaFilePath
    );

DWORD
VmDirSchemaPatchLegacyViaFile(
    PCSTR       pszSchemaFilePath
    );

/* service.c */

ULONG
VmDirRpcServerInit(
    VOID
    );

VOID
VmDirRpcServerShutdown(
    VOID
    );

ULONG
VmDirRpcAuthCallback(
    PVOID         Context
);

/* rpc.c */
ULONG
VmDirRpcServerStartListen(
    VOID
);

ULONG
VmDirRpcServerStopListen(
    VOID
);

ULONG
VmDirRpcServerRegisterIf(
    VMDIR_IF_HANDLE_T pInterfaceSpec
);

ULONG
VmDirRpcServerUnRegisterIf(
    VMDIR_IF_HANDLE_T pInterfaceSpec
);

ULONG VmDirRpcServerUseProtSeq(
    PCSTR pszProtSeq
);

ULONG VmDirRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
);

ULONG
VmDirRpcServerInqBindings(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
);

ULONG
VmDirRpcEpRegister(
    VMDIR_RPC_BINDING_VECTOR_P_T pServerBinding,
    VMDIR_IF_HANDLE_T            pInterfaceSpec,
    PCSTR                        pszAnnotation
);

ULONG
VmDirRpcServerRegisterAuthInfo(
    VOID
);

ULONG
VmDirRpcBindingInqAuthClient(
    VMDIR_RPC_BINDING_HANDLE hClientBinding,
    VMDIR_RPC_AUTHZ_HANDLE* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
);

ULONG
VmDirRpcBindingVectorFree(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
);

/* rpcserv.c */

DWORD
VmDirSrvInitializeHost(
    PWSTR    pwszDomainName,
    PWSTR    pwszUsername,
    PWSTR    pwszPassword,
    PWSTR    pwszSiteName,
    PWSTR    pwszReplURI,
    UINT32   firstReplCycleMode
    );

DWORD
VmDirSrvForceResetPassword(
    PWSTR                   pwszTargetDN,
    VMDIR_DATA_CONTAINER*   pContainer
    );

DWORD
VmDirSrvGeneratePassword(
    VMDIR_DATA_CONTAINER*   pContainer
    );

DWORD
VmDirSrvSetSRPSecret(
    PWSTR       pwszUPN,
    PWSTR       pwszSecret
    );

DWORD
VmDirSrvGetServerState(
    PDWORD   pServerState
    );

#ifndef _WIN32

// signals.c
VOID
VmDirBlockSelectedSignals(
    VOID
    );

DWORD
VmDirHandleSignals(
    VOID
    );

#endif

/*parseargs.c*/

DWORD
VmDirParseArgs(
    int         argc,
    char*       argv[],
    PCSTR*      ppszBootstrapSchemaFile,
    int*        pLoggingLevel,
    PCSTR*      ppszLogFileName,
    PBOOLEAN    pbEnableSysLog,
    PBOOLEAN    pbConsoleMode,
    PBOOLEAN    pbPatchSchema
    );

VOID
ShowUsage(
    PSTR pName
    );

/* krb.c */
DWORD
VmDirGetKrbMasterKey(
    PSTR        pszDomainName, // [in] FQDN
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    );

DWORD
VmDirGetKrbUPNKey(
    PSTR       pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    );


DWORD
VmDirGetKeyTabRecBlob(
    PSTR      pszUpnName,
    PBYTE*    ppBYTE,
    DWORD*    pSize
    );

/* accountmgmt.c */
DWORD
VmDirResetPassword(
    PCSTR       pszDN,
    PCSTR       pszNewPassword
    );

#ifdef _WIN32
__declspec(dllexport)
#endif
DWORD
VmDirSRPGetIdentityData(
    PCSTR       pszUPN,
    PBYTE*      ppByteSecret,
    DWORD*      pdwSecretLen
    );

DWORD
VmDirSRPSetIdentityData(
    PCSTR       pszUPN,
    PCSTR       pszSecret
    );

#ifdef _WIN32
DWORD
VmDirGetBootStrapSchemaFilePath(
    _TCHAR *lpBootStrapSchemaFile
);
DWORD
VmDirGetLogFilePath(
    _TCHAR *lpLogFile
);

void
VmDirGetLogMaximumOldLogs(
    PDWORD pdwMaximumOldLogs
    );

void
VmDirGetLogMaximumLogSize(
    PINT64 pI64MaximumLogSize
    );
#endif

/* utils.c */

DWORD
VmDirSrvValidateUserCreateParams(
    PVMDIR_USER_CREATE_PARAMS_RPC pCreateParams
    );

DWORD
VmDirSetAdministratorPasswordNeverExpires(
    VOID
    );

//IPC

//ipcserver.c

DWORD
VmDirIpcServerInit(
    VOID
    );

VOID
VmDirIpcServerShutDown(
    VOID
    );

//ipcapihandler.c

DWORD
VmDirLocalAPIHandler(
    PVM_DIR_SECURITY_CONTEXT pSecIdentity,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    DWORD * pdwResponseSize
    );

//ipclocalapi.c

DWORD
VmDirIpcInitializeHost(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcGetServerState(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcInitializeTenant(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcCreateTenant(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcDeleteTenant(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcEnumerateTenants(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcForceResetPassword(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcGeneratePassword(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcGetSRPSecret(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcSetSRPSecret(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    );

// superlogging.h
DWORD
VmDirInitializeSuperLogging(
    PVMSUPERLOGGING *ppLogger
    );

DWORD
VmDirLoadEventLogLibrary(
    PFEVENTLOG_ADD *ppfEventLogAdd
    );

// tenantmgmt.c
DWORD
VmDirSrvInitializeTenant(
    PWSTR    pwszDomainName,
    PWSTR    pwszUsername,
    PWSTR    pwszPassword
    );

DWORD
VmDirSrvCreateTenant(
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword
    );

DWORD
VmDirSrvEnumerateTenants(
    PVMDIR_STRING_LIST pTenantList
    );

DWORD
VmDirSrvDeleteTenant(
    PCSTR pszDomainName
    );

// tombstone.c
DWORD
VmDirInitTombstoneReapingThread(
    VOID
    );

// tracklastlogin.c

DWORD
VmDirInitTrackLastLoginThread(
    VOID
    );

DWORD VmDirInitDbCopyThread(
    VOID
    );

DWORD
VmDirCreateHeartbeatThread(
    );

VOID
VmDirKillHeartbeatThread(
    );

#ifdef __cplusplus
}
#endif
