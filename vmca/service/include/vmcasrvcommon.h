/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef _VMCA_SRV_COMMON_H_
#define _VMCA_SRV_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#else
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#endif

#if 0 /* TBD: Adam */
#define VMDIR_REG_KEY_DC_ACCOUNT_DN "dcAccountDN"
#endif
#define VMDIR_REG_KEY_DC_PASSWORD   "dcAccountPassword"
#define VMDIR_REG_KEY_DC_ACCOUNT    "dcAccount"
#define VMAFD_REG_KEY_DOMAIN_NAME   "DomainName"

typedef struct _VMCA_REQ_CONTEXT
{
    PSTR            pszAuthPrincipal;
} VMCA_REQ_CONTEXT, *PVMCA_REQ_CONTEXT;

/* ../common/config.c */

DWORD
VMCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    );

/* ../common/ldap.c */

DWORD
VMCAOpenLocalLdapServer(
    PVMCA_LDAP_CONTEXT* pLd
    );

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VMCA_SRV_COMMON_H_ */
