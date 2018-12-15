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

REST_PROCESSOR sLwCARestMetricsApiHandlers =
{
    .pfnHandleCreate = &LwCARestMetricsApiRequestHandler,
    .pfnHandleRead   = &LwCARestMetricsApiRequestHandler,
    .pfnHandleUpdate = &LwCARestMetricsApiRequestHandler,
    .pfnHandleDelete = &LwCARestMetricsApiRequestHandler,
    .pfnHandleOthers = &LwCARestMetricsApiRequestHandler
};

static
DWORD
_LwCARestServerInit(
    VOID
    );

static
DWORD
_LwCARestMetricsServerInit(
    VOID
    );

static
VOID
_LwCARestServerShutdown(
    VOID
    );

static
VOID
_LwCARestMetricsServerShutdown(
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

    MODULE_REG_MAP stRegMapMetrics[] =
    {
        {"metrics", LwCARestMetricsModule},
        {NULL, NULL}
    };

    // HTTPS Server for APIs
    dwError = coapi_load_from_file(LWCA_REST_API_SPEC, &gpLwCARestApiDef);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to load rest api spec file");

    dwError = coapi_map_api_impl(gpLwCARestApiDef, stRegMap);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to map rest api implementations");

    dwError = LwCAOpensslInit();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestServerInit();
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

    // HTTP Server for Metrics
    dwError = coapi_load_from_file(LWCA_REST_METRICS_API_SPEC, &gpLwCARestMetricsApiDef);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to load metrics rest api spec file");

    dwError = coapi_map_api_impl(gpLwCARestMetricsApiDef, stRegMapMetrics);
    BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, "Failed to map metrics rest api implementations");

    dwError = _LwCARestMetricsServerInit();
    BAIL_ON_LWCA_ERROR(dwError);

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
    DWORD dwStopServer = 0;
    DWORD dwStopMetricsServer = 0;

    dwStopServer = _LwCAStopRestHandle(gpLwCARestHandle);
    dwStopMetricsServer = _LwCAStopRestHandle(gpLwCARestMetricsHandle);

    return dwStopServer | dwStopMetricsServer;
}

VOID
LwCARestServerShutdown(
    VOID
    )
{
    _LwCARestServerShutdown();
    _LwCARestMetricsServerShutdown();

    coapi_free_api_def(gpLwCARestApiDef);
    coapi_free_api_def(gpLwCARestMetricsApiDef);

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
_LwCARestServerInit(
    VOID
    )
{
    DWORD               dwError         = 0;
    REST_CONF           config          = {0};
    PREST_PROCESSOR     pHandlers       = &sLwCARestApiHandlers;
    PREST_API_MODULE    pModule         = NULL;
    PVMREST_HANDLE      pHandle         = NULL;

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

    dwError = VmRESTInit(&config, &pHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to init REST API server");

    for (pModule = gpLwCARestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(pHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to register REST API handler");
        }
    }

    dwError = VmRESTStart(pHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to start REST API server handle");

    gpLwCARestHandle = pHandle;

cleanup:
    return dwError;

error:
    if (_LwCAStopRestHandle(pHandle) == 0)
    {
        _LwCAFreeRestHandle(pHandle, gpLwCARestApiDef);
    }

    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
DWORD
_LwCARestMetricsServerInit(
    VOID
    )
{
    DWORD               dwError         = 0;
    REST_CONF           config          = {0};
    PREST_PROCESSOR     pHandlers       = &sLwCARestMetricsApiHandlers;
    PREST_API_MODULE    pModule         = NULL;
    PVMREST_HANDLE      pHandle         = NULL;

    config.serverPort = LWCA_HTTP_PORT_NUM;
    config.connTimeoutSec = LWCA_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = LWCA_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;
    config.nWorkerThr = LWCA_REST_WORKERTHCNT;
    config.nClientCnt = LWCA_REST_CLIENTCNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = LWCA_DAEMON_NAME;
    config.isSecure = FALSE;
    config.useSysLog = TRUE;
    config.debugLogLevel = LwCAToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to init REST metrics server");

    for (pModule = gpLwCARestMetricsApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(pHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to register REST metrics handler");
        }
    }

    dwError = VmRESTStart(pHandle);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to start REST metrics server handle");

    gpLwCARestMetricsHandle = pHandle;

cleanup:
    return dwError;

error:
    if (_LwCAStopRestHandle(pHandle) == 0)
    {
        _LwCAFreeRestHandle(pHandle, gpLwCARestMetricsApiDef);
    }

    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
VOID
_LwCARestServerShutdown(
    VOID
    )
{
    _LwCAFreeRestHandle(gpLwCARestHandle, gpLwCARestApiDef);
    gpLwCARestHandle = NULL;
}

static
VOID
_LwCARestMetricsServerShutdown(
    VOID
    )
{
    _LwCAFreeRestHandle(gpLwCARestMetricsHandle, gpLwCARestMetricsApiDef);
    gpLwCARestMetricsHandle = NULL;
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
