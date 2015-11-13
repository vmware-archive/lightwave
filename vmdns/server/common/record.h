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

