/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef VM_HTTPCLIENT_H_
#define VM_HTTPCLIENT_H_

/* client handle */
typedef struct _VM_HTTP_CLIENT VM_HTTP_CLIENT, *PVM_HTTP_CLIENT;

typedef enum _VM_HTTP_METHOD
{
    VMHTTP_METHOD_GET,
    VMHTTP_METHOD_POST,
    VMHTTP_METHOD_PUT,
    VMHTTP_METHOD_DELETE,
    VMHTTP_METHOD_PATCH
}VM_HTTP_METHOD;

typedef enum _VM_HTTP_TOKEN_TYPE
{
    VMHTTP_TOKEN_TYPE_BEARER,
    VMHTTP_TOKEN_TYPE_HOTK_PK
} VM_HTTP_TOKEN_TYPE;

/*
 * Initialize http client
 */
DWORD
VmHttpClientInit(
    PVM_HTTP_CLIENT *pClient,
    PCSTR pszCAPath
    );

DWORD
VmHttpClientGetQueryStringLength(
    PVM_HTTP_CLIENT pClient,
    int *pnLength
    );

DWORD
VmHttpClientGetQueryString(
    PVM_HTTP_CLIENT pClient,
    PSTR *ppszParams
    );

DWORD
VmHttpClientPerform(
    PVM_HTTP_CLIENT pClient,
    VM_HTTP_METHOD nMethod,
    PCSTR pszUrl
    );

DWORD
VmHttpClientSetToken(
    PVM_HTTP_CLIENT pClient,
    VM_HTTP_TOKEN_TYPE tokenType,
    PCSTR pszToken
    );

DWORD
VmHttpClientSkipCertValidation(
    PVM_HTTP_CLIENT pClient
    );

DWORD
VmHttpClientGetResult(
    PVM_HTTP_CLIENT pClient,
    PCSTR *ppszResult
    );

DWORD
GetRequestMethodInString(
    VM_HTTP_METHOD  httpMethod,
    PCSTR           *ppcszHttpMethod
    );

DWORD
VmHttpClientRequestPOPSignature(
    VM_HTTP_METHOD  httpMethod,
    PCSTR           pcszRequestURI,
    PCSTR           pcszRequestBody,
    PCSTR           pcszPEM,
    PCSTR           pcszRequestTime,
    PSTR            *ppszSignature
    );
/*
 * Free handle
 */
VOID
VmHttpClientFreeHandle(
    PVM_HTTP_CLIENT pClient
    );

#endif /* VM_HTTPCLIENT_H_ */
