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
        unsigned char QR     : 1;
        unsigned char opcode : 4;
        unsigned char AA     : 1;
        unsigned char TC     : 1;
        unsigned char RD     : 1;
        unsigned char RA     : 1;
        unsigned char Z      : 3;
        unsigned char RCODE  : 4;
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
        unsigned char QR     : 1;
        unsigned char opcode : 4;
        unsigned char AA     : 1;
        unsigned char TC     : 1;
        unsigned char RD     : 1;
        unsigned char RA     : 1;
        unsigned char Z      : 3;
        unsigned char RCODE  : 4;
    } __attribute__((__packed__)) codes;

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
    PVMDNS_HEADER pHeader;
    PVMDNS_QUESTION *pQuestions;
    PVMDNS_RECORD *pRecords;
} VMDNS_MESSAGE, *PVMDNS_MESSAGE;

typedef struct _VMDNS_UPDATE_MESSAGE
{
    PVMDNS_HEADER pHeader;
    PVMDNS_UPDATE_ZONE pZone;
    PVMDNS_RECORD *pPrerequisite;
    PVMDNS_RECORD *pUpdate;
    PVMDNS_RECORD *pAdditional;
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

#endif

typedef struct _VMDNS_RECORD_ENTRY
{
    SINGLE_LIST_ENTRY           ListEntry;
    PVMDNS_RECORD               Record;
} VMDNS_RECORD_ENTRY;

typedef struct _VMDNS_DIR_ZONE_ENTRY
{
    SINGLE_LIST_ENTRY           ListEntry;
    PSTR                        ZoneName;
    SINGLE_LIST_ENTRY           Records;
} VMDNS_DIR_ZONE_ENTRY;

typedef struct _VMDNS_DIR_DNS_INFO
{
    DWORD                       NumZones;
    SINGLE_LIST_ENTRY           Zones;
} VMDNS_DIR_DNS_INFO;

typedef struct _VMDNS_ZONE
{
    PSTR                pszName;
    PVMDNS_HASH_TABLE    pNameEntries;
    VMDNS_ZONE_FLAGS    dwFlags;
    PVMDNS_RWLOCK       pLock;
    DWORD               refCount;
} VMDNS_ZONE;

typedef struct _VMDNS_ZONE_ENTRY
{
    SINGLE_LIST_ENTRY   ListEntry;
    PVMDNS_ZONE         pZone;
} VMDNS_ZONE_ENTRY;

typedef struct _VMDNS_ZONE_LIST
{
    SINGLE_LIST_ENTRY   Zones;
    PVMDNS_RWLOCK       pLock;
} VMDNS_ZONE_LIST;

typedef struct _VMDNS_NAME_ENTRY
{
    PSTR                pszZone;
    PSTR                pszName;
    SINGLE_LIST_ENTRY   Records;
    PVMDNS_RWLOCK       pLock;
} VMDNS_NAME_ENTRY;

typedef struct _VMDNS_ZONE_UPDATE_CONTEXT
{
    PVOID pData;
} VMDNS_ZONE_UPDATE_CONTEXT;

typedef struct _VMDNS_FORWARDER_CONETXT
{
    PSTR                ppszForwarders[VMDNS_MAX_NUM_FORWARDS];
    DWORD               dwCount;
    PVMDNS_RWLOCK       pLock;
} VMDNS_FORWARDER_CONETXT;

typedef struct _VMW_DNS_DRIVER_GLOBALS
{
    PVMDNS_ZONE_LIST            pZoneList;
    PVMDNS_FORWARDER_CONETXT    pForwarderContext;
    PVMDNS_SOCK_CONTEXT         pSockContext;
    VMDNS_STATE                 state;
    BOOL                        bUseDirectoryStore;
} VMW_DNS_DRIVER_GLOBALS, *PVMW_DNS_DRIVER_GLOBALS;

