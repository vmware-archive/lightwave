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

#ifdef REST_V2_ENABLED

REST_PROCESSOR sVMCARestApiHandlers =
{
    .pfnHandleCreate = &VMCARestApiRequestHandler,
    .pfnHandleRead   = &VMCARestApiRequestHandler,
    .pfnHandleUpdate = &VMCARestApiRequestHandler,
    .pfnHandleDelete = &VMCARestApiRequestHandler,
    .pfnHandleOthers = &VMCARestApiRequestHandler
};

static
DWORD
_VMCARestServerInitHTTPS(
    VOID
    );

static
VOID
_VMCARestServerShutdownHTTPS(
    VOID
    );

static
DWORD
_VMCAStopRestHandle(
    PVMREST_HANDLE    pHandle
    );

static
VOID
_VMCAFreeRestHandle(
    PVMREST_HANDLE    pHandle,
    PREST_API_DEF     pRestApiDef
    );

DWORD
VMCARestServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    MODULE_REG_MAP stRegMap[] =
    {
        //{"endpoint", VMCARestFunctionModule},
        {NULL, NULL}
    };

    /*
     * We can use the same REST_API_SPEC for both HTTP and HTTPS
     * if the rest init code refers to common API definitions
     */
    dwError = coapi_load_from_file(REST_API_SPEC, &gpVMCARestApiDef);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVMCARestApiDef, stRegMap);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = _VMCARestServerInitHTTPS();
    if (dwError != 0)
    {
        /*
         * Before promoting lightwave node, obtaining cert from VECS will fail which is expected
         * hence treat it as soft fail
         */
         VMCA_LOG_WARNING(
                 "VmRESTServerInit: HTTPS port init failed with error %d, (failure is expected before promote)",
                 dwError);
         dwError = 0;
    }

cleanup:
    return dwError;

error:
    if (VMCARestServerStop() == 0)
    {
        VMCARestServerShutdown();
    }
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestServerStop(
    VOID
    )
{
    DWORD dwStopHttps = 0;
    dwStopHttps = _VMCAStopRestHandle(gpVMCARestHTTPSHandle);

    return dwStopHttps;
}

VOID
VMCARestServerShutdown(
    VOID
    )
{
    _VMCARestServerShutdownHTTPS();

    coapi_free_api_def(gpVMCARestApiDef);
}

VMREST_LOG_LEVEL
VMCAToCRestEngineLogLevel(
    VOID
    )
{
    VMCA_LOG_LEVEL      vmcaLogLevel         = VMCALogGetLevel();
    VMREST_LOG_LEVEL    cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;

    switch (vmcaLogLevel)
    {
    case VMCA_LOG_LEVEL_ERROR:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
        break;

    case VMCA_LOG_LEVEL_WARNING:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_WARNING;
        break;

    case VMCA_LOG_LEVEL_NOTICE:
    case VMCA_LOG_LEVEL_INFO:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_INFO;
        break;

    case VMCA_LOG_LEVEL_DEBUG:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_DEBUG;
        break;

    default:
        cResetEngineLogLevel = VMREST_LOG_LEVEL_ERROR;
    }

    return cResetEngineLogLevel;
}

static
DWORD
_VMCARestServerInitHTTPS(
    VOID
    )
{
    DWORD               dwError = 0;
    REST_CONF           config = {0};
    PREST_PROCESSOR     pHandlers = &sVMCARestApiHandlers;
    PREST_API_MODULE    pModule = NULL;
    PVMREST_HANDLE      pHTTPSHandle = NULL;

    config.serverPort = VMCA_HTTPS_V2_PORT_NUM;
    config.connTimeoutSec = VMCA_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMCA_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;                  // TODO: Set SSL Context before initiazlizing
    config.nWorkerThr = VMCA_REST_WORKERTHCNT;
    config.nClientCnt = VMCA_REST_CLIENTCNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMCA_DAEMON_NAME;
    config.isSecure = TRUE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VMCAToCRestEngineLogLevel();

    dwError = VmRESTInit(&config, &pHTTPSHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    for (pModule = gpVMCARestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    pHTTPSHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(pHTTPSHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    gpVMCARestHTTPSHandle = pHTTPSHandle;

cleanup:
    return dwError;

error:
    if (_VMCAStopRestHandle(pHTTPSHandle) == 0)
    {
        _VMCAFreeRestHandle(pHTTPSHandle, gpVMCARestApiDef);
    }
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
VOID
_VMCARestServerShutdownHTTPS(
    VOID
    )
{
    _VMCAFreeRestHandle(gpVMCARestHTTPSHandle, gpVMCARestApiDef);
    gpVMCARestHTTPSHandle = NULL;
}

static
DWORD
_VMCAStopRestHandle(
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
        dwError = VmRESTStop(pHandle, VMCA_REST_STOP_TIMEOUT_SEC);
        if (dwError != 0)
        {
            VMCA_LOG_WARNING(
                   "%s: Rest stop error: %d",
                   __FUNCTION__,
                   dwError);
        }
    }

    return dwError;
}

static
VOID
_VMCAFreeRestHandle(
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
VMCARestServerInit(
    VOID
    )
{
    return 0;
}

VOID
VMCARestServerShutdown(
    VOID
    )
{
    return;
}

#endif
