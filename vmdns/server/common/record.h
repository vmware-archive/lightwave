/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */


#ifndef RECORD_H
#define	RECORD_H

typedef BOOLEAN (*VMDNS_RECORD_COMPARE_FUNC) (
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

typedef VOID (*VMDNS_RECORD_CLEAR_FUNC) (
    PVMDNS_RECORD   pRecord
    );

typedef DWORD (*VMDNS_RECORD_DUPLICATE_FUNC) (
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD*  ppDest
    );

typedef DWORD (*VMDNS_RECORD_COPY_FUNC) (
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

typedef DWORD (*VMDNS_RECORD_SERIALIZE_DATA) (
    VMDNS_RECORD_DATA    RecordData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

typedef DWORD (*VMDNS_RECORD_DESERIALIZE_DATA) (
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA    pRecordData
    );

DWORD
VmDnsCreateRecord(
    PWSTR               pwszName,
    VMDNS_RR_TYPE       type,
    PVMDNS_RECORD_DATA  pRecordData,
    PVMDNS_RECORD*      ppRecord
    );

BOOLEAN
VmDnsCompareRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

VOID
VmDnsClearRecord(
    PVMDNS_RECORD   pRecord
    );

DWORD
VmDnsDuplicateRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD*  ppDest
    );

DWORD
VmDnsCopyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

VOID
VmDnsClearRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    );

DWORD
VmDnsAddRecordToArray(
    PVMDNS_RECORD_ARRAY     pRecordArray,
    PVMDNS_RECORD           pRecord
    );

DWORD
VmDnsRemoveRecordInArray(
    PVMDNS_RECORD_ARRAY     pRecordArray,
    PVMDNS_RECORD           pRecord
    );

DWORD
VmDnsCreateSoaRecord(
    PVMDNS_ZONE_INFO    pZoneInfo,
    PVMDNS_RECORD*      ppRecord
    );

DWORD
VmDnsSerializeDnsRecord(
    PVMDNS_RECORD       pDnsRecord,
    PBYTE*              ppBytes,
    DWORD*              pdwSize
    );

DWORD
VmDnsDeserializeDnsRecord(
    PBYTE               pBytes,
    DWORD               dwBytes,
    PVMDNS_RECORD      *ppDnsRecord
    );

#define VMDNS_FREE_RECORD(pRecord) \
    if (pRecord) \
    { \
        VmDnsClearRecord(pRecord); \
        VmDnsFreeMemory(pRecord); \
        pRecord = NULL; \
    }

#define VMDNS_FREE_RECORD_ARRAY(pRecords) \
    if (pRecords) \
    { \
        VmDnsClearRecordArray(pRecords); \
        VmDnsFreeMemory(((PVMDNS_RECORD_ARRAY)pRecords)->Records); \
        VmDnsFreeMemory(pRecords); \
    }

#endif	/* RECORD_H */

