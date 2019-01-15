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

#ifdef __cplusplus
extern "C" {
#endif

// auth.c

DWORD
LwCARestAuth(
    PLWCA_REST_OPERATION    pRestOp,
    PLWCA_REQ_CONTEXT       *ppReqCtx,
    PBOOLEAN                pbAuthenticated
    );

// handler.c
DWORD
LwCARestApiRequestHandler(
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest,
    PREST_RESPONSE*         ppResponse,
    uint32_t                paramsCount
    );

DWORD
LwCARestMetricsApiRequestHandler(
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest,
    PREST_RESPONSE*         ppResponse,
    uint32_t                paramsCount
    );

// httperror.c
PLWCA_HTTP_ERROR
LwCARestGetHttpError(
    int                     httpStatus
    );

// operation.c
DWORD
LwCARestOperationCreate(
    PLWCA_REST_OPERATION*   ppRestOp
    );

DWORD
LwCARestOperationReadRequest(
    PLWCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    );

DWORD
LwCARestOperationParseRequestPayload(
    PLWCA_REST_OPERATION    pRestOp
    );

DWORD
LwCARestOperationProcessRequest(
    PLWCA_REST_OPERATION    pRestOp,
    PREST_API_DEF           pRestApiDef
    );

DWORD
LwCARestOperationWriteResponse(
    PLWCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    );

VOID
LwCAFreeRESTOperation(
    PLWCA_REST_OPERATION    pRestOp
    );

// openssl.c
DWORD
LwCAOpensslInit(
    VOID
    );

VOID
LwCAOpensslShutdown(
    VOID
    );

// param.c
DWORD
LwCARestGetStrParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PSTR*                   ppszVal,
    BOOLEAN                 bRequired
    );

DWORD
LwCARestGetIntParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    int*                    piVal,
    BOOLEAN                 bRequired
    );

DWORD
LwCARestGetBoolParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    BOOLEAN*                pbVal,
    BOOLEAN                 bRequired
    );

// resource.c
PLWCA_REST_RESOURCE
LwCARestGetResource(
    PCSTR                   pcszPath
    );

DWORD
LwCARestUnknownSetResult(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszRequestId,
    DWORD                   dwErr,
    PCSTR                   pcszErrMsg
    );

DWORD
LwCARestUnknownGetHttpError(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszMethod,
    PLWCA_HTTP_ERROR*       ppHttpError
    );

// result.c
DWORD
LwCARestResultCreate(
    PLWCA_REST_RESULT*      ppRestRslt
    );

DWORD
LwCARestResultSetError(
    PLWCA_REST_RESULT       pRestRslt,
    int                     errCode,
    PCSTR                   pcszErrMsg
    );

DWORD
LwCARestResultUnsetError(
    PLWCA_REST_RESULT       pRestRslt
    );

DWORD
LwCARestResultSetStrData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    PCSTR                   pcszVal
    );

DWORD
LwCARestResultSetIntData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    int                     iVal
    );

DWORD
LwCARestResultSetBooleanData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    BOOLEAN                 bVal
    );

DWORD
LwCARestResultSetStrArrayData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    PLWCA_STRING_ARRAY      pVal
    );

DWORD
LwCARestResultSetCertArrayData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    PLWCA_CERTIFICATE_ARRAY pVal
    );

DWORD
LwCARestResultSetObjData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    PLWCA_JSON_OBJECT       pjVal
    );

DWORD
LwCARestResultGenerateResponseBody(
    PLWCA_REST_RESULT       pRestRslt,
    PLWCA_REST_RESOURCE     pResource,
    PLWCA_HTTP_ERROR        pHttpError
    );

VOID
LwCASetRestResult(
    PLWCA_REST_OPERATION    pRestOp,
    DWORD                   dwError
    );

VOID
LwCAFreeRESTResult(
    PLWCA_REST_RESULT       pRestRslt
    );


// rootcaapi.c
DWORD
LwCARestRootCAModule(
    PREST_MODULE*           ppRestModule
    );

DWORD
LwCARestGetVersion(
    PVOID                   pIn,
    PVOID*                  ppOut
    );

DWORD
LwCARestGetRootCACert(
    PVOID                   pIn,
    PVOID*                  ppOut
    );

// intermediatecaapi.c
DWORD
LwCARestIntermediateCAModule(
    PREST_MODULE*           ppRestModule
    );

DWORD
LwCARestCreateIntermediateCA(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestGetIntermediateCACert(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestRevokeIntermediateCA(
    PVOID                   pIn,
    PVOID*                  pOut
    );


// crlapi.c
DWORD
LwCARestCRLModule(
    PREST_MODULE*           ppRestModule
    );

DWORD
LwCARestGetRootCACRL(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestGetIntermediateCACRL(
    PVOID                   pIn,
    PVOID*                  pOut
    );

// certificatesapi.c
DWORD
LwCARestCertificatesModule(
    PREST_MODULE*           ppRestModule
    );

DWORD
LwCARestGetRootCASignedCert(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestRevokeRootCASignedCert(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestGetIntermediateCASignedCert(
    PVOID                   pIn,
    PVOID*                  pOut
    );

DWORD
LwCARestRevokeIntermediateCASignedCert(
    PVOID                   pIn,
    PVOID*                  pOut
    );

// metricsapi.c
DWORD
LwCARestMetricsModule(
    PREST_MODULE*           ppRestModule
    );

DWORD
LwCARestGetMetrics(
    PVOID                   pIn,
    PVOID*                  ppOut
    );

// utils.c
DWORD
LwCARestMakeGetCAJsonResponse(
    PLWCA_CERTIFICATE_ARRAY     pCACerts,
    PLWCA_STRING_ARRAY          pCRLs,
    BOOLEAN                     bDetail,
    PLWCA_JSON_OBJECT           *ppJsonRespArray
    );

LWCA_METRICS_REQ_URLS
LwCARestMetricsGetReqUrl(
    PCSTR                   pcszPath
    );

LWCA_METRICS_HTTP_METHODS
LwCARestMetricsGetHttpMethod(
    PCSTR                   pcszMethod
    );

LWCA_METRICS_HTTP_CODES
LwCARestMetricsGetHttpCode(
    int                     httpStatus
    );
#ifdef __cplusplus
}
#endif
