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

REST_PROCESSOR sVmDirRESTLdapHandlers =
{
    .pfnHandleCreate = &VmDirRESTLdapRequestHandler,
    .pfnHandleRead   = &VmDirRESTLdapRequestHandler,
    .pfnHandleUpdate = &VmDirRESTLdapRequestHandler,
    .pfnHandleDelete = &VmDirRESTLdapRequestHandler,
    .pfnHandleOthers = &VmDirRESTLdapRequestHandler
};

REST_PROCESSOR sVmDirRESTApiHandlers =
{
    .pfnHandleCreate = &VmDirRESTApiRequestHandler,
    .pfnHandleRead   = &VmDirRESTApiRequestHandler,
    .pfnHandleUpdate = &VmDirRESTApiRequestHandler,
    .pfnHandleDelete = &VmDirRESTApiRequestHandler,
    .pfnHandleOthers = &VmDirRESTApiRequestHandler
};

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
DWORD
_VmDirRESTServerInitApiHTTPS(
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
_VmDirRESTServerShutdownApi(
    VOID
    );

static
DWORD
_VmDirStopRESTHandle(
    PVMREST_HANDLE    pHandle
    );

static
VOID
_VmDirFreeRESTHandle(
    PVMREST_HANDLE    pHandle,
    PREST_API_DEF     pRestApiDef
    );

DWORD
VmDirRESTServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    MODULE_REG_MAP stRegMap[] =
    {
        {"ldap", VmDirRESTGetLdapModule},
        {"metrics", VmDirRESTGetMetricsModule},
        {"account", VmDirRESTGetAccountModule},
        {NULL, NULL}
    };

    MODULE_REG_MAP stRegMapApi[] =
    {
        {"certs", VmDirRESTApiGetCertsModule},
        {"password", VmDirRESTApiGetPasswordModule},
        {"join", VmDirRESTApiGetJoinModule},
        {NULL, NULL}
    };

    // cache is only required for token auth
    // vmdir should still handle simple auth
    (VOID)VmDirRESTCacheInit(&gpVdirRestCache);

    /*
     * We can use the same REST_API_SPEC for both HTTP and HTTPS because vmdir
     * rest init code only refers to API definitions (which is common)
     */
    dwError = coapi_load_from_file(REST_API_SPEC, &gpVdirRestApiDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVdirRestApiDef, stRegMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTServerInitHTTP();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTServerInitHTTPS();
    if (dwError != 0)
    {
        /*
         * Before promoting lightwave node, obtaining cert from VECS will fail which is expected
         * hence treat it as soft fail
         */
         VMDIR_LOG_WARNING(
                 VMDIR_LOG_MASK_ALL,
                 "VmRESTServerInit: HTTPS port init failed with error %d, (failure is expected before promote)",
                 dwError);
         dwError = 0;
    }

    dwError = coapi_load_from_file(REST_API_SPEC_2, &gpVdirRestApiDef2);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVdirRestApiDef2, stRegMapApi);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTServerInitApiHTTPS();
    if (dwError != 0)
    {
        /*
         * Before promoting lightwave node, obtaining cert from VECS will fail which is expected
         * hence treat it as soft fail
         */
         VMDIR_LOG_WARNING(
                 VMDIR_LOG_MASK_ALL,
                 "VmRESTServerInit: Api server HTTPS port init failed with error %d, (failure is expected before promote)",
                 dwError);
         dwError = 0;
    }

cleanup:
    return dwError;

error:
    if (VmDirRESTServerStop() == 0)
    {
        VmDirRESTServerShutdown();
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirRESTServerStop(
    VOID
    )
{
    DWORD   dwStopHttp = 0;
    DWORD   dwStopHttps = 0;
    DWORD   dwStopApi = 0;

    dwStopHttp  = _VmDirStopRESTHandle(gpVdirRestHTTPHandle);
    dwStopHttps = _VmDirStopRESTHandle(gpVdirRestHTTPSHandle);
    dwStopApi = _VmDirStopRESTHandle(gpVdirRestApiHTTPSHandle);

    return dwStopHttp | dwStopHttps | dwStopApi;
}

VOID
VmDirRESTServerShutdown(
    VOID
    )
{
    _VmDirRESTServerShutdownHTTP();
    _VmDirRESTServerShutdownHTTPS();
    _VmDirRESTServerShutdownApi();

    VmDirFreeRESTCache(gpVdirRestCache);
    coapi_free_api_def(gpVdirRestApiDef);
    coapi_free_api_def(gpVdirRestApiDef2);
}

VMREST_LOG_LEVEL
VmDirToCRestEngineLogLevel(
    VOID
    )
{
    VMDIR_LOG_LEVEL  vmdirLogLevel = VmDirLogGetLevel();
    VMREST_LOG_LEVEL cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;

    switch (vmdirLogLevel)
    {
    case VMDIR_LOG_ERROR:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
        break;

    case VMDIR_LOG_WARNING:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_WARNING;
        break;

    case VMDIR_LOG_INFO:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_INFO;
        break;

    case VMDIR_LOG_VERBOSE:
    case VMDIR_LOG_DEBUG:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_DEBUG;
        break;

    default:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
    }

    return cResetEngineLogLevel;
}

static
DWORD
_VmDirRESTServerInitHTTP(
    VOID
    )
{
    DWORD   dwError = 0;
    REST_CONF   config = {0};
    PREST_PROCESSOR    pHandlers = &sVmDirRESTLdapHandlers;
    PREST_API_MODULE   pModule = NULL;
    PVMREST_HANDLE     pHTTPHandle = NULL;

    /*
     * dwHTTPListenPort is '0' then user wants to disable HTTP endpoint
     */
    if (gVmdirGlobals.dwHTTPListenPort == 0)
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTP port",
                __FUNCTION__);
        goto cleanup;
    }

    config.serverPort = gVmdirGlobals.dwHTTPListenPort;
    config.connTimeoutSec = VMDIR_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMDIR_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;
    config.nWorkerThr = gVmdirGlobals.dwRESTWorker;
    config.nClientCnt = gVmdirGlobals.dwRESTClient;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMDIR_DAEMON_NAME;
    config.isSecure = FALSE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VmDirToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pHTTPHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                     pHTTPHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(pHTTPHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    gpVdirRestHTTPHandle = pHTTPHandle;

cleanup:
    return dwError;

error:
    if (_VmDirStopRESTHandle(pHTTPHandle) == 0)
    {
        _VmDirFreeRESTHandle(pHTTPHandle, gpVdirRestApiDef);
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
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
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDirRESTLdapHandlers;
    PREST_API_MODULE    pModule = NULL;
    PVMREST_HANDLE     pHTTPSHandle = NULL;

    /*
     * dwHTTPSListenPort is '0' then user wants to disable HTTPS endpoint
     * Initializing openssl context is treated as soft fail, gpVdirSslCtx can be NULL
     * If gpVdirSslCtx NULL, don't start the service
     */
    if (gVmdirGlobals.dwHTTPSListenPort == 0 || gVmdirGlobals.gpVdirSslCtx == NULL)
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTPS port",
                __FUNCTION__);
        goto cleanup;
    }

    config.serverPort = gVmdirGlobals.dwHTTPSListenPort;
    config.connTimeoutSec = VMDIR_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMDIR_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = gVmdirGlobals.gpVdirSslCtx;
    config.nWorkerThr = gVmdirGlobals.dwRESTWorker;
    config.nClientCnt = gVmdirGlobals.dwRESTClient;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMDIR_DAEMON_NAME;
    config.isSecure = TRUE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VmDirToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pHTTPSHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    pHTTPSHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(pHTTPSHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    gpVdirRestHTTPSHandle = pHTTPSHandle;

cleanup:
    return dwError;

error:
    if (_VmDirStopRESTHandle(pHTTPSHandle) == 0)
    {
        _VmDirFreeRESTHandle(pHTTPSHandle, gpVdirRestApiDef);
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * api server that exposes specific functions to listen on a separate port
 * rather than combine with existing generic REST endpoints.
*/
static
DWORD
_VmDirRESTServerInitApiHTTPS(
    VOID
    )
{
    DWORD              dwError = 0;
    REST_CONF          config = {0};
    PREST_PROCESSOR    pHandlers = &sVmDirRESTApiHandlers;
    PREST_API_MODULE   pModule = NULL;
    PVMREST_HANDLE     pApiHandle = NULL;

    /*
     * dwHTTPSApiListenPort is '0' then user wants to disable HTTPS functions endpoint
     * Initializing openssl context is treated as soft fail, gpVdirSslCtx can be NULL
     * If gpVdirSslCtx NULL, don't start the service
     */
    if (gVmdirGlobals.dwHTTPSApiListenPort == 0 || gVmdirGlobals.gpVdirSslCtx == NULL)
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTPS port",
                __FUNCTION__);
        goto cleanup;
    }

    config.serverPort = gVmdirGlobals.dwHTTPSApiListenPort;
    config.connTimeoutSec = VMDIR_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMDIR_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = gVmdirGlobals.gpVdirSslCtx;
    config.nWorkerThr = VMDIR_REST_API_WORKERTHCNT;
    config.nClientCnt = VMDIR_REST_API_CLIENTCNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMDIR_DAEMON_NAME;
    config.isSecure = TRUE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VmDirToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pApiHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef2->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    pApiHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(pApiHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    gpVdirRestHTTPSHandle = pApiHandle;

cleanup:
    return dwError;

error:
    if (_VmDirStopRESTHandle(pApiHandle) == 0)
    {
        _VmDirFreeRESTHandle(pApiHandle, gpVdirRestApiDef2);
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
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
    _VmDirFreeRESTHandle(gpVdirRestHTTPHandle, gpVdirRestApiDef);
    gpVdirRestHTTPHandle = NULL;
}

static
VOID
_VmDirRESTServerShutdownHTTPS(
    VOID
    )
{
    _VmDirFreeRESTHandle(gpVdirRestHTTPSHandle, gpVdirRestApiDef);
    gpVdirRestHTTPSHandle = NULL;
}

static
VOID
_VmDirRESTServerShutdownApi(
    VOID
    )
{
    _VmDirFreeRESTHandle(gpVdirRestApiHTTPSHandle, gpVdirRestApiDef2);
    gpVdirRestApiHTTPSHandle= NULL;
}

static
DWORD
_VmDirStopRESTHandle(
    PVMREST_HANDLE    pHandle
    )
{
    DWORD dwError = 0;

    if (pHandle)
    {
        /*
         * REST library have detached threads, maximum time out specified is the max time
         * allowed for the threads to  finish their execution.
         * If finished early, it will return success.
         * If not able to finish in specified time, failure will be returned
         */
        dwError = VmRESTStop(pHandle, VMDIR_REST_STOP_TIMEOUT_SEC);
        if (dwError != 0)
        {
            VMDIR_LOG_WARNING(
                   VMDIR_LOG_MASK_ALL,
                   "%s: Rest stop error: %d",
                   __FUNCTION__,
                   dwError);
        }
    }

    return dwError;
}


static
VOID
_VmDirFreeRESTHandle(
    PVMREST_HANDLE    pHandle,
    PREST_API_DEF     pRestApiDef
    )
{
    PREST_API_MODULE  pModule = NULL;

    if (pHandle)
    {
        if (pRestApiDef)
        {
            pModule = pRestApiDef->pModules;
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
