/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  interface.h
 *
 * Abstract: VMware Domain Name Service.
 *
 * Created on: Sep 18, 2012
 *
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
#define VMDNS_HASHTABLE_ITER_INIT       {NULL, 0}
#define VMDNS_UDP_PACKET_SIZE           (512)

#define VMDNS_LDAP_SRV_NAME "_ldap._tcp"

/*hash table*/

typedef struct _VMDNS_HASHTABLE *PVMDNS_HASHTABLE;
typedef struct _VMDNS_HASHTABLE const *PCVMDNS_HASHTABLE;

typedef struct _VMDNS_HASHTABLE_NODE
{
    struct _VMDNS_HASHTABLE_NODE* pNext;
    ULONG ulDigest;
    PVOID pData;
} VMDNS_HASHTABLE_NODE, *PVMDNS_HASHTABLE_NODE;

typedef struct _VMDNS_HASHTABLE_ITER
{
    PVMDNS_HASHTABLE_NODE pNext;
    ULONG ulIndex;
} VMDNS_HASHTABLE_ITER, *PVMDNS_HASHTABLE_ITER;


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
VmDnsCreateHashTable(
    PVMDNS_HASHTABLE* ppTable,
    VMDNS_HASH_GET_KEY_FUNCTION pfnGetKey,
    VMDNS_HASH_FREE_KEY_FUNCTION pfnFreeKey,
    VMDNS_HASH_DIGEST_FUNCTION pfnDigest,
    VMDNS_HASH_EQUAL_FUNCTION pfnEqual,
    PVOID pUserData,
    ULONG ulSize
    );

VOID
VmDnsHashTableInsert(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode,
    PVMDNS_HASHTABLE_NODE* ppPrevNode
    );

VOID
VmDnsHashTableResizeAndInsert(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode,
    PVMDNS_HASHTABLE_NODE* ppPrevNode
    );

DWORD
VmDnsHashTableRemove(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode
    );

DWORD
VmDnsHashTableFindKey(
    PCVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE* ppNode,
    PCVOID pKey
    );

VOID
VmDnsHashTableResetIter(
    PVMDNS_HASHTABLE_ITER pIter
    );

PVMDNS_HASHTABLE_NODE
VmDnsHashTableIterate(
    PCVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_ITER pIter
    );

VOID
VmDnsHashTableClear(
    PVMDNS_HASHTABLE pTable,
    VMDNS_HASHNODE_FREE_FUNCTION pFree,
    PVOID pUserData
    );

ULONG
VmDnsHashTableGetSize(
    PCVMDNS_HASHTABLE pTable
    );

ULONG
VmDnsHashTableGetCount(
    PCVMDNS_HASHTABLE pTable
    );

DWORD
VmDnsHashTableResize(
    PVMDNS_HASHTABLE pTable,
    ULONG ulSize
    );

VOID
VmDnsFreeHashTable(
    PVMDNS_HASHTABLE* ppTable
    );

ULONG
VmDnsHashDigestPstr(
    PCVOID pKey,
    PVOID pUnused
    );

BOOLEAN
VmDnsHashEqualPstr(
    PCVOID pKey1,
    PCVOID pKey2,
    PVOID pUnused
    );

// LDAP
typedef enum _VMDNS_USER_TYPE
{
    VMDNS_ADMINISTRATORS,
    VMDNS_USERS
} VMDNS_USER_TYPE;

#ifndef _WIN32
typedef struct _SINGLE_LIST_ENTRY *PSINGLE_LIST_ENTRY;
#endif

typedef struct _VMDNS_DIR_CONTEXT       *PVMDNS_DIR_CONTEXT;
typedef struct _VMDNS_DIR_ZONE_ENTRY    *PVMDNS_DIR_ZONE_ENTRY;
typedef struct _VMDNS_DIR_DNS_INFO      *PVMDNS_DIR_DNS_INFO;
typedef struct _VMDNS_RECORD_ENTRY      *PVMDNS_RECORD_ENTRY;
typedef struct _VMDNS_ZONE              *PVMDNS_ZONE;
typedef struct _VMDNS_ZONE_ENTRY        *PVMDNS_ZONE_ENTRY;
typedef struct _VMDNS_ZONE_LIST         *PVMDNS_ZONE_LIST;
typedef struct _VMDNS_NAME_ENTRY        *PVMDNS_NAME_ENTRY;
typedef struct _VMDNS_FORWARDER_CONETXT *PVMDNS_FORWARDER_CONETXT;

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
VmDnsLdapGetMemberships(
    PVMDNS_DIR_CONTEXT pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    );

DWORD
VmDnsDirGetLotusZoneInfo(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO*    ppDirDnsInfo
    );

VOID
VmDnsCleanupZoneInfo(
    PVMDNS_DIR_DNS_INFO     pDirZoneInfo
    );

DWORD
VmDnsDirGetForwarders(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR**           pppszForwarders,
    PDWORD           pdwCount
    );

DWORD
VmDnsDirSetForwarders(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR*               ppszForwarders,
    DWORD               dwCount
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
VmDnsDirUpdateCachedZoneInfo(
    PVMDNS_DIR_DNS_INFO     pZoneInfo
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

//Store

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
VmDnsStoreSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    );

// Zone

#define DEFAULT_ZONE_HASHTABLE_SIZE 5
#define DEFAULT_NAME_HASHTABLE_SIZE 97
#define RECORD_KEY_STRING_FORMAT    "%s:%u"

#define VMDNS_FREE_RECORD_NODE(pNode) \
{ \
    if (pNode) \
    { \
        if (pNode->pData) \
        { \
            VMDNS_FREE_RECORD_ARRAY(pNode->pData); \
        } \
        VmDnsFreeMemory(pNode); \
        pNode = NULL; \
    } \
}

typedef struct _VMDNS_ZONE_UPDATE_CONTEXT* PVMDNS_ZONE_UPDATE_CONTEXT;
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
VmdnsZoneBeginUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT* ppCtx
    );

DWORD
VmdnsZoneEndUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx
    );

DWORD
VmDnsCoreInit(
    );

VOID
VmDnsCoreCleanup(
    );

VMDNS_STATE
VmDnsGetState(
    );

VMDNS_STATE
VmDnsSetState(
    VMDNS_STATE newState
    );

VMDNS_STATE VmDnsConditionalSetState(
    VMDNS_STATE newState,
    VMDNS_STATE oldState
    );

DWORD
VmDnsZoneCreate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_ZONE_INFO pZoneInfo,
    BOOL             bDirSync
    );

DWORD
VmDnsZoneList(
    PVMDNS_ZONE_INFO_ARRAY *ppZoneArray
    );

DWORD
VmDnsZoneGetDefaultDomainName(
    PSTR* ppszDomainName
    );

DWORD
VmDnsZoneUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_ZONE_INFO pZoneInfo,
    BOOL             bDirSync
    );

DWORD
VmDnsZoneDelete(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR       pszZone
    );

DWORD
VmDnsZoneAddRecord(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR         pszZone,
    PVMDNS_RECORD pRecord,
    BOOL          bDirSync
    );

DWORD
VmDnsZoneDeleteRecord(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR         pszZone,
    PVMDNS_RECORD pRecord,
    BOOL          bDirSync
    );

DWORD
VmDnsZoneFindAndDeleteRecords(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR                   pszZone,
    PCSTR                   pszName,
    BOOL                    bDirSync
    );

DWORD
VmDnsZoneQuery(
    PCSTR                   pszZone,
    PCSTR                   pszName,
    VMDNS_RR_TYPE           type,
    PVMDNS_RECORD_ARRAY*    ppRecords
    );

DWORD
VmDnsZoneListRecord(
    PCSTR                   pszZone,
    PVMDNS_RECORD_ARRAY*    ppRecords
    );

DWORD
VmDnsZoneFindByName(
    PCSTR                   pszZone,
    PVMDNS_ZONE*            ppZone
    );

DWORD
VmDnsZoneListInit(
    PVMDNS_ZONE_LIST*       ppZoneList
    );

VOID
VmDnsZoneListCleanup(
    PVMDNS_ZONE_LIST        pZoneList
    );

DWORD
VmDnsForwarderInit(
    PVMDNS_FORWARDER_CONETXT*   ppForwarder
    );

VOID
VmDnsForwarderCleanup(
    PVMDNS_FORWARDER_CONETXT    pForwarder
    );

DWORD
VmDnsSetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    DWORD                       dwCount,
    PSTR*                       ppszForwarders
    );

DWORD
VmDnsGetForwarders(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    );

DWORD
VmDnsAddForwarder(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PSTR                        pszForwarders
    );

DWORD
VmDnsDeleteForwarder(
    PVMDNS_FORWARDER_CONETXT    pForwarder,
    PSTR                        pszForwarders
    );

PVMDNS_FORWARDER_CONETXT
VmDnsGetForwarderContext(
    );

// Config

DWORD
VmDnsConfigGetDword(
    PCSTR   pcszValueName,
    DWORD*  pdwOutput
    );

DWORD
VmDnsConfigGetStringA(
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


#ifdef __cplusplus
}
#endif

#endif /* _SRV_COMMON_H_ */

