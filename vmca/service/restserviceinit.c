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
VOID
_VMCARestFreeHandle(
    PVMREST_HANDLE    pHandle
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
    DWORD dwPort = 0;
    REST_CONF config = {0};
    PREST_PROCESSOR pHandlers = &sVmcaRestHandlers;
    PVMREST_HANDLE  pHTTPHandle = NULL;

    (VOID)VMCAGetRegKeyValueDword(
                  VMCA_KEY_PARAMETERS,//VMCA_CONFIG_PARAMETER_KEY_PATH,
                  VMCA_HTTP_PORT_REG_KEY,
                  &dwPort,
                  VMCA_HTTP_PORT_NUM
                  );

    // port value '0' indicates don't start HTTP service
    if (dwPort == 0)
    {
        goto cleanup;
    }

    config.serverPort = dwPort;
    config.connTimeoutSec = VMCA_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMCA_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;
    config.nWorkerThr = VMCA_REST_WORKER_TH_CNT;
    config.nClientCnt = VMCA_REST_CLIENT_CNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMCA_DAEMON_NAME;
    config.isSecure = FALSE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VMREST_LOG_LEVEL_ERROR;

    dwError = VmRESTInit(&config, &pHTTPHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    endPointCnt = ARRAY_SIZE(restEndPoints);

    for (iter = 0; iter < endPointCnt; iter++)
    {
        dwError = VmRESTRegisterHandler(
                pHTTPHandle,
                restEndPoints[iter],
                pHandlers,
                NULL);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = VmRESTStart(pHTTPHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    gpVMCAHTTPHandle = pHTTPHandle;

cleanup:
    return dwError;

error:
    _VMCARestFreeHandle(pHTTPHandle);
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
    DWORD dwPort = 0;
    PREST_PROCESSOR pHandlers = &sVmcaRestHandlers;
    PVMREST_HANDLE  pHTTPSHandle = NULL;

    (VOID)VMCAGetRegKeyValueDword(
                  VMCA_KEY_PARAMETERS,//VMCA_CONFIG_PARAMETER_KEY_PATH,
                  VMCA_HTTPS_PORT_REG_KEY,
                  &dwPort,
                  VMCA_HTTPS_PORT_NUM
                  );

    // port value '0' indicates don't start HTTPS service
    if (dwPort == 0)
    {
        goto cleanup;
    }

    config.serverPort = dwPort;
    config.connTimeoutSec = VMCA_REST_CONN_TIMEOUT_SEC;
    config.maxDataPerConnMB = VMCA_MAX_DATA_PER_CONN_MB;
    config.pSSLContext = NULL;
    config.nWorkerThr = VMCA_REST_WORKER_TH_CNT;
    config.nClientCnt = VMCA_REST_CLIENT_CNT;
    config.SSLCtxOptionsFlag = 0;
    config.pszSSLCertificate = NULL;
    config.pszSSLKey = NULL;
    config.pszSSLCipherList = NULL;
    config.pszDebugLogFile = NULL;
    config.pszDaemonName = VMCA_DAEMON_NAME;
    config.isSecure = TRUE;
    config.useSysLog = TRUE;
    config.debugLogLevel = VMREST_LOG_LEVEL_ERROR;

    //Get Certificate and Key from VECS and Set it to Rest Engine
    dwError = VMCAGetVecsMachineCert(&pszCert, &pszKey);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTInit(&config, &pHTTPSHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(pHTTPSHandle, pszCert, VMCAStringLenA(pszCert)+1, SSL_DATA_TYPE_CERT);
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = VmRESTSetSSLInfo(pHTTPSHandle, pszKey, VMCAStringLenA(pszKey)+1, SSL_DATA_TYPE_KEY);
    BAIL_ON_VMREST_ERROR(dwError);

    endPointCnt = ARRAY_SIZE(restEndPoints);

    for (iter = 0; iter < endPointCnt; iter++)
    {
        dwError = VmRESTRegisterHandler(
                pHTTPSHandle,
                restEndPoints[iter],
                pHandlers,
                NULL);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = VmRESTStart(pHTTPSHandle);
    BAIL_ON_VMREST_ERROR(dwError);

    gpVMCAHTTPSHandle = pHTTPSHandle;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszCert);
    VMCA_SAFE_FREE_MEMORY(pszKey);
    return dwError;

error:
    _VMCARestFreeHandle(pHTTPSHandle);
    VMCA_LOG_ERROR("%s: failure while starting REST HTTPS service, error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

static
VOID
_VMCAHttpServiceShutdown(
    VOID
    )
{
    VMCA_LOG_INFO("%s: starting http rest server shutdown:", __FUNCTION__);

    _VMCARestFreeHandle(gpVMCAHTTPHandle);
    gpVMCAHTTPHandle = NULL;

    VMCA_LOG_INFO("%s: completed http rest server shutdown:", __FUNCTION__);
}

static
VOID
_VMCAHttpsServiceShutdown(
    VOID
    )
{
    VMCA_LOG_INFO("%s: starting https rest server shutdown:", __FUNCTION__);

    _VMCARestFreeHandle(gpVMCAHTTPSHandle);
    gpVMCAHTTPSHandle = NULL;

    VMCA_LOG_INFO("%s: completed https rest server shutdown:", __FUNCTION__);
}

static
VOID
_VMCARestFreeHandle(
    PVMREST_HANDLE    pHandle
    )
{
    DWORD iter = 0;
    DWORD dwError = 0;
    DWORD endPointCnt = 0;

    if (pHandle)
    {
        /*
        * REST library have detached threads, maximum time out specified is the max time
        * allowed for the threads to  finish their execution.
        * If finished early, it will return success.
        * If not able to finish in specified time, failure will be returned
        */
        dwError = VmRESTStop(pHandle, VMCA_REST_STOP_TIMEOUT_SEC);
        BAIL_ON_VMREST_ERROR(dwError);

        endPointCnt = ARRAY_SIZE(restEndPoints);
        for (iter = 0; iter < endPointCnt; iter++)
        {
            (VOID)VmRESTUnRegisterHandler(
                    pHandle,
                    restEndPoints[iter]);
        }
        VmRESTShutdown(pHandle);
    }

cleanup:
    return;

error:
    VMCA_LOG_WARNING("%s: rest server stop error:%d", __FUNCTION__, dwError);
    goto cleanup;
}

#endif
#endif
