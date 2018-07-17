/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
 * Module Name: VMCA
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Structure definitions
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VMCAD_STARTUP = 0,
    VMCAD_RUNNING,
    VMCA_SHUTDOWN
} VMCA_SERVER_STATE;

typedef struct _VMCA_ENDPOINT
{
    PCSTR pszProtSeq;
    PCSTR pszEndpoint;
} VMCA_ENDPOINT, *PVMCA_ENDPOINT;

typedef PVOID (*PFN_VMCA_THR_FUNC)(PVOID pData);

typedef struct _VMCA_THREAD_DATA
{
    pthread_mutex_t   mutex;
    pthread_mutex_t*  pMutex;

    pthread_cond_t    cond;
    pthread_cond_t*   pCond;

    BOOLEAN           bShutdown;

    PVOID             pData;

} VMCA_THREAD_DATA, *PVMCA_THREAD_DATA;

typedef struct _VMCA_THREAD
{
	LONG              refCount;

    pthread_t         thread;
    pthread_t*        pThread;

    PVMCA_THREAD_DATA pThrData;

} VMCA_THREAD, *PVMCA_THREAD;

typedef struct _VMCA_DIR_SYNC_PARAMS
{
	LONG refCount;

	pthread_mutex_t  mutex;
	pthread_mutex_t* pMutex;

	DWORD   dwSyncIntervalSecs;

	time_t  lastUpdateTime;

	BOOLEAN bRefresh;

} VMCA_DIR_SYNC_PARAMS, *PVMCA_DIR_SYNC_PARAMS;

/* VMCA Policy */

typedef enum _VMCA_POLICY_TYPE
{
    VMCA_POLICY_TYPE_UNDEFINED = 0,
    VMCA_POLICY_TYPE_SN
} VMCA_POLICY_TYPE, *PVMCA_POLICY_TYPE;

// SN Policy Objects

typedef struct _VMCA_SNPOLICY_OPERATION
{
    PSTR            pszData;
    PSTR            pszWith;
} VMCA_SNPOLICY_OPERATION, *PVMCA_SNPOLICY_OPERATION;

typedef struct _VMCA_SNPOLICY
{
    BOOLEAN                         bEnabled;
    DWORD                           dwMatchLen;
    PVMCA_SNPOLICY_OPERATION        *ppMatch;
    DWORD                           dwValidateLen;
    PVMCA_SNPOLICY_OPERATION        *ppValidate;
} VMCA_SNPOLICY, *PVMCA_SNPOLICY;

// High Level Policy Objects

typedef union _VMCA_POLICY_RULES
{
    VMCA_SNPOLICY       SN;
} VMCA_POLICY_RULES, *PVMCA_POLICY_RULES;

typedef struct _VMCA_POLICY
{
    VMCA_POLICY_TYPE            type;
    VMCA_POLICY_RULES           Rules;
} VMCA_POLICY, *PVMCA_POLICY;

typedef DWORD (*VMCA_POLICY_LOAD_FUNC) (
    json_t*                 pJsonPolicy;
    PVMCA_POLICY*           ppPolicy;
    );

typedef VOID (*VMCA_POLICY_FREE_FUNC) (
    PVMCA_POLICY_RULES      pPolicyRules;
    );

typedef struct _VMCA_POLICY_METHODS
{
    VMCA_POLICY_TYPE            type;
    VMCA_POLICY_LOAD_FUNC       pfnLoad;
    VMCA_POLICY_FREE_FUNC       pfnFree;
} VMCA_POLICY_METHODS;

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

} VMCA_SERVER_GLOBALS, *PVMCA_SERVER_GLOBALS;

#ifdef REST_ENABLED

typedef enum
{
    VMCA_AUTHORIZATION_TYPE_UNDEFINED = 0,
    VMCA_AUTHORIZATION_TYPE_BEARER_TOKEN,
    VMCA_AUTHORIZATION_TOKEN_TYPE_KRB,
    VMCA_AUTHORIZATION_TYPE_HOTK_TOKEN,
    VMCA_AUTHORIZATION_TOKEN_TYPE_MAX
} VMCA_AUTHORIZATION_TYPE;

typedef struct _VMCA_ACCESS_TOKEN
{
    VMCA_AUTHORIZATION_TYPE  tokenType;
    PCSTR pszSubjectName;
    int* bKrbTicketValid;
    const PSTR* pszGroups;
    DWORD dwGroupSize;
//    union
//    {
        POIDC_ACCESS_TOKEN pOidcToken;
//    }
} VMCA_ACCESS_TOKEN, *PVMCA_ACCESS_TOKEN;

typedef struct _VMCA_HTTP_REQ_OBJ {
    PSTR                            pszMethod;
    PSTR                            pszUri;
    PSTR                            pszVer;
    PSTR                            pszConnection;
    PSTR                            pszTransferEncoding;
    PSTR                            pszContentLength;
    PSTR                            pszAuthorization;
    PSTR                            pszContentType;
    PSTR                            pszDate;
    PSTR                            pszPayload;
    FILE*                           debugFile;
    PVMCA_ACCESS_TOKEN              pAccessToken;
} VMCA_HTTP_REQ_OBJ, *PVMCA_HTTP_REQ_OBJ;

typedef struct _VMCA_AUTHORIZATION_PARAM
{
    PSTR pszAuthorizationToken;
    VMCA_AUTHORIZATION_TYPE tokenType;
} VMCA_AUTHORIZATION_PARAM, *PVMCA_AUTHORIZATION_PARAM;

typedef DWORD (*VMCA_ACCESS_TOKEN_VERIFY) (
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    );

typedef VOID (*VMCA_ACCESS_TOKEN_FREE) (
    PVMCA_ACCESS_TOKEN pAccessToken
    );

typedef struct _VMCA_ACCESS_TOKEN_METHODS
{
    VMCA_AUTHORIZATION_TYPE       type;
    VMCA_ACCESS_TOKEN_VERIFY      pfnVerify;
    VMCA_ACCESS_TOKEN_FREE        pfnFree;
} VMCA_ACCESS_TOKEN_METHODS;

#endif

#ifdef _WIN32
typedef struct _VMCA_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMCA_NTSERVICE_DATA, *PVMCA_NTSERVICE_DATA;

#endif // _WIN32


#ifdef __cplusplus
}
#endif
