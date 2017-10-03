/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#ifdef REST_ENABLED

static
DWORD
_VmDirRESTServerInitHTTP(
    VOID
    );

static
DWORD
_VmDirRESTServerInitHTTPS(
    VOID
    );

static
VOID
_VmDirRESTServerShutdownHTTP(
    VOID
    );

static
VOID
_VmDirRESTServerShutdownHTTPS(
    VOID
    );

static
VOID
_VmDirFreeRESTHandle(
    PVMREST_HANDLE pHandle
    );

REST_PROCESSOR sVmDirHTTPHandlers =
{
    .pfnHandleCreate = &VmDirHTTPRequestHandler,
    .pfnHandleRead = &VmDirHTTPRequestHandler,
    .pfnHandleUpdate = &VmDirHTTPRequestHandler,
    .pfnHandleDelete = &VmDirHTTPRequestHandler,
    .pfnHandleOthers = &VmDirHTTPRequestHandler
};

REST_PROCESSOR sVmDirHTTPSHandlers =
{
    .pfnHandleCreate = &VmDirHTTPSRequestHandler,
    .pfnHandleRead = &VmDirHTTPSRequestHandler,
    .pfnHandleUpdate = &VmDirHTTPSRequestHandler,
    .pfnHandleDelete = &VmDirHTTPSRequestHandler,
    .pfnHandleOthers = &VmDirHTTPSRequestHandler
};

// TODO
// should we call this only if promoted? or we need rest-head
// to return unwilling to perform in unpromoted state.
DWORD
VmDirRESTServerInit(
    VOID
    )
{
    DWORD   dwError = 0;

    MODULE_REG_MAP stRegMap[] =
    {
        {"ldap", VmDirRESTGetLdapModule},
        {"object", VmDirRESTGetObjectModule},
        {"etcd", VmDirRESTGetEtcdModule},
        {"metrics", VmDirRESTGetMetricsModule},
        {NULL, NULL}
    };

    dwError = OidcClientGlobalInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTLoadVmAfdAPI(&gpVdirVmAfdApi);
    BAIL_ON_VMDIR_ERROR(dwError);

    // cache is only required for token auth
    // post should still handle simple auth
    (VOID)VmDirRESTCacheInit(&gpVdirRestCache);

    dwError = coapi_load_from_file(REST_API_SPEC, &gpVdirRestApiDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVdirRestApiDef, stRegMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTServerInitHTTP();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTServerInitHTTPS();
    if (dwError != 0)
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "VmDirRESTServerInit: HTTPS port init failed with error %d, (failure is expected before promote)",
                dwError);
        dwError = 0;
    }

cleanup:
    return dwError;

error:
    VmDirRESTServerShutdown();
    goto cleanup;
}

VOID
VmDirRESTServerShutdown(
    VOID
    )
{
    _VmDirRESTServerShutdownHTTP();
    _VmDirRESTServerShutdownHTTPS();
    //cleanup all global variables
    OidcClientGlobalCleanup();
    VmDirRESTUnloadVmAfdAPI(gpVdirVmAfdApi);
    VmDirFreeRESTCache(gpVdirRestCache);
    VMDIR_SAFE_FREE_MEMORY(gpVdirRestApiDef);
}

static
DWORD
_VmDirRESTServerInitHTTP(
    VOID
    )
{
    DWORD   dwError = 0;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDirHTTPHandlers;
    PREST_API_MODULE    pModule = NULL;

    /*
     * pszHTTPListenPort can never be NULL because of default values assigned to them
     * if Port string is empty, it means user wants to disable corresponding service
     */
    if (IsNullOrEmptyString(gVmdirGlobals.pszHTTPListenPort))
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTP port",
                __FUNCTION__);
        goto cleanup;
    }

    config.pSSLCertificate = RSA_SERVER_CERT;
    config.pSSLKey = RSA_SERVER_KEY;
    config.pServerPort = gVmdirGlobals.pszHTTPListenPort;
    config.pDebugLogFile = VMDIR_HTTP_DEBUGLOGFILE;
    config.pClientCount = VMDIR_REST_CLIENTCNT;
    config.pMaxWorkerThread = VMDIR_REST_WORKERTHCNT;

    dwError = VmRESTInit(&config, NULL, &gpVdirRestHTTPHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    gpVdirRestHTTPHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(gpVdirRestHTTPHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    _VmDirRESTServerShutdownHTTP();
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with error %d, not going to listen on REST port",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
DWORD
_VmDirRESTServerInitHTTPS(
    VOID
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDirHTTPSHandlers;
    PREST_API_MODULE    pModule = NULL;

    /*
     * pszHTTPSListenPort can never be NULL because of default values assigned to them
     * if Port string is empty, it means user wants to disable corresponding service
     */
    if (IsNullOrEmptyString(gVmdirGlobals.pszHTTPSListenPort))
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTP port",
                __FUNCTION__);
        goto cleanup;
    }

    config.pSSLCertificate = NULL;
    config.pSSLKey = NULL;
    config.pServerPort = gVmdirGlobals.pszHTTPSListenPort;
    config.pDebugLogFile = VMDIR_HTTPS_DEBUGLOGFILE;
    config.pClientCount = VMDIR_REST_CLIENTCNT;
    config.pMaxWorkerThread = VMDIR_REST_WORKERTHCNT;

    dwError = VmRESTInit(&config, NULL, &gpVdirRestHTTPSHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Get Certificate and Key from VECS and Set it to Rest Engine
    dwError = VmDirGetVecsMachineCert(&pszCert, &pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(gpVdirRestHTTPSHandle, pszCert, VmDirStringLenA(pszCert)+1, SSL_DATA_TYPE_CERT);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(gpVdirRestHTTPSHandle, pszKey, VmDirStringLenA(pszKey)+1, SSL_DATA_TYPE_KEY);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    gpVdirRestHTTPSHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(gpVdirRestHTTPSHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszCert);
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    return dwError;

error:
    _VmDirRESTServerShutdownHTTPS();
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with error %d, not going to listen on REST port (expected before promote)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
VOID
_VmDirRESTServerShutdownHTTP(
    VOID
    )
{
    _VmDirFreeRESTHandle(gpVdirRestHTTPHandle);
    gpVdirRestHTTPHandle = NULL;
}

static
VOID
_VmDirRESTServerShutdownHTTPS(
    VOID
    )
{
    _VmDirFreeRESTHandle(gpVdirRestHTTPSHandle);
    gpVdirRestHTTPSHandle = NULL;
}

static
VOID
_VmDirFreeRESTHandle(
    PVMREST_HANDLE    pHandle
    )
{
    PREST_API_MODULE  pModule = NULL;

    if (pHandle)
    {
        VmRESTStop(pHandle);
        if (gpVdirRestApiDef)
        {
            pModule = gpVdirRestApiDef->pModules;
            for (; pModule; pModule = pModule->pNext)
            {
                PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
                for (; pEndPoint; pEndPoint = pEndPoint->pNext)
                {
                    (VOID)VmRESTUnRegisterHandler(
                            pHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(pHandle);
    }
}

#else

DWORD
VmDirRESTServerInit(
    VOID
    )
{
    return 0;
}

VOID
VmDirRESTServerShutdown(
    VOID
    )
{
    return;
}

#endif
