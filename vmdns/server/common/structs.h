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
 * Module   : logging.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Private Structures
 */

#define VMDNS_MAX_NUM_FORWARDS 10
#define VMDNS_MAX_ZONES        64

// WE need to move this to socket package
typedef struct _VMDNS_SOCK_BUF_CONTEXT
{
    LONG                     refCount;
    PBYTE                    pData;
    UINT16                   usExpectedSize;
    UINT16                   usCurrentSize;
    //Not sure if we need the following two members. Keep them for now
    struct sockaddr_storage  clientAddr;
    socklen_t                addrLen;
} VMDNS_SOCK_BUF_CONTEXT, *PVMDNS_SOCK_BUF_CONTEXT;

//TODO: Temp fix to fix build issues.
#ifdef _WIN32
#pragma pack(push,1)
typedef struct _VMDNS_HEADER
{
    UINT16 usId;

#pragma pack(push,1)
    struct
    {
        unsigned char QR;
        unsigned char opcode;
        unsigned char AA;
        unsigned char TC;
        unsigned char RD;
        unsigned char RA;
        unsigned char Z;
        unsigned char RCODE;
    } codes;
#pragma pack(pop)

union
{
    UINT16 usQDCount;
    UINT16 usZOCount;
};
union
{
    UINT16 usANCount;
    UINT16 usPRCount;
};
union
{
    UINT16 usNSCount;
    UINT16 usUPCount;
};
union
{
    UINT16 usARCount;
    UINT16 usADCount;
};
}VMDNS_HEADER, *PVMDNS_HEADER;
#pragma pack(pop)

#else

typedef struct _VMDNS_HEADER
{
    UINT16 usId;

    struct
    {
        unsigned char QR;
        unsigned char opcode;
        unsigned char AA;
        unsigned char TC;
        unsigned char RD;
        unsigned char RA;
        unsigned char Z;
        unsigned char RCODE;
    } codes;

    union
    {
        UINT16 usQDCount;
        UINT16 usZOCount;
    };
    union
    {
        UINT16 usANCount;
        UINT16 usPRCount;
    };
    union
    {
        UINT16 usNSCount;
        UINT16 usUPCount;
    };
    union
    {
        UINT16 usARCount;
        UINT16 usADCount;
    };
} __attribute__((__packed__)) VMDNS_HEADER, *PVMDNS_HEADER;
#endif

typedef struct _VMDNS_QUESTION
{
    PSTR   pszQName;
    UINT16 uQType;
    UINT16 uQClass;
} VMDNS_QUESTION, *PVMDNS_QUESTION;

typedef struct _VMDNS_UPDATE_ZONE
{
    PSTR   pszName;
    UINT16 uType;
    UINT16 uClass;
} VMDNS_UPDATE_ZONE, *PVMDNS_UPDATE_ZONE;

typedef struct _VMDNS_MESSAGE
{
    PVMDNS_HEADER       pHeader;
    PVMDNS_QUESTION     *pQuestions;
    PVMDNS_RECORD       *pAnswers;
    PVMDNS_RECORD       *pAuthority;
    PVMDNS_RECORD       *pAdditional;
    PVMDNS_BLOB         pRawDnsMessage;
} VMDNS_MESSAGE, *PVMDNS_MESSAGE;

typedef struct _VMDNS_UPDATE_MESSAGE
{
    PVMDNS_HEADER       pHeader;
    PVMDNS_UPDATE_ZONE  pZone;
    PVMDNS_RECORD       *pPrerequisite;
    PVMDNS_RECORD       *pUpdate;
    PVMDNS_RECORD       *pAdditional;
    PVMDNS_BLOB         pRawDnsMessage;
} VMDNS_UPDATE_MESSAGE, *PVMDNS_UPDATE_MESSAGE;

typedef struct _VMDNS_SOCK_CONTEXT
{
    PVMDNS_MUTEX         pMutex;

    BOOLEAN              bShutdown;

    PVM_SOCKET           pListenerUDP;
    PVM_SOCKET           pListenerUDP6;
    PVM_SOCKET           pListenerTCP;
    PVM_SOCKET           pListenerTCP6;

    PVM_SOCK_EVENT_QUEUE pEventQueue;

    PVMDNS_THREAD*       pWorkerThreads;
    DWORD                dwNumThreads;

} VMDNS_SOCK_CONTEXT, *PVMDNS_SOCK_CONTEXT;

typedef struct _VMDNS_HASHTABLE
{
    ULONG ulSize;
    ULONG ulThreshold;
    ULONG ulCount;
    PVMDNS_HASHTABLE_NODE* ppBuckets;
    VMDNS_HASH_GET_KEY_FUNCTION pfnGetKey;
    VMDNS_HASH_FREE_KEY_FUNCTION pfnFreeKey;
    VMDNS_HASH_DIGEST_FUNCTION pfnDigest;
    VMDNS_HASH_EQUAL_FUNCTION pfnEqual;
    PVOID pUserData;
} VMDNS_HASHTABLE;

typedef struct _VMDNS_HASH_TABLE
{
	ULONG ulSize;
	ULONG ulThreshold;
	ULONG ulCount;
	PVMDNS_HASH_TABLE_NODE* ppData;
} VMDNS_HASH_TABLE;

typedef struct _VMDNS_DIR_CONTEXT
{
    LDAP*   pLdap;
} VMDNS_DIR_CONTEXT;

#ifndef _WIN32

typedef struct _SINGLE_LIST_ENTRY
{
    struct _SINGLE_LIST_ENTRY *Next;
} SINGLE_LIST_ENTRY;

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#endif

typedef enum _LRU_STATE
{
    LRU_STATE_IDLE = 0,
    LRU_STATE_STARTING,
    LRU_STATE_RUNNIG,
    LRU_STATE_STOPPING,
}LRU_STATE, *PLRU_STATE;

typedef DWORD(*LPVMDNS_PURGE_ENTRY_PROC)(
                        PVMDNS_NAME_ENTRY pNameEntry,
                        PVMDNS_ZONE_OBJECT pZoneObject
                        );

typedef DWORD(*LPVMDNS_ADD_REMOVE_ZONE_PROC)(PVOID pData, PCSTR pszZone);

typedef DWORD(*LPVMDNS_PURGE_RECORD_PROC)(
                        PVOID pData,
                        PCSTR pszZone,
                        PCSTR pszNode
                        );

typedef struct _VMDNS_ZONE_OBJECT
{
    volatile ULONG      lRefCount;
    PSTR                pszName;
    VMDNS_ZONE_FLAGS    dwFlags;
    PVMDNS_HASH_TABLE   pNameEntries;
    PVMDNS_LRU_LIST     pLruList;
    PVMDNS_RWLOCK       pLock;
} VMDNS_ZONE_OBJECT;

typedef struct _VMDNS_LRU_LIST
{
    LIST_ENTRY          LruListHead;
    PVMDNS_ZONE_OBJECT  pZoneObject;
    LPVMDNS_PURGE_ENTRY_PROC pPurgeEntryProc;
    DWORD               dwCurrentCount;
    DWORD               dwMaxCount;
    DWORD               dwLowerThreshold;
    DWORD               dwUpperThreshold;
    PVMDNS_MUTEX        pLock;
} VMDNS_LRU_LIST;

typedef struct _VMDNS_ZONE_LIST
{
    DWORD               dwZoneCount;
    PVMDNS_ZONE_OBJECT  Zones[VMDNS_MAX_ZONES];
} VMDNS_ZONE_LIST;

typedef struct _VMDNS_NAME_ENTRY
{
    volatile ULONG      lRefCount;
    LIST_ENTRY          LruList;
    PSTR                pszName;
    PVMDNS_RECORD_LIST  pRecords;
} VMDNS_NAME_ENTRY;

typedef struct _VMDNS_RECORD_OBJECT
{
    volatile ULONG      lRefCount;
    PVMDNS_RECORD       pRecord;
} VMDNS_RECORD_OBJECT;

typedef struct _VMDNS_RECORD_LIST
{
    volatile ULONG      lRefCount;
    DWORD               dwMaxSize;
    DWORD               dwCurrentSize;
    PVMDNS_RECORD_OBJECT *ppRecords;
} VMDNS_RECORD_LIST;

typedef struct _VMDNS_CACHE_CONTEXT
{
    PVMDNS_ZONE_LIST    pZoneList;
    PVMDNS_THREAD       pRefreshThread;
    PVMDNS_COND         pRefreshEvent;
    PVMDNS_MUTEX        pThreadLock;
    BOOL                bShutdown;
    BOOL                bRunning;
    PVMDNS_RWLOCK       pLock;
    DWORD               dwLastUSN;
} VMDNS_CACHE_CONTEXT, *PVMDNS_CACHE_CONTEXT;

typedef struct _VMDNS_FORWARDER_CONETXT
{
    PSTR                ppszForwarders[VMDNS_MAX_NUM_FORWARDS];
    DWORD               dwCount;
    PVMDNS_RWLOCK       pLock;
} VMDNS_FORWARDER_CONETXT;

typedef struct _VMDNS_GSS_CONTEXT_HANDLE
{
    PSTR                    pszKeyName;
    gss_ctx_id_t            gss_ctx_hdl;
    VM_DNS_GSS_CTX_STATE    state;
    UINT16                  unCtxUsage;
    PVMDNS_BLOB             pLastRecvSig;
} VMDNS_GSS_CONTEXT_HANDLE, *PVMDNS_GSS_CONTEXT_HANDLE;

typedef struct _VMDNS_SECURITY_CONTEXT
{
    PVMDNS_HASH_TABLE   pActiveGSSContexts;
    PVMDNS_RWLOCK       pLock;
} VMDNS_SECURITY_CONTEXT;

typedef struct _VMDNS_DRIVER_GLOBALS
{
    PVMDNS_SECURITY_CONTEXT     pSecurityContext;
    PVMDNS_CACHE_CONTEXT        pCacheContext;
    PVMDNS_FORWARDER_CONETXT    pForwarderContext;
    PVMDNS_SOCK_CONTEXT         pSockContext;
    VMDNS_STATE                 state;
    BOOL                        bUseDirectoryStore;
} VMW_DNS_DRIVER_GLOBALS, *PVMW_DNS_DRIVER_GLOBALS;
