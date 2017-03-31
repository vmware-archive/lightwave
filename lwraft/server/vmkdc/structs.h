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
 * Module Name: Kdc Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Kdc Main module
 *
 * Private Structures
 *
 */

typedef enum
{
    VMKDC_CONFIG_VALUE_TYPE_STRING = 0,
    VMKDC_CONFIG_VALUE_TYPE_DWORD,
    VMKDC_CONFIG_VALUE_TYPE_BOOLEAN
} VMKDC_CONFIG_VALUE_TYPE;

typedef struct _VMKDC_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMKDC_RPC_ENDPOINT, *PVMKDC_RPC_ENDPOINT;

#ifdef _WIN32

typedef struct _VMKDC_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMKDC_NTSERVICE_DATA, *PVMKDC_NTSERVICE_DATA;

#endif

typedef struct _VMKDC_CONFIG_CONNECTION_HANDLE
{
#ifndef _WIN32
    HANDLE hConnection;
#endif
    HKEY hKey;
} VMKDC_CONFIG_CONNECTION_HANDLE, *PVMKDC_CONFIG_CONNECTION_HANDLE;

typedef struct _VMKDC_CONFIG_ENTRY
{
    PCSTR   pszName;
    VMKDC_CONFIG_VALUE_TYPE Type;
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
} VMKDC_CONFIG_ENTRY, *PVMKDC_CONFIG_ENTRY;
