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
    VDNS_REST_RSC_METRICS,
    VDNS_REST_RSC_UNKNOWN,
    VDNS_REST_RSC_COUNT,

} VDNS_REST_RESOURCE_TYPE;

typedef struct _VDNS_REST_RESULT
{
    int         errCode;
    PSTR        pszErrMsg;
    PLW_HASHMAP pDataMap;
    BOOLEAN     bErrSet;
    PSTR        pszData;
    DWORD       dwDataLen;

} VDNS_REST_RESULT, *PVDNS_REST_RESULT;

typedef DWORD (*PFN_SET_RESULT)(
        PVDNS_REST_RESULT   pRestRslt,
        DWORD               dwErr,
        PSTR                pszErrMsg
        );

typedef DWORD (*PFN_GET_HTTP_ERROR)(
        PVDNS_REST_RESULT   pRestRslt,
        PSTR*               ppszHttpStatus,
        PSTR*               ppszHttpReason
        );

typedef struct _VDNS_REST_RESOURCE
{
    VDNS_REST_RESOURCE_TYPE rscType;
    PCSTR                   pszEndpoint;
    BOOLEAN                 bIsEndpointPrefix;
    PFN_SET_RESULT          pfnSetResult;
    PFN_GET_HTTP_ERROR      pfnGetHttpError;
    PCSTR                   pszErrCodeKey;
    PCSTR                   pszErrMsgKey;

} VDNS_REST_RESOURCE, *PVDNS_REST_RESOURCE;

typedef struct _VDNS_REST_OPERATION
{
    PSTR                pszAuth;
    PSTR                pszMethod;
    PSTR                pszPath;
    PSTR                pszSubPath;
    PSTR                pszHeaderIfMatch;
    json_t*             pjInput;
    PVDNS_REST_RESULT   pResult;
    PVDNS_REST_RESOURCE pResource;

} VDNS_REST_OPERATION, *PVDNS_REST_OPERATION;

//// httperror.c
typedef struct _VDNS_HTTP_ERROR
{
    int     httpStatus;
    PSTR    pszHttpStatus;
    PSTR    pszHttpReason;

} VDNS_HTTP_ERROR, *PVDNS_HTTP_ERROR;
