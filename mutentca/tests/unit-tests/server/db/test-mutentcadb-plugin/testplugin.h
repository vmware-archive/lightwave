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

DWORD
LwCAPluginLoad(
   PLWCA_DB_FUNCTION_TABLE pFt
   );

VOID
LwCAPluginUnload(
   VOID
   );

DWORD
LwCADbTestPluginInitialize(
    PLWCA_DB_HANDLE  *ppHandle
    );

DWORD
LwCADbTestPluginAddCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData,
    PCSTR                   pcszParentCA
    );

DWORD
LwCADbTestPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

DWORD
LwCADbTestPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_DB_CERTIFICATE_ARRAY   *ppCertArray
    );

DWORD
LwCADbTestPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    );

DWORD
LwCADbTestPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    );


DWORD
LwCADbTestPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_DB_CA_STATUS       status
    );

DWORD
LwCADbTestPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

VOID
LwCADbTestPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    );

VOID
LwCADbTestPluginFreeCertificates(
    PLWCA_DB_CERTIFICATE_ARRAY pCertArray
    );

VOID
LwCADbTestPluginFreeHandle(
    PLWCA_DB_HANDLE pDbHandle
    );
