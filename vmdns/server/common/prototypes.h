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

// Name Entry

DWORD
VmDnsCreateNameEntry(
    PCSTR               pszZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    );

DWORD
VmDnsDeleteNameEntry(
    PVMDNS_NAME_ENTRY   pNameEntry,
    BOOL                bDirSync
    );

DWORD
VmDnsNameEntryListRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_ARRAY*    ppRecords
    );

DWORD
VmDnsNameEntryCopyAllRecords(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecordsBuffer,
    DWORD               dwSize,
    DWORD*              pdwCopied
    );

DWORD
VmDnsNameEntryAddRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord,
    BOOL                bDirSync
    );

DWORD
VmDnsNameEntryDeleteRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord,
    BOOL                bDirSync
    );

DWORD
VmDnsNameEntryFindRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsNameEntryGetSoaRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD*      ppRecord
    );

DWORD
VmDnsNameEntryUpdateSoaRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_ZONE_INFO    pZoneInfo,
    BOOL                bDirSync
    );

DWORD
VmDnsNameEntryGetRecordCount(
    PVMDNS_NAME_ENTRY   pNameEntry
    );

//dnsparser.c

DWORD
VmDnsParseMessage(
    PVMDNS_MESSAGE_BUFFER pMessageBuffer,
    PVMDNS_MESSAGE *ppMessage
    );

DWORD
VmDnsWriteHeaderToBuffer(
    PVMDNS_HEADER pHeader,
    PVMDNS_MESSAGE_BUFFER pMessageBuffer
    );

DWORD
VmDnsForwarderResolveRequest(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    BOOL                        bUseUDP,
    BOOL                        bRecusive,
    DWORD                       dwQuerySize,
    PBYTE                       pQueryBuffer,
    PDWORD                      pdwResponseSize,
    PBYTE*                      ppResopnse
    );

//dnsutils.c
//
DWORD
VmDnsCreateBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT *ppMessage
    );

PVMDNS_SOCK_BUF_CONTEXT
VmDnsAcquireBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );

VOID
VmDnsReleaseBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );

DWORD
VmDnsGetAuthorityZone(
    PCSTR pszQueryName,
    PVMDNS_ZONE_INFO *ppZoneInfo
    );

DWORD
VmDnsProcessRequest(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize
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
VmDnsFreeRecordsArray(
    PVMDNS_RECORD *pRecords,
    DWORD dwCount
    );

DWORD
VmDnsGetDnsMessage(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PVMDNS_MESSAGE *ppDnsMessage
    );

VOID
VmDnsFreeDnsMessage(
    PVMDNS_MESSAGE pVmDnsMessage
    );


#ifdef	__cplusplus
}
#endif

#endif	/* PROTOTYPES_H */

