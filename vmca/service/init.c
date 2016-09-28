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
InitializeDatabase(
    VOID
    );

static
DWORD
InitializeLog(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    );

static
DWORD
InitializeEventLog(
    VOID
    );

// init global/static in single thread mode during server startup
DWORD
VMCAInitialize(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    )
{
    DWORD dwError = 0;

    dwError = VMCACommonInit();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = InitializeLog(FALSE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = InitializeEventLog();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = InitializeDatabase();
    BAIL_ON_VMCA_ERROR(dwError);

    // Don't bail on Error , this just sets up the current state
    dwError = VMCASrvInitCA();

    dwError = VMCASrvDirSyncInit();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARPCInit();
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

VOID
VMCAShutdown(
    VOID
    )
{
    VMCARPCShutdown();
    VMCAServiceShutdown();
    VMCASrvDirSyncShutdown();
    VMCATerminateLogging();
    VMCASrvCleanupGlobalState();
    VMCACommonShutdown();
}

static
DWORD
InitializeDatabase(
    VOID
    )
{
    DWORD dwError = 0 ;
    PSTR pszCertDBPath = NULL;

    dwError = VMCACreateDataDirectory();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCertsDBPath(&pszCertDBPath);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO(
    	"Initializing database: [%s]",
    	VMCA_SAFE_LOG_STRING(pszCertDBPath));

    dwError = VmcaDbInitialize(pszCertDBPath);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_STRINGA(pszCertDBPath);

    return dwError;
}

static
DWORD
InitializeLog(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    )
{
    return VMCAInitLog();
}

static
DWORD
InitializeEventLog(
    VOID
    )
{
    return 0;
}

DWORD
VMCASrvInitCA(
    VOID
    )
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pRootCACert = NULL;
    PVMCA_KEY pPrivateKey = NULL;
    PSTR pszRootCertFile = NULL;
    PSTR pszPrivateKeyFile = NULL;
    PSTR pszPasswordFile = NULL;
    PVMCA_X509_CA pCA = NULL;
    DWORD dwCRLNumberCurrent = 0;
    BOOL bIsHoldingMutex = FALSE;

    dwError = VMCAGetRootCertificateFilePath(&pszRootCertFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetPrivateKeyPath(&pszPrivateKeyFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetPrivateKeyPasswordPath(&pszPasswordFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAReadCertificateChainFromFile(pszRootCertFile,&pRootCACert);
    BAIL_ON_VMCA_ERROR(dwError);

    //
    // TODO : Support Passwords for private key
    //
    dwError =  VMCAReadPrivateKeyFromFilePrivate(
                    pszPrivateKeyFile,
                    NULL,
                    &pPrivateKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAValidateCACertificatePrivate(
                    (LPSTR) pRootCACert,
                    NULL,
                    pPrivateKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACreateCA( pRootCACert, pPrivateKey, NULL, &pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    if (BN_num_bits(pCA->pKey->pkey.rsa->n) < VMCA_MIN_CA_CERT_PRIV_KEY_LENGTH)
    {
        dwError = VMCA_ERROR_INVALID_KEY_LENGTH;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCASrvSetCA(pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    pthread_mutex_lock (&gVMCAServerGlobals.mutexCRL);

    bIsHoldingMutex = TRUE;

    dwError = VmcaDbGetCurrentCRLNumber(&dwCRLNumberCurrent);

    if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        dwError = 0;
        dwCRLNumberCurrent = 0;
    }
    BAIL_ON_VMCA_ERROR (dwError);

    gVMCAServerGlobals.dwCurrentCRLNumber = dwCRLNumberCurrent;

    pthread_mutex_unlock (&gVMCAServerGlobals.mutexCRL);

    bIsHoldingMutex = FALSE;

error:

    if ( pPrivateKey != NULL )
    {
        VMCAFreeKey(pPrivateKey);
    }
    if (pRootCACert != NULL)
    {
        VMCAFreeCertificate(pRootCACert);
    }
    if (bIsHoldingMutex)
    {
        pthread_mutex_unlock(&gVMCAServerGlobals.mutexCRL);
    }
    VMCA_SAFE_FREE_STRINGA(pszRootCertFile);
    VMCA_SAFE_FREE_STRINGA(pszPrivateKeyFile);
    VMCA_SAFE_FREE_STRINGA(pszPasswordFile);

    if (pCA)
    {
        VMCAReleaseCA(pCA);
    }

    return dwError;
}


