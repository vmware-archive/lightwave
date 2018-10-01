/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

#ifdef REST_ENABLED

REST_PROCESSOR sLwCARestApiHandlers =
{
    .pfnHandleCreate = &LwCARestApiRequestHandler,
    .pfnHandleRead   = &LwCARestApiRequestHandler,
    .pfnHandleUpdate = &LwCARestApiRequestHandler,
    .pfnHandleDelete = &LwCARestApiRequestHandler,
    .pfnHandleOthers = &LwCARestApiRequestHandler
};

static
DWORD
_LwCARestServerInitHTTPS(
    VOID
    );

static
VOID
_LwCARestServerShutdownHTTPS(
    VOID
    );

static
DWORD
_LwCAStopRestHandle(
    PVMREST_HANDLE    pHandle
    );

static
VOID
_LwCAFreeRestHandle(
    PVMREST_HANDLE    pHandle,
    PREST_API_DEF     pRestApiDef
    );

DWORD
LwCARestServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    MODULE_REG_MAP stRegMap[] =
    {
        {"rootca", LwCARestRootCAModule},
        {"intermediateca", LwCARestIntermediateCAModule},
        {"crl", LwCARestCRLModule},
        {"certificates", LwCARestCertificatesModule},
        {NULL, NULL}
    };

    dwError = coapi_load_from_file(LWCA_REST_API_SPEC, &gpLwCARestApiDef);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to load rest api spec file");

    dwError = coapi_map_api_impl(gpLwCARestApiDef, stRegMap);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to map rest api implementations");

    dwError = LwCAOpensslInit();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestServerInitHTTPS();
    if (dwError != 0)
    {
        /*
         * Before promoting lightwave node, obtaining cert from VECS will fail which is expected
         * hence treat it as soft fail
         */
         LWCA_LOG_WARNING(
                "[%s,%d] HTTPS port init failed with error (%d), (failure is expected before promote)",
                __FUNCTION__,
                __LINE__,
                dwError);
         dwError = 0;
    }

cleanup:
    return dwError;

error:
    if (LwCARestServerStop() == 0)
    {
        LwCARestServerShutdown();
    }

    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestServerStop(
    VOID
    )
{
    DWORD dwStopHttps = 0;

    dwStopHttps = _LwCAStopRestHandle(gpLwCARestHTTPSHandle);

    return dwStopHttps;
}

VOID
LwCARestServerShutdown(
    VOID
    )
{
    _LwCARestServerShutdownHTTPS();

    coapi_free_api_def(gpLwCARestApiDef);

    LwCAOpensslShutdown();
}

VMREST_LOG_LEVEL
LwCAToCRestEngineLogLevel(
    VOID
    )
{
    LWCA_LOG_LEVEL      lwcaLogLevel         = LwCALogGetLevel();
    VMREST_LOG_LEVEL    cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;

    switch (lwcaLogLevel)
    {
    case LWCA_LOG_LEVEL_ERROR:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
        break;

    case LWCA_LOG_LEVEL_WARNING:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_WARNING;
        break;

    case LWCA_LOG_LEVEL_NOTICE:
    case LWCA_LOG_LEVEL_INFO:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_INFO;
        break;

    case LWCA_LOG_LEVEL_DEBUG:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_DEBUG;
        break;

    default:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
    }

    return cResetEngineLogLevel;
}

static
DWORD
_LwCARestServerInitHTTPS(
    VOID
    )
{
    DWORD               dwError         = 0;
    REST_CONF           config          = {0};
    PREST_PROCESSOR     pHandlers       = &sLwCARestApiHandlers;
    PREST_API_MODULE    pModule         = NULL;
    PVMREST_HANDLE      pHTTPSHandle    = NULL;

    config.serverPort = LWCA_HTTPS_PORT_NUM;
    config.connTimeoutSec = LWCA_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = LWCA_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = gLwCAServerGlobals.pSslCtx;
    config.nWorkerThr = LWCA_REST_WORKERTHCNT;
    config.nClientCnt = LWCA_REST_CLIENTCNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = LWCA_DAEMON_NAME;
    config.isSecure = TRUE;
    config.useSysLog = TRUE;
    config.debugLogLevel = LwCAToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pHTTPSHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to init REST server");

    for (pModule = gpLwCARestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(pHTTPSHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to register REST handler");
        }
    }

    dwError = VmRESTStart(pHTTPSHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to start REST server handle");

    gpLwCARestHTTPSHandle = pHTTPSHandle;

cleanup:
    return dwError;

error:
    if (_LwCAStopRestHandle(pHTTPSHandle) == 0)
    {
        _LwCAFreeRestHandle(pHTTPSHandle, gpLwCARestApiDef);
    }

    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
VOID
_LwCARestServerShutdownHTTPS(
    VOID
    )
{
    _LwCAFreeRestHandle(gpLwCARestHTTPSHandle, gpLwCARestApiDef);
    gpLwCARestHTTPSHandle = NULL;
}

static
DWORD
_LwCAStopRestHandle(
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
        dwError = VmRESTStop(pHandle, LWCA_REST_STOP_TIMEOUT_SEC);
        if (dwError != 0)
        {
            LWCA_LOG_WARNING(
                   "%s: Rest stop error: %d",
                   __FUNCTION__,
                   dwError);
        }
    }

    return dwError;
}

static
VOID
_LwCAFreeRestHandle(
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
                    (VOID)VmRESTUnRegisterHandler(pHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(pHandle);
    }
}

#else

DWORD
LwCARestServerInit(
    VOID
    )
{
    return 0;
}

DWORD
LwCARestServerStop(
    VOID
    )
{
    return 0;
}

VOID
LwCARestServerShutdown(
    VOID
    )
{
    return;
}

#endif
