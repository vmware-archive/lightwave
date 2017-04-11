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

#include "includes.h"
#include "../../../../vmafd/include/public/vmafdtypes.h"

#ifdef _WIN32
static PCSTRING const VMAFD_CLIENT_LIB_PATH = "libvmafdclient";
#else
static PCSTRING const VMAFD_CLIENT_LIB_PATH = "/usr/lib/vmware-vmafd/lib64/libvmafdclient.so";
#endif

static
SSOERROR
SSOCdcLoadLibrary(
    PCSTR pszLibPath,
    PSSO_LIB_HANDLE* ppLibHandle)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_LIB_HANDLE pLibHandle = NULL;

    if (IS_NULL_OR_EMPTY_STRING(pszLibPath) || ppLibHandle == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

#ifdef _WIN32
    pLibHandle = LoadLibraryA(pszLibPath);
#else
    pLibHandle = dlopen(pszLibPath, RTLD_LAZY);
#endif

    if (pLibHandle == NULL)
    {
        e = SSOERROR_VMAFD_LOAD_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppLibHandle = pLibHandle;

    error:

    return e;
}

static
SSOERROR
SSOCdcGetLibrarySymbol(
    PSSO_LIB_HANDLE pLibHandle,
    PCSTR pszFunctionName,
    PSSO_FUNC_HANDLE* ppFuncHandle)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_FUNC_HANDLE pFuncHandle = NULL;

    if (pLibHandle == NULL || IS_NULL_OR_EMPTY_STRING(pszFunctionName) || ppFuncHandle == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

#ifdef _WIN32
    pFuncHandle = GetProcAddress(pLibHandle, pszFunctionName);
#else
    pFuncHandle = dlsym(pLibHandle, pszFunctionName);
#endif

    if (pFuncHandle == NULL)
    {
        e = SSOERROR_VMAFD_LOAD_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppFuncHandle = pFuncHandle;

    error:

    return e;
}

static
void
SSOCdcCloseLibrary(
    PSSO_LIB_HANDLE pLibHandle)
{
    if (pLibHandle != NULL)
    {
#ifdef _WIN32
        FreeLibrary(pLibHandle);
#else
        dlclose(pLibHandle);
#endif
    }
}

SSOERROR
SSOCdcGetAffinitizedHost(
    PCSSO_CDC pCdc,
    PCSTRING domainName,
    int cdcFlags,
    PSTRING* ppszAffinitizedHost)
{
    SSOERROR e = SSOERROR_NONE;

    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PCDC_DC_INFO_A pDCInfo = NULL;
    PSTRING pszAffinitizedHost = NULL;

    if (pCdc == NULL || ppszAffinitizedHost == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    dwError = (*(DWORD (*)(PCSTR, PCSTR, PCSTR, PVMAFD_SERVER*))pCdc->pVmAfdOpenServer)(NULL, NULL, NULL, &pServer);
    if (dwError != 0)
    {
        e = SSOERROR_VMAFD_CALL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    dwError = (*(DWORD (*)(PVMAFD_SERVER, PCSTR, GUID_A, PCSTR, DWORD, PCDC_DC_INFO_A*))pCdc->pCdcGetDCName)(pServer, domainName, NULL, NULL, cdcFlags, &pDCInfo);
    if (dwError != 0)
    {
        e = SSOERROR_VMAFD_CALL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringAllocate(pDCInfo->pszDCName, &pszAffinitizedHost);
    BAIL_ON_ERROR(e);

    *ppszAffinitizedHost = pszAffinitizedHost;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pszAffinitizedHost);
    }

    // cleanup
    if (pServer != NULL)
    {
        (*(VOID (*)(PVMAFD_SERVER))pCdc->pVmAfdCloseServer)(pServer);
    }

    if (pDCInfo != NULL)
    {
        (*(VOID (*)(PCDC_DC_INFO_A))pCdc->pCdcFreeDomainControllerInfo)(pDCInfo);
    }

    return e;
}

SSOERROR
SSOCdcNew(
    PSSO_CDC* ppCdc)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_CDC* pCdc = NULL;

    if (ppCdc == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_CDC), (void**) &pCdc);
    BAIL_ON_ERROR(e);

    e = SSOCdcLoadLibrary(VMAFD_CLIENT_LIB_PATH, &(pCdc->pHandle));
    BAIL_ON_ERROR(e);

    e = SSOCdcGetLibrarySymbol(pCdc->pHandle, "VmAfdOpenServerA", &(pCdc->pVmAfdOpenServer));
    BAIL_ON_ERROR(e);

    e = SSOCdcGetLibrarySymbol(pCdc->pHandle, "CdcGetDCNameA", &(pCdc->pCdcGetDCName));
    BAIL_ON_ERROR(e);

    e = SSOCdcGetLibrarySymbol(pCdc->pHandle, "VmAfdCloseServer", &(pCdc->pVmAfdCloseServer));
    BAIL_ON_ERROR(e);

    e = SSOCdcGetLibrarySymbol(pCdc->pHandle, "CdcFreeDomainControllerInfoA", &(pCdc->pCdcFreeDomainControllerInfo));
    BAIL_ON_ERROR(e);

    *ppCdc = pCdc;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOCdcDelete(pCdc);
    }

    return e;
}

void
SSOCdcDelete(
    PSSO_CDC pCdc)
{
    if (pCdc != NULL)
    {
        SSOCdcCloseLibrary(pCdc->pHandle);
        SSOMemoryFree(pCdc, sizeof(SSO_CDC));
    }
}
