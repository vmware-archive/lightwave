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

#define VMDIR_MAX_UPN_LEN       512

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

VOID
VmDirFreeStringArray(
    PSTR* ppszStr,
    DWORD size
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
VmDirJoin(
    PCSTR   pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszSiteName,
    PCSTR   pszReplHostName,
    UINT32  firstReplCycleMode
    );

DWORD
VmDirSetupTenantInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    );

DWORD
VmDirRaftLeader(
    PCSTR   pszServerName,
    PSTR*   ppszLeader
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

VOID
VmDirFreeMemberships(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VmDirGetVmDirLogPath(
    PSTR  pszPath,
    PCSTR pszLogFile
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
    PSTR    pszActPasswrod,
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
VmDirGetMode(
    PVMDIR_SERVER_CONTEXT hInBinding,
    UINT32*               pdwMode);

DWORD
VmDirSetMode(
    PVMDIR_SERVER_CONTEXT hInBinding,
    UINT32                dwMode);

DWORD
VmDirRaftListCluster(
    PCSTR                   pszServerName,
    PVMDIR_RAFT_CLUSTER*    ppRaftCluster
    );

DWORD
VmDirRaftShowClusterState(
    PCSTR                   pszServerName,
    PCSTR                   pszDomainName,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PVMDIR_RAFT_CLUSTER*    ppRaftCluster
    );

DWORD
VmDirRaftLeaveCluster(
    PCSTR                   pszServerName,
    PCSTR                   pszDomainName,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PCSTR                   pszLeaveNode
    );

VOID
VmDirFreeRaftNode(
    PVMDIR_RAFT_NODE        pRaftNode
    );

VOID
VmDirFreeRaftCluster(
    PVMDIR_RAFT_CLUSTER     pRaftCluster
    );

#ifdef __cplusplus
}
#endif

#endif /* VMDIRCLIENT_H_ */
