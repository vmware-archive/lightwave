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

PCSTR
LwCADbPostPluginGetVersion(
    VOID
    );

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE             pHandle,            // IN
    PCSTR                       pcszCAId,           // IN
    PLWCA_DB_CA_DATA            pCAData,            // IN
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData     // IN
    );

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CA_DATA            pCAData,
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData
    );

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        *ppCAData
    );

DWORD
LwCADbPostPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    );

VOID
LwCADbPostPluginFreeCAData(
    PLWCA_DB_CA_DATA  pCAData
    );

DWORD
LwCADbPostPluginAddCACert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    );

DWORD
LwCADbPostPluginGetCACerts(
    PLWCA_DB_HANDLE                 pHandle,
    PCSTR                           pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    );

DWORD
LwCADbPostPluginGetCACert(
    PLWCA_DB_HANDLE                 pHandle,
    PCSTR                           pcszCAId,   // UNUSED
    PCSTR                           pcszSKI,    // REQUIRED
    PLWCA_DB_ROOT_CERT_DATA         *ppCACert
    );

DWORD
LwCADbPostPluginUpdateCACert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    );

VOID
LwCADbPostPluginFreeRootCertData(
    PLWCA_DB_ROOT_CERT_DATA         pRootCertData
    );

VOID
LwCADbPostPluginFreeRootCertDataArray(
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   pRootCertDataArray
    );

DWORD
LwCADbPostPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

DWORD
LwCADbPostPluginGetCerts(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    );

DWORD
LwCADbPostPluginGetCert(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,           // UNUSED
    PCSTR                       pcszSerialNumber,   // REQUIRED
    PLWCA_DB_CERT_DATA          *ppCert
    );

DWORD
LwCADbPostPluginGetRevokedCerts(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PCSTR                       pcszAKI,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCerts
    );

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

VOID
LwCADbPostPluginFreeCertData(
    PLWCA_DB_CERT_DATA      pCertData
    );

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
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
    PCSTR               pcszDN,
    PSTR                *ppszReqBody
    );

DWORD
LwCASerializeContainerToJSON(
    PCSTR       pcszDN,
    PCSTR       pcszCN,
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
    PLWCA_DB_CERT_DATA  pCertData,
    PCSTR               pcszDN,
    PSTR                *ppszResponse
    );

DWORD
LwCADeserializeJSONToCertData(
    PCSTR                   pcszResponse,
    PLWCA_DB_CERT_DATA      *ppCertData
    );

DWORD
LwCADeserializeJSONToCertDataArray(
    PCSTR                       pcszResponse,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    );

DWORD
LwCADeserializeJSONToRootCertData(
    PCSTR                           pcszResponse,
    PLWCA_DB_ROOT_CERT_DATA         *ppRootCertData
    );

DWORD
LwCADeserializeJSONToRootCertDataArray(
    PCSTR                           pcszResponse,
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppRootCertDataArray
    );

DWORD
LwCASerializeRootCertDataToJSON(
    PLWCA_DB_ROOT_CERT_DATA     pRootCertData,
    PCSTR                       pcszDN,
    PSTR                        *ppszReqBody
    );

DWORD
LwCAGenerateRootCertPatchRequestBody(
    PLWCA_DB_ROOT_CERT_DATA     pCertData,
    PSTR                        *ppszBody
    );

DWORD
LwCAGenerateCertPatchRequestBody(
    PLWCA_DB_CERT_DATA  pCertData,
    PSTR                *ppszBody
    );

DWORD
LwCAGenerateLockRequestBody(
    PCSTR           pcszUuid,
    ULONG           expireTime,
    PSTR            *ppszReqBody
    );

DWORD
LwCARestExecutePatchImpl(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszReqBody,
    PCSTR               pcszIfMatch,
    PSTR                *ppszResponse,
    long                *pStatusCode
    );

DWORD
LwCADbPostPluginLockCA(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PSTR            *ppszUuid
    );

DWORD
LwCADbPostPluginUnlockCA(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszUuid
    );

DWORD
LwCADbPostPluginLockCACert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PSTR            *ppszUuid
    );

DWORD
LwCADbPostPluginUnlockCACert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PCSTR           pcszUuid
    );

DWORD
LwCADbPostPluginLockCert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PSTR            *ppszUuid
    );

DWORD
LwCADbPostPluginUnlockCert(
    PLWCA_DB_HANDLE pHandle,
    PCSTR           pcszCAId,
    PCSTR           pcszSerialNumber,
    PCSTR           pcszUuid
    );

DWORD
LwCALockDN(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PSTR                *ppszUuid
    );

DWORD
LwCAUnlockDN(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszUUID
    );
