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

DWORD
VmDnsRESTServerInit(
    VOID
    )
{

    DWORD   dwError = 0;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDnsRESTHandlers;
    PREST_API_MODULE    pModule = NULL;

    MODULE_REG_MAP stRegMap[] =
    {
          {"VmDnsMetrics", VmDnsRESTGetMetricsModule},
          {NULL, NULL}
    };

    config.pSSLCertificate = NULL;
    config.pSSLKey = NULL;

    //get the listen port from the registry
    dwError = VmDnsConfigGetStringA(
                VMDNS_REG_CONFIG_KEY_PATH,
                VMDNS_REG_KEY_REST_LISTEN_PORT,
                &gVmdnsGlobals.pszRestListenPort
                );
    BAIL_ON_VMDNS_ERROR(dwError);

    config.pServerPort = gVmdnsGlobals.pszRestListenPort;
    config.pDebugLogFile = VMDNS_REST_DEBUGLOGFILE;
    config.pClientCount = VMDNS_REST_CLIENTCNT;
    config.pMaxWorkerThread = VMDNS_REST_WORKERTHCNT;

    dwError = VmRESTInit(&config, NULL, &gpVdnsRESTHandle);
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
                            gpVdnsRESTHandle,
                            pEndPoint->pszName,
                            pHandlers,
                            NULL
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(gpVdnsRESTHandle);
    if (dwError)
    {
        // soft fail - will not listen on REST port.
        VmDnsLog(VMDNS_LOG_LEVEL_ERROR,"VmRESTStart failed with error %d, not going to listen on REST port",dwError);
        dwError = 0;
    }

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDnsRESTServerShutdown(
    VOID
    )
{
    PREST_API_MODULE    pModule = NULL;

    if (gpVdnsRESTHandle)
    {
        VmRESTStop(gpVdnsRESTHandle);
        if (gpVdnsRestApiDef)
        {
            pModule = gpVdnsRestApiDef->pModules;
            for (; pModule; pModule = pModule->pNext)
            {
                PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
                for (; pEndPoint; pEndPoint = pEndPoint->pNext)
                {
                    (VOID)VmRESTUnRegisterHandler(
                            gpVdnsRESTHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(gpVdnsRESTHandle);
    }
}

DWORD
VmDnsRESTRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD   dwError = 0;
    PVDNS_REST_OPERATION    pRestOp = NULL;
    PREST_API_METHOD    pMethod = NULL;

    if (!pRESTHandle || !pRequest || !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VmDnsdState() == VMDNS_SHUTDOWN)
    {
        goto cleanup;
    }

    dwError = VmDnsRESTOperationCreate(&pRestOp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRESTOperationReadRequest(
                    pRestOp,
                    pRESTHandle,
                    pRequest
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = coapi_find_handler(
                    gpVdnsRestApiDef,
                    pRestOp->pszPath,
                    pRestOp->pszMethod,
                    &pMethod
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pMethod->pFnImpl((void*)pRestOp,NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

response:
    VMDNS_SET_REST_RESULT(pRestOp, dwError, NULL);
    /*
     * Send response irrespective of error or success
     * Nothing can be done if failed to send response
     * All the module under response and cleanup flag, handles NULL pointer case
     */
    dwError = VmDnsRESTOperationWriteResponse(
                    pRestOp,
                    pRESTHandle,
                    ppResponse
                    );
    goto cleanup;

cleanup:
    VmDnsFreeRESTOperation(pRestOp);
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto response;
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
