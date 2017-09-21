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

DWORD
VmDirRESTServerInit(
    VOID
    )
{
    DWORD   dwError = 0;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDirRESTHandlers;
    PREST_API_MODULE    pModule = NULL;

    MODULE_REG_MAP stRegMap[] =
    {
        {"ldap", VmDirRESTGetLdapModule},
        {"metrics", VmDirRESTGetMetricsModule},
        {NULL, NULL}
    };

    config.pSSLCertificate = RSA_SERVER_CERT;
    config.pSSLKey = RSA_SERVER_KEY;
    config.pServerPort = gVmdirGlobals.pszRestListenPort;
    config.pDebugLogFile = VMDIR_REST_DEBUGLOGFILE;
    config.pClientCount = VMDIR_REST_CLIENTCNT;
    config.pMaxWorkerThread = VMDIR_REST_WORKERTHCNT;

    dwError = OidcClientGlobalInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTInit(&config, NULL, &gpVdirRestHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = coapi_load_from_file(REST_API_SPEC, &gpVdirRestApiDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = coapi_map_api_impl(gpVdirRestApiDef, stRegMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pModule = gpVdirRestApiDef->pModules; pModule; pModule = pModule->pNext)
    {
        PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
        for (; pEndPoint; pEndPoint = pEndPoint->pNext)
        {
            dwError = VmRESTRegisterHandler(
                    gpVdirRestHandle, pEndPoint->pszName, pHandlers, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmRESTStart(gpVdirRestHandle);
    if (dwError)
    {
        // soft fail - will not listen on REST port.
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "VmRESTStart failed with error %d, not going to listen on REST port",
                dwError);
        dwError = 0;
    }

cleanup:
    return dwError;

error:
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
    PREST_API_MODULE    pModule = NULL;

    if (gpVdirRestHandle)
    {
        VmRESTStop(gpVdirRestHandle);
        if (gpVdirRestApiDef)
        {
            pModule = gpVdirRestApiDef->pModules;
            for (; pModule; pModule = pModule->pNext)
            {
                PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
                for (; pEndPoint; pEndPoint = pEndPoint->pNext)
                {
                    (VOID)VmRESTUnRegisterHandler(
                            gpVdirRestHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(gpVdirRestHandle);
    }

    OidcClientGlobalCleanup();
    VMDIR_SAFE_FREE_MEMORY(gpVdirRestApiDef);
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
