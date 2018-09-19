/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

typedef enum
{
    VMCA_REST_RSC_API,
    VMCA_REST_RSC_UNKNOWN,
    VMCA_REST_RSC_COUNT,

} VMCA_REST_RESOURCE_TYPE;

typedef struct _VMCA_HTTP_ERROR
{
    int     httpStatus;
    PSTR    pszHttpStatus;
    PSTR    pszHttpReason;

} VMCA_HTTP_ERROR, *PVMCA_HTTP_ERROR;

typedef struct _VMCA_REST_RESULT
{
    // error result
    BOOLEAN             bErrSet;
    int                 errCode;
    PSTR                pszErrMsg;

    //result data
    PLW_HASHMAP         pDataMap;
    PSTR                pszBody;
    DWORD               dwBodyLen;

} VMCA_REST_RESULT, *PVMCA_REST_RESULT;

typedef DWORD (*PFN_SET_RESULT)(
        PVMCA_REST_RESULT   pRestRslt,
        DWORD               dwErr,
        PSTR                pszErrMsg
        );

typedef DWORD (*PFN_GET_HTTP_ERROR)(
        PVMCA_REST_RESULT   pRestRslt,
        PVMCA_HTTP_ERROR*   ppHttpStatus
        );

typedef struct _VMCA_REST_RESOURCE
{
    VMCA_REST_RESOURCE_TYPE rscType;
    PCSTR                   pszEndpoint;
    BOOLEAN                 bIsEndpointPrefix;
    PFN_SET_RESULT          pfnSetResult;
    PFN_GET_HTTP_ERROR      pfnGetHttpError;
    PCSTR                   pszContentType;
    PCSTR                   pszErrCodeKey;
    PCSTR                   pszErrMsgKey;

} VMCA_REST_RESOURCE, *PVMCA_REST_RESOURCE;

typedef struct _VMCA_REST_OPERATION
{
    PSTR                pszAuth;
    PSTR                pszMethod;
    PSTR                pszURI;
    PSTR                pszPath;
    PSTR                pszSubPath;
    DWORD               dwPort;

    // header
    PSTR                pszOrigin;
    PSTR                pszClientIP;
    PSTR                pszHeaderConnection;
    PSTR                pszHeaderIfMatch;
    PSTR                pszContentType;
    PSTR                pszDate;

    //payload
    PSTR                pszBody;
    json_t*             pjBody;
    PLW_HASHMAP         pParamMap;
    PVMCA_REST_RESULT   pResult;
    PVMCA_REST_RESOURCE pResource;
    PREST_API_METHOD    pMethod;

    PFN_GET_HTTP_ERROR  pfnGetHttpError;
} VMCA_REST_OPERATION, *PVMCA_REST_OPERATION;
