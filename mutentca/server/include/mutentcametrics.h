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

#ifndef __MUTENTCA_METRICS_H__
#define __MUTENTCA_METRICS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern PVM_METRICS_CONTEXT gpmContext;

typedef enum
{
    LWCA_METRICS_REQ_URL_UNKNOWN = -1,
    LWCA_METRICS_REQ_URL_ROOT,
    LWCA_METRICS_REQ_URL_ROOT_CERT,
    LWCA_METRICS_REQ_URL_ROOT_CRL,
    LWCA_METRICS_REQ_URL_INTERMEDIATE,
    LWCA_METRICS_REQ_URL_INTERMEDIATE_CERT,
    LWCA_METRICS_REQ_URL_INTERMEDIATE_CRL,
    LWCA_METRICS_REQ_URL_COUNT
} LWCA_METRICS_REQ_URLS;

typedef enum
{
    LWCA_METRICS_HTTP_METHOD_UNKNOWN = -1,
    LWCA_METRICS_HTTP_METHOD_GET,
    LWCA_METRICS_HTTP_METHOD_POST,
    LWCA_METRICS_HTTP_METHOD_DELETE,
    LWCA_METRICS_HTTP_METHOD_COUNT
} LWCA_METRICS_HTTP_METHODS;

typedef enum
{
    LWCA_METRICS_HTTP_OK = 0,
    LWCA_METRICS_HTTP_NO_CONTENT,
    LWCA_METRICS_HTTP_BAD_REQUEST,
    LWCA_METRICS_HTTP_UNAUTHORIZED,
    LWCA_METRICS_HTTP_FORBIDDEN,
    LWCA_METRICS_HTTP_NOT_FOUND,
    LWCA_METRICS_HTTP_CONFLICT,
    LWCA_METRICS_HTTP_SERVER_ERROR,
    LWCA_METRICS_HTTP_CODE_COUNT
} LWCA_METRICS_HTTP_CODES;

typedef enum
{
    LWCA_METRICS_API_CREATE_ROOT_CA = 0,
    LWCA_METRICS_API_CREATE_INTERMEDIATE_CA,
    LWCA_METRICS_API_GET_CA_CERTIFICATES,
    LWCA_METRICS_API_GET_SIGNED_CERTIFICATE,
    LWCA_METRICS_API_GET_CHAIN_OF_TRUST,
    LWCA_METRICS_API_GET_CA_CRL,
    LWCA_METRICS_API_REVOKE_INTERMEDIATE_CA,
    LWCA_METRICS_API_REVOKE_CERTIFICATE,
    LWCA_METRICS_API_COUNT
} LWCA_METRICS_API_NAMES;

typedef enum
{
    LWCA_METRICS_RESPONSE_SUCCESS = 0,
    LWCA_METRICS_RESPONSE_ERROR,
    LWCA_METRICS_RESPONSE_COUNT
} LWCA_METRICS_RESPONSE_CODES;

// metrics/libmain.c
DWORD
LwCAMetricsInitialize(
    VOID
    );

VOID
LwCAMetricsShutdown(
    VOID
    );

// metrics/operation.c
VOID
LwCARestMetricsUpdate(
    LWCA_METRICS_REQ_URLS       reqUrl,
    LWCA_METRICS_HTTP_METHODS   method,
    LWCA_METRICS_HTTP_CODES     httpCode,
    uint64_t                    iStartTime,
    uint64_t                    iEndTime
    );

VOID
LwCAApiMetricsUpdate(
    LWCA_METRICS_API_NAMES      api,
    LWCA_METRICS_RESPONSE_CODES code,
    uint64_t                    iStartTime,
    uint64_t                    iEndTime
    );

#ifdef __cplusplus
}
#endif

#endif // __MUTENTCA_METRICS_H__
