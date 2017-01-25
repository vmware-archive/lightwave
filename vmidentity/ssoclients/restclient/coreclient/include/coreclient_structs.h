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

#ifndef INCLUDE_CORECLIENT_STRUCTS_H_
#define INCLUDE_CORECLIENT_STRUCTS_H_

typedef struct REST_ACCESS_TOKEN
{
    PSTRING value;
    REST_ACCESS_TOKEN_TYPE type;
    PSTRING privateKey;
} REST_ACCESS_TOKEN;

typedef struct REST_CLIENT
{
    PSTRING serverHost;
    bool highAvailabilityEnabled;
    PSSO_CDC pCdc;
    size_t serverPort;
    REST_SCHEME_TYPE schemeType;
    REST_ACCESS_TOKEN* pAccessToken;
} REST_CLIENT;

typedef enum
{
    REST_HTTP_METHOD_TYPE_POST,
    REST_HTTP_METHOD_TYPE_GET,
    REST_HTTP_METHOD_TYPE_PUT,
    REST_HTTP_METHOD_TYPE_DELETE
} REST_HTTP_METHOD_TYPE;

typedef enum
{
    REST_JSON_OBJECT_TYPE_STRING,
    REST_JSON_OBJECT_TYPE_INTEGER,
    REST_JSON_OBJECT_TYPE_LONG,
    REST_JSON_OBJECT_TYPE_BOOLEAN,
    REST_JSON_OBJECT_TYPE_OBJECT,
    REST_JSON_OBJECT_TYPE_ARRAY
} REST_JSON_OBJECT_TYPE;

typedef struct
{
    void** ppEntry;
    size_t length;
} REST_GENERIC_ARRAY_DATA;

typedef struct
{
    const PCSTRING error;
    const SSOERROR code;
} REST_SERVER_ERROR_MAP_ENTRY;

typedef SSOERROR
(*DataObjectToJsonFunc)(
    const void*,
    PSSO_JSON);

typedef SSOERROR
(*JsonToDataObjectFunc)(
    PCSSO_JSON,
    void**);

#endif /* INCLUDE_CORECLIENT_STRUCTS_H_ */
