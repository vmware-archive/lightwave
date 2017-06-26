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

#ifndef INCLUDE_PUBLIC_SSOCORECLIENT_H_
#define INCLUDE_PUBLIC_SSOCORECLIENT_H_

// data types

typedef enum
{
    REST_ACCESS_TOKEN_TYPE_JWT,
    REST_ACCESS_TOKEN_TYPE_JWT_HOK,
    REST_ACCESS_TOKEN_TYPE_SAML,
    REST_ACCESS_TOKEN_TYPE_SAML_HOK
} REST_ACCESS_TOKEN_TYPE;

typedef enum
{
    REST_SCHEME_TYPE_HTTP,
    REST_SCHEME_TYPE_HTTPS
} REST_SCHEME_TYPE;

typedef struct REST_ACCESS_TOKEN* PREST_ACCESS_TOKEN;

typedef const struct REST_ACCESS_TOKEN* PCREST_ACCESS_TOKEN;

typedef struct REST_CLIENT* PREST_CLIENT;

typedef const struct REST_CLIENT* PCREST_CLIENT;

typedef struct
{
    long httpStatusCode;
    PSTRING error;
    PSTRING details;
    PSTRING cause;
} REST_SERVER_ERROR;

// Data Object Struct

typedef struct
{
    PSTRING encoded;
    PSTRING fingerprint; // ignored in JSON conversion.
} REST_CERTIFICATE_DATA;

typedef struct
{
    REST_CERTIFICATE_DATA** ppEntry;
    size_t length;
} REST_CERTIFICATE_ARRAY_DATA;

typedef struct
{
    PSTRING username;
    PSTRING password;
} REST_CREDENTIALS_DATA;

// Misc APIs

SSOERROR
RestAccessTokenNew(
    PREST_ACCESS_TOKEN* ppAccessToken,
    PCSTRING value,
    REST_ACCESS_TOKEN_TYPE type,
    PCSTRING privateKey);

void
RestAccessTokenDelete(
    PREST_ACCESS_TOKEN pAccessToken);

/*
 * IMPORTANT: you must call this function at process startup while there is only a single thread running
 * This is a wrapper for curl_global_init, from its documentation:
 * This function is not thread safe.
 * You must not call it when any other thread in the program (i.e. a thread sharing the same memory) is running.
 * This doesn't just mean no other thread that is using libcurl.
 * Because curl_global_init calls functions of other libraries that are similarly thread unsafe,
 * it could conflict with any other thread that uses these other libraries.
 */
SSOERROR
RestClientGlobalInit();

// this function is not thread safe. Call it right before process exit
void
RestClientGlobalCleanup();

// make sure you call RestClientGlobalInit once per process before calling this
// tlsCAPath: NULL means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
SSOERROR
RestClientNew(
    PREST_CLIENT* ppClient,
    PCSTRING serverHost,
    bool highAvailabilityEnabled,
    size_t serverPort,
    REST_SCHEME_TYPE schemeType,
    PCSTRING tlsCAPath, // optional, see comment above
    PCREST_ACCESS_TOKEN pAccessToken);

void
RestClientDelete(
    PREST_CLIENT pClient);

SSOERROR
RestClientSetAccessToken(
    PREST_CLIENT pClient,
    PCREST_ACCESS_TOKEN pAccessToken);

// Data APIs

SSOERROR
RestCertificateDataNew(
    REST_CERTIFICATE_DATA** ppCertificate,
    PCSTRING encoded);

void
RestCertificateDataDelete(
    REST_CERTIFICATE_DATA* pCertificate);

SSOERROR
RestCertificateArrayDataNew(
    REST_CERTIFICATE_ARRAY_DATA** ppCertificates,
    REST_CERTIFICATE_DATA** ppEntry,
    size_t length);

void
RestCertificateArrayDataDelete(
    REST_CERTIFICATE_ARRAY_DATA* pCertificates);

SSOERROR
RestCredentialsDataNew(
    REST_CREDENTIALS_DATA** ppCredentials,
    PCSTRING username,
    PCSTRING password);

void
RestCredentialsDataDelete(
    REST_CREDENTIALS_DATA* pCredentials);

#endif /* INCLUDE_PUBLIC_SSOCORECLIENT_H_ */
