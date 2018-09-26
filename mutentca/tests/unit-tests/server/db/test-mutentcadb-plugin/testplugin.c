/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”) you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#include "includes.h"

DWORD
LwCAPluginLoad(
   PLWCA_DB_FUNCTION_TABLE pFt
   )
{
    DWORD dwError = 0;

    if (!pFt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_TEST_PLUGIN_ERROR(dwError);
    }

    pFt->pFnInit = &LwCADbTestPluginInitialize;
    pFt->pFnAddCA = &LwCADbTestPluginAddCA;
    pFt->pFnAddCertData = &LwCADbTestPluginAddCertData;
    pFt->pFnGetCACertificates = &LwCADbTestPluginGetCACertificates;
    pFt->pFnGetCertData = &LwCADbTestPluginGetCertData;
    pFt->pFnUpdateCA = &LwCADbTestPluginUpdateCA;
    pFt->pFnUpdateCAStatus = &LwCADbTestPluginUpdateCAStatus;
    pFt->pFnUpdateCertData = &LwCADbTestPluginUpdateCertData;
    pFt->pFnFreeCertDataArray = &LwCADbTestPluginFreeCertDataArray;
    pFt->pFnFreeCertArray = &LwCADbTestPluginFreeCertificates;
    pFt->pFnFreeHandle = &LwCADbTestPluginFreeHandle;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
LwCAPluginUnload(
   VOID
   )
{
}


DWORD
LwCADbTestPluginInitialize(
    PLWCA_DB_HANDLE  *ppHandle
    )
{
    return 0;
}

DWORD
LwCADbTestPluginAddCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData,
    PCSTR                   pcszParentCA
    )
{
    return 0;
}

DWORD
LwCADbTestPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    return 0;
}

DWORD
LwCADbTestPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_DB_CERTIFICATE_ARRAY   *ppCertArray
    )
{
    return 0;
}

DWORD
LwCADbTestPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    return 0;
}

DWORD
LwCADbTestPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    )
{
    return 0;
}


DWORD
LwCADbTestPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_DB_CA_STATUS       status
    )
{
    return 0;
}

DWORD
LwCADbTestPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    return 0;
}

VOID
LwCADbTestPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
}

VOID
LwCADbTestPluginFreeCertificates(
    PLWCA_DB_CERTIFICATE_ARRAY pCertArray
    )
{
}

VOID
LwCADbTestPluginFreeHandle(
    PLWCA_DB_HANDLE pDbHandle
    )
{
}
