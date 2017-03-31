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

#ifndef INCLUDE_CORECLIENT_PROTOTYPES_H_
#define INCLUDE_CORECLIENT_PROTOTYPES_H_

// Misc

SSOERROR
RestGetHttpFormattedDate(
    PSTRING* ppDate,
    PSTRING* ppDateHeader);

SSOERROR
RestEncodeHex(
    const unsigned char data[],
    const size_t length,
    PSTRING* ppHex);

SSOERROR
RestBuildSigningBytes(
    REST_HTTP_METHOD_TYPE httpMethodType,
    PCSTRING md5Hex,
    PCSTRING mediaType,
    PCSTRING httpFormattedDate,
    PCSTRING resourceUri,
    unsigned char** ppData,
    size_t* pDataSize);

void
RestServerErrorDelete(
    REST_SERVER_ERROR* pError);

SSOERROR
RestJsonToServerError(
    PCSSO_JSON pJson,
    REST_SERVER_ERROR** ppError);

SSOERROR
RestServerErrorGetSSOErrorCode(
    PCSTRING error);

SSOERROR
RestBuildResourceUri(
    PCREST_CLIENT pClient,
    PCSTRING tenantPath,
    PCSTRING tenant,
    PCSTRING resourcePath,
    PCSTRING resource,
    PCSTRING subResourcePath,
    PSTRING* ppResourceUri);

SSOERROR
RestBuildUPN(
    PCSTRING name,
    PCSTRING domain,
    PSTRING* ppUpn);

SSOERROR
RestBuildPostEntity(
    const REST_ACCESS_TOKEN* pAccessToken,
    REST_HTTP_METHOD_TYPE httpMethodType,
    PCSSO_JSON pJson,
    PCSTRING httpFormattedDate,
    PCSTRING resourceUri,
    PSTRING* ppPost);

SSOERROR
RestAppendQueryStringOnResourceUri(
    PCSTRING key,
    PCSTRING value,
    const bool isFirstQuery,
    PSTRING pResourceUriIn,
    PSTRING* ppResourceUriOut);

SSOERROR
RestParseHttpResponse(
    PCSTRING httpResponse,
    long httpStatusCode,
    JsonToDataObjectFunc fJsonToDataObject,
    void** ppDataObjectReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
RestBuildAndExecuteHttp(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson,
    const REST_ACCESS_TOKEN* pAccessToken,
    PCSTRING resourceUri,
    REST_HTTP_METHOD_TYPE httpMethodType,
    JsonToDataObjectFunc fJsonToDataObject,
    void** ppDataObjectReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
RestDataToJson(
    const void* pData,
    REST_JSON_OBJECT_TYPE type,
    DataObjectToJsonFunc f,
    PCSTRING key,
    PSSO_JSON pJson);

SSOERROR
RestJsonToData(
    PCSSO_JSON pJson,
    REST_JSON_OBJECT_TYPE type,
    JsonToDataObjectFunc f,
    PCSTRING key,
    void** ppData);

SSOERROR
RestArrayDataToJson(
    const REST_GENERIC_ARRAY_DATA* pArray,
    DataObjectToJsonFunc f,
    PSSO_JSON pJson);

SSOERROR
RestJsonToArrayData(
    PCSSO_JSON pJson,
    JsonToDataObjectFunc f,
    REST_GENERIC_ARRAY_DATA** ppArray);

SSOERROR
RestStringDataNew(
    PSTRING* ppOut,
    PCSTRING pIn);

void
RestStringDataDelete(
    PSTRING pIn);

SSOERROR
RestStringDataToJson(
    PCSTRING pString,
    PSSO_JSON pJson);

SSOERROR
RestJsonToStringData(
    PCSSO_JSON pJson,
    PSTRING* ppString);

SSOERROR
RestIntegerDataNew(
    INTEGER** ppOut,
    INTEGER in);

void
RestIntegerDataDelete(
    INTEGER* pIn);

SSOERROR
RestLongDataNew(
    SSO_LONG** ppOut,
    SSO_LONG in);

void
RestLongDataDelete(
    SSO_LONG* pIn);

SSOERROR
RestBooleanDataNew(
    bool** ppOut,
    bool in);

void
RestBooleanDataDelete(
    bool* pIn);

SSOERROR
RestBooleanDataToJson(
    const bool* pBoolean,
    PSSO_JSON pJson);

SSOERROR
RestJsonToBooleanData(
    PCSSO_JSON pJson,
    bool** ppBoolean);

// data object to/from Json

SSOERROR
RestCertificateDataToJson(
    const REST_CERTIFICATE_DATA* pCertificateData,
    PSSO_JSON pJson);

SSOERROR
RestJsonToCertificateData(
    PCSSO_JSON pJson,
    REST_CERTIFICATE_DATA** ppCertificateData);

SSOERROR
RestCertificateArrayDataToJson(
    const REST_CERTIFICATE_ARRAY_DATA* pCertificates,
    PSSO_JSON pJson);

SSOERROR
RestJsonToCertificateArrayData(
    PCSSO_JSON pJson,
    REST_CERTIFICATE_ARRAY_DATA** ppCertificates);

SSOERROR
RestCredentialsDataToJson(
    const REST_CREDENTIALS_DATA* pCredentials,
    PSSO_JSON pJson);

SSOERROR
RestJsonToCredentialsData(
    PCSSO_JSON pJson,
    REST_CREDENTIALS_DATA** ppCredentials);

SSOERROR
RestDebugJsonObject(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson);

SSOERROR
RestDebugJsonArray(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson);

#endif /* INCLUDE_CORECLIENT_PROTOTYPES_H_ */
