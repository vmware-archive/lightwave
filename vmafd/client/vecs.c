/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

VOID
VmAfdFreeCertArray(PVMAFD_CERT_ARRAY pArray)
{
    if (pArray != NULL)
    {
        if (pArray->certificates)
        {
            DWORD dwCount = 0;
            unsigned int uCounter = 0;

            dwCount = pArray->dwCount;
            for(uCounter = 0; uCounter < dwCount; uCounter ++)
            {
                if (pArray->certificates[uCounter].pPrivateKey != NULL)
                {
                    VmAfdRpcClientFreeMemory(
                        pArray->certificates[uCounter].pPrivateKey);
                }
                if (pArray->certificates[uCounter].pCert != NULL)
                {
                    VmAfdRpcClientFreeMemory(
                        pArray->certificates[uCounter].pCert);
                }
                if (pArray->certificates[uCounter].pAlias != NULL)
                {
                    VmAfdRpcClientFreeMemory(
                        pArray->certificates[uCounter].pAlias);
                }
                if (pArray->certificates[uCounter].pPassword != NULL)
                {
                    VmAfdRpcClientFreeMemory(
                        pArray->certificates[uCounter].pPassword);
                }
            }

            VmAfdRpcClientFreeMemory(pArray->certificates);
        }

        VmAfdRpcClientFreeMemory(pArray);
    }
}


DWORD
VmAfdGetSSLCertificate(
    PCSTR pszServerName,
    PSTR *ppszCert,
    PSTR *ppszPrivateKey
)
{
    DWORD dwError = 0;
    PSTR pszCertificate = NULL;
    PSTR pszPrivateKey = NULL;
    CHAR szStoreName[] = SYSTEM_CERT_STORE_NAME;
    CHAR szMachineCertAlias[] = VECS_MACHINE_CERT_ALIAS;
    PVECS_STORE pStore = NULL;

    if (!ppszCert || !ppszPrivateKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsOpenCertStoreA(
                                 pszServerName,
                                 szStoreName,
                                 NULL,
                                 &pStore
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetCertificateByAliasA(
                                         pStore,
                                         szMachineCertAlias,
                                         &pszCertificate
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetKeyByAliasA(
                                 pStore,
                                 szMachineCertAlias,
                                 NULL,
                                 &pszPrivateKey
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszCert = pszCertificate;
    *ppszPrivateKey = pszPrivateKey;

cleanup:

    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }
    return dwError;

error:

    if (ppszCert)
    {
        *ppszCert = NULL;
    }
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszCertificate);
    VMAFD_SAFE_FREE_MEMORY (pszPrivateKey);

    goto cleanup;
}



DWORD
VmAfdSetSSLCertificate(
    PCSTR pszServerName,
    PSTR  pszCert,
    PSTR  pszPrivateKey
)
{

    DWORD dwError = 0;
    CHAR szMachineCertAlias[] = VECS_MACHINE_CERT_ALIAS;
    CHAR szStoreName[] = SYSTEM_CERT_STORE_NAME;
    PVECS_STORE pStore = NULL;
    PVECS_CERT_ENTRY_A pEntry = NULL;

    if(IsNullOrEmptyString(pszCert) ||
            IsNullOrEmptyString(pszPrivateKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsCreateCertStoreA(
                                  pszServerName,
                                  szStoreName,
                                  NULL,
                                  &pStore
                                  );
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = VecsOpenCertStoreA(
                                     pszServerName,
                                     szStoreName,
                                     NULL,
                                     &pStore
                                    );
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetEntryByAliasA(
                                   pStore,
                                   szMachineCertAlias,
                                   ENTRY_INFO_LEVEL_1,
                                   &pEntry
                                  );
    if (!dwError)
    {
        dwError = VecsDeleteEntryA(
                                    pStore,
                                    szMachineCertAlias
                                  );
    }
    else if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsAddEntryA(
                            pStore,
                            CERT_ENTRY_TYPE_PRIVATE_KEY,
                            szMachineCertAlias,
                            pszCert,
                            pszPrivateKey,
                            NULL,
                            1
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    if (pStore)
    {
        VecsCloseCertStore (pStore);
    }
    VecsFreeCertEntryA(pEntry);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdTriggerRootCertsRefresh(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword
    )
{
    handle_t hBinding = NULL;
    DWORD dwError = 0;

    if (VmAfdIsLocalHost(pszServerName))
    {
        dwError = VmAfdLocalTriggerRootCertsRefresh();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleAuthA(
                      pszServerName,
                      NULL,
                      pszUserName,
                      NULL,
                      pszPassword,
                      &hBinding
                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcTriggerRootCertsRefresh(
                          hBinding
                      );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    if (hBinding)
    {
        VmAfdFreeBindingHandle( &hBinding);
    }
    goto cleanup;
}
