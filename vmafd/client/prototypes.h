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

DWORD
VmAfdCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t * ppBinding
    );

DWORD
VmAfdCreateBindingHandleW(
    PCWSTR      pwszNetworkAddress,
    PCWSTR      pwszNetworkEndpoint,
    handle_t * ppBinding
    );

#ifdef UNICODE
    #define VmAfdCreateBindingHandle VmAfdCreateBindingHandleW
#else // non- UNICODE
    #define VmAfdCreateBindingHandle VmAfdCreateBindingHandleA
#endif // #ifdef UNICODE

VOID
VmAfdFreeBindingHandle(
    handle_t * ppBinding
    );

DWORD
VmAfdCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t  *ppBinding
    );

BOOLEAN
VmAfdIsLocalHost(
    PCSTR pszHostname
    );

/* client.c */

DWORD
VmAfdRpcGetErrorCode(
    dcethread_exc* pDceException
    );

PVMAFD_SERVER
VmAfdAcquireServer(
    PVMAFD_SERVER pServer
    );

VOID
VmAfdReleaseServer(
    PVMAFD_SERVER pServer
    );

DWORD
VmAfdAllocateFromRpcHeartbeatStatus(
   PVMAFD_HB_STATUS_W   pHeartbeatStatusSrc,
   PVMAFD_HB_STATUS_W *ppHeartbeatStatusDest
   );

VOID
VmAfdRpcClientFreeHeartbeatStatus(
   PVMAFD_HB_STATUS_W pHeartbeatStatus
   );

DWORD
VmAfdHeartbeatStatusAFromW(
    PVMAFD_HB_STATUS_W  pwHeartbeatStatus,
    PVMAFD_HB_STATUS_A* ppHeartbeatStatus
    );

/* rpc.c */

DWORD
VmAfdRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
);

DWORD
VmAfdRpcBindingFromStringBinding(
  PCSTR pszStringBinding,
  handle_t* pBinding
);

DWORD
VmAfdRpcFreeString(
    PSTR* ppszString
);

DWORD
VmAfdRpcFreeBinding(
    handle_t* pBinding
);

VOID
VmAfdRpcClientFreeMemory(
    PVOID pMemory
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

VOID
VmAfdRpcClientFreeStringArrayW(
    PWSTR*  ppwszStrArray,
    DWORD  dwCount
);

VOID
VmAfdRpcClientFreeStringW(
    PWSTR pszStr
);

/* vecslocalclient.c */

DWORD
VecsLocalIPCRequest(
    UINT32 apiType,
    DWORD noOfArgsIn,
    DWORD noOfArgsOut,
    VMW_TYPE_SPEC *input_spec,
    VMW_TYPE_SPEC *output_spec
);

DWORD
VecsLocalIPCRequestH(
	PVM_AFD_CONNECTION pConnection,
	UINT32 apiType,
	DWORD noOfArgsIn,
	DWORD noOfArgsOut,
	VMW_TYPE_SPEC *input_spec,
	VMW_TYPE_SPEC *output_spec
	);

/* afdlocalclient.c */

DWORD
VmAfdLocalGetStatus(
    PVMAFD_STATUS pStatus
);

DWORD
VmAfdLocalGetDomainName(
    PWSTR* ppwszDomain
);

DWORD
VmAfdLocalSetDomainName(
    PCWSTR pwszDomain
);

DWORD
VmAfdLocalGetDomainState(
    VMAFD_DOMAIN_STATE* pDomainState
);

DWORD
VmAfdLocalGetLDU(
    PWSTR* ppwszLDU
);

DWORD
VmAfdLocalSetLDU(
    PCWSTR pwszLDU
);

DWORD
VmAfdLocalGetRHTTPProxyPort(
    PDWORD pdwPort
    );

DWORD
VmAfdLocalSetRHTTPProxyPort(
    DWORD dwPort
);

DWORD
VmAfdLocalSetDCPort(
    DWORD dwPort
);

DWORD
VmAfdLocalGetCMLocation(
    PWSTR* ppwszCMLocation
);

DWORD
VmAfdLocalGetLSLocation(
    PWSTR* ppwszLSLocation
);

DWORD
VmAfdLocalGetDCName(
    PWSTR* ppwszDCName
);

DWORD
VmAfdLocalSetDCName(
    PCWSTR pwszDCName
);

DWORD
VmAfdLocalGetMachineAccountInfo(
    PWSTR* ppwszAccount,
    PWSTR* ppwszPassword
);

DWORD
VmAfdLocalGetSiteGUID(
    PWSTR* ppwszSiteGUID
);

DWORD
VmAfdLocalGetSiteName(
    PWSTR* ppwszSiteName
);

DWORD
VmAfdLocalGetMachineID(
    PWSTR* ppwszMachineID
);

DWORD
VmAfdLocalSetMachineID(
    PCWSTR pwszMachineID
);

DWORD
VmAfdLocalPromoteVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszSiteName,
    PCWSTR pwszPartnerHostName
);

DWORD
VmAfdLocalDemoteVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
);

DWORD
VmAfdLocalJoinValidateDomainCredentials(
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    );

DWORD
VmAfdLocalJoinVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit
);

DWORD
VmAfdLocalJoinVmDir2(
    PCWSTR           pwszDomainName,
    PCWSTR           pwszUserName,
    PCWSTR           pwszPassword,
    PCWSTR           pwszMachineName,
    PCWSTR           pwszOrgUnit,
    VMAFD_JOIN_FLAGS dwFlags
    );

DWORD
VmAfdLocalLeaveVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    DWORD  dwLeaveFlags
);

DWORD
VmAfdLocalCreateComputerAccount(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszOrgUnit,
    PWSTR* ppwszOutPassword
);

DWORD
VmAfdLocalJoinAD(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit
);

DWORD
VmAfdLocalLeaveAD(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
);

DWORD
VmAfdLocalQueryAD(
    PWSTR *pwszComputer,
    PWSTR *pwszDomain,
    PWSTR *pwszDistinguishedName,
    PWSTR *pwszNetbiosName
);

DWORD
VmAfdLocalGetDCList(
    PCSTR pszDomain,
    PDWORD pdwServerCount,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfo
);

DWORD
VmAfdLocalForceReplication(
    PCWSTR pwszServerName
);

DWORD
VmAfdLocalGetPNID(
    PWSTR* ppwszPNID
);

DWORD
VmAfdLocalSetPNID(
    PCWSTR pwszPNID
);

DWORD
VmAfdLocalGetCAPath(
    PWSTR* ppwszCAPath
);

DWORD
VmAfdLocalSetCAPath(
    PCWSTR pwszPath
);

DWORD
VmAfdLocalTriggerRootCertsRefresh(
    VOID
);

DWORD
VmAfdLocalRefreshSiteName(
    VOID
);

DWORD
VmAfdLocalPostHeartbeat(
    PCWSTR pwszServiceName,
    DWORD  dwPort
);

DWORD
VmAfdLocalGetHeartbeatStatus(
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
);

DWORD
VmAfdLocalConfigureDNSW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    );

DWORD
VmAfdLocalChangePNID(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    );

// cdclocalclient.c

DWORD
CdcLocalEnableClientAffinity(
    VOID
    );

DWORD
CdcLocalDisableClientAffinity(
    VOID
    );

DWORD
CdcLocalGetDCNameW(
    PCWSTR pszDomainName,
    GUID_W pDomainGuid,
    PCWSTR pszSiteName,
    DWORD dwFlags,
    PCDC_DC_INFO_W *ppDomainControllerInfo
    );

DWORD
CdcLocalEnumDCEntries(
    PWSTR **ppszDCEntries,
    PDWORD pdwCount
    );

DWORD
CdcLocalGetDCStatusInfo(
    PCWSTR pwszDCName,
    PCWSTR pwszDomainName,
    PCDC_DC_STATUS_INFO_W *ppDCStatusInfo,
    PVMAFD_HB_STATUS_W    *ppHbStatus
    );

DWORD
CdcLocalGetCurrentState(
    PDWORD pdwState
    );

