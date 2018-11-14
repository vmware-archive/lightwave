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

// post.c
DWORD
LwCADbPostPluginInitialize(
    PCSTR               pcszConfigPath,
    PLWCA_DB_HANDLE     *ppHandle
    );

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PLWCA_DB_CA_DATA    pCAData,
    PCSTR               pcszParentCA
    );

DWORD
LwCADbPostPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
);

DWORD
LwCADbPostPluginCheckCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PBOOLEAN            pbExists
    );

DWORD
LwCADbPostPluginCheckCertData(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszSerialNumber,
    PBOOLEAN            pbExists
    );

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE          pHandle,
    PCSTR                    pcszCAId,
    PLWCA_DB_CA_DATA         *ppCAData
    );

DWORD
LwCADbPostPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_CERTIFICATE_ARRAY      *ppCertArray
    );

DWORD
LwCADbPostPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    );

DWORD
LwCADbPostPluginGetCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCRLNumber
    );

DWORD
LwCADbPostPluginGetParentCAId(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszParentCAId
    );

DWORD
LwCADbPostPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    );

DWORD
LwCADbPostPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_CA_STATUS          status
    );

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

DWORD
LwCADbPostPluginUpdateCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszCRLNumber
    );

VOID
LwCADbPostPluginFreeCAData(
    PLWCA_DB_CA_DATA  pCAData
    );

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    );

VOID
LwCADbPostPluginFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    );

VOID
LwCADbPostPluginFreeString(
    PSTR    pszString
    );

VOID
LwCADbPostPluginFreeHandle(
    PLWCA_DB_HANDLE pDbHandle
    );

DWORD
LwCADbPostCNFilterBuilder(
    PCSTR   pcszContainer,
    PCSTR   pcszObjClass,
    PSTR    *ppszResultCond
    );

// serializer.c
DWORD
LwCASerializeCAToJSON(
    PCSTR               pcszCAId,
    PLWCA_DB_CA_DATA    pCAData,
    PCSTR               pcszParentCA,
    PCSTR               pcszDN,
    PSTR                *ppszReqBody
    );

DWORD
LwCASerializeConfigCAToJSON(
    PCSTR       pcszCAId,
    PCSTR       pcszDomain,
    PSTR        *ppszReqBody
    );

DWORD
LwCADeserializeJSONToCA(
    PCSTR               pcszResponse,
    PLWCA_DB_CA_DATA    *ppCaData
    );

DWORD
LwCAGenerateCAPatchRequestBody(
    PLWCA_DB_CA_DATA    pCaData,
    PSTR                *ppszBody
    );

DWORD
LwCAGetStringAttrFromResponse(
    PCSTR       pcszResponse,
    PCSTR       pcszKey,
    PSTR        *ppszAttrValue
    );

DWORD
LwCAGetIntAttrFromResponse(
    PCSTR       pcszResponse,
    PCSTR       pcszKey,
    int         *pValue
    );

DWORD
LwCAGetStringArrayAttrFromResponse(
    PCSTR               pcszResponse,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  *ppStrArray
    );

DWORD
LwCASerializeCertDataToJSON(
    PCSTR               pcszCAId,
    PLWCA_DB_CERT_DATA  pCertData,
    PCSTR               pcszCADN,
    PSTR                *ppszResponse
    );

DWORD
LwCADeserializeJSONToCertData(
    PCSTR                       pcszResponse,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    );

DWORD
LwCAGetCertArrayFromEncodedStringArray(
    PLWCA_STRING_ARRAY          pEncodedStrArray,
    PLWCA_CERTIFICATE_ARRAY     *ppCertArray
    );

DWORD
LwCAGenerateCertPatchRequestBody(
    PLWCA_DB_CERT_DATA  pCertData,
    PSTR                *ppszBody
    );
