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



/* binding.c */

#ifndef _WIN32

#define VMDIR_STOP_SERVICE "/opt/likewise/bin/lwsm stop vmdir"
#define VMDIR_START_SERVICE "/opt/likewise/bin/lwsm start vmdir"
// in embedded VCHA, snapshot database live under vmware-vmdir/
#define VMDIR_CLEANUP_DATA "rm -rf /storage/db/vmware-vmdir/*"

#define VMKDC_STOP_SERVICE "/opt/likewise/bin/lwsm stop vmkdc"
#define VMKDC_START_SERVICE "/opt/likewise/bin/lwsm start vmkdc"
#else
#define VMDIR_STOP_SERVICE "net stop vmwaredirectoryservice /y"
#define VMDIR_START_SERVICE "net start vmwaredirectoryservice"
#define VMDIR_CLEANUP_DATA "del /q \"%allusersprofile%\\application data\\vmware\\cis\\data\\vmdird\\*\""

#define VMKDC_STOP_SERVICE "net stop vmwarekdcservice /y"
#define VMKDC_START_SERVICE "net start vmwarekdcservice"
#endif

ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

ULONG
VmDirCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t  *ppBinding
    );

ULONG
VmDirCreateBindingHandleNoauthA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

ULONG
VmDirCreateBindingHandleW(
    PCWSTR      pwszNetworkAddress,
    PCWSTR      pwszNetworkEndpoint,
    handle_t    *ppBinding
    );

ULONG
VmDirCreateBindingHandleMachineAccountA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

#ifdef UNICODE
    #define VmDirCreateBindingHandle VmDirCreateBindingHandleW
#else // non- UNICODE
    #define VmDirCreateBindingHandle VmDirCreateBindingHandleA
#endif // #ifdef UNICODE

VOID
VmDirFreeBindingHandle(
    handle_t *ppBinding
    );

/* rpc.c */
DWORD
VmDirRpcFreeString(
    PSTR* ppszString
);

DWORD
VmDirRpcFreeBinding(
    handle_t *pBinding
);

DWORD
VmDirConfigSetDCAccountInfo(
    PCSTR pszDCAccount,
    PCSTR pszDCAccountDN,
    PCSTR pszDCAccountPassword,
    DWORD dwPasswordSize,
    PCSTR pszMachineGUID
    );

DWORD
VmDirConfigSetDCAccountPassword(
    PCSTR pszDCAccountPassword,
    DWORD dwPasswordSize
    );

DWORD
VmDirConfigSetSZKey(
    PCSTR pszKeyPath,
    PCSTR pszKeyName,
    PCSTR pszKeyValue
    );

DWORD
VmDirConfigSetDefaultSiteandLduGuid(
    PCSTR pszDefaultSiteGuid,
    PCSTR pszDefaultLduGuid
    );

DWORD
VmDirGetRegGuid(
    PCSTR pszKey,
    PSTR  pszValue
    );

DWORD
VmDirGetRegKeyTabFile(
    PSTR  pszKeyTabFile
    );

DWORD
VmDirStartService(
    VOID
    );

DWORD
VmDirStopService(
    VOID
    );

DWORD
VmDirCleanupData(
    VOID
    );

DWORD
VmDirResetVmdir(
    VOID);

DWORD
VmKdcStartService(
    VOID
    );

DWORD
VmKdcStopService(
    VOID
    );

DWORD
VmDirGetSiteGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PSTR* ppszGUID
    );

DWORD
VmDirGetSiteNameInternal(
    LDAP* pLd,
    PSTR* ppszSiteName
    );

DWORD
VmDirGetSiteDN(
    PCSTR pszDomain,
    PCSTR pszSiteName,
    PSTR* ppszSiteDN
    );

DWORD
VmDirGetReplicationInfo(
    LDAP* pLd,
    PCSTR pszHost,
    PCSTR pszDomain,
    PREPLICATION_INFO* ppReplicationInfo,
    DWORD* pdwInfoCount
    );

DWORD
VmDirGetServersInfo(
    LDAP* pLd,
    PCSTR pszHost,
    PCSTR pszDomain,
    PINTERNAL_SERVER_INFO* ppInternalServerInfo,
    DWORD* pdwInfoCount
    );

DWORD
VmDirCreateCMSubtree(
    void* pLd,
    PCSTR pszDomain,
    PCSTR pszSiteGuid,
    PCSTR pszLduGuid);

DWORD
VmDirGetDomainName(
    PCSTR pszHostName,
    PSTR* ppszDomainName);

DWORD
VmDirGetServerName(
    PCSTR pszHostName,
    PSTR* ppszServerName);

DWORD
VmDirGetLocalLduGuid(
    PSTR pszLduGuid
    );

DWORD
VmDirGetLocalSiteGuid(
    PSTR pszSiteGuid
    );

DWORD
VmDirGetTargetDN(
    PCSTR pszSourceBase,
    PCSTR pszTargetBase,
    PCSTR pszSourceDN,
    PSTR* ppszTargetDN
    );

DWORD
VmDirLdapSetupRemoteHostRA(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszReplHostName,
    DWORD dwHighWatermark
    );

DWORD
VmDirLdapRemoveRemoteHostRA(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszReplHostName
    );

DWORD
VmDirLdapSetupDCAccountOnPartner(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszDCHostName
    );

DWORD
VmDirLdapSetupComputerAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszComputerHostName
    );

DWORD
VmDirLdapRemoveComputerAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszComputerHostName
    );

DWORD
VmDirLdapDeleteDCAccountOnPartner(
    PCSTR   pszDomainName,
    PCSTR   pszHostName,              // Partner host name
    PCSTR   pszUsername,
    PCSTR   pszPassword,
    PCSTR   pszDCHostName,           // Self host name
    BOOLEAN bActuallyDelete
    );

DWORD
VmDirLdapDeleteDCAccount(
    LDAP*   pLd,
    PCSTR   pszDomainName,
    PCSTR   pszDCHostName,           // Self host name
    BOOLEAN bActuallyDelete
    );

DWORD
VmDirLdapSetupServiceAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszServiceName,
    PCSTR pszDCHostName
    );

DWORD
VmDirLdapDeleteServiceAccountOnPartner(
    PCSTR   pszDomainName,
    PCSTR   pszHostName,          // Partner host name
    PCSTR   pszUsername,
    PCSTR   pszPassword,
    PCSTR   pszServiceName,
    PCSTR   pszDCHostName,         // Self host name
    BOOLEAN bActuallyDelete
    );

DWORD
VmDirLdapDeleteServiceAccount(
    LDAP    *pLd,
    PCSTR   pszDomainName,
    PCSTR   pszServiceName,
    PCSTR   pszDCHostName,         // Self host name
    BOOLEAN bActuallyDelete
    );

DWORD
VmDirMergeGroups(
    LDAP*   pSourceLd,
    LDAP*   pTargetLd,
    PCSTR   pszSourceDomainDN,
    PCSTR   pszTargetDomainDN
    );

DWORD
VmDirAddVmIdentityGroup(
    LDAP* pLd,
    PCSTR pszCN,
    PCSTR pszDN
    );

BOOLEAN
VmDirIfDNExist(
    LDAP* pLd,
    PCSTR pszDN);

DWORD
VmDirGetServerObjectDN(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszLotusServerObjectName,
    PSTR* ppszCurrentServerObjectDN
    );

DWORD
VmDirConnectLDAPServerByDN(
    LDAP**  pLd,
    PCSTR    pszLdapURI,
    PCSTR    pszUserDN,
    PCSTR    pszPassword
    );

DWORD
VmDirConnectLDAPServerByURI(
    LDAP**      pLd,
    PCSTR       pszLdapURI,
    PCSTR       pszDomain,
    PCSTR       pszUserName,
    PCSTR       pszPassword
    );

DWORD
VmDirConnectLDAPServer(
    LDAP**      pLd,
    PCSTR       pszHostName,
    PCSTR       pszDomain,
    PCSTR       pszUserName,
    PCSTR       pszPassword
    );

DWORD
VmDirAddVmIdentityContainer(
    LDAP* pLd,
    PCSTR pszCN,
    PCSTR pszDN
    );

DWORD
VmDirGetAdminName(
    PCSTR pszHostName,
    PSTR* ppszAdminName
    );

DWORD
VmDirDeleteDITSubtree(
   LDAP*    pLD,
   PCSTR    pszDN
   );

DWORD
VmDirGetAllRAToHost(
    LDAP*   pLD,
    PCSTR   pszHost,
    PSTR**  pppRADNArray,
    DWORD*  pdwSize
    );

DWORD
VmDirIsPartnerReplicationUpToDate(
    LDAP *pLD,
    PCSTR pszPartnerName,
    PCSTR pszDomain,
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassowrd,
    PBOOLEAN pbUpToDate
    );

DWORD
VmDirGetPartnerReplicationStatus(
    LDAP *pLD,
    PCSTR pszAccount,
    PVMDIR_REPL_PARTNER_STATUS pPartnerStatus
    );

DWORD
VmDirGetServerDN(
    PCSTR pszHostName,
    PSTR* ppszServerDN
    );

DWORD
VmDirDnLastRDNToCn(
    PCSTR   pszDN,
    PSTR*   ppszCN
    );

DWORD
VmDirLdapGetSingleAttribute(
    LDAP*   pLD,
    PCSTR   pszDN,
    PCSTR   pszAttr,
    PBYTE*  ppByte,
    DWORD*  pdwLen
    );

DWORD
VmDirLdapModReplaceAttribute(
    LDAP*   pLd,
    PCSTR   pszDN,
    PCSTR   pszAttribute,
    PCSTR   pszValue
    );

DWORD
VmDirLdapModReplAttributesValue(
    LDAP*   pLd,
    PCSTR   pszDN,
    PCSTR*  ppszAttValPair
    );

DWORD
VmDirGetDCContainerDN(
    PCSTR pszDomain,
    PSTR* ppszContainerDN
    );

DWORD
VmDirGetServerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszServerDN
    );

DWORD
VmDirGetServerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    );

DWORD
VmDirSetServerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PCSTR pszGUID
    );

DWORD
VmDirGetComputerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszAccountDN
    );

DWORD
VmDirGetComputerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    );

DWORD
VmDirSetComputerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PCSTR pszGUID
    );

DWORD
VmDirGeneratePassword(
    PCSTR       pszServerName,
    PCSTR       pszSRPUPN,
    PCSTR       pszSRPPassword,
    PBYTE*      ppByte,
    DWORD*      pSize
    );

BOOLEAN
VmDirIsLocalHost(
    PCSTR pszHostname
    );

DWORD
VmDirGetReplicateCycleCountInternal(
    PVMDIR_CONNECTION   pConnection,
    DWORD* pdwCycleCount
    );

/* localclient.c */

DWORD
VmDirLocalIPCRequest(
    UINT32 apiType,
    DWORD noOfArgsIn,
    DWORD noOfArgsOut,
    VMW_TYPE_SPEC *input_spec,
    VMW_TYPE_SPEC *output_spec
);

DWORD
VmDirLocalInitializeHost(
    PWSTR   pwszNamingContext,
    PWSTR   pwszUserName,
    PWSTR   pwszPassword,
    PWSTR   pwszSiteName,
    PWSTR   pwszReplURI,
    UINT32  firstReplCycleMode
);

DWORD
VmDirLocalGetServerState(
    UINT32  *pServerState
    );

DWORD
VmDirLocalInitializeTenant(
    PWSTR   pwszNamingContext,
    PWSTR   pwszUserName,
    PWSTR   pwszPassword
    );

DWORD
VmDirLocalCreateTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pszDomainName,
    PCSTR pszNewUserName,
    PCSTR pszNewUserPassword
    );

DWORD
VmDirLocalDeleteTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pwszDomain
    );

DWORD
VmDirLocalEnumerateTenants(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PSTR **pppszTenants,
    PDWORD pdwNumTenants
    );

DWORD
VmDirLocalForceResetPassword(
    PWSTR        pszTargetDN,
    VMDIR_DATA_CONTAINER* pPasswdContainer
);

DWORD
VmDirLocalGeneratePassword(
    VMDIR_DATA_CONTAINER* pPasswdContainer
);

DWORD
VmDirLocalSetSRPSecret(
    PCWSTR      pwszUPN,
    PCWSTR      pwszSecret
);

DWORD
VmDirGetLastLocalUsnProcessedForHostFromRADN(
    LDAP *pLD,
    PCSTR pszRADN,
    USN* pUsn
    );

DWORD
VmDirLdapCreateReplHostNameDN(
    PSTR* ppszReplHostNameDN,
    LDAP* pLd,
    PCSTR pszReplHostName
    );

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
DWORD
VmDirDCEGetErrorCode(
    dcethread_exc* pDceException
    );

#endif

DWORD
VmDirGetAllDCInternal(
    LDAP*   pLd,
    PCSTR   pszDomainName,
    PVMDIR_STRING_LIST* ppStrList
    );

DWORD
VmDirGetPSCVersionInternal(
    LDAP* pLd,
    PSTR* ppszPSCVer
    );

DWORD
VmDirPSCVersion(
    PCSTR       pszHostName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PCSTR       pszDomainName,
    PSTR*       ppszVersion
    );

DWORD
VmDirGetObjectAttribute(
    LDAP*   pLd,
    PCSTR   pszDomain,
    PCSTR   pszSearchDNPrefix,
    PCSTR   pszObjectClass,
    PCSTR   pszAttribute,
    int     scope,
    PSTR**  pppszValues,
    DWORD*  pdwNumValues
    );

DWORD
VmDirGetReplicationStateInternal(
    LDAP*               pLd,
    PVMDIR_REPL_STATE*  ppReplState
    );

VOID
VmDirFreeReplicationStateInternal(
    PVMDIR_REPL_STATE   pReplState
    );

DWORD
VmDirParseMetadata(
    PCSTR  pszMetadata,
    PVMDIR_METADATA *ppMetadata
    );

VOID
VmDirFreeMetadataInternal(
    PVMDIR_METADATA pMetadata
    );

VOID
VmDirFreeMetadataListInternal(
    PVMDIR_METADATA_LIST pMetadataList
    );

DWORD
VmDirGetAttributeMetadataInternal(
    PVMDIR_CONNECTION   pConnection,
    PCSTR               pszEntryDn,
    PCSTR               pszAttribute,
    PVMDIR_METADATA_LIST*    ppMetadataList
    );

DWORD
VmDirLdapGetHighWatermark(
    LDAP*      pLocalLd,
    PCSTR      pszLocalHost,
    PCSTR      pszPartnerHost,
    PCSTR      pszDomainName,
    PCSTR      pszUsername,
    PCSTR      pszPassword,
    USN*       pLastLocalUsn
    );

DWORD
VmDirSetupDefaultAccount(
    PCSTR pszDomainName,
    PCSTR pszPartnerServerName,
    PCSTR pszLdapHostName,
    PCSTR pszBindUserName,
    PCSTR pszBindPassword
    );

DWORD
VmDirUpdateKeytabFile(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    BOOLEAN bIsServer
    );
