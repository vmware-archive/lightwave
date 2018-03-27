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
    VDIR_REST_RSC_LDAP,
    VDIR_REST_RSC_METRICS,
    VDIR_REST_RSC_UNKNOWN,
    VDIR_REST_RSC_COUNT,

} VDIR_REST_RESOURCE_TYPE;

typedef struct _VDIR_REST_RESULT
{
    int         errCode;
    PSTR        pszErrMsg;
    PLW_HASHMAP pDataMap;
    PSTR        pszData;
    DWORD       dwDataLen;
    BOOLEAN     bErrSet;

} VDIR_REST_RESULT, *PVDIR_REST_RESULT;

typedef DWORD (*PFN_SET_RESULT)(
        PVDIR_REST_RESULT   pRestRslt,
        PVDIR_LDAP_RESULT   pLdapRslt,
        DWORD               dwErr,
        PSTR                pszErrMsg
        );

typedef DWORD (*PFN_GET_HTTP_ERROR)(
        PVDIR_REST_RESULT   pRestRslt,
        PSTR*               ppszHttpStatus,
        PSTR*               ppszHttpReason
        );

typedef struct _VDIR_REST_RESOURCE
{
    VDIR_REST_RESOURCE_TYPE rscType;
    PCSTR                   pszEndpoint;
    BOOLEAN                 bIsEndpointPrefix;
    PFN_SET_RESULT          pfnSetResult;
    PFN_GET_HTTP_ERROR      pfnGetHttpError;
    PCSTR                   pszErrCodeKey;
    PCSTR                   pszErrMsgKey;

} VDIR_REST_RESOURCE, *PVDIR_REST_RESOURCE;

typedef struct _VDIR_REST_OPERATION
{
    PSTR                pszAuth;
    PSTR                pszMethod;
    PSTR                pszPath;
    PSTR                pszSubPath;
    PSTR                pszOrigin;
    PSTR                pszHeaderIfMatch; // lwraft only
    PSTR                pszHeaderConnection;
    json_t*             pjInput;
    BOOLEAN             bisValidOrigin;
    PLW_HASHMAP         pParamMap;
    PVDIR_CONNECTION    pConn;
    PVDIR_REST_RESULT   pResult;
    PVDIR_REST_RESOURCE pResource;
    PREST_API_METHOD    pMethod;
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
    PSTR                        pszTenant;
    PSTR                        pszBindUPN;

} VDIR_REST_AUTH_TOKEN, *PVDIR_REST_AUTH_TOKEN;

// httperror.c
typedef struct _VDIR_HTTP_ERROR
{
    int     httpStatus;
    PSTR    pszHttpStatus;
    PSTR    pszHttpReason;

} VDIR_HTTP_ERROR, *PVDIR_HTTP_ERROR;

// cache.c
typedef struct _VDIR_REST_HEAD_CACHE
{
    PVMDIR_RWLOCK   pRWLock;
    PLW_HASHMAP     pOIDCSigningCertPEM;

} VDIR_REST_HEAD_CACHE, *PVDIR_REST_HEAD_CACHE;
