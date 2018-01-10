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

REST_PROCESSOR sVmDirRESTHandlers =
{
    .pfnHandleCreate = &VmDirRESTRequestHandler,
    .pfnHandleRead = &VmDirRESTRequestHandler,
    .pfnHandleUpdate = &VmDirRESTRequestHandler,
    .pfnHandleDelete = &VmDirRESTRequestHandler,
    .pfnHandleOthers = &VmDirRESTRequestHandler
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
    PVMREST_HANDLE    pHandle
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

cleanup:
    return dwError;

error:
    VmDirRESTServerShutdown();
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

VOID
VmDirRESTServerShutdown(
    VOID
    )
{
    _VmDirRESTServerShutdownHTTP();
    _VmDirRESTServerShutdownHTTPS();

    VmDirFreeRESTCache(gpVdirRestCache);
    VMDIR_SAFE_FREE_MEMORY(gpVdirRestApiDef);
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
    PREST_PROCESSOR    pHandlers = &sVmDirRESTHandlers;
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
    config.nWorkerThr = VMDIR_REST_WORKERTHCNT;
    config.nClientCnt = VMDIR_REST_CLIENTCNT;
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
    _VmDirFreeRESTHandle(pHTTPHandle);
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
    PREST_PROCESSOR     pHandlers = &sVmDirRESTHandlers;
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
    config.nWorkerThr = VMDIR_REST_WORKERTHCNT;
    config.nClientCnt = VMDIR_REST_CLIENTCNT;
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
    _VmDirFreeRESTHandle(pHTTPSHandle);
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
    DWORD dwError = 0;
    PREST_API_MODULE  pModule = NULL;

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
