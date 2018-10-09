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

LWCA_SECURITY_INTERFACE _interface = {0};

PCSTR
LwCASecurityGetVersion(
    VOID
    )
{
    return LWCA_SECURITY_VERSION_MAJOR"."\
           LWCA_SECURITY_VERSION_MINOR"."\
           LWCA_SECURITY_VERSION_RELEASE;
}

DWORD
Security_Test_Impl_Initialize(
    PLWCA_SECURITY_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PLWCA_SECURITY_HANDLE pHandle = NULL;

    *ppHandle = pHandle;

    return dwError;
}

DWORD
Security_Test_Impl_GetCaps(
    PLWCA_SECURITY_HANDLE pHandle,
    LWCA_SECURITY_CAP *pnCaps
    )
{
    DWORD dwError = 0;
    LWCA_SECURITY_CAP nCaps = LWCA_SECURITY_CAP_ALL;

    *pnCaps = nCaps;

    return dwError;
}

DWORD
Security_Test_Impl_CapOverride(
    PLWCA_SECURITY_HANDLE pHandle,
    PLWCA_SECURITY_CAP_OVERRIDE pOverride
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
Security_Test_Impl_AddKeyPair(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    PCSTR pszPassPhrase,
    PCSTR pszPrivateKey
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
Security_Test_Impl_CreateKeyPair(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    PCSTR pszPassPhrase,
    size_t nKeyLength,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
Security_Test_Impl_SignCertificate(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    X509 *pX509Certificate,
    LWCA_SECURITY_MESSAGE_DIGEST md,
    PCSTR pszKeyId,
    PCSTR pszPassPhrase,
    PSTR *ppszCertificate
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
Security_Test_Impl_VerifyCertificate(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszCertificate,
    PCSTR pKeyId,
    PCSTR pszPassPhrase,
    BOOLEAN *pbValid
    )
{
    DWORD dwError = 0;

    return dwError;
}

VOID
Security_Test_Impl_CloseHandle(
    PLWCA_SECURITY_HANDLE pHandle
    )
{
    if (pHandle)
    {
    }
}

VOID
Security_Test_Impl_FreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }
}

DWORD
LwCASecurityLoadInterface(
    PLWCA_SECURITY_INTERFACE *ppInterface
    )
{
    DWORD dwError = 0;

    _interface.pFnInitialize = Security_Test_Impl_Initialize;
    _interface.pFnGetCaps = Security_Test_Impl_GetCaps;
    _interface.pFnCapOverride = Security_Test_Impl_CapOverride;
    _interface.pFnAddKeyPair = Security_Test_Impl_AddKeyPair;
    _interface.pFnCreateKeyPair = Security_Test_Impl_CreateKeyPair;
    _interface.pFnSignCertificate = Security_Test_Impl_SignCertificate;
    _interface.pFnVerifyCertificate = Security_Test_Impl_VerifyCertificate;
    _interface.pFnCloseHandle = Security_Test_Impl_CloseHandle;
    _interface.pFnFreeMemory = Security_Test_Impl_FreeMemory;

    *ppInterface = &_interface;

    return dwError;
}

DWORD
LwCASecurityUnloadInterface(
    PLWCA_SECURITY_INTERFACE pInterface
    )
{
    DWORD dwError = 0;

    if (pInterface)
    {
        pInterface->pFnInitialize = NULL;
        pInterface->pFnGetCaps = NULL;
        pInterface->pFnCapOverride = NULL;
        pInterface->pFnAddKeyPair = NULL;
        pInterface->pFnCreateKeyPair = NULL;
        pInterface->pFnSignCertificate = NULL;
        pInterface->pFnVerifyCertificate = NULL;
        pInterface->pFnCloseHandle = NULL;
        pInterface->pFnFreeMemory = NULL;
    }

    return dwError;
}
