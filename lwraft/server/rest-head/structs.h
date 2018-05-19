/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

typedef enum
{
    VDIR_REST_AUTH_METHOD_UNDEF,
    VDIR_REST_AUTH_METHOD_BASIC,
    VDIR_REST_AUTH_METHOD_TOKEN

} VDIR_REST_AUTH_METHOD;

typedef enum
{
    VDIR_REST_RSC_LDAP,
    VDIR_REST_RSC_OBJECT,
    VDIR_REST_RSC_ETCD,
    VDIR_REST_RSC_METRICS,
    VDIR_REST_RSC_UNKNOWN,
    VDIR_REST_RSC_IDP,
    VDIR_REST_RSC_COUNT

} VDIR_REST_RESOURCE_TYPE;

// httperror.c
typedef struct _VDIR_HTTP_ERROR
{
    int     httpStatus;
    DWORD   dwHttpStatus;
    PSTR    pszHttpStatus;
    PSTR    pszHttpReason;

} VDIR_HTTP_ERROR, *PVDIR_HTTP_ERROR;

// proxy.c
typedef struct _VDIR_PROXY_RESULT
{
    DWORD   statusCode;
    DWORD   dwError;
    DWORD   dwCurlError;
    PSTR    pResponse;
    DWORD   dwResponseLen;

} VDIR_PROXY_RESULT, *PVDIR_PROXY_RESULT;

// result.c
typedef struct _VDIR_REST_RESULT
{
    // error result
    BOOLEAN             bErrSet;
    int                 errCode;
    PSTR                pszErrMsg;
    PVDIR_HTTP_ERROR    pHttpError;

    // proxy result
    PVDIR_PROXY_RESULT  pProxyResult;

    // result data
    PLW_HASHMAP         pDataMap;
    PSTR                pszBody;
    DWORD               dwBodyLen;

} VDIR_REST_RESULT, *PVDIR_REST_RESULT;

// resource.c
typedef DWORD (*PFN_SET_RESULT)(
        PVDIR_REST_RESULT   pRestRslt,
        PVDIR_LDAP_RESULT   pLdapRslt,
        DWORD               dwErr,
        PSTR                pszErrMsg
        );

typedef DWORD (*PFN_GET_HTTP_ERROR)(
        PVDIR_REST_RESULT   pRestRslt,
        PVDIR_HTTP_ERROR*   ppHttpStatus
        );

typedef struct _VDIR_REST_RESOURCE
{
    VDIR_REST_RESOURCE_TYPE rscType;
    PCSTR                   pszEndpoint;
    BOOLEAN                 bIsEndpointPrefix;
    PFN_SET_RESULT          pfnSetResult;
    PFN_GET_HTTP_ERROR      pfnGetHttpError;
    PCSTR                   pszContentType;
    PCSTR                   pszErrCodeKey;
    PCSTR                   pszErrMsgKey;

} VDIR_REST_RESOURCE, *PVDIR_REST_RESOURCE;

// operation.c
typedef struct _VDIR_REST_OPERATION
{
    DWORD                   dwPort;
    PSTR                    pszAuth;
    PSTR                    pszMethod;
    PSTR                    pszPath;
    PSTR                    pszSubPath;

    // header
    PSTR                    pszHeaderIfMatch;
    PSTR                    pszContentType;
    PSTR                    pszClientIP;
    PSTR                    pszHeaderConnection;
    PSTR                    pszDate;
    PSTR                    pszOrigin;
    BOOLEAN                 bisValidOrigin;

    // payload
    PSTR                    pszBody;
    json_t*                 pjBody;
    PLW_HASHMAP             pParamMap;
    VDIR_REST_AUTH_METHOD   authMthd;
    PVDIR_CONNECTION        pConn;
    PVDIR_REST_RESULT       pResult;
    PVDIR_REST_RESOURCE     pResource;
    PREST_API_METHOD        pMethod;

    // if leader, get it from resource.c
    // otherwise, get it from proxy.c
    PFN_GET_HTTP_ERROR      pfnGetHttpError;

} VDIR_REST_OPERATION, *PVDIR_REST_OPERATION;

// authtoken.c
typedef enum
{
    VDIR_REST_AUTH_TOKEN_BEARER,
    VDIR_REST_AUTH_TOKEN_HOTK

} VDIR_REST_AUTH_TOKEN_TYPE;

typedef struct _VDIR_REST_AUTH_TOKEN
{
    VDIR_REST_AUTH_TOKEN_TYPE   tokenType;
    PSTR                        pszAccessToken;
    PSTR                        pszSignatureHex;
    PSTR                        pszBindUPN;
    PSTR                        pszHOTKPEM;

} VDIR_REST_AUTH_TOKEN, *PVDIR_REST_AUTH_TOKEN;

// vmafd.c
typedef DWORD (*PFN_VMAFD_GET_DC_NAME)(
        PCSTR,
        PSTR*
        );

typedef DWORD (*PFN_VMAFD_GET_DOMAIN_NAME)(
        PCSTR,
        PSTR*
        );

typedef DWORD (*PFN_VMAFD_GET_MACHINE_ACCOUNT_INFO)(
        PCSTR,
        PSTR*,
        PSTR*
        );

typedef struct _VDIR_VMAFD_API
{
    VMDIR_LIB_HANDLE                    pVmAfdLib;
    PFN_VMAFD_GET_DC_NAME               pfnGetDCName;
    PFN_VMAFD_GET_DOMAIN_NAME           pfnGetDomainName;
    PFN_VMAFD_GET_MACHINE_ACCOUNT_INFO  pfnGetMachineAccountInfo;

} VDIR_VMAFD_API, *PVDIR_VMAFD_API;

// cache.c
typedef struct _VDIR_REST_HEAD_CACHE
{
    PVMDIR_RWLOCK   pRWLock;
    PSTR            pszOIDCSigningCertPEM;
    PSID            pBuiltInAdminsGroupSid;

} VDIR_REST_HEAD_CACHE, *PVDIR_REST_HEAD_CACHE;

// curlhandlecache.c
typedef struct _VDIR_REST_CURL_HANDLE_CACHE
{
    PVMDIR_MUTEX    pCacheMutex;
    PLW_HASHMAP     pCurlHandleMap;
} VDIR_REST_CURL_HANDLE_CACHE, *PVDIR_REST_CURL_HANDLE_CACHE;
