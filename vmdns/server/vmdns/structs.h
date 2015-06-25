/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: dns Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * dns Main module
 *
 * Private Structures
 *
 */

typedef enum _VMDNS_CONFIG_VALUE_TYPE
{
    VMDNS_CONFIG_VALUE_TYPE_STRING = 0,
    VMDNS_CONFIG_VALUE_TYPE_DWORD,
    VMDNS_CONFIG_VALUE_TYPE_BOOLEAN
} VMDNS_CONFIG_VALUE_TYPE;

typedef struct _VMDNS_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMDNS_RPC_ENDPOINT, *PVMDNS_RPC_ENDPOINT;

typedef struct _VMCA_SASL_INTERACTIVE_DEFAULT
{
    PCSTR   pszRealm;
    PCSTR   pszAuthName;
    PCSTR   pszUser;
    PCSTR   pszPass;
} VMDNS_SASL_INTERACTIVE_DEFAULT, *PVMDNS_SASL_INTERACTIVE_DEFAULT;

#ifdef _WIN32

typedef struct _VMDNS_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE   hServiceStatus;
    HANDLE                  stopServiceEvent;
} VMDNS_NTSERVICE_DATA, *PVMDNS_NTSERVICE_DATA;

#endif

typedef struct _VMDNS_DIR_SYNC_CONTEXT
{
    PVMDNS_THREAD               pSyncThread;
    PVMDNS_COND                 pRefreshEvent;
    PVMDNS_MUTEX                pThreadLock;
    BOOL                        bShutdown;
    BOOL                        bRunning;
} VMDNS_DIR_SYNC_CONTEXT, *PVMDNS_DIR_SYNC_CONTEXT;

