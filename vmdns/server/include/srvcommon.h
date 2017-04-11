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


#ifndef _SRV_COMMON_H_
#define _SRV_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VMDNS_ORIG_TIME_STR_LEN         ( 4 /* year */ + 2 /* month */ + 2 /* day */ + 2 /* hour */ + 2 /* minute */ + \
                                          2 /* sec */ + 1 /* . */ + 3 /* milli sec */ + 1 /* null byte terminator */ )

#define VMDNS_MAX_I64_ASCII_STR_LEN     (19 + 1 /* null byte terminator */) /* Max value for i64_t is 9,223,372,036,854,775,807 */
#define VMDNS_UUID_LEN                  16 /* typedef __darwin_uuid_t uuid_t; typedef unsigned char __darwin_uuid_t[16] */
#define VMDNS_MAX_USN_STR_LEN           VMDNS_MAX_I64_ASCII_STR_LEN
#define VMDNS_MAX_VERSION_NO_STR_LEN    VMDNS_MAX_I64_ASCII_STR_LEN /* Version number used in attribute meta-data */


#define VMDNS_GUID_STR_LEN             (32 + 4 /* -s */ + 1 /* \0 */) // "%08x-%04x-%04x-%04x-%04x%08x"
#define VMDNS_HASH_TABLE_ITER_INIT     {NULL, 0}
#define VMDNS_UDP_PACKET_SIZE           (64*1024)

#define VMDNS_LDAP_SRV_NAME "_ldap._tcp"
#define VMDNS_KERBEROS_SRV_NAME "_kerberos._tcp"
#define VMDNS_LDAP_DC_SRV_NAME "_ldap._tcp.dc._msdcs"
#define VMDNS_KERBEROS_DC_SRV_NAME "_kerberos._tcp.dc._msdcs"

/*hash table*/

typedef struct _VMDNS_HASHTABLE *PVMDNS_HASHTABLE;
typedef struct _VMDNS_HASHTABLE const *PCVMDNS_HASHTABLE;

typedef struct _VMDNS_HASH_TABLE *PVMDNS_HASH_TABLE;
typedef struct _VMDNS_HASH_TABLE const *PCVMDNS_HASH_TABLE;

typedef struct _VMDNS_HASHTABLE_NODE
{
    struct _VMDNS_HASHTABLE_NODE* pNext;
    ULONG ulDigest;
    PVOID pData;
} VMDNS_HASHTABLE_NODE, *PVMDNS_HASHTABLE_NODE;

typedef struct _VMDNS_HASH_TABLE_NODE
{
    PSTR pKey;
    ULONG ulDigest;
    PVOID pData;
    struct _VMDNS_HASH_TABLE_NODE* pNext;
} VMDNS_HASH_TABLE_NODE, *PVMDNS_HASH_TABLE_NODE;

typedef struct _VMDNS_HASH_TABLE_ITER
{
    PVMDNS_HASH_TABLE_NODE pNext;
    ULONG ulIndex;
} VMDNS_HASH_TABLE_ITER, *PVMDNS_HASH_TABLE_ITER;

typedef PVOID
(*VMDNS_HASH_GET_KEY_FUNCTION)(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
    );

typedef VOID
(*VMDNS_HASH_FREE_KEY_FUNCTION)(
    PVOID pKey
    );

typedef ULONG
(*VMDNS_HASH_DIGEST_FUNCTION)(
    PCVOID pKey,
    PVOID pUserData
    );

typedef BOOLEAN
(*VMDNS_HASH_EQUAL_FUNCTION)(
    PCVOID pKey1,
    PCVOID pKey2,
    PVOID pUserData
    );

typedef VOID
(*VMDNS_HASHNODE_FREE_FUNCTION)(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
    );

DWORD
VmDnsHashTableAllocate(
    PVMDNS_HASH_TABLE* ppTable,
    ULONG ulSize
    );

DWORD
VmDnsHashTableInsert(
    PVMDNS_HASH_TABLE pTable,
    PCSTR key,
    PVOID value
    );

DWORD
VmDnsHashTableGet(
    PCVMDNS_HASH_TABLE pTable,
    PCSTR pKey,
    PVOID* value
);

DWORD
VmDnsHashTableRemove(
    PVMDNS_HASH_TABLE pTable,
    PCSTR key
    );

ULONG
VmDnsHashTableGetSize(
    PCVMDNS_HASH_TABLE pTable
    );

ULONG
VmDnsHashTableGetCount(
    PCVMDNS_HASH_TABLE pTable
    );

VOID
VmDnsHashTableFree(
    PVMDNS_HASH_TABLE ppTable
    );

PVMDNS_HASH_TABLE_NODE
VmDnsHashTableIterate(
    PCVMDNS_HASH_TABLE pTable,
    PVMDNS_HASH_TABLE_ITER pIter
    );

VOID
VmDnsHashTableResetIter(
    PVMDNS_HASH_TABLE_ITER pIter
    );

// serviceapi.c
typedef enum _VMDNS_USER_TYPE
{
    VMDNS_ADMINISTRATORS,
    VMDNS_USERS
} VMDNS_USER_TYPE;

#ifndef _WIN32
typedef struct _SINGLE_LIST_ENTRY *PSINGLE_LIST_ENTRY;
#endif

typedef struct _VMDNS_DIR_CONTEXT       *PVMDNS_DIR_CONTEXT;
typedef struct _VMDNS_ZONE_LIST         *PVMDNS_ZONE_LIST;
typedef struct _VMDNS_ZONE_OBJECT       *PVMDNS_ZONE_OBJECT;
typedef struct _VMDNS_LRU_LIST          *PVMDNS_LRU_LIST;
typedef struct _VMDNS_NAME_ENTRY        *PVMDNS_NAME_ENTRY;
typedef struct _VMDNS_RECORD_LIST       *PVMDNS_RECORD_LIST;
typedef struct _VMDNS_RECORD_OBJECT     *PVMDNS_RECORD_OBJECT;
typedef struct _VMDNS_FORWARDER_CONETXT *PVMDNS_FORWARDER_CONETXT;
typedef struct _VMDNS_CACHE_CONETXT     *PVMDNS_CACHE_CONETXT;
typedef struct _VMDNS_SECURITY_CONTEXT  *PVMDNS_SECURITY_CONTEXT;

#define DEFAULT_ZONE_HASHTABLE_SIZE 5
#define DEFAULT_NAME_HASHTABLE_SIZE 97
#define DEFAULT_SECURITY_HASHTABLE_SIZE 64
#define RECORD_KEY_STRING_FORMAT    "%s:%u"

typedef enum _VMDNS_STATE
{
    // Right after installation, and not promoted to DC yet;
    // Or explicitly uninitialized.
    VMDNS_UNINITIALIZED = 0,
    // DC being demoted, and DNS being uninitialized.
    VMDNS_UNINITIALIZING,
    // DC promoting and preparing DNS with SRV records.
    VMDNS_INITIALIZING,
    // DC promoted and DNS SRV records added.
    VMDNS_INITIALIZED,
    // A directory sync has succeeded.
    VMDNS_READY
} VMDNS_STATE;

DWORD
VmDnsSrvInitialize(
    BOOL bUseDirectoryStore
    );

VOID
VmDnsSrvCleanup(
    );

VMDNS_STATE
VmDnsSrvGetState(
    );

VMDNS_STATE
VmDnsSrvSetState(
    VMDNS_STATE newState
    );

VMDNS_STATE
VmDnsSrvConditionalSetState(
    VMDNS_STATE newState,
    VMDNS_STATE oldState
    );

DWORD
VmDnsSrvInitDomain(
    PVMDNS_INIT_INFO    pInitInfo
    );

DWORD
VmDnsSrvCleanupDomain(
    PVMDNS_INIT_INFO    pInitInfo
    );

DWORD
VmDnsSrvZoneCreate(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsSrvFindZone(
    PCSTR               pszZoneName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    );

DWORD
VmDnsSrvZoneUpdate(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    );

DWORD
VmDnsSrvZoneDelete(
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

DWORD
VmDnsSrvListZones(
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    );

DWORD
VmDnsSrvAddRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsSrvDeleteRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD       pRecord
    );

DWORD
VmDnsSrvUpdateRecord(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PVMDNS_RECORD pOldRecord,
    PVMDNS_RECORD pNewRecord
    );

DWORD
VmDnsSrvAddRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST pRecords
    );

DWORD
VmDnsSrvDeleteRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST pRecords
    );

DWORD
VmDnsSrvQueryRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    DWORD               dwOptions,
    PVMDNS_RECORD_LIST  *ppRecordList
    );

DWORD
VmDnsSrvListRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST *ppRecordList
    );

DWORD
VmDnsSrvAddForwarder(
    PCSTR               pszForwarder
    );

DWORD
VmDnsSrvGetForwarders(
    PSTR**              pppszForwarders,
    PDWORD              pdwCount
    );

DWORD
VmDnsSrvDeleteForwarder(
    PCSTR               pszForwarder
    );

DWORD
VmDnsSrvGetInverseRRTypeRecordList(
    PVMDNS_RECORD_LIST pFullRecordList,
    VMDNS_RR_TYPE dwExcludeType,
    PVMDNS_RECORD_LIST *ppParsedList
    );


// Config

DWORD
VmDnsConfigGetDword(
    PCSTR   pcszValueName,
    DWORD*  pdwOutput
    );

DWORD
VmDnsConfigGetStringA(
    PCSTR   pcszKeyPath,
    PCSTR   pcszValueName,
    PSTR*   ppszOutput
    );

//sockinterface.c
DWORD
VmDnsInitProtocolServer(
    VOID
    );

VOID
VmDnsShutdownProtocolServer(
    VOID
    );

VOID
VmDnsRpcServerFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    );

//record-list
DWORD
VmDnsRecordListCreate(
    PVMDNS_RECORD_LIST      *ppList
    );

DWORD
VmDnsRecordListAdd(
    PVMDNS_RECORD_LIST      pList,
    PVMDNS_RECORD_OBJECT    pRecord
    );

DWORD
VmDnsRecordListRemove(
    PVMDNS_RECORD_LIST      pList,
    PVMDNS_RECORD_OBJECT    pRecord
    );

UINT
VmDnsRecordListGetSize(
    PVMDNS_RECORD_LIST      pList
    );

PVMDNS_RECORD_OBJECT
VmDnsRecordListGetRecord(
    PVMDNS_RECORD_LIST      pList,
    UINT                    nIndex
    );

DWORD
VmDnsCopyRecordArray(
    PVMDNS_RECORD_LIST  pRecordList,
    PVMDNS_RECORD       **pppRecordArray
    );

DWORD
VmDnsRpcCopyRecordArray(
    PVMDNS_RECORD_LIST      pRecordList,
    PVMDNS_RECORD_ARRAY     *ppRecordArray
    );

ULONG
VmDnsRecordListAddRef(
    PVMDNS_RECORD_LIST      pRecordList
    );

VOID
VmDnsRecordListRelease(
    PVMDNS_RECORD_LIST      pRecordList
    );

DWORD
VmDnsRecordObjectCreate(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD_OBJECT    *ppRecordObj
    );

DWORD
VmDnsRecordObjectAddRef(
    PVMDNS_RECORD_OBJECT    pRecordObj
    );

VOID
VmDnsRecordObjectRelease(
    PVMDNS_RECORD_OBJECT    pRecordObj
    );

ULONG
VmDnsZoneObjectAddRef(
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

VOID
VmDnsZoneObjectRelease(
    PVMDNS_ZONE_OBJECT  pZoneObject
    );

// LDAP

DWORD
VmDnsDirConnect(
    PCSTR                   szHostName,
    PVMDNS_DIR_CONTEXT*     ppDirContext
    );

DWORD
VmDnsLdapGetMemberships(
    PVMDNS_DIR_CONTEXT      pConnection,
    PCSTR                   pszUPNName,
    PSTR                    **pppszMemberships,
    PDWORD                  pdwMemberships
    );

DWORD
VmDnsGetDefaultDomainName(
    PVMDNS_DIR_CONTEXT      pConnection,
    PSTR*                   ppDomainName
    );

VOID
VmDnsDirClose(
    PVMDNS_DIR_CONTEXT pDirContext
    );

#ifdef __cplusplus
}
#endif

#endif /* _SRV_COMMON_H_ */
