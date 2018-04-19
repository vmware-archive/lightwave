/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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



#ifndef VMDIRCLIENT_H_
#define VMDIRCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOGGING_API
#if defined(_WIN32)
#define LOGGING_API __declspec(dllimport)
#else
#define LOGGING_API
#endif
#endif

#include <lber.h>
#include <ldap.h>
#include "vmdirtypes.h"

#define VMDIR_MAX_UPN_LEN                   512
#define VMDIR_CLIENT_JOIN_FLAGS_PREJOINED   0x00000001

/*
 * API exposed for HA Topology Management
 */
DWORD
VmDirGetCurrentTopologyAtSite(
    PCSTR                           pszUserName,
    PCSTR                           pszPassword,
    PCSTR                           pszHostName,
    PCSTR                           pszSiteName,
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppCurTopology // Output
    );

DWORD
VmDirGetCurrentGlobalTopology(
    PCSTR                           pszUserName,
    PCSTR                           pszPassword,
    PCSTR                           pszHostName,
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppCurTopology // Output
    );

DWORD
VmDirGetProposedTopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pCurTopology,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppNewTopology // Output
    );

DWORD
VmDirGetChangesInTopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pCurTopology,
    PVMDIR_HA_REPLICATION_TOPOLOGY  pNewTopology,
    PVMDIR_HA_TOPOLOGY_CHANGES*     ppTopologyChanges //Output
    );

DWORD
VmDirApplyTopologyChanges(
    PVMDIR_HA_TOPOLOGY_CHANGES  pTopologyChanges
    );

VOID
VmDirFreeHATopologyData(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology
    );

VOID
VmDirFreeHAServerInfo(
    PVMDIR_HA_SERVER_INFO   pServer
    );

VOID
VmDirFreeHATopologyChanges(
    PVMDIR_HA_TOPOLOGY_CHANGES  pTopologyChanges
    );
/*
 * API for HA Topology Management end here
 */
DWORD
VmDirConnectionOpen(
    PCSTR pszLdapURI,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PVMDIR_CONNECTION* ppConnection
    );

DWORD
VmDirConnectionOpenByHost(
    PCSTR pszHost,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PVMDIR_CONNECTION* ppConnection
    );

DWORD
VmDirGetSiteGuid(
    PVMDIR_CONNECTION pConnection,
    PSTR*             ppszGuid
    );

DWORD
VmDirGetSiteName(
    PVMDIR_CONNECTION pConnection,
    PSTR*             ppszGuid
    );

LDAP*
VmDirConnectionGetLdap(
    PVMDIR_CONNECTION pConnection
    );

VOID
VmDirConnectionClose(
    PVMDIR_CONNECTION pConnection
    );

DWORD
VmDirGetReplicationPartners(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_REPL_PARTNER_INFO*  ppReplPartnerInfo,
    DWORD*              pdwNumReplPartner
    );

DWORD
VmDirGetReplicationPartnerStatus(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_REPL_PARTNER_STATUS* ppReplPartnerStatus,
    DWORD*              pdwNumReplPartner
    );

DWORD
VmDirGetServers(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_SERVER_INFO* ppServerInfo,
    DWORD*              pdwNumServer
    );

DWORD
VmDirGetComputers(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PSTR**              pppszComputers,
    DWORD*              pdwNumComputers
    );

DWORD
VmDirGetComputersByOrgUnit(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PCSTR               pszOrgUnit,
    PSTR**              pppszComputers,
    DWORD*              pdwNumComputers
    );

DWORD
VmDirGetDCInfo(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_DC_INFO**    pppDC,
    DWORD*              pdwNumDC
    );

VOID
VmDirFreeDCInfo(
    PVMDIR_DC_INFO      pDC
    );

VOID
VmDirFreeDCInfoArray(
    PVMDIR_DC_INFO*     ppDC,
    DWORD               dwNumDC
    );

VOID
VmDirFreeStringArray(
    PSTR* ppszStr,
    DWORD size
    );

DWORD
VmDirAddReplicationAgreement(
    BOOLEAN bTwoWayRepl,
    PCSTR pszSrcHostName,
    PCSTR pszSrcPort,
    PCSTR pszSrcUserName,
    PCSTR pszSrcPassword,
    PCSTR pszTgtHostName,
    PCSTR pszTgtPort
);

DWORD
VmDirRemoveReplicationAgreement(
    BOOLEAN bTwoWayRepl,
    PCSTR pszSrcHostName,
    PCSTR pszSrcPort,
    PCSTR pszSrcUserName,
    PCSTR pszSrcPassword,
    PCSTR pszTgtHostName,
    PCSTR pszTgtPort
);

/*
 * Domain Controller/Client Life cycle management functions
 */
DWORD
VmDirSetupHostInstance(
    PCSTR   pszDomainName,
    PCSTR   pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszSiteName
    );

DWORD
VmDirSetupHostInstanceTrust(
    PCSTR   pszDomainName,
    PCSTR   pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszSiteName,
    PVMDIR_TRUST_INFO_A pTrustInfoA
    );

DWORD
VmDirDemote(
    PCSTR   pszUserName,
    PCSTR   pszPassword
    );

DWORD
VmDirJoin(
    PCSTR   pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszSiteName,
    PCSTR   pszReplHostName,
    UINT32  firstReplCycleMode
    );

DWORD
VmDirClientJoin(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszMachineName,
    PCSTR    pszOrgUnit,
    DWORD    dwJoinFlags);

DWORD
VmDirClientJoinAtomic(
    PCSTR                   pszServerName,
    PCSTR                   pszSRPUPN,
    PCSTR                   pszPassword,
    PCSTR                   pszDomainName,
    PCSTR                   pszMachineName,
    PCSTR                   pszOrgUnit,
    DWORD                   dwFlags,
    PVMDIR_MACHINE_INFO_A   *ppMachineInfo);

DWORD
VmDirClientLeave(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword
    );

DWORD
VmDirClientLeaveEx(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszDomainName,
    PCSTR    pszMachine
    );

DWORD
VmDirCreateComputerAccount(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszMachineName,
    PCSTR pszOrgUnit,
    PSTR* ppszOutPassword
    );

DWORD
VmDirCreateComputerAccountAtomic(
    PCSTR                   pszServerName,
    PCSTR                   pszSRPUPN,
    PCSTR                   pszPassword,
    PCSTR                   pszDomainName,
    PCSTR                   pszMachineName,
    PCSTR                   pszOrgUnit,
    PVMDIR_MACHINE_INFO_A   *ppMachineInfo);

DWORD
VmDirSetupTenantInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    );

DWORD
VmDirCreateTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pszDomainName,
    PCSTR pszNewUserName,
    PCSTR pszNewUserPassword
    );


DWORD
VmDirDeleteTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pszDomainName
    );

DWORD
VmDirEnumerateTenants(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PSTR **pppszTenants,
    DWORD *pdwNumTenants
    );

DWORD
VmDirGetDomainDN(
    PCSTR pszHostName,
    PSTR* ppszDomainDN
    );

DWORD
VmDirGetDomainName(
    PCSTR pszHostName,
    PSTR* ppszDomainName
    );

DWORD
VmDirGetPartnerSiteName(
    PCSTR pszHostName,
    PSTR* ppszSiteName
    );

DWORD
VmDirGetWin32ErrorDesc(
    DWORD dwErrorCode,
    PSTR* ppszErrorMessage
    );

DWORD
VmDirGetErrorMessage(
    DWORD dwErrorCode,
    PSTR* ppszErrorMessage
    );

DWORD
VmDirForceResetPassword(
    PCSTR       pszTargetDN,
    PBYTE*      ppByte,
    DWORD*      pSize
    );

DWORD
VmDirGetKeyTabeRecBlob(
    PCSTR       pszServerName,
    PCSTR       pszUPN,
    PBYTE*      ppByte,
    DWORD*      pSize
    );

DWORD
VmDirGetKrbMasterKey(
    PSTR        pszDomainName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    );

DWORD
VmDirGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    );

DWORD
VmDirLocalGetSRPSecret(
    PCSTR       pszUPN,
    PBYTE*      ppSecretBlob,
    DWORD*      pSize
);

DWORD
VmDirSetSRPSecret(
    PCSTR       pszUPN,
    PCSTR       pszSecret
    );

DWORD
VmDirCreateUser(
    PSTR    pszUserName,	/* IN              */
    PSTR    pszPassword,	/* IN              */
    PSTR    pszUPNName,		/* IN              */
    BOOLEAN bRandKey		/* IN              */
    );

DWORD
VmDirCreateUserA(
    PVMDIR_SERVER_CONTEXT       pServerContext,
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    );

DWORD
VmDirCreateUserW(
    PVMDIR_SERVER_CONTEXT       pServerContext,
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    );

DWORD
VmDirSetPassword(
    PCSTR pszHostName,		/* IN              */
    PCSTR pszAdminUPN,		/* IN              */
    PCSTR pszAdminPassword,	/* IN              */
    PCSTR pszUserUPN,		/* IN              */
    PCSTR pszNewPassword	/* IN              */
    );

DWORD
VmDirChangePassword(
    PCSTR pszHostName,		/* IN              */
    PCSTR pszUserUPN,		/* IN              */
    PCSTR pszOldPassword,	/* IN              */
    PCSTR pszNewPassword	/* IN              */
    );

DWORD
VmDirCreateService(
    PCSTR pszSvcname,  		/* IN              */
    PCSTR pszPassword, 	 	/* IN     OPTIONAL */
    PSTR* ppszUPN,      	/*    OUT OPTIONAL */
    PSTR* ppszPassword  	/*    OUT OPTIONAL */
    );

DWORD
VmDirGetMemberships(
    PVMDIR_CONNECTION pConnection,  /* IN  */
    PCSTR pszUPNName,               /* IN  */
    PSTR **pppszMemberships,        /* OUT */
    PDWORD pdwMemberships           /* OUT */
    );

VOID
VmDirFreeMemberships(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VmDirCreateGroup(
    PCSTR pszGroupname,   	/* IN              */
    PSTR* ppszUPN         	/*    OUT OPTIONAL */
    );

DWORD
VmDirGroupAddMember(
    PCSTR pszGroupUPN,  	/* IN              */
    PCSTR pszMemberUPN  	/* IN              */
    );

DWORD
VmDirGroupRemoveMember(
    PCSTR pszGroupUPN,  	/* IN              */
    PCSTR pszMemberUPN  	/* IN              */
    );

DWORD
VmDirGetVmDirLogPath(
    PSTR  pszPath,
    PCSTR pszLogFile
    );

DWORD
VmDirBackupDB(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       pszBackupPath
    );

DWORD
VmDirSetLogLevel(
    PCSTR   pszLogLevel
    );

DWORD
VmDirSetLogLevelH(
    PVMDIR_SERVER_CONTEXT   hInBinding,
    PCSTR                   pszLogLevel
    );

DWORD
VmDirGetLogLevelH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    VMDIR_LOG_LEVEL*         pLogLevel
    );

DWORD
VmDirSetLogMask(
    UINT32  iVmDirLogMask
    );

DWORD
VmDirSetLogMaskH(
    PVMDIR_SERVER_CONTEXT   hInBinding,
    UINT32                  iVmDirLogMask
    );

DWORD
VmDirGetLogMaskH(
    PVMDIR_SERVER_CONTEXT   hInBinding,
    UINT32*                 piVmDirLogMask
    );

DWORD
VmDirSetState(
    PVMDIR_SERVER_CONTEXT   hBinding,
    UINT32                  dwState);

DWORD
VmDirGetState(
    PVMDIR_SERVER_CONTEXT   hBinding,
    UINT32*                 pdwState);

DWORD
VmDirGetLocalState(
    UINT32* pdwState);

DWORD
VmDirReplNow(
    PCSTR   pszServerName);

VOID
VmDirFreeMemory(
    PVOID   pMemory
    );

DWORD
VmDirRefreshActPassword(
    PCSTR   pszHost,
    PCSTR   pszDomain,
    PCSTR   pszActUPN,
    PCSTR   pszActDN,
    PSTR    pszActPassword,
    PSTR*   ppszNewPassword
    );

DWORD
VmDirResetActPassword(
    PCSTR   pszHost,
    PCSTR   pszDomain,
    PCSTR   pszActUPN,
    PCSTR   pszActDN,
    PCSTR   pszActPassword,
    BOOLEAN bRefreshPassword,
    PSTR*   ppszNewPassword
    );

DWORD
VmDirResetMachineActCred(
    PCSTR pszLocalHostName,
    PCSTR   pszPartnerHost,
    PCSTR   pszUPN,
    PCSTR   pszPassword
    );

DWORD
VmDirModDcActPwd(
    PCSTR   pszHost,
    PCSTR   pszDomain,
    PCSTR   pszActUPN,
    PCSTR   pszActDN,
    PCSTR   pszActPasswrod,
    PCSTR   pszNewPassword
    );

DWORD
VmDirGeneratePassword(
    PCSTR       pszServerName,
    PCSTR       pszSRPUPN,
    PCSTR       pszSRPPassword,
    PBYTE*      ppByte,
    DWORD*      pSize
    );

DWORD
VmDirGetServerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PSTR*             ppszGuid
    );

DWORD
VmDirSetServerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PCSTR             pszGuid
    );

DWORD
VmDirGetComputerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PSTR*             ppszGuid
    );

DWORD
VmDirSetComputerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PCSTR             pszGuid
    );

#ifndef _VMDIR_COMMON_H__
LOGGING_API
void
VmDirLog(
   ULONG        level,
   const char*  fmt,
   ...);

LOGGING_API
void
VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    ULONG           iMask,
    const char*     fmt,
    ...);

LOGGING_API
void
VmDirLogTerminate(
    VOID
    );

LOGGING_API
DWORD
VmDirLogInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iLogLevel,
   ULONG            iInitLogMask
   );
#endif

DWORD
VmDirOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMDIR_SERVER_CONTEXT *pServerContext
    );

DWORD
VmDirOpenServerW(
    PCWSTR pszNetworkAddress,
    PCWSTR pszUserName,
    PCWSTR pszDomain,
    PCWSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMDIR_SERVER_CONTEXT *pServerContext
    );

DWORD
VmDirCloseServer(
    PVMDIR_SERVER_CONTEXT pServerContext
    );

DWORD
VmDirLeaveFederation(
    PSTR pszServerName,
    PSTR pszUserName,
    PSTR pszPassword
    );

DWORD
VmDirGetReplicationCycleCount(
    PVMDIR_CONNECTION   pConnection,
    DWORD*              pdwReplCycleCount
    );

DWORD
VmDirGetReplicationState(
    PVMDIR_CONNECTION  pConnection,
    PVMDIR_REPL_STATE* ppReplState
    );

VOID
VmDirFreeReplicationState(
    PVMDIR_REPL_STATE   pReplState
    );

DWORD
VmDirGetAttributeMetadata(
    PVMDIR_CONNECTION pConnection,
    PCSTR pszEntryDn,
    PCSTR pszAttribute,
    PVMDIR_METADATA_LIST* ppMetadataList
    );

VOID
VmDirFreeMetadata(
    PVMDIR_METADATA pMetadata
    );

VOID
VmDirFreeMetadataList(
    PVMDIR_METADATA_LIST pMetadataList
    );

DWORD
VmDirSuperLogQueryServerData(
    PVMDIR_SERVER_CONTEXT pContext,
    PVMDIR_SUPERLOG_SERVER_DATA *ppServerData);

DWORD
VmDirSuperLogEnable(
    PVMDIR_SERVER_CONTEXT pContext
    );

DWORD
VmDirSuperLogDisable(
    PVMDIR_SERVER_CONTEXT pContext
    );

DWORD
VmDirIsSuperLogEnabled(
    PVMDIR_SERVER_CONTEXT pContext,
    PBOOLEAN pbEnabled
);

DWORD
VmDirSuperLogFlush(
    PVMDIR_SERVER_CONTEXT pContext
    );

DWORD
VmDirSuperLogSetSize(
    PVMDIR_SERVER_CONTEXT pContext,
    DWORD dwSize
    );

DWORD
VmDirSuperLogGetSize(
    PVMDIR_SERVER_CONTEXT pContext,
    PDWORD pdwSize
    );

DWORD
VmDirSuperLogGetEntriesLdapOperation(
    PVMDIR_SERVER_CONTEXT pContext,
    ULONG64 **ppEnumerationCookie,
    DWORD dwCount,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries
    );

DWORD
VmDirSuperLogGetTable(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries,
    PVMDIR_SUPERLOG_TABLE_COLUMN_SET pColumnSet,
    PVMDIR_SUPERLOG_TABLE *ppTable
    );

VOID
VmDirFreeSuperLogEntryLdapOperationArray(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries
    );

VOID
VmDirFreeSuperLogTable(
    PVMDIR_SUPERLOG_TABLE pTable
    );

DWORD
VmDirGetDomainFunctionalLevel(
    PCSTR       pszHostName,
    PCSTR       pszUPN,
    PCSTR       pszPassword,
    PCSTR	pszDomainName,
    PDWORD	pdwFuncLvl
    );

DWORD
VmDirSetDomainFunctionalLevel(
    PCSTR       pszHostName,
    PCSTR       pszUPN,
    PCSTR       pszPassword,
    PCSTR	pszDomainName,
    PDWORD	pdwFuncLvl,
    BOOLEAN	bUseDefault
    );

DWORD
VmDirGetDCNodesVersion (
    PCSTR       pszHostName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PCSTR       pszDomainName,
    PVMDIR_DC_VERSION_INFO *pDCVerInfo
    );

VOID
VmDirFreeDCVersionInfo(
    PVMDIR_DC_VERSION_INFO pDCVerInfo
    );

DWORD
VmDirSetBackendState(
    PVMDIR_SERVER_CONTEXT    hBinding,
    UINT32     dwFileTransferState,
    UINT32     *pdwLogNum,
    UINT32     *pdwDbSizeMb,
    UINT32     *pdwDbMapSizeMb,
    PBYTE      pDbPath,
    UINT32     dwDbPathSize
);

DWORD
VmDirOpenDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hBinding,
    PCSTR                   pszDBFileName,
    FILE **                 ppFileHandl
);

DWORD
VmDirReadDatabaseFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    FILE *      pFileHandle,
    UINT32 *    pdwCount,
    PBYTE       pReadBuffer,
    UINT32      bufferSize
);

DWORD
VmDirCloseDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hBinding,
    FILE **                 ppFileHandle
);

DWORD
VmDirCreateDomainTrust(
    PCSTR    pszServerName,
    PCSTR    pszTrustName,
    PCSTR    pszDomainName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszTrustPasswdIn,
    PCSTR    pszTrustPasswdOut
    );

DWORD
VmDirAllocateTrustInfoA(
    PCSTR   pszName,
    PCSTR   pszDC,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PVMDIR_TRUST_INFO_A *ppTrustInfoA
    );

VOID
VmDirFreeTrustInfoA(
    PVMDIR_TRUST_INFO_A pTrustInfoA
    );

DWORD
VmDirAllocateTrustInfoW(
    PCWSTR   pwszName,
    PCWSTR   pwszDC,
    PCWSTR   pwszUserName,
    PCWSTR   pwszPassword,
    PVMDIR_TRUST_INFO_W *ppTrustInfoW
    );

VOID
VmDirFreeTrustInfoW(
    PVMDIR_TRUST_INFO_W pTrustInfoW
    );

DWORD
VmDirAllocateTrustInfoAFromW(
    PVMDIR_TRUST_INFO_W pTrustInfoW,
    PVMDIR_TRUST_INFO_A *ppTrustInfoA
    );

DWORD
VmDirAllocateTrustInfoWFromA(
    PVMDIR_TRUST_INFO_A pTrustInfoA,
    PVMDIR_TRUST_INFO_W *ppTrustInfoW
    );

/*
 * Deprecated function in LW 1.2
 */
DWORD
VmDirUrgentReplicationRequest(
    PCSTR pszRemoteServerName
    );

/*
 * Deprecated function in LW 1.2
 */
DWORD
VmDirUrgentReplicationResponse(
    PCSTR    pszRemoteServerName,
    PCSTR    pszUtdVector,
    PCSTR    pszInvocationId,
    PCSTR    pszHostName
    );

DWORD
VmDirChangePNID(
    PSTR    pszUsername,
    PSTR    pszPassword,
    PSTR    pszNewPNID
    );

VOID
VmDirClientFreeMachineInfo(
    PVMDIR_MACHINE_INFO_A pMachineInfo
    );

VOID
VmDirClientFreeMachineInfoW(
    PVMDIR_MACHINE_INFO_W pMachineInfo
    );

#ifdef __cplusplus
}
#endif

#endif /* VMDIRCLIENT_H_ */
