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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LWCA_REST_RSC_API,
    LWCA_REST_RSC_UNKNOWN,
    LWCA_REST_RSC_COUNT,
} LWCA_REST_RESOURCE_TYPE;

typedef struct _LWCA_HTTP_ERROR
{
    int                         httpStatus;
    PSTR                        pszHttpStatus;
    PSTR                        pszHttpReason;
} LWCA_HTTP_ERROR, *PLWCA_HTTP_ERROR;

typedef struct _LWCA_REST_RESULT
{
    // request id
    PSTR                        pszRequestId;

    // error result
    BOOLEAN                     bErrSet;
    int                         errCode;
    PSTR                        pszErrDetail;

    //result data
    PLW_HASHMAP                 pDataMap;
    PSTR                        pszBody;
    DWORD                       dwBodyLen;
} LWCA_REST_RESULT, *PLWCA_REST_RESULT;

typedef DWORD (*PFN_SET_RESULT)(
        PLWCA_REST_RESULT       pRestRslt,
        PSTR                    pRequestId,
        DWORD                   dwErr,
        PCSTR                   pcszErrDetail
        );

typedef DWORD (*PFN_GET_HTTP_ERROR)(
        PLWCA_REST_RESULT       pRestRslt,
        PLWCA_HTTP_ERROR*       ppHttpStatus
        );

typedef struct _LWCA_REST_RESOURCE
{
    LWCA_REST_RESOURCE_TYPE     rscType;
    PCSTR                       pszEndpoint;
    BOOLEAN                     bIsEndpointPrefix;
    PFN_SET_RESULT              pfnSetResult;
    PFN_GET_HTTP_ERROR          pfnGetHttpError;
    PCSTR                       pszContentType;
} LWCA_REST_RESOURCE, *PLWCA_REST_RESOURCE;

typedef struct _LWCA_REST_OPERATION
{
    // basic
    PSTR                        pszMethod;
    PSTR                        pszURI;
    PSTR                        pszPath;
    PSTR                        pszSubPath;
    PSTR                        pszClientIP;
    DWORD                       dwPort;

    // header
    PSTR                        pszAuth;
    PSTR                        pszOrigin;
    PSTR                        pszHeaderConnection;
    PSTR                        pszHeaderIfMatch;
    PSTR                        pszContentType;
    PSTR                        pszDate;

    //payload
    PSTR                        pszBody;
    PLWCA_JSON_OBJECT           pjBody;
    PLW_HASHMAP                 pParamMap;
    PLWCA_REST_RESULT           pResult;
    PLWCA_REST_RESOURCE         pResource;
    PREST_API_METHOD            pMethod;

    // error
    PFN_GET_HTTP_ERROR          pfnGetHttpError;

    // context
    PLWCA_REQ_CONTEXT           pReqCtx;
} LWCA_REST_OPERATION, *PLWCA_REST_OPERATION;

typedef struct _LWCA_SERVER_OPENSSL_GLOBALS
{
    pthread_mutex_t*            pMutexBuf;
    DWORD                       dwMutexBufSize;
    BOOLEAN                     bSSLInitialized;
} LWCA_SERVER_OPENSSL_GLOBALS, *PLWCA_SERVER_OPENSSL_GLOBALS;

#ifdef __cplusplus
}
#endif
