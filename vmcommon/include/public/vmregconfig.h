/*
 * Copyright © 219 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef VM_REGCONFIG_H_
#define VM_REGCONFIG_H_

/* Config context identifier */
typedef struct _VM_REGCONFIG_CONTEXT* PVM_REGCONFIG_CONTEXT;

extern PVM_REGCONFIG_CONTEXT _gpVmRegConfig;

#define VMREGCONFIG_VMDIR_REG_CONFIG_FILE               "/opt/vmware/share/config/vmdircfg.yaml"
#define VMREGCONFIG_VMAFD_REG_CONFIG_FILE               "/opt/vmware/share/config/vmafdcfg.yaml"
#define VMREGCONFIG_VMCA_REG_CONFIG_FILE                "/opt/vmware/share/config/vmcacfg.yaml"
#define VMREGCONFIG_VMDNS_REG_CONFIG_FILE               "/opt/vmware/share/config/vmdnscfg.yaml"
#define VMREGCONFIG_VMSTS_REG_CONFIG_FILE               "/opt/vmware/share/config/vmstscfg.yaml"


/*
 * Initialize the config context
 */
DWORD
VmRegConfigInit(
    VOID
    );

/*
 * Free the config context
 */
VOID
VmRegConfigFree(
    VOID
    );

/*
 * merge new into current config
 * TBD
 */
DWORD
VmRegConfigMergeFile(
    PCSTR               pszCurrentFileName,
    PCSTR               pszNewFileName
    );

/*
 * merge new into current config
 * TBD
 */
DWORD
VmRegConfigMergeKey(
    PCSTR               pszCurrentFileName,
    PCSTR               pszKey,
    PCSTR               pszValue
    );

/*
 * add config file to context
 */
DWORD
VmRegConfigAddFile(
    PCSTR               pszFileName,
    BOOLEAN             bReadOnly
    );

/*
 * delete config file in context
 */
DWORD
VmRegConfigDeleteFile(
    PCSTR               pszFileName
    );

/*
 * get key value
 */
DWORD
VmRegConfigGetKeyA(
    PCSTR              pszKeyName,
    PSTR               pszValue,     /* out */
    size_t*            piValueSize  /* in/out */
    );

/*
 * get MultiSZ key value
 */
DWORD
VmRegConfigGetMultiSZKeyA(
    PCSTR              pszKeyName,
    PSTR               pszValue,     /* out */
    size_t*            piValueSize  /* in/out */
    );

/*
 * set MultiSZ key value
 */
DWORD
VmRegConfigSetMultiSZKeyA(
    PCSTR               pszKeyName,
    PCSTR               pszValue,
    size_t              iValueSize
    );

/*
 * set key value
 */
DWORD
VmRegConfigSetKeyA(
    PCSTR              pszKeyName,
    PCSTR              pszValue,
    size_t             iValueSize
    );

/*
 * delete key
 */
DWORD
VmRegConfigDeleteKeyA(
    PCSTR               pszKeyName
    );

/*
 * wrapper get string key
 */
DWORD
VmRegCfgGetKeyStringA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PSTR                pszValue,
    size_t              iValueLen
    );

/*
 * wrapper get multisz key
 */
DWORD
VmRegCfgGetKeyMultiSZA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PSTR                pszValue,     /* out */
    size_t*             piValueSize   /* in/out */
    );

/*
 * wrapper get dword key
 */
DWORD
VmRegCfgGetKeyDword(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PDWORD              pdwValue,
    DWORD               dwDefault
    );

/*
 * wrapper set string key
 */
DWORD
VmRegCfgSetKeyStringA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PCSTR               pszValue
    );

/*
 * wrapper set multisz key
 */
DWORD
VmRegCfgSetKeyMultiSZA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PCSTR               pszValue,
    size_t              iValueSize
    );

/*
 * wrapper set dword key
 */
DWORD
VmRegCfgSetKeyDword(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    DWORD               dwValue
    );

/*
 * wrapper delete key
 */
DWORD
VmRegCfgDeleteKeyA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName
    );


#endif /* VM_REGCONFIG_H_ */
