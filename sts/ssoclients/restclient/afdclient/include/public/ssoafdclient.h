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

#ifndef INCLUDE_PUBLIC_SSOAFDCLIENT_H_
#define INCLUDE_PUBLIC_SSOAFDCLIENT_H_

// Data Object Struct

typedef struct
{
    PSTRING alias;
    PSTRING dn;
    PSTRING name;
    PSTRING status;
} AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA;

typedef struct
{
    PSTRING username;
    PSTRING password;
    PSTRING domain;
    PSTRING ou;
} AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA;

// Misc APIs

SSOERROR
AfdActiveDirectoryJoinInfoDataNew(
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfo,
    PCSTRING alias,
    PCSTRING dn,
    PCSTRING name,
    PCSTRING status);

void
AfdActiveDirectoryJoinInfoDataDelete(
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo);

SSOERROR
AfdActiveDirectoryJoinRequestDataNew(
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA** ppActiveDirectoryJoinRequest,
    PCSTRING username,
    PCSTRING password,
    PCSTRING domain,
    PCSTRING ou);

void
AfdActiveDirectoryJoinRequestDataDelete(
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest);

// Resource APIs

SSOERROR
AfdAdProviderJoin(
    PCREST_CLIENT pClient,
    const AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfoReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
AfdAdProviderLeave(
    PCREST_CLIENT pClient,
    const REST_CREDENTIALS_DATA* pCredentials,
    REST_SERVER_ERROR** ppError);

SSOERROR
AfdAdProviderGetStatus(
    PCREST_CLIENT pClient,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfoReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
AfdVecsGetSSLCertificates(
    PCREST_CLIENT pClient,
    REST_CERTIFICATE_ARRAY_DATA** ppCertificateArrayReturn,
    REST_SERVER_ERROR** ppError);

#endif /* INCLUDE_PUBLIC_SSOAFDCLIENT_H_ */
