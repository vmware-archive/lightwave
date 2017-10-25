/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define VMCA_REST_PORT_STR_MAX_SIZE 8

REST_PROCESSOR sVmcaRestHandlers =
{
    .pfnHandleRequest  = NULL,
    .pfnHandleCreate  = &VMCAHandleHttpRequest,
    .pfnHandleRead  = &VMCAHandleHttpRequest,
    .pfnHandleUpdate  = &VMCAHandleHttpRequest,
    .pfnHandleDelete  = &VMCAHandleHttpRequest
};

PSTR restEndPoints[] =
{
    "/v1/vmca/certificates",
    "/v1/vmca/root",
    "/v1/vmca/crl",
    "/v1/vmca"
};

#ifndef _WIN32

static
DWORD
_VMCAHttpServiceStartup(
    VOID
    );

static
DWORD
_VMCAHttpsServiceStartup(
    VOID
    );

static
VOID
_VMCAHttpServiceShutdown(
    VOID
    );

static
VOID
_VMCAHttpsServiceShutdown(
    VOID
    );

static
DWORD
_VMCAGetRestRegPort(
    PSTR  pszPortRegKeyStr,
    PSTR  pszDefaultPort,
    PSTR* ppszRegPort
    );

DWORD
VMCARestServiceStartup(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = _VMCAHttpServiceStartup();
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = _VMCAHttpsServiceStartup();
    //soft fail
    if (dwError != 0)
    {
        VMCA_LOG_ERROR("%s: failure while starting HTTPS service(expected before promote), error: %d",
                __FUNCTION__, dwError);
        dwError = 0;
    }

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR("%s: failure while starting REST service, error: %d", __FUNCTION__, dwError);
    goto cleanup;

}

VOID
VMCARestServiceShutdown(
    VOID
    )
{
    _VMCAHttpServiceShutdown();
    _VMCAHttpsServiceShutdown();
}

static
DWORD
_VMCAHttpServiceStartup(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD iter = 0;
    DWORD endPointCnt = 0;
    PSTR  pszPort = NULL;
    REST_CONF config = {0};
    PREST_PROCESSOR pHandlers = &sVmcaRestHandlers;

    dwError = _VMCAGetRestRegPort(
            VMCA_HTTP_PORT_REG_KEY,
            VMCAHTTPPORT,
            &pszPort
            );
    BAIL_ON_VMCA_ERROR(dwError);

    // empty port string indicates don't start corresponding service
    if (IsNullOrEmptyString(pszPort))
    {
        goto cleanup;
    }

    config.pSSLCertificate = VMCARESTSSLCERT;
    config.pSSLKey = VMCARESTSSLKEY;
    config.pServerPort = pszPort;
    config.pDebugLogFile = VMCAHTTPDEBUGLOGFILE;
    config.pClientCount = VMCARESTCLIENTCNT;
    config.pMaxWorkerThread = VMCARESTWORKERTHCNT;

    dwError = VmRESTInit(&config, NULL, &gpVMCAHTTPHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    endPointCnt = ARRAY_SIZE(restEndPoints);

    for (iter = 0; iter < endPointCnt; iter++)
    {
        dwError = VmRESTRegisterHandler(
                gpVMCAHTTPHandle,
                restEndPoints[iter],
                pHandlers,
                NULL);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = VmRESTStart(gpVMCAHTTPHandle);
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszPort);
    return dwError;

error:
    _VMCAHttpServiceShutdown();
    VMCA_LOG_ERROR("%s: failure while starting REST HTTP service, error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VMCAHttpsServiceStartup(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD iter = 0;
    DWORD endPointCnt = 0;
    REST_CONF config = {0};
    PSTR  pszCert = NULL;
    PSTR  pszKey = NULL;
    PSTR  pszPort = NULL;
    PREST_PROCESSOR pHandlers = &sVmcaRestHandlers;

    dwError = _VMCAGetRestRegPort(
            VMCA_HTTPS_PORT_REG_KEY,
            VMCAHTTPSPORT,
            &pszPort
            );
    BAIL_ON_VMCA_ERROR(dwError);

    // empty port string indicates don't start corresponding service
    if (IsNullOrEmptyString(pszPort))
    {
        goto cleanup;
    }

    config.pSSLCertificate = NULL;
    config.pSSLKey = NULL;
    config.pServerPort = pszPort;
    config.pDebugLogFile = VMCAHTTPSDEBUGLOGFILE;
    config.pClientCount = VMCARESTCLIENTCNT;
    config.pMaxWorkerThread = VMCARESTWORKERTHCNT;

    //Get Certificate and Key from VECS and Set it to Rest Engine
    dwError = VMCAGetVecsMachineCert(&pszCert, &pszKey);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTInit(&config, NULL, &gpVMCAHTTPSHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(gpVMCAHTTPSHandle, pszCert, VMCAStringLenA(pszCert)+1, SSL_DATA_TYPE_CERT);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(gpVMCAHTTPSHandle, pszKey, VMCAStringLenA(pszKey)+1, SSL_DATA_TYPE_KEY);
    BAIL_ON_VMREST_ERROR(dwError);

    endPointCnt = ARRAY_SIZE(restEndPoints);

    for (iter = 0; iter < endPointCnt; iter++)
    {
        dwError = VmRESTRegisterHandler(
                gpVMCAHTTPSHandle,
                restEndPoints[iter],
                pHandlers,
                NULL);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = VmRESTStart(gpVMCAHTTPSHandle);
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszCert);
    VMCA_SAFE_FREE_MEMORY(pszKey);
    VMCA_SAFE_FREE_MEMORY(pszPort);
    return dwError;

error:
    _VMCAHttpsServiceShutdown();
    VMCA_LOG_ERROR("%s: failure while starting REST HTTPS service, error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

static
VOID
_VMCAHttpServiceShutdown(
    VOID
    )
{
    DWORD iter = 0;
    DWORD endPointCnt = 0;

    VMCA_LOG_INFO("%s: starting http rest server shutdown:", __FUNCTION__);
    if (gpVMCAHTTPHandle)
    {
        VmRESTStop(gpVMCAHTTPHandle);
        endPointCnt = ARRAY_SIZE(restEndPoints);
        for (iter = 0; iter < endPointCnt; iter++)
        {
            (VOID)VmRESTUnRegisterHandler(
                    gpVMCAHTTPHandle,
                    restEndPoints[iter]);
        }
        VmRESTShutdown(gpVMCAHTTPHandle);
    }
    gpVMCAHTTPHandle = NULL;
    VMCA_LOG_INFO("%s: completed http rest server shutdown:", __FUNCTION__);
}

static
VOID
_VMCAHttpsServiceShutdown(
    VOID
    )
{
    DWORD iter = 0;
    DWORD endPointCnt = 0;

    VMCA_LOG_INFO("%s: starting https rest server shutdown:", __FUNCTION__);
    if (gpVMCAHTTPSHandle)
    {
        VmRESTStop(gpVMCAHTTPSHandle);
        endPointCnt = ARRAY_SIZE(restEndPoints);
        for (iter = 0; iter < endPointCnt; iter++)
        {
            (VOID)VmRESTUnRegisterHandler(
                    gpVMCAHTTPSHandle,
                    restEndPoints[iter]);
        }
        VmRESTShutdown(gpVMCAHTTPSHandle);
    }
    gpVMCAHTTPSHandle = NULL;
    VMCA_LOG_INFO("%s: completed https rest server shutdown:", __FUNCTION__);
}

static
DWORD
_VMCAGetRestRegPort(
    PSTR  pszPortRegKeyStr,
    PSTR  pszDefaultPort,
    PSTR* ppszRegPort
    )
{
    DWORD dwError = 0;
    PSTR  pszPort = NULL;

    if (ppszRegPort == NULL)
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                  VMCA_REST_PORT_STR_MAX_SIZE,
                  (PVOID*)&pszPort
                  );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetRegKeyValue(
                  VMCA_KEY_PARAMETERS,
                  pszPortRegKeyStr,
                  pszPort,
                  VMCA_REST_PORT_STR_MAX_SIZE
                  );

    if (dwError != 0)
    {
        //assign default value
        VMCAStringNCpyA(
            pszPort,
            VMCAStringLenA(pszDefaultPort)+1,
            pszDefaultPort,
            VMCAStringLenA(pszDefaultPort)+1
            );
        //not a failure
        dwError = 0;
    }

    //transfer ownership
    *ppszRegPort = pszPort;
     pszPort = NULL;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszPort);
    return dwError;

error:
    VMCA_LOG_ERROR("%s: failed error: %d", __FUNCTION__, dwError);
    goto cleanup;

}
#endif
#endif
