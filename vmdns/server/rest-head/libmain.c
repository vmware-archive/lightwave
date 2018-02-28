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

REST_PROCESSOR sVmDnsRESTHandlers =
{
    .pfnHandleCreate = &VmDnsRESTRequestHandler,
    .pfnHandleRead = &VmDnsRESTRequestHandler,
    .pfnHandleUpdate = &VmDnsRESTRequestHandler,
    .pfnHandleDelete = &VmDnsRESTRequestHandler,
    .pfnHandleOthers = &VmDnsRESTRequestHandler
};

static
VOID
VmDnsFreeRESTHandle(
    PVMREST_HANDLE    pHandle
    );

DWORD
VmDnsRESTServerInit(
    VOID
    )
{

    DWORD   dwError = 0;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDnsRESTHandlers;
    PREST_API_MODULE    pModule = NULL;
    PVMREST_HANDLE      pHTTPHandle = NULL;

    MODULE_REG_MAP stRegMap[] =
    {
          {"VmDnsMetrics", VmDnsRESTGetMetricsModule},
          {NULL, NULL}
    };

    //get the listen port from the registry
    dwError = VmDnsConfigGetDword(
                VMDNS_REG_KEY_REST_LISTEN_PORT,
                &gVmdnsGlobals.dwRestListenPort
                );
    if (dwError != 0)
    {
        gVmdnsGlobals.dwRestListenPort = DEFAULT_HTTP_PORT_NUM;
        dwError = 0;
    }

    // if Rest port is '0' then user wants to disable HTTP endpoint
    if (gVmdnsGlobals.dwRestListenPort == 0)
    {
        goto cleanup;
    }

    config.serverPort = gVmdnsGlobals.dwRestListenPort;
    config.connTimeoutSec = VMDNS_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMDNS_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;
    config.nWorkerThr = VMDNS_REST_WORKERTHCNT;
    config.nClientCnt = VMDNS_REST_CLIENTCNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMDNS_DAEMON_NAME;
    config.isSecure = FALSE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VMREST_LOG_LEVEL_ERROR;

    dwError = VmRESTInit(&config, &pHTTPHandle);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = coapi_load_from_file(REST_API_SPEC, &gpVdnsRestApiDef);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVdnsRestApiDef, stRegMap);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (pModule = gpVdnsRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                            pHTTPHandle,
                            pEndPoint->pszName,
                            pHandlers,
                            NULL
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(pHTTPHandle);
    if (dwError)
    {
        // soft fail - will not listen on REST port.
        VmDnsLog(VMDNS_LOG_LEVEL_ERROR,"VmRESTStart failed with error %d, not going to listen on REST port",dwError);
        dwError = 0;
    }

    gpVdnsRESTHandle = pHTTPHandle;

cleanup:
    return dwError;

error:
    VmDnsFreeRESTHandle(pHTTPHandle);
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}


VOID
VmDnsRESTServerShutdown(
    VOID
    )
{
    VmDnsFreeRESTHandle(gpVdnsRESTHandle);
    gpVdnsRESTHandle = NULL;
}

static
VOID
VmDnsFreeRESTHandle(
    PVMREST_HANDLE    pHandle
    )
{
    PREST_API_MODULE    pModule = NULL;
    DWORD               dwError = 0;

    if (pHandle)
    {
        /*
         * REST library have detached threads, maximum time out specified is the max time
         * allowed for the threads to  finish their execution.
         * If finished early, it will return success.
         * If not able to finish in specified time, failure will be returned
         */
        dwError = VmRESTStop(pHandle, VMDNS_REST_STOP_TIMEOUT_SEC);
        BAIL_ON_VMDNS_ERROR(dwError);

        if (gpVdnsRestApiDef)
        {
            pModule = gpVdnsRestApiDef->pModules;
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

cleanup:
    return;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_WARNING,"%s: rest stop error: %d",__FUNCTION__, dwError);
    goto cleanup;
}

#else

DWORD
VmDnsRESTServerInit(
    VOID
    )
{
    return 0;
}

VOID
VmDnsRESTServerShutdown(
    VOID
    )
{
    return;
}

#endif
