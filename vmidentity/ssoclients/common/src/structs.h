/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

typedef struct SSO_HTTP_CLIENT
{
    CURL* pCurl;
} SSO_HTTP_CLIENT;

typedef struct SSO_KEY_VALUE_PAIR
{
    PSTRING pszKey;
    PSTRING pszValue;
} SSO_KEY_VALUE_PAIR;

// internal, not public
typedef struct SSO_HTTP_CLIENT_RESPONSE
{
    PSTRING pszBuffer;
    size_t bufferSize;
} SSO_HTTP_CLIENT_RESPONSE;
typedef       struct SSO_HTTP_CLIENT_RESPONSE*  PSSO_HTTP_CLIENT_RESPONSE;
typedef const struct SSO_HTTP_CLIENT_RESPONSE* PCSSO_HTTP_CLIENT_RESPONSE;

typedef struct SSO_STRING_BUILDER
{
    PSTRING pszBuffer;
    size_t bufferLength;
} SSO_STRING_BUILDER;

typedef struct SSO_JSON
{
    json_t* pJson_t;
} SSO_JSON;

typedef struct SSO_JWT
{
    SSO_JSON* pJsonPayload;
    PSTRING pszSignedData;
    unsigned char* pSignatureBytes;
    size_t signatureByteCount;
} SSO_JWT;

typedef struct SSO_JWK
{
    PSTRING pszModulus;
    PSTRING pszExponent;
    PSTRING pszCertificate;
} SSO_JWK;

typedef struct SSO_JSON_ITERATOR
{
    void* pIter;
} SSO_JSON_ITERATOR;

typedef struct SSO_CDC
{
    PSSO_LIB_HANDLE pHandle;
    PSSO_FUNC_HANDLE pVmAfdOpenServer;
    PSSO_FUNC_HANDLE pCdcGetDCName;
    PSSO_FUNC_HANDLE pVmAfdCloseServer;
    PSSO_FUNC_HANDLE pCdcFreeDomainControllerInfo;
} SSO_CDC;

#endif
