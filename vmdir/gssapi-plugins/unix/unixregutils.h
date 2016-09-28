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

#ifdef _WIN32
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#else
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#endif

#define VMAFD_REG_KEY_DOMAIN_STATE "DomainState"
#define VMDIR_REG_KEY_MACHINE_ACCT "dcAccountDN"
#define VMDIR_REG_KEY_MACHINE_PWD  "dcAccountPassword"
#define VMDIR_REG_KEY_DC_NAME      "DCName"
#define VMDIR_REG_KEY_DC_NAME_HA   "DCNameHA"


typedef struct _VMDIR_CONFIG_CONNECTION_HANDLE
{
    HANDLE hConnection;
    HKEY hKey;
} VMDIR_CONFIG_CONNECTION_HANDLE, *PVMDIR_CONFIG_CONNECTION_HANDLE;

DWORD
VMCISLIBAccountDnToUpn(
    PSTR dn,
    PSTR *retUpn);

DWORD
VmDirRegConfigHandleOpen(
    PVMDIR_CONFIG_CONNECTION_HANDLE *ppCfgHandle);

VOID
VmDirRegConfigHandleClose(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle
    );

DWORD
VmDirRegConfigGetValue(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    DWORD  valueType,
    PBYTE  pRetValue,
    PDWORD pRetValueLen
    );
