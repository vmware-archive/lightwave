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
#define VMCA_CONFIG_PARAMETER_KEY_PATH  "Services\\vmca\\Parameters"
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

#ifndef _WIN32
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

typedef DWORD VMCA_FUNC_LEVEL;

#define VMCA_UNKNOWN                0x00000001
#define VMCA_ROOT_INIT_FAILED       0x00000002
#define VMCA_ROOT_INIT_DONE         0x00000004

#define VMCA_FUNC_LEVEL_INITIAL     0x00000000
#define VMCA_FUNC_LEVEL_SELF_CA     0x00000004

typedef struct _VMCA_POLICY VMCA_POLICY, *PVMCA_POLICY;

typedef struct _VMCA_REQ_CONTEXT
{
    PSTR            pszAuthPrincipal;
    BOOLEAN         bHasAdminPrivilege;
} VMCA_REQ_CONTEXT, *PVMCA_REQ_CONTEXT;

typedef enum
{
    VMCAD_STARTUP = 0,
    VMCAD_NORMAL,
    VMCAD_SHUTDOWN
} VMCA_SERVER_STATE;

typedef PVOID (*PFN_VMCA_THR_FUNC)(PVOID pData);

typedef struct _VMCA_THREAD_DATA
{
    pthread_mutex_t                 mutex;
    pthread_mutex_t*                pMutex;

    pthread_cond_t                  cond;
    pthread_cond_t*                 pCond;

    BOOLEAN                         bShutdown;

    PVOID                           pData;

} VMCA_THREAD_DATA, *PVMCA_THREAD_DATA;

typedef struct _VMCA_THREAD
{
    LONG                            refCount;

    pthread_t                       thread;
    pthread_t*                      pThread;

    PVMCA_THREAD_DATA               pThrData;

} VMCA_THREAD, *PVMCA_THREAD;

typedef struct _VMCA_DIR_SYNC_PARAMS
{
    LONG                            refCount;

    pthread_mutex_t                 mutex;
    pthread_mutex_t*                pMutex;

    DWORD                           dwSyncIntervalSecs;

    time_t                          lastUpdateTime;

    BOOLEAN                         bRefresh;

} VMCA_DIR_SYNC_PARAMS, *PVMCA_DIR_SYNC_PARAMS;

typedef struct _VMCA_SERVER_GLOBALS
{
    pthread_mutex_t                 mutex;
    pthread_mutex_t                 mutexCRL;
    DWORD                           dwCurrentCRLNumber;

    pthread_rwlock_t                svcMutex;

    FILE*                           fVMCALog;

    // Security descriptor for VMCA-Service resources.
    // PSECURITY_DESCRIPTOR_ABSOLUTE   gpVMCAServSD;

    dcethread*                      pRPCServerThread;
    VMCA_SERVER_STATE               vmcadState;

    PVMCA_X509_CA                   pCA;
    VMCA_FUNC_LEVEL                 dwFuncLevel;

    PVMCA_DIR_SYNC_PARAMS           pDirSyncParams;
    PVMCA_THREAD                    pDirSyncThr;

    HANDLE                          gpEventLog;

    PVMCA_POLICY                    *gppPolicies;

    BOOLEAN                         bDisableVECSIntegration;
    SSL_CTX*                        gpVMCASslCtx;

} VMCA_SERVER_GLOBALS, *PVMCA_SERVER_GLOBALS;

extern VMCA_SERVER_GLOBALS gVMCAServerGlobals;

/* ../common/config.c */

DWORD
VMCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    );

/* ../common/jsonutils.c */

typedef struct json_t   _VMCA_JSON_OBJECT;
typedef _VMCA_JSON_OBJECT   *PVMCA_JSON_OBJECT;

DWORD
VMCAJsonLoadObjectFromFile(
    PCSTR                   pcszFilePath,
    PVMCA_JSON_OBJECT       *ppJsonConfig
    );

DWORD
VMCAJsonGetObjectFromKey(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszKey,
    PVMCA_JSON_OBJECT       *ppJsonValue
    );

DWORD
VMCAJsonGetStringFromKey(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszKey,
    PSTR                    *ppszValue
    );

VOID
VMCAJsonCleanupObject(
    PVMCA_JSON_OBJECT       pJson
    );

/* ../common/ldap.c */

DWORD
VMCAOpenLocalLdapServer(
    PVMCA_LDAP_CONTEXT* pLd
    );

/* ../common/opensslutil.c */

DWORD
VMCAOpenSSLGetValuesFromSubjectName(
    PCSTR                           pszPKCS10Request,
    DWORD                           dwNIDType,
    PDWORD                          pdwNumValues,
    PSTR                            **pppszValues
    );

DWORD
VMCAOpenSSLGetSANEntries(
    PCSTR                           pszPKCS10Request,
    PDWORD                          pdwNumSANDNSEntries,
    PSTR                            **pppszSANDNSEntires,
    PDWORD                          pdwNumSANIPEntries,
    PSTR                            **pppszSANIPEntires
    );

/* ../common/util.c */

BOOLEAN
VMCAUtilIsValueIPAddr(
    PCSTR           pszValue
    );

BOOLEAN
VMCAUtilIsValuePrivateOrLocalIPAddr(
    PSTR            pszValue
    );

DWORD
VMCAUtilIsValueFQDN(
    PCSTR           pszValue,
    PBOOLEAN        pbIsValid
    );

BOOLEAN
VMCAUtilDoesValueHaveWildcards(
    PCSTR            pszValue
    );

DWORD
VMCAUtilIsValueInWhitelist(
    PCSTR                           pszValue,
    PCSTR                           pszAuthUPN,
    PCSTR                           pcszRegValue,
    PBOOLEAN                        pbInWhitelist
    );

DWORD
VMCAGenerateX509Serial(
    ASN1_INTEGER *pSerial
);

//../common/misc.h
DWORD
VMCAGetRegKeyValueDword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue
    );

#ifdef __cplusplus
}
#endif

#endif /* _VMCA_SRV_COMMON_H_ */
