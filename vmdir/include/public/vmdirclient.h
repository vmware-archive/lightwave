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


#if 0
// Just plain old vanilla windows include
#if !defined(HAVE_DCERPC_WIN32) && defined(_WIN32)

#include <Rpc.h>
#define wchar16_t   wchar_t
#define UINT32      unsigned long
#define mode_t      int
#define unsigned32  unsigned int

#elif defined(NO_LIKEWISE)
// We don't want the LikeWise headers since we conflict
// with Unix ODBC, and we are on Unix. Define all types ourselves

typedef unsigned short int wchar16_t;
typedef char* PSTR;
typedef const char* PCSTR;
typedef wchar16_t* PCWSTR;
typedef void* PVOID;
typedef char  CHAR;
typedef unsigned int UINT32;
typedef unsigned int DWORD;
typedef int BOOLEAN;
typedef void VOID;

#else
// On Unix and we don't have headers that conflict,
// Just use likewise headers

#if !defined(NO_LIKEWISE) && !defined(_WIN32)
#include <lw/types.h>
#endif
#include <dce/rpcbase.h>

#endif
#endif /* if 0 */


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
VmDirGetSiteGuid(
    PVMDIR_CONNECTION pConnection,
    PSTR*             ppszGuid
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

DWORD
VmDirSetupHostInstance(
    PCSTR                       pszDomainName,
    PCSTR                       pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR                       pszUserName,
    PCSTR                       pszPassword,
    PCSTR                       pszSiteName
    );

DWORD
VmDirDemote(
    PCSTR                       pszUserName,
    PCSTR                       pszPassword
    );

DWORD
VmDirJoin(
    PCSTR                       pszLotusServerName, // optional Lotus Server Name (FQDN/IP/hostname)
    PCSTR                       pszUserName,
    PCSTR                       pszPassword,
    PCSTR                       pszSiteName,
    PCSTR                       pszReplHostName,
    UINT32                      firstReplCycleMode
    );

DWORD
VmDirClientJoin(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszMachineName,
    PCSTR    pszOrgUnit
    );

DWORD
VmDirClientLeave(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword
    );

DWORD
VmDirCleanupPartner(
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszSiteName,
    PCSTR    pszPartnerHostName,
    BOOLEAN  bActuallyDelete
    );

DWORD
VmDirSetupTenantInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    );

DWORD
VmDirMerge(
    PCSTR    pszSourceUserName,
    PCSTR    pszSourcePassword,
    PCSTR    pszTargetHost,
    PCSTR    pszTargetUserName,
    PCSTR    pszTargetPassword
    );

DWORD
VmDirSplit(
    PCSTR    pszSourceUserName,
    PCSTR    pszSourcePassword
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
VmDirGetLocalLduGuid(
    PSTR pszLduGuid);

DWORD
VmDirGetLocalSiteGuid(
    PSTR pszSiteGuid);

DWORD
VmDirMigrateKrbUPNKey(
    PBYTE   pOldUpnKeys,
    DWORD   oldUpnKeysLen,
    PBYTE   pOldMasterKey,
    DWORD   oldMasterKeyLen,
    PBYTE   pNewMasterKey,
    DWORD   newMasterKeyLen,
    PBYTE*  ppNewUpnKeys,
    PDWORD  pNewUpnKeysLen
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
VmDirSetSRPSecret(
    PCSTR       pszUPN,
    PCSTR       pszSecret
    );

DWORD
VmDirGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
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
    PCSTR pszHostURI,		/* IN              */
    PCSTR pszAdminDN,		/* IN              */
    PCSTR pszAdminPassword,	/* IN              */
    PCSTR pszUserDN,		/* IN              */
    PCSTR pszNewPassword	/* IN              */
    );

DWORD
VmDirChangePassword(
    PCSTR pszHostURI,		/* IN              */
    PCSTR pszUserDN,		/* IN              */
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

#if 0 /* Don't expose handle_t RPC binding handle in public API */
ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t * ppBinding
    );

ULONG
VmDirCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t* ppBinding
    );

VOID
VmDirFreeBindingHandle(
    handle_t * ppBinding
    );
#endif /* #if 0 */

DWORD
VmDirSetLogLevel(
    PCSTR       pszLogLevel
    );

DWORD
VmDirSetLogLevelH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    PCSTR       pszLogLevel
    );

DWORD
VmDirSetLogMask(
    UINT32      iVmDirLogMask
    );

DWORD
VmDirSetLogMaskH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    UINT32      iVmDirLogMask
    );

DWORD
VmDirSetState(
    PVMDIR_SERVER_CONTEXT    hBinding,
    UINT32      dwState);

DWORD
VmDirGetLocalState(
    UINT32*     pdwState);

DWORD
VmDirReplNow(
    PCSTR pszServerName);

DWORD
VmDirOpenDBFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    PCSTR       pszDBFileName,
    FILE **     ppFileHandle);

DWORD
VmDirReadDBFile(
    PVMDIR_SERVER_CONTEXT            hBinding,
    FILE *              pFileHandle,
    UINT32 *            pdwCount,
    PBYTE *             ppReadBuffer);

DWORD
VmDirCloseDBFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    FILE *      pFileHandle);

VOID
VmDirFreeMemory(
    PVOID pMemory
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
void
VmDirLog(
   ULONG        level,
   const char*  fmt,
   ...);

void
VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    ULONG           iMask,
    const char*     fmt,
    ...);

void
VmDirLogTerminate(
    VOID
    );
#endif

DWORD
VmDirLogInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iLogLevel,
   ULONG            iInitLogMask
   );

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
VmDirGetUsnFromPartners(
    PCSTR pszHostName,
    USN   *pUsn
    );

DWORD
VmDirLeaveFederation(
    PSTR pszServerName,
    PSTR pszUserName,
    PSTR pszPassword
    );

#ifdef __cplusplus
}
#endif

#endif /* VMDIRCLIENT_H_ */
