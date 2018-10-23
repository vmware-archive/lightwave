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

#include "includes.h"

DWORD
LwCAPluginLoad(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{

    DWORD dwError = 0;

    if (!pFt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pFt->pFnInit = &LwCADbPostPluginInitialize;
    pFt->pFnAddCA = &LwCADbPostPluginAddCA;
    pFt->pFnAddCertData = &LwCADbPostPluginAddCertData;
    pFt->pFnCheckCA = &LwCADbPostPluginCheckCA;
    pFt->pFnCheckCertData = &LwCADbPostPluginCheckCertData;
    pFt->pFnGetCACertificates = &LwCADbPostPluginGetCACertificates;
    pFt->pFnGetCA = &LwCADbPostPluginGetCA;
    pFt->pFnGetCertData = &LwCADbPostPluginGetCertData;
    pFt->pFnGetCACRLNumber = &LwCADbPostPluginGetCACRLNumber;
    pFt->pFnGetParentCAId = &LwCADbPostPluginGetParentCAId;
    pFt->pFnUpdateCA = &LwCADbPostPluginUpdateCA;
    pFt->pFnUpdateCAStatus = &LwCADbPostPluginUpdateCAStatus;
    pFt->pFnUpdateCACRLNumber = &LwCADbPostPluginUpdateCACRLNumber;
    pFt->pFnFreeCAData = &LwCADbPostPluginFreeCAData;
    pFt->pFnUpdateCertData = &LwCADbPostPluginUpdateCertData;
    pFt->pFnFreeCertDataArray = &LwCADbPostPluginFreeCertDataArray;
    pFt->pFnFreeCertArray = &LwCADbPostPluginFreeCertificates;
    pFt->pFnFreeString = &LwCADbPostPluginFreeString;
    pFt->pFnFreeHandle = &LwCADbPostPluginFreeHandle;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCADbPostPluginInitialize(
    PCSTR               pcszConfigPath,
    PLWCA_DB_HANDLE     *ppHandle
    )
{
    DWORD               dwError = 0;
    PLWCA_POST_HANDLE   pHandle = NULL;
    PLWCA_JSON_OBJECT   pJson = NULL;

    if (IsNullOrEmptyString(pcszConfigPath) || !ppHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_POST_HANDLE), (PVOID *)&pHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = pthread_mutex_init(&(pHandle->accessTokenMutex), NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonLoadObjectFromFile(pcszConfigPath, &pJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_LW_SERVER,
                                       &(pHandle->pszLwServer)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_POST_SERVER,
                                       &(pHandle->pszPostServer)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_DB_DOMAIN,
                                       &(pHandle->pszDomain)
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppHandle = (PLWCA_DB_HANDLE)pHandle;

cleanup:
    LwCAJsonCleanupObject(pJson);
    return dwError;

error:
    LwCADbPostPluginFreeHandle((PLWCA_DB_HANDLE)pHandle);
    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    goto cleanup;
}

DWORD
LwCADbPostPluginAddCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PLWCA_DB_CA_DATA    pCAData,
    PCSTR               pcszParentCA
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginAddCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginCheckCA(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PBOOLEAN            pbExists
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginCheckCertData(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszSerialNumber,
    PBOOLEAN            pbExists
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCA(
    PLWCA_DB_HANDLE          pHandle,
    PCSTR                    pcszCAId,
    PLWCA_DB_CA_DATA         *ppCAData
)
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCACertificates(
    PLWCA_DB_HANDLE              pHandle,
    PCSTR                        pcszCAId,
    PLWCA_CERTIFICATE_ARRAY      *ppCertArray
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCertData(
    PLWCA_DB_HANDLE             pHandle,
    PCSTR                       pcszCAId,
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszCRLNumber
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginGetParentCAId(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PSTR                *ppszParentCAId
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCA(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    )
{
    return LWCA_NOT_IMPLEMENTED;
}


DWORD
LwCADbPostPluginUpdateCAStatus(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    LWCA_CA_STATUS          status
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCertData(
    PLWCA_DB_HANDLE         pHandle,
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

DWORD
LwCADbPostPluginUpdateCACRLNumber(
    PLWCA_DB_HANDLE     pHandle,
    PCSTR               pcszCAId,
    PCSTR               pcszCRLNumber
    )
{
    return LWCA_NOT_IMPLEMENTED;
}

VOID
LwCADbPostPluginFreeCAData(
    PLWCA_DB_CA_DATA  pCAData
    )
{
}

VOID
LwCADbPostPluginFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    )
{
}

VOID
LwCADbPostPluginFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    )
{
}

VOID
LwCADbPostPluginFreeString(
    PSTR    pszString
    )
{
}

VOID
LwCADbPostPluginFreeHandle(
    PLWCA_DB_HANDLE     pHandle
    )
{
    PLWCA_POST_HANDLE pPostHandle = NULL;

    if (pHandle)
    {
        pPostHandle = (PLWCA_POST_HANDLE)pHandle;

        OidcAccessTokenDelete(pPostHandle->pOidcToken);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszAccessToken);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszLwServer);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszPostServer);
        LWCA_SAFE_FREE_STRINGA(pPostHandle->pszDomain);

        LWCA_SAFE_FREE_MEMORY(pPostHandle);
    }

}
