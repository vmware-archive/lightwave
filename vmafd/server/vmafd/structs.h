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
 * Module Name: afd Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * afd Main module
 *
 * Private Structures
 *
 */

typedef struct _VECS_SERV_STORE
{
    LONG refCount;
    DWORD dwStoreId;

}VECS_SERV_STORE, *PVECS_SERV_STORE;

typedef struct _VECS_SRV_ENUM_CONTEXT
{
    LONG refCount;
    PVECS_SERV_STORE pStore;
    DWORD dwIndex;
    ENTRY_INFO_LEVEL infoLevel;

    DWORD dwLimit;

} VECS_SRV_ENUM_CONTEXT, *PVECS_SRV_ENUM_CONTEXT;

typedef enum
{
    VMAFD_CONFIG_VALUE_TYPE_STRING = 0,
    VMAFD_CONFIG_VALUE_TYPE_DWORD,
    VMAFD_CONFIG_VALUE_TYPE_BOOLEAN
} VMAFD_CONFIG_VALUE_TYPE;

typedef enum
{
    STORE_MAP_ENTRY_STATUS_EMPTY = 0,
    STORE_MAP_ENTRY_STATUS_OPEN,
    STORE_MAP_ENTRY_STATUS_CLOSED,
    STORE_MAP_ENTRY_STATUS_REMOVED,
    STORE_MAP_ENTRY_STATUS_DELETED
} STORE_MAP_ENTRY_STATUS;

typedef struct _VMAFD_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMAFD_RPC_ENDPOINT, *PVMAFD_RPC_ENDPOINT;

#ifdef _WIN32

typedef struct _VMAFD_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMAFD_NTSERVICE_DATA, *PVMAFD_NTSERVICE_DATA;

#endif

typedef struct _VMAFD_CONFIG_CONNECTION_HANDLE
{
    HANDLE hConnection;
    HKEY hKey;
} VMAFD_CONFIG_CONNECTION_HANDLE, *PVMAFD_CONFIG_CONNECTION_HANDLE;

typedef struct _VMAFD_CONFIG_ENTRY
{
    PCSTR   pszName;
    VMAFD_CONFIG_VALUE_TYPE Type;
#ifdef _WIN32
    DWORD RegDataType;
#else
    REG_DATA_TYPE RegDataType;    //Corresponding likewise type
#endif
    DWORD dwMin;                  //DWORD type min value
    DWORD dwMax;                  //DWORD type max value
    struct
    {
        DWORD dwDefault;          //DWORD type default value
        PSTR  pszDefault;         //SZ type default value
    } defaultValue;
    struct
    {
        DWORD dwValue;            //DWORD type value
        // User own this memory
        PSTR  pszValue;           //SZ type value
    } cfgValue;

} VMAFD_CONFIG_ENTRY, *PVMAFD_CONFIG_ENTRY;

typedef struct _VMAFD_CERT_THR_DATA
{
    pthread_mutex_t  mutex;
    pthread_mutex_t* pMutex;

    pthread_cond_t   cond;
    pthread_cond_t*  pCond;

    BOOLEAN          bShutdown;
    BOOLEAN          forceFlush;

} VMAFD_CERT_THR_DATA, *PVMAFD_CERT_THR_DATA;

typedef struct _VMAFD_THREAD
{
    pthread_t  thread;
    pthread_t* pThread;

    VMAFD_CERT_THR_DATA thrData;

} VMAFD_THREAD, *PVMAFD_THREAD;

typedef struct _CDC_THREAD_CONTEXT
{
    pthread_t       thread;
    pthread_t*      pThread;
    pthread_mutex_t thr_mutex;
    pthread_cond_t  thr_cond;
    BOOLEAN         bShutdown;
} CDC_THREAD_CONTEXT, *PCDC_THREAD_CONTEXT;

typedef enum
{
    CDC_UPDATE_THREAD_STATE_UNDEFINED = 0,
    CDC_UPDATE_THREAD_STATE_SIGNALLED,
    CDC_UPDATE_THREAD_STATE_POLLING,
    CDC_UPDATE_THREAD_STATE_UPDATED
} CDC_UPDATE_THREAD_STATE;

typedef struct _CDC_CACHE_UPDATE_CONTEXT
{
    PCDC_THREAD_CONTEXT     pUpdateThrContext;
    BOOLEAN                 bRefreshCache;
    BOOLEAN                 bActiveAlive;
    DWORD                   dwOffsiteRefresh;
    CDC_UPDATE_THREAD_STATE cdcThrState;
    pthread_mutex_t         update_mutex;
    pthread_cond_t          update_cond;
} CDC_CACHE_UPDATE_CONTEXT, *PCDC_CACHE_UPDATE_CONTEXT;

typedef enum
{
    CDC_STATE_THREAD_STATE_UNDEFINED = 0,
    CDC_STATE_THREAD_STATE_SIGNALLED,
    CDC_STATE_THREAD_STATE_RUNNING,
    CDC_STATE_THREAD_STATE_UPDATED
} CDC_STATE_THREAD_STATE;

typedef struct _CDC_STATE_MACHINE_CONTEXT
{
    PCDC_THREAD_CONTEXT       pStateThrContext;
    pthread_mutex_t           state_mutex;
    CDC_STATE_THREAD_STATE    cdcThrState;
    pthread_mutex_t           update_mutex;
    pthread_cond_t            update_cond;
    volatile UINT32           bFirstRun;
} CDC_STATE_MACHINE_CONTEXT, *PCDC_STATE_MACHINE_CONTEXT;

typedef struct _CDC_CONTEXT
{
    PCDC_STATE_MACHINE_CONTEXT pCdcStateMachineContext;
    PCDC_CACHE_UPDATE_CONTEXT  pCdcCacheUpdateContext;
    pthread_mutex_t            context_mutex;
} CDC_CONTEXT, *PCDC_CONTEXT;

typedef struct _SOURCE_IP_CONTEXT
{
    pthread_t srcIpThread;
    DWORD     sourceIpSocket;
} SOURCE_IP_CONTEXT, *PSOURCE_IP_CONTEXT;

typedef struct _DDNS_UPDATE_HEADER
{
    UINT16 headerId;
    UINT16 headerCodes;
    UINT16 zoCount;
    UINT16 prCount;
    UINT16 upCount;
    UINT16 adCount;
} DDNS_UPDATE_HEADER, *PDDNS_UPDATE_HEADER;

typedef struct _DDNS_UPDATE_ZONE
{
    PSTR    pZoneName;
    UINT16  zType;
    UINT16  zClass;
} DDNS_UPDATE_ZONE, *PDDNS_UPDATE_ZONE;

typedef struct _DDNS_UPDATE_RR
{
    PSTR    pName;
    UINT16  rType;
    UINT16  rClass;
    UINT32  ttl;
    UINT16  rdLength;
    PSTR    rData;
} DDNS_UPDATE_RR, *PDDNS_UPDATE_RR;

typedef struct _DDNS_CONTEXT
{
    pthread_t       thread;
    DWORD           flag;
    DWORD           netLinkFd;
    DWORD           pipeFd[2];
    pthread_mutex_t ddnsMutex;
    DWORD           idSeed;
    DWORD           bIsEnabledDnsUpdates;
} DDNS_CONTEXT, *PDDNS_CONTEXT;

typedef struct _VMAFD_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszLogFile;
    DWORD                           dwMaxOldLogs;
    DWORD                           dwMaxLogSize;

    PSTR                            pszKrb5Config;
    PSTR                            pszKrb5Keytab;

    PVMAFD_THREAD                   pCertUpdateThr;
    pthread_mutex_t                 pCertUpdateMutex;
    PVMAFD_THREAD                   pPassRefreshThr;
    PCDC_CONTEXT                    pCdcContext;

    PDDNS_CONTEXT                   pDdnsContext;
    PSOURCE_IP_CONTEXT              pSourceIpContext;

    // following fields are protected by mutex
    pthread_t                       thread;
    pthread_mutex_t                 mutex;

    pthread_cond_t                  statusCond;
    VMAFD_STATUS                    status;
    VMAFD_DOMAIN_STATE              domainState;
    PWSTR                           pwszMachineId;

    dcethread*                      pRPCServerThread;

    pthread_t                       pIPCServerThread;
    DWORD                           dwCertCheckSec; // Frequency to check certificates in Seconds
    BOOLEAN                         bEnableRPC;

    PVM_AFD_CONNECTION              pConnection;
    pthread_mutex_t                 mutexConnection;
    pthread_mutex_t                 mutexStoreState;

    pthread_mutex_t                 mutexCreateStore;

    pthread_rwlock_t                rwlockStoreMap;

    PVMSUPERLOGGING                 pLogger;

    PVMSUPERLOG_THREAD_INFO               pSrvThrInfo;
} VMAFD_GLOBALS, *PVMAFD_GLOBALS;

typedef struct _VMAFD_KRB_CONFIG {
    PSTR pszFileName;
    PSTR pszDefaultRealm;
    PSTR pszDefaultKeytabName;
    DWORD iNumKdcServers;
    PSTR pszKdcServer[VMAFD_MAX_KDC_SERVERS];
} VMAFD_KRB_CONFIG, *PVMAFD_KRB_CONFIG;


typedef struct _VMAFD_ROOT_FETCH_ARG {
    PSTR pszUserName;
    PSTR pszAccountDN;
    PSTR pszPassword;
    PSTR pszDomain;
    PSTR pszDCName;
    PSTR pszUpn;
    unsigned int nPort;
}VMAFD_ROOT_FETCH_ARG, *PVMAFD_ROOT_FETCH_ARG;

typedef struct _VMAFD_REG_ARG {
    PSTR pszAccount;        // dc/machine account name
    PSTR pszAccountDN;
    PSTR pszPassword;
    PSTR pszDomain;
    PSTR pszAccountUPN;
}VMAFD_REG_ARG, *PVMAFD_REG_ARG;

//STORE_MAP structs

typedef struct _VECS_STORE_HANDLE {
  DWORD dwStoreHandle;
  uintptr_t dwClientInstance;
  DWORD dwStoreSessionID;
} VECS_SRV_STORE_HANDLE;

typedef struct _VECS_STORE_CONTEXT_LIST {
  PVM_AFD_SECURITY_CONTEXT pSecurityContext;
  struct _VECS_STORE_CONTEXT_LIST *pNext;
  struct _VECS_STORE_CONTEXT_LIST *pPrev;
} VECS_STORE_CONTEXT_LIST, *PVECS_STORE_CONTEXT_LIST;

typedef struct _VECS_SRV_STORE_MAP {
  STORE_MAP_ENTRY_STATUS status;
  DWORD dwStoreSessionID;
  PVECS_SERV_STORE pStore;
  PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor;
  PVECS_STORE_CONTEXT_LIST pStoreContextList;
} VECS_SRV_STORE_MAP, *PVECS_SRV_STORE_MAP;

typedef struct _VECS_SRV_ENUM_CONTEXT_WITH_HANDLE
{
    PVECS_SRV_STORE_HANDLE pStore;
    DWORD dwIndex;
    ENTRY_INFO_LEVEL infoLevel;

    DWORD dwLimit;

} VECS_SRV_ENUM_CONTEXT_HANDLE, *PVECS_SRV_ENUM_CONTEXT_HANDLE;

typedef struct _VMAFD_HB_NODE
{
    PWSTR  pszServiceName;
    DWORD  dwPort;
    time_t tLastPing;
} VMAFD_HB_NODE, *PVMAFD_HB_NODE;

typedef struct _VMAFD_HB_TABLE
{
    DWORD dwNextAvailableIndex;
    PVMAFD_HB_NODE pEntries[VMAFD_HEARTBEAT_TABLE_COUNT]; //TODO: Configurable
} VMAFD_HB_TABLE, *PVMAFD_HB_TABLE;
