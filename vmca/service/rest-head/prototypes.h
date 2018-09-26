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

// handler.c
DWORD
VMCARestApiRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    );

DWORD
VMCARestRequestHandler(
    PREST_API_DEF   pRestApiDef,
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    );

DWORD
VMCARestProcessRequest(
    PREST_API_DEF           pRestApiDef,
    PVMCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest,
    uint32_t                paramsCount
    );

DWORD
VMCARestWriteSimpleErrorResponse(
    PVMREST_HANDLE  pRESTHandle,
    PREST_RESPONSE* ppResponse,
    int             httpStatus
    );

// httperror.c
PVMCA_HTTP_ERROR
VMCARestGetHttpError(
    int httpStatus
    );

// operation.c
DWORD
VMCARestOperationCreate(
    PVMCA_REST_OPERATION*   ppRestOp
    );

DWORD
VMCARestOperationReadRequest(
    PVMCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    );

DWORD
VMCARestOperationParseRequestPayload(
    PVMCA_REST_OPERATION    pRestOp
    );

DWORD
VMCARestOperationProcessRequest(
    PVMCA_REST_OPERATION    pRestOp
    );

DWORD
VMCARestOperationWriteResponse(
    PVMCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    );

VOID
VMCAFreeRESTOperation(
    PVMCA_REST_OPERATION    pRestOp
    );

// resource.c
PVMCA_REST_RESOURCE
VMCARestGetResource(
    PSTR    pszPath
    );

DWORD
VMCARestUnknownSetResult(
    PVMCA_REST_RESULT   pRestRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    );

DWORD
VMCARestUnknownGetHttpError(
    PVMCA_REST_RESULT   pRestRslt,
    PVMCA_HTTP_ERROR*   ppHttpError
    );

// result.c
DWORD
VMCARestResultCreate(
    PVMCA_REST_RESULT*  ppRestRslt
    );

DWORD
VMCARestResultSetError(
    PVMCA_REST_RESULT   pRestRslt,
    int                 errCode,
    PSTR                pszErrMsg
    );

DWORD
VMCARestResultUnsetError(
    PVMCA_REST_RESULT   pRestRslt
    );

DWORD
VMCARestResultSetStrData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    PSTR                pszVal
    );

DWORD
VMCARestResultSetIntData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    int                 iVal
    );

DWORD
VMCARestResultSetBooleanData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    BOOLEAN             bVal
    );

DWORD
VMCARestResultSetObjData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    json_t*             pjVal
    );

DWORD
VMCARestResultGenerateResponseBody(
    PVMCA_REST_RESULT   pRestRslt,
    PVMCA_REST_RESOURCE pResource
    );

VOID
VMCASetRestResult(
    PVMCA_REST_OPERATION    pRestOp,
    DWORD                   dwError,
    PSTR                    pszErrMsg
    );

VOID
VMCAFreeRESTResult(
    PVMCA_REST_RESULT   pRestRslt
    );

// openssl.c
DWORD
VMCAOpensslInit(
    VOID
    );

VOID
VMCAOpensslShutdown(
    VOID
    );
