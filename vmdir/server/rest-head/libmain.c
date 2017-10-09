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
    PREST_PROCESSOR     pHandlers = &sVmDirRESTHandlers;
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
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;
    REST_CONF   config = {0};
    PREST_PROCESSOR     pHandlers = &sVmDirRESTHandlers;
    PREST_API_MODULE    pModule = NULL;

    /*
     * pszHTTPSListenPort can never be NULL because of default values assigned to them
     * if Port string is empty, it means user wants to disable corresponding service
     */
    if (IsNullOrEmptyString(gVmdirGlobals.pszHTTPSListenPort))
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s : not listening in HTTPS port",
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
    PREST_API_MODULE    pModule = NULL;

    if (IsNullOrEmptyString(gVmdirGlobals.pszHTTPListenPort))
    {
        //No operation - HTTP port was not initialized
        return;
    }

    if (gpVdirRestHTTPHandle)
    {
        VmRESTStop(gpVdirRestHTTPHandle);
        if (gpVdirRestApiDef)
        {
            pModule = gpVdirRestApiDef->pModules;
            for (; pModule; pModule = pModule->pNext)
            {
                PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
                for (; pEndPoint; pEndPoint = pEndPoint->pNext)
                {
                    (VOID)VmRESTUnRegisterHandler(
                            gpVdirRestHTTPHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(gpVdirRestHTTPHandle);
    }
}

static
VOID
_VmDirRESTServerShutdownHTTPS(
    VOID
    )
{
    PREST_API_MODULE    pModule = NULL;

    if (IsNullOrEmptyString(gVmdirGlobals.pszHTTPSListenPort))
    {
        //No operation - HTTPS port was not initialized
        return;
    }

    if (gpVdirRestHTTPSHandle)
    {
        VmRESTStop(gpVdirRestHTTPSHandle);
        if (gpVdirRestApiDef)
        {
            pModule = gpVdirRestApiDef->pModules;
            for (; pModule; pModule = pModule->pNext)
            {
                PREST_API_ENDPOINT pEndPoint = pModule->pEndPoints;
                for (; pEndPoint; pEndPoint = pEndPoint->pNext)
                {
                    (VOID)VmRESTUnRegisterHandler(
                            gpVdirRestHTTPSHandle, pEndPoint->pszName);
                }
            }
        }
        VmRESTShutdown(gpVdirRestHTTPSHandle);
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
