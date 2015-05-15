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
 * Module Name: Authsvc Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Authsvc Main module
 *
 * Private Structures
 *
 */

typedef enum
{
    VMAUTHSVC_CONFIG_VALUE_TYPE_STRING = 0,
    VMAUTHSVC_CONFIG_VALUE_TYPE_DWORD,
    VMAUTHSVC_CONFIG_VALUE_TYPE_BOOLEAN
} VMAUTHSVC_CONFIG_VALUE_TYPE;

typedef struct _VMAUTHSVC_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMAUTHSVC_RPC_ENDPOINT, *PVMAUTHSVC_RPC_ENDPOINT;

#ifdef _WIN32

typedef struct _VMAUTHSVC_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMAUTHSVC_NTSERVICE_DATA, *PVMAUTHSVC_NTSERVICE_DATA;

#endif

typedef struct _VMAUTHSVC_CONFIG_CONNECTION_HANDLE
{
    HANDLE hConnection;
    HKEY hKey;
} VMAUTHSVC_CONFIG_CONNECTION_HANDLE, *PVMAUTHSVC_CONFIG_CONNECTION_HANDLE;

#ifndef _WIN32
typedef struct _VMAUTHSVC_CONFIG_ENTRY
{
    PCSTR   pszName;
    VMAUTHSVC_CONFIG_VALUE_TYPE Type;
    REG_DATA_TYPE RegDataType;    //Corresponding likewise type
    DWORD dwMin;                  //DWORD type min value
    DWORD dwMax;                  //DWORD type max value
    union
    {
        DWORD dwDefault;          //DWORD type default value
        PSTR  pszDefault;         //SZ type default value
    } defaultValue;
    union
    {
        DWORD dwValue;            //DWORD type value
        // User own this memory
        PSTR  pszValue;           //SZ type value
    } cfgValue;

} VMAUTHSVC_CONFIG_ENTRY, *PVMAUTHSVC_CONFIG_ENTRY;
#endif

