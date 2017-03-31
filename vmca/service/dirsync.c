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

static
DWORD
VMCASrvCreateDirSyncParams(
    DWORD dwInterval,
    PVMCA_DIR_SYNC_PARAMS* ppSyncInfo
    );

static
DWORD
VMCASrvGetDirSyncNotifyInterval(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo,
    PDWORD pdwInterval
    );

static
VOID
VMCASrvFreeDirSyncParams(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo
    );

static
PVOID
VMCASrvDirSyncThrFunc(
    PVOID pData
    );

static
DWORD
VMCASrvUpdateRootCerts(
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams,
    PBOOLEAN              pbSynced
    );

DWORD
VMCASrvDirSyncInit(
    VOID
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD pThread = NULL;
    BOOLEAN bLocked = FALSE;
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams = NULL;
    DWORD dwSyncInterval = 5 * 60 * 60; // every 5 minutes

    dwError = VMCASrvCreateDirSyncParams(dwSyncInterval, &pDirSyncParams);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    gVMCAServerGlobals.pDirSyncParams = VMCASrvAcquireDirSyncParams(
                                                        pDirSyncParams);

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    dwError = VMCACreateThread(
                   &VMCASrvDirSyncThrFunc,
                   pDirSyncParams,
                   &pThread);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    gVMCAServerGlobals.pDirSyncThr = VMCAAcquireThread(pThread);

cleanup:

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (pDirSyncParams)
    {
        VMCASrvReleaseDirSyncParams(pDirSyncParams);
    }
    if (pThread)
    {
        VMCAReleaseThread(pThread);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VMCASrvPublishRootCerts(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvNotifyDirSync();
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

PVMCA_DIR_SYNC_PARAMS
VMCASrvAcquireDirSyncParams(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo
    )
{
    if (pSyncInfo)
    {
        InterlockedIncrement(&pSyncInfo->refCount);
    }

    return pSyncInfo;
}

VOID
VMCASrvReleaseDirSyncParams(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo
    )
{
    if (InterlockedDecrement(&pSyncInfo->refCount) == 0)
    {
        VMCASrvFreeDirSyncParams(pSyncInfo);
    }
}

VOID
VMCASrvDirSyncShutdown(
    VOID
    )
{
    BOOLEAN bLocked = FALSE;
    PVMCA_THREAD pThread = NULL;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (gVMCAServerGlobals.pDirSyncThr)
    {
        pThread = gVMCAServerGlobals.pDirSyncThr;
        gVMCAServerGlobals.pDirSyncThr = NULL;
    }

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (pThread)
    {
        VMCAReleaseThread(pThread);
    }

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (gVMCAServerGlobals.pDirSyncParams)
    {
        VMCASrvReleaseDirSyncParams(gVMCAServerGlobals.pDirSyncParams);
        gVMCAServerGlobals.pDirSyncParams = NULL;
    }

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);
}

static
DWORD
VMCASrvCreateDirSyncParams(
    DWORD dwInterval,
    PVMCA_DIR_SYNC_PARAMS* ppSyncInfo
    )
{
    DWORD dwError = 0;
    PVMCA_DIR_SYNC_PARAMS pSyncInfo = NULL;

    dwError = VMCAAllocateMemory(
                    sizeof(VMCA_DIR_SYNC_PARAMS),
                    (PVOID*)&pSyncInfo);
    BAIL_ON_VMCA_ERROR(dwError);

    pSyncInfo->refCount = 1;

    dwError = pthread_mutex_init(&pSyncInfo->mutex, NULL);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pSyncInfo->pMutex = &pSyncInfo->mutex;

    pSyncInfo->dwSyncIntervalSecs = dwInterval;

    *ppSyncInfo = pSyncInfo;

cleanup:

    return dwError;

error:

    *ppSyncInfo = NULL;

    if (pSyncInfo)
    {
        VMCASrvReleaseDirSyncParams(pSyncInfo);
    }

    goto cleanup;
}

static
DWORD
VMCASrvGetDirSyncNotifyInterval(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo,
    PDWORD pdwInterval
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, pSyncInfo->pMutex);

    *pdwInterval = pSyncInfo->dwSyncIntervalSecs;

    VMCA_UNLOCK_MUTEX(bLocked, pSyncInfo->pMutex);

    return dwError;
}

static
VOID
VMCASrvFreeDirSyncParams(
    PVMCA_DIR_SYNC_PARAMS pSyncInfo
    )
{
    if (pSyncInfo->pMutex)
    {
        pthread_mutex_destroy(&pSyncInfo->mutex);
        pSyncInfo->pMutex = NULL;
    }
    VMCAFreeMemory(pSyncInfo);
}

static
PVOID
VMCASrvDirSyncThrFunc(
    PVOID pData
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD_DATA pThrData = (PVMCA_THREAD_DATA)pData;
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams = NULL;
    BOOLEAN bShutdown = FALSE;

    VMCA_LOG_INFO("Directory sync thread starting");

    pDirSyncParams = VMCASrvAcquireDirSyncParams(
                        (PVMCA_DIR_SYNC_PARAMS)pThrData->pData);
    if (!pDirSyncParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    while (!bShutdown)
    {
        BOOLEAN bSynced   = FALSE;

        dwError = VMCACheckThreadShutdown(pThrData, &bShutdown);
        BAIL_ON_VMCA_ERROR(dwError);

        if (!bShutdown)
        {
            dwError = VMCASrvUpdateRootCerts(pDirSyncParams, &bSynced);
            BAIL_ON_VMCA_ERROR(dwError);

        }

        dwError = VMCACheckThreadShutdown(pThrData, &bShutdown);
        BAIL_ON_VMCA_ERROR(dwError);

        if (!bShutdown)
        {
            DWORD dwIntervalSecs = 60; // wait a minute on sync failure

            if (bSynced)
            {
                dwError = VMCASrvGetDirSyncNotifyInterval(
                                pDirSyncParams,
                                &dwIntervalSecs);
                BAIL_ON_VMCA_ERROR(dwError);
            }

            dwError = VMCAWaitNotifyThread(pThrData, dwIntervalSecs);
            if (dwError == ETIMEDOUT)
            {
                dwError = 0;
            }
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

cleanup:

    if (pDirSyncParams)
    {
        VMCASrvReleaseDirSyncParams(pDirSyncParams);
    }

    VMCA_LOG_INFO("Directory sync thread exiting");

    return NULL;

error:

    VMCA_LOG_ERROR("Directory sync thread exiting due to error [%u]", dwError);

    goto cleanup;
}


static
DWORD
VMCASrvUpdateRootCerts(
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams,
    PBOOLEAN              pbSynced
    )
{
    DWORD dwError = 0;
    PVMCA_X509_CA pCA = NULL;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszCAContainerDN = NULL;
    PSTR pszCertificate = NULL;
    PSTR pszCRL = NULL;
    X509_CRL* pCrl = NULL;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    PVMCA_LDAP_CONTEXT pContext = NULL;
    PSTR pszUPN = NULL;

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvGetCA(&pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvGetMachineAccountInfoA(
                &pszAccount,
                &pszDomainName,
                &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                &pszUPN,
                "%s@%s",
                pszAccount,
                pszDomainName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCALdapConnect(
                    "localhost",
                    0, /* use default port */
                    pszUPN,
                    pszPassword,
                    &pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetDSERootAttribute(
                    pContext,
                    "configurationNamingContext",
                    &pszCAContainerDN);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaSrvReGenCRL(
                     &pCrl
                     );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCACRLToPEM(
                            pCrl,
                            &pszCRL
                          );
    BAIL_ON_VMCA_ERROR (dwError);

    dwCount = sk_X509_num(pCA->skCAChain);

    for (; dwIndex <dwCount; dwIndex++)
    {
        X509 *pCert = sk_X509_value(
                                    pCA->skCAChain,
                                    dwIndex
                                   );

        dwError = VMCAUpdatePkiCAAttribute(
                                           pContext,
                                           pszCAContainerDN,
                                           pCert
                                          );
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAUpdateCrlCAAttribute(
                    pContext,
                    pszCAContainerDN,
                    pszCRL
                    );
    BAIL_ON_VMCA_ERROR (dwError);

    *pbSynced = TRUE;

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszUPN);
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszCertificate);
    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszPassword);
    VMCA_SAFE_FREE_STRINGA(pszCRL);

    if (pContext)
    {
        VMCALdapClose(pContext);
    }
    if (pCA)
    {
        VMCAReleaseCA(pCA);
    }

    return dwError;

error:

    *pbSynced = FALSE;

    VMCA_LOG_ERROR("Failed to update root certs due to error [%u]", dwError);

    // TODO : Check specific errors

    dwError = 0;

    goto cleanup;
}

DWORD
VMCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    CHAR         szParamsKeyPath[] = VMDIR_CONFIG_PARAMETER_KEY_PATH;
    CHAR         szRegValName_Acct[] = VMDIR_REG_KEY_DC_ACCOUNT;
    CHAR         szRegValName_Pwd[] = VMDIR_REG_KEY_DC_PASSWORD;
    CHAR         szAfdParamsKeyPath[] = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    CHAR         szRegValDomain_Name[] = VMAFD_REG_KEY_DOMAIN_NAME;
    PVMW_CFG_KEY pParamsKey = NULL;
    PVMW_CFG_KEY pAfdParamsKey = NULL;
    PSTR         pszAccount = NULL;
    PSTR         pszPassword = NULL;
    PSTR         pszDomainName = NULL;

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    &szRegValName_Acct[0],
                    &pszAccount);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    &szRegValName_Pwd[0],
                    &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);


    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szAfdParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pAfdParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pAfdParamsKey,
                    NULL,
                    &szRegValDomain_Name[0],
                    &pszDomainName);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszDomainName = pszDomainName;
    *ppszPassword = pszPassword;
cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pAfdParamsKey)
    {
        VmwConfigCloseKey(pAfdParamsKey);
    }
    if (pRootKey)
    {
        VmwConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmwConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszPassword);

    goto cleanup;
}

DWORD
VMCASrvNotifyDirSync(
    VOID
    )
{
    DWORD dwError = 0;
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams = NULL;
    PVMCA_THREAD pDirSyncThread = NULL;
    BOOLEAN bLocked = FALSE;

    pDirSyncThread = VMCASrvGetDirSvcThread();

    if (pDirSyncThread)
    {
        pDirSyncParams = VMCASrvGetDirSyncParams();

        VMCA_LOCK_MUTEX(bLocked, &pDirSyncParams->mutex);

        pDirSyncParams->bRefresh = TRUE;

        VMCA_UNLOCK_MUTEX(bLocked, &pDirSyncParams->mutex);

        dwError = VMCANotifyThread(pDirSyncThread);
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    if (pDirSyncParams)
    {
        VMCA_UNLOCK_MUTEX(bLocked, &pDirSyncParams->mutex);

        VMCASrvReleaseDirSyncParams(pDirSyncParams);
    }
    if (pDirSyncThread)
    {
        VMCAReleaseThread(pDirSyncThread);
    }

    return dwError;

error:

    goto cleanup;
}
