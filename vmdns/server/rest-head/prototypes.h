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

//// httperror.c
PVDNS_HTTP_ERROR
VmDnsRESTGetHttpError(
    int httpStatus
    );

// libmain.c
DWORD
VmDnsRESTRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    );

// metricsapi.c
DWORD
VmDnsRESTGetMetricsModule(
    PREST_MODULE*   ppRestModule
    );

DWORD
VmDnsRESTMetricsGet(
    PVOID           pIn,
    PVOID*          ppOut
    );

// operation.c
DWORD
VmDnsRESTOperationCreate(
    PVDNS_REST_OPERATION*   ppRestOp
    );

DWORD
VmDnsRESTOperationReadRequest(
    PVDNS_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq
    );

DWORD
VmDnsRESTOperationWriteResponse(
    PVDNS_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    );

VOID
VmDnsFreeRESTOperation(
    PVDNS_REST_OPERATION    pRestOp
    );

VOID
VmDnsSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

// resource.c
PVDNS_REST_RESOURCE
VmDnsRESTGetResource(
    PSTR    pszPath
    );

DWORD
VmDnsRESTUnknownSetResult(
    PVDNS_REST_RESULT   pRestRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    );

DWORD
VmDnsRESTUnknownGetHttpError(
    PVDNS_REST_RESULT   pRestRslt,
    PSTR*               ppszHttpStatus,
    PSTR*               ppszHttpReason
    );

// result.c
DWORD
VmDnsRESTResultCreate(
    PVDNS_REST_RESULT*  ppRestRslt
    );

DWORD
VmDnsRESTResultSetError(
    PVDNS_REST_RESULT   pRestRslt,
    int                 errCode,
    PSTR                pszErrMsg
    );

DWORD
VmDnsRESTResultSetStrData(
    PVDNS_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    PSTR                pszVal
    );

DWORD
VmDnsRESTResultSetIntData(
    PVDNS_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    int                 iVal
    );

DWORD
VmDnsRESTResultSetObjData(
    PVDNS_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    json_t*             pjVal
    );

DWORD
VmDnsRESTResultToResponseBody(
    PVDNS_REST_RESULT   pRestRslt,
    PVDNS_REST_RESOURCE pResource,
    PSTR*               ppszBody
    );

VOID
VmDnsFreeRESTResult(
    PVDNS_REST_RESULT   pRestRslt
    );
