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
 * File:   prototypes.h
 * Author: huz
 *
 * Created on March 30, 2015, 3:47 PM
 */

#ifndef PROTOTYPES_H
#define	PROTOTYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

//dnsparser.c

DWORD
VmDnsReadDnsHeaderFromBuffer(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER *ppHeader
    );

DWORD
VmDnsParseQueryMessage(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER pHeader,
    PVMDNS_MESSAGE *ppMessage
    );

DWORD
VmDnsParseUpdateMessage(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_HEADER pHeader,
    PVMDNS_UPDATE_MESSAGE *ppUpdateMessage
    );

DWORD
VmDnsWriteDnsHeaderToBuffer(
    PVMDNS_HEADER pHeader,
    PVMDNS_MESSAGE_BUFFER pMessageBuffer
    );

DWORD
VmDnsWriteQueryMessageToBuffer(
    PVMDNS_MESSAGE          pDnsMessage,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );

DWORD
VmDnsWriteUpdateMessageToBuffer(
    PVMDNS_UPDATE_MESSAGE   pDnsUpdateMessage,
    PVMDNS_MESSAGE_BUFFER   pMessageBuffer
    );

//dnsutils.c
DWORD
VmDnsCreateBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT *ppMessage
    );

PVMDNS_SOCK_BUF_CONTEXT
VmDnsAcquireBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );

DWORD
VmDnsCopyRecords(
    PVMDNS_RECORD   *ppSrc,
    DWORD           dwCount,
    PVMDNS_RECORD   **pppDst
    );

DWORD
VmDnsCopyQuestion(
    PVMDNS_QUESTION     pSrc,
    PVMDNS_QUESTION     *ppDst
    );

DWORD
VmDnsCopyQuestions(
    PVMDNS_QUESTION     *ppSrc,
    DWORD               dwCount,
    PVMDNS_QUESTION     **pppDst
    );

DWORD
VmDnsCopyZone(
    PVMDNS_UPDATE_ZONE  pSrcZone,
    PVMDNS_UPDATE_ZONE  *ppDstZone
    );

VOID
VmDnsReleaseBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );

VOID
VmDnsFreeQuestion(
    PVMDNS_QUESTION pQuestion
    );

VOID
VmDnsFreeQuestions(
    PVMDNS_QUESTION *pQuestions,
    DWORD dwCount
    );

VOID
VmDnsFreeZone(
    PVMDNS_UPDATE_ZONE  pZone
    );

VOID
VmDnsFreeRecordsArray(
    PVMDNS_RECORD *pRecords,
    DWORD dwCount
    );

DWORD
VmDnsGetDnsMessageBuffer(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PVMDNS_MESSAGE_BUFFER *ppDnsMessageBuffer
    );

VOID
VmDnsCleanupDnsMessage(
    PVMDNS_MESSAGE pVmDnsMessage
    );

VOID
VmDnsFreeDnsMessage(
    PVMDNS_MESSAGE pVmDnsMessage
    );

VOID
VmDnsCleanupDnsUpdateMessage(
    PVMDNS_UPDATE_MESSAGE pVmDnsMessage
    );

VOID
VmDnsFreeDnsUpdateMessage(
    PVMDNS_UPDATE_MESSAGE pVmDnsMessage
    );

/* cache.c */

DWORD
VmDnsCacheInitialize(
    PVMDNS_CACHE_CONTEXT    *ppContext
    );

VOID
VmDnsCacheCleanup(
    PVMDNS_CACHE_CONTEXT    pContext
    );

DWORD
VmDnsCacheAddZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    );

DWORD
VmDnsCacheLoadZoneFromStore(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   pZoneName
    );

DWORD
VmDnsCacheUpdateZone(
    PVMDNS_CACHE_CONTEXT   pContext,
    PVMDNS_ZONE_INFO       pZoneInfo
    );

DWORD
VmDnsCacheRemoveZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_OBJECT      pZoneObject
    );

DWORD
VmDnsCacheGetZoneName(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PSTR                *pszZoneName
    );

DWORD
VmDnsCacheListZones(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    );

DWORD
VmDnsCacheFindZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   szZoneName,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    );

DWORD
VmDnsCacheFindZoneByQName(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   szQName,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    );

DWORD
VmDnsCachePurgeRecord(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCSTR              pszRecord
    );

DWORD
VmDnsCachePurgeRecordProc(
    PVOID pData,
    PCSTR pszZone,
    PCSTR pszNode
    );

DWORD
VmDnsCacheSyncZoneProc(
    PVOID pData,
    PCSTR pszZone
    );

DWORD
VmDnsCacheRemoveZoneProc(
    PVOID pData,
    PCSTR pszZone
    );

DWORD
VmDnsCacheSyncZones(
    DWORD                dwLastChangedUSN,
    DWORD                dwNewUSN,
    PVMDNS_CACHE_CONTEXT pCacheContext
    );

DWORD
VmDnsCacheEvictEntryProc(
    PVMDNS_NAME_ENTRY  pNameEntry,
    PVMDNS_ZONE_OBJECT pZoneObject
    );

/* lru.c */


DWORD
VmDnsLruInitialize(
    PVMDNS_ZONE_OBJECT pZoneObject,
    LPVMDNS_PURGE_ENTRY_PROC pPurgeEntryProc,
    PVMDNS_LRU_LIST* ppLruList
    );

VOID
VmDnsLruFree(
    PVMDNS_LRU_LIST pLruList
    );

DWORD
VmDnsLruAddNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    );

DWORD
VmDnsLruRemoveNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    );

DWORD
VmDnsLruRefreshNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    );

DWORD
VmDnsLruClearEntries(
    PVMDNS_LRU_LIST pLruList,
    DWORD dwCount
    );

DWORD
VmDnsLruTrimEntries(
    PVMDNS_LRU_LIST pLruList,
    DWORD dwCount
    );

DWORD
VmDnsLruGetPurgeInterval(
    PVMDNS_LRU_LIST pLruList
    );

BOOL
VmDnsLruIsPurgingNeeded(
    PVMDNS_LRU_LIST pLruList
    );

DWORD
VmDnsLruGetPurgeRate(
    PVMDNS_LRU_LIST pLruList
    );

// zonelist.c
DWORD
VmDnsZoneListInit(
    PVMDNS_ZONE_LIST* ppDnsList
    );

VOID
VmDnsZoneListCleanup(
    PVMDNS_ZONE_LIST pZoneList
    );

DWORD
VmDnsZoneListAddZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

DWORD
VmDnsZoneListRemoveZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

DWORD
VmDnsZoneListFindZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PCSTR               szZoneName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    );

DWORD
VmDnsZoneListFindZoneByQName(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PCSTR               sQName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    );

DWORD
VmDnsZoneListGetZones(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    );

// zone.c

DWORD
VmDnsZoneCreate(
    PVMDNS_ZONE_INFO        pZoneInfo,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    );

DWORD
VmDnsZoneCreateFromRecordList(
    PCSTR                   szZoneName,
    PVMDNS_RECORD_LIST      pRecordList,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    );

DWORD
VmDnsZoneObjectAddRef(
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

VOID
VmDnsZoneObjectRelease(
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

DWORD
VmDnsZoneUpdate(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsZoneGetName(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PSTR                *ppszZoneName
    );

DWORD
VmDnsZoneCopyZoneInfo(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsZoneUpdateRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    PVMDNS_RECORD_LIST  pRecordList
    );

DWORD
VmDnsZoneAddRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_OBJECT pRecord
    );

DWORD
VmDnsZoneRemoveRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_OBJECT pRecord
    );

DWORD
VmDnsZoneGetRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    PVMDNS_RECORD_LIST *ppRecordList
    );

DWORD
VmDnsZonePurgeRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName
    );

DWORD
VmDnsZoneRemoveNameEntry(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_NAME_ENTRY       pNameEntry
    );

DWORD
VmDnsZoneIsPurgingNeeded(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PDWORD pdwPurge
    );

// nameentry.c

DWORD
VmDnsNameEntryCreate(
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY   *ppNameEntry
    );

ULONG
VmDnsNameEntryAddRef(
    PVMDNS_NAME_ENTRY  pNameEntry
    );

VOID
VmDnsNameEntryRelease(
    PVMDNS_NAME_ENTRY   pNameEntry
    );

VOID
VmDnsNameEntryDelete(
    PVMDNS_NAME_ENTRY   pNameEntry
    );

DWORD
VmDnsNameEntryReplaceRecords(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD_LIST  pRecordList
    );

DWORD
VmDnsNameEntryAddRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_OBJECT    pRecordObject
    );

DWORD
VmDnsNameEntryRemoveRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_OBJECT    pRecordObject
    );

DWORD
VmDnsNameEntryGetRecords(
    PVMDNS_NAME_ENTRY       pNameEntry,
    VMDNS_RR_TYPE           rrType,
    PVMDNS_RECORD_LIST      *ppRecordList
    );

// forward.c

DWORD
VmDnsForwarderInit(
    PVMDNS_FORWARDER_CONETXT*   ppForwarder
    );

VOID
VmDnsForwarderCleanup(
    PVMDNS_FORWARDER_CONETXT    pForwarder
    );

DWORD
VmDnsGetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    );

DWORD
VmDnsSetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    DWORD                       dwCount,
    PSTR*                       ppszForwarders
    );

DWORD
VmDnsAddForwarder(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwarder
    );

DWORD
VmDnsDeleteForwarder(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PCSTR                       pszForwarder
    );

DWORD
VmDnsForwarderResolveRequest(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    BOOL                        bUseUDP,
    BOOL                        bRecusive,
    DWORD                       dwQuerySize,
    PBYTE                       pQueryBuffer,
    PDWORD                      pdwResponseSize,
    PBYTE*                      ppResopnse,
    PUCHAR                      prCode
    );

//dirfacade

DWORD
VmDnsDirConnect(
    PCSTR               szHostName,
    PVMDNS_DIR_CONTEXT* ppDirContext
    );

VOID
VmDnsDirClose(
    PVMDNS_DIR_CONTEXT pDirContext
    );

DWORD
VmDnsGetDSERootAttribute(
    PVMDNS_DIR_CONTEXT pContext,
    PSTR  pszAttribute,
    PSTR* ppszAttrValue
    );

DWORD
VmDnsGetDefaultDomainName(
    PVMDNS_DIR_CONTEXT pConnection,
    PSTR* ppDomainName
    );

DWORD
VmDnsLdapGetMemberships(
    PVMDNS_DIR_CONTEXT pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    );

DWORD
VmDnsDirLoadForwarders(
    PDWORD              pdwCount,
    PSTR*               *pppszForwarders
    );

DWORD
VmDnsDirSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    );

DWORD
VmDnsCreateInitZoneContainer(
    );

DWORD
VmDnsDirCreateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsDirDeleteZone(
    PCSTR               pszZone
    );

DWORD
VmDnsDirUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsDirListZones(
    PSTR**              pppszZones,
    PDWORD              pdwCount
    );

DWORD
VmDnsDirAddZoneRecord(
    PCSTR               pZoneName,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDirCreateZoneRecord(
    PCSTR               pZoneName,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDirDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsDirUpdateZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord,
    BOOL                bCreateIfNotExists
    );

DWORD
VmDnsDirListRecords(
    PCSTR               pszZone,
    PVMDNS_RECORD_LIST  *ppRecordList
    );

DWORD
VmDnsDirGetRecords(
    PCSTR               pszZone,
    PCSTR               pszName,
    PVMDNS_RECORD_LIST  *ppRecordList
    );

DWORD
VmDnsDirGetReplicationStatus(
    PDWORD              pdwLastChangedUSN
    );

DWORD
VmDnsDirSyncDeleted(
    DWORD                        pdwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpRemoveZoneProc,
    PVOID                        pData
    );

DWORD
VmDnsDirSyncNewObjects(
    DWORD                        pdwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpSyncZoneProc,
    LPVMDNS_PURGE_RECORD_PROC    LpPurgeRecordProc,
    PVOID                        pData
    );

//Store

DWORD
VmDnsStoreInitialize(
    );

VOID
VmDnsStoreCleanup(
    );

DWORD
VmDnsStoreCreateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsStoreUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsStoreDeleteZone(
    PCSTR               pszZoneName
    );

DWORD
VmDnsStoreListZones(
    PSTR**              pppszZones,
    PDWORD              pdwCount
    );

DWORD
VmDnsStoreAddZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsStoreDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsStoreListRecords(
    PCSTR               pszZone,
    PVMDNS_RECORD_LIST  *ppRecordArray
    );

DWORD
VmDnsStoreGetRecords(
    PCSTR               pszZone,
    PCSTR               pszName,
    PVMDNS_RECORD_LIST  *ppRecordList
    );

DWORD
VmDnsStoreSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    );

DWORD
VmDnsStoreGetForwarders(
    PDWORD              pdwCount,
    PSTR**              pppszForwarders
    );

DWORD
VmDnsStoreGetReplicationStatus(
    PDWORD             pdwLastChangedUSN
    );

DWORD
VmDnsStoreSyncDeleted(
    DWORD                        dwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpRemoveZoneProc,
    PVOID                        pData
    );

DWORD
VmDnsStoreSyncNewObjects(
    DWORD                        dwLastChangedUSN,
    DWORD                        dwNewUSN,
    LPVMDNS_ADD_REMOVE_ZONE_PROC LpSyncZoneProc,
    LPVMDNS_PURGE_RECORD_PROC    LpPurgeRecordProc,
    PVOID                        pData
    );

// dnsprotocol.c
DWORD
VmDnsProcessRequest(
    PBYTE   pDnsRequest,
    DWORD   dwDnsRequestSize,
    PBYTE   *ppDnsResponse,
    PDWORD  pdwDnsResponseSize,
    PUCHAR  pRCode
    );

// securityutils.c
DWORD
VmDnsSecInitialize(
    PVMDNS_SECURITY_CONTEXT     *ppContext
    );

VOID
VmDnsSecCleanup(
    PVMDNS_SECURITY_CONTEXT     pContext
    );

DWORD
VmDnsSecCreateGssCtx(
    PCSTR                       pszKeyName,
    gss_ctx_id_t                gss_ctx_hdl,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx
    );

DWORD
VmDnsSecAddGssCtx(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx
    );

DWORD
VmDnsSecGetGssCtx(
    PCSTR                       pszKeyName,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx
    );

DWORD
VmDnsSecDeleteGssCtx(
    PSTR   pszKeyName
    );

VOID
VmDnsSecCleanupGssCtx(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx
    );

DWORD
VmDnsSecProcessTkeyQuery(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_MESSAGE  pResponse
    );

DWORD
VmDnsSecAuthNegotiate(
    PVMDNS_RECORD               pTkeyRR,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_RECORD               *ppOutTkeyRR,
    PBOOL                       pbAuthSuccess
    );

DWORD
VmDnsSecCheckDeletePermissions(
    PVMDNS_MESSAGE  pRequest,
    PVMDNS_MESSAGE  pResponse,
    PBOOL           pbIsVerified
    );

DWORD
VmDnsSecCheckUpdatePermissions(
    PVMDNS_UPDATE_MESSAGE       pRequest,
    PVMDNS_GSS_CONTEXT_HANDLE   *ppGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pResponse,
    PBOOL                       pbIsVerified
    );

DWORD
VmDnsSecSignMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_MESSAGE              pResponse
    );

DWORD
VmDnsSecSignUpdateMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pResponse
    );

DWORD
VmDnsSecVerifyMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_MESSAGE              pRequest,
    PVMDNS_RECORD               pTsig,
    PBOOL                       pbIsVerified
    );

DWORD
VmDnsSecVerifyUpdateMessage(
    PVMDNS_GSS_CONTEXT_HANDLE   pGssSecCtx,
    PVMDNS_UPDATE_MESSAGE       pRequest,
    PVMDNS_RECORD               pTsig,
    PBOOL                       pbIsVerified
    );

BOOL
VmDnsSecIsRRTypeSec(
    DWORD dwRecordType
    );

// Zone
#ifdef	__cplusplus
}
#endif

#endif	/* PROTOTYPES_H */
