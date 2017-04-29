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
 * Module Name: Directory Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory Main module
 *
 * Private Structures
 *
 */

typedef enum
{
    VMDIR_CONFIG_VALUE_TYPE_STRING = 0,
    VMDIR_CONFIG_VALUE_TYPE_MULTISTRING,
    VMDIR_CONFIG_VALUE_TYPE_DWORD,
    VMDIR_CONFIG_VALUE_TYPE_BOOLEAN
} VMDIR_CONFIG_VALUE_TYPE;

typedef struct _VMDIR_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMDIR_RPC_ENDPOINT, *PVMDIR_RPC_ENDPOINT;

#ifdef _WIN32

typedef struct _VMDIR_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMDIR_NTSERVICE_DATA, *PVMDIR_NTSERVICE_DATA;

#endif

typedef struct _VMDIR_CONFIG_CONNECTION_HANDLE
{
    HANDLE hConnection;
    HKEY hKey;
} VMDIR_CONFIG_CONNECTION_HANDLE, *PVMDIR_CONFIG_CONNECTION_HANDLE;

typedef struct _VMDIR_CONFIG_ENTRY
{
    PCSTR   pszName;
    VMDIR_CONFIG_VALUE_TYPE Type;
    REG_DATA_TYPE RegDataType;    //Corresponding likewise type

    DWORD dwMin;                  //DWORD type min value
    DWORD dwMax;                  //DWORD type max value
    DWORD dwDefault;              //DWORD type default value
    DWORD dwValue;                //DWORD type value

    PSTR  pszDefault;             //SZ type default value
    PSTR  pszValue;               //SZ type value

} VMDIR_CONFIG_ENTRY, *PVMDIR_CONFIG_ENTRY;

typedef struct _VMDIR_SRV_ACCESS_TOKEN
{
    LONG refCount;

    PSTR pszUPN;

} VMDIR_SRV_ACCESS_TOKEN, *PVMDIR_SRV_ACCESS_TOKEN;

typedef struct _VMDIR_LOGIN_TIME
{
    uint64_t loginTime;
    PSTR    pszDN;

} VMDIR_LOGIN_TIME, *PVMDIR_LOGIN_TIME;

typedef struct _VMDIR_INTEGRITY_JOB_CTX
{
    PSTR        pszPartnerName;
    LDAP*       pLd;
    PSTR        pszRptFileName;
    FILE*       fp;
    DWORD       dwFailedDigestCnt;
    DWORD       dwMissedEntryCnt;
    VMDIR_INTEGRITY_CHECK_JOBCTX_STATE state;

} VMDIR_INTEGRITY_JOB_CTX, *PVMDIR_INTEGRITY_JOB_CTX;

typedef struct _VMDIR_INTEGRITY_JOB
{
    struct timespec             startTime;
    struct tm                   startTM;
    struct timespec             endTime;
    CHAR                        finishedTimebuf[MAX_PATH];
    ENTRYID                     maxEntryID;
    ENTRYID                     currentEntryID;
    DWORD                       dwNumProcessed;
    PVMDIR_INTEGRITY_JOB_CTX    pJobctx;
    DWORD                       dwNumJobCtx;
    VMDIR_INTEGRITY_CHECK_JOB_STATE state;

} VMDIR_INTEGRITY_JOB, *PVMDIR_INTEGRITY_JOB;

