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

static
DWORD
VecsGetEntryTypeFromInt (
    UINT32 uEntryType,
    CERT_ENTRY_TYPE *pEntryType
    );

static
DWORD
VecsValidateAddEntryInput (
    CERT_ENTRY_TYPE entryType,
    PCWSTR pwszCertificate,
    PCWSTR pwszPrivateKey
    );

static
VOID
VecsRpcFreeCertStoreArray(
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray
    );

static
DWORD
VecsAllocateCertEntryAFromW (
    PVECS_CERT_ENTRY_W pEntryW,
    PVECS_CERT_ENTRY_A* ppEntry
    );

static
DWORD
VecsAllocateCertEntryArrayAFromW (
      PVECS_CERT_ENTRY_W pEntriesW,
      DWORD dwCount,
      PVECS_CERT_ENTRY_A *ppEntriesA
      );

static
DWORD
VecsAllocateCertArray(
    PVMAFD_CERT_ARRAY pCertArray,
    PVECS_CERT_ENTRY_W* ppCertArray,
    PDWORD            pdwCount
    );

static
DWORD
VecsAllocateStorePermissionsAFromW(
    PVECS_STORE_PERMISSION_W pStorePermissionsW,
    DWORD dwUserCount,
    PVECS_STORE_PERMISSION_A *ppStorePermissionsA
    );

static
VOID
VecsFreeEnumContext(
    PVECS_ENUM_CONTEXT pContext
    );

static
PVECS_STORE
VecsAcquireCertStore(
    PVECS_STORE pStore
    );

static
VOID
VecsReleaseCertStore(
    PVECS_STORE pStore
    );

static
VOID
VecsFreeCertStore(
    PVECS_STORE pStore
    );


/*
 * @brief Creates a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out,optional] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCreateCertStoreA(
        PCSTR pszServerName,
        PCSTR pszStoreName,
        PCSTR pszPassword,
        PVECS_STORE *ppStore
        )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszStoreName = NULL;
    PWSTR pwszPassword = NULL;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (strlen(pszStoreName) > STORE_LABEL_MAX_LENGTH)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsCreateCertStoreW(
                pwszServerName,
                pwszStoreName,
                pwszPassword,
                &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (ppStore)
    {
        *ppStore = pStore;
        pStore = NULL;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }
    return dwError;

error:

    if (ppStore)
    {
        *ppStore = NULL;
    }

    goto cleanup;
}

DWORD
VecsCreateCertStoreHA(
        PVMAFD_SERVER pServer,
        PCSTR pszStoreName,
        PCSTR pszPassword,
        PVECS_STORE *ppStore
        )
{
    DWORD dwError = 0;
    PWSTR pwszStoreName = NULL;
    PWSTR pwszPassword = NULL;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszStoreName) || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (strlen(pszStoreName) > STORE_LABEL_MAX_LENGTH)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsCreateCertStoreHW(
                pServer,
                pwszStoreName,
                pwszPassword,
                &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (ppStore)
    {
        *ppStore = pStore;
        pStore = NULL;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }
    return dwError;

error:

    if (ppStore)
    {
        *ppStore = NULL;
    }

    goto cleanup;
}

/*
 * @brief Creates a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out,optional] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCreateCertStoreW(
    PCWSTR pszServerName,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    vecs_store_handle_t pStoreHandle = NULL;
    handle_t hBinding = NULL;
    PWSTR pszServerEndpoint = NULL;
    size_t storeNameLength = 0;
    PVM_AFD_CONNECTION pConnection = NULL;

    if (!pszStoreName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW(pszStoreName, &storeNameLength);
    BAIL_ON_VMAFD_ERROR (dwError);

    if (storeNameLength > STORE_LABEL_MAX_LENGTH)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszServerName) ||
        VmAfdIsLocalHostW(pszServerName)
       )
    {
        dwError = VmAfdOpenClientConnection (&pConnection);
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsLocalCreateCertStoreW (
                                pConnection,
                                pszStoreName,
                                pszPassword,
                                (PBYTE *)&pStoreHandle
                                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    else
    {

        dwError = VmAfdCreateBindingHandleW(
                  pszServerName,
                  pszServerEndpoint,
                  &hBinding
              );
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VecsRpcCreateCertStore(
                      hBinding,
                      (PWSTR)pszStoreName,
                      (PWSTR)pszPassword,
                       &pStoreHandle
                      );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(sizeof(VECS_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;

    pStore->hBinding = hBinding;
    pStore->bOwnBinding = TRUE;

    pStore->pStoreHandle = pStoreHandle;
    pStore->pConnection = pConnection;
    pConnection = NULL;

    if (ppStore)
    {
        *ppStore = pStore;
        pStore = NULL;
    }

cleanup:
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }

    return dwError;

error:
    if (ppStore)
    {
        *ppStore = NULL;
    }
    if (pStoreHandle)
    {
        VecsRpcCloseCertStore(hBinding, &pStoreHandle);
    }
    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }
    if (pConnection)
    {
        VmAfdFreeClientConnection (pConnection);
    }
    goto cleanup;
}

DWORD
VecsCreateCertStoreHW(
    PVMAFD_SERVER pServer,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    vecs_store_handle_t pStoreHandle = NULL;
    size_t storeNameLength = 0;
    PVM_AFD_CONNECTION pConnection = NULL;

    if (!pszStoreName || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW(pszStoreName, &storeNameLength);
    BAIL_ON_VMAFD_ERROR (dwError);

    if (storeNameLength > STORE_LABEL_MAX_LENGTH)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = VmAfdOpenClientConnection ( &pConnection);
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsLocalCreateCertStoreW (
                        pConnection,
                        pszStoreName,
                        pszPassword,
                        (PBYTE *)&pStoreHandle
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcCreateCertStore(
                      pServer->hBinding,
                      (PWSTR)pszStoreName,
                      (PWSTR)pszPassword,
                       &pStoreHandle
                      );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(sizeof(VECS_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;

    pStore->hBinding = pServer->hBinding;
    pStore->bOwnBinding = FALSE;

    pStore->pServer = VmAfdAcquireServer(pServer);

    pStore->pStoreHandle = pStoreHandle;
    pStore->pConnection = pConnection;
    pConnection = NULL;

    if (ppStore)
    {
        *ppStore = pStore;
        pStore = NULL;
    }

cleanup:
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }

    return dwError;

error:
    if (ppStore)
    {
        *ppStore = NULL;
    }
    if (pStoreHandle && pServer && pServer->hBinding)
    {
        VecsRpcCloseCertStore(pServer->hBinding, &pStoreHandle);
    }
    if (pConnection)
    {
        VmAfdFreeClientConnection (pConnection);
    }

    goto cleanup;
}

DWORD
VecsOpenCertStoreHA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PWSTR pwszStoreName = NULL;
    PWSTR pwszPassword = NULL;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszStoreName) || !ppStore || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsOpenCertStoreHW(
                  pServer,
                  pwszStoreName,
                  pwszPassword,
                  &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppStore = pStore;


cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);
    VMAFD_SAFE_FREE_MEMORY (pwszPassword);

    return dwError;

error:
    if (ppStore){
        *ppStore = NULL;
    }
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }

    goto cleanup;
}

DWORD
VecsOpenCertStoreHW(
    PVMAFD_SERVER pServer,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    vecs_store_handle_t pStoreHandle = NULL;
    PVM_AFD_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pszStoreName) || !ppStore || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = VmAfdOpenClientConnection ( &pConnection);
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsLocalOpenCertStoreW (
                                            pConnection,
                                            pszStoreName,
                                            pszPassword,
                                            (PBYTE *)&pStoreHandle
                                          );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcOpenCertStore(
                      pServer->hBinding,
                      (PWSTR)pszStoreName,
                      (PWSTR)pszPassword,
                      &pStoreHandle
                      );
        }

        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(sizeof(VECS_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;

    pStore->hBinding = pServer->hBinding;
    pStore->bOwnBinding = FALSE;

    pStore->pConnection = pConnection;
    pConnection = NULL;

    pStore->pServer = VmAfdAcquireServer(pServer);

    pStore->pStoreHandle = pStoreHandle;

    *ppStore = pStore;

cleanup:

    return dwError;

error:
    if (ppStore)
    {
        *ppStore = NULL;
    }
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }
    if (pStoreHandle && pServer && pServer->hBinding)
    {
        DWORD dwError2 = 0;

        DCETHREAD_TRY
        {
            dwError2 = VecsRpcCloseCertStore(pServer->hBinding, &pStoreHandle);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError2 = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    if (pConnection)
    {
        VmAfdFreeClientConnection (pConnection);
    }
    goto cleanup;
}

/*
 * @brief Opens a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsOpenCertStoreA(
    PCSTR pszServerName,
    PCSTR pszStoreName,
    PCSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszStoreName = NULL;
    PWSTR pwszPassword = NULL;
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszStoreName) || !ppStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsOpenCertStoreW(
                  pwszServerName,
                  pwszStoreName,
                  pwszPassword,
                  &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppStore = pStore;


cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);
    VMAFD_SAFE_FREE_MEMORY (pwszPassword);

    return dwError;

error:
    if (ppStore){
        *ppStore = NULL;
    }
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }

    goto cleanup;
}

/*
 * @brief Opens a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the store
 * @param[out] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsOpenCertStoreW(
    PCWSTR pszServerName,
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVECS_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    handle_t hBinding = NULL;
    vecs_store_handle_t pStoreHandle = NULL;
    PWSTR pszServerEndpoint = NULL;
    PVM_AFD_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pszStoreName) || !ppStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString (pszServerName) ||
        VmAfdIsLocalHostW(pszServerName)
       )
    {
        dwError = VmAfdOpenClientConnection (&pConnection);
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsLocalOpenCertStoreW (
                            pConnection,
                            pszStoreName,
                            pszPassword,
                            (PBYTE *)&pStoreHandle
                            );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    else
    {
        dwError = VmAfdCreateBindingHandleW(
                  pszServerName,
                  pszServerEndpoint,
                  &hBinding
              );
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VecsRpcOpenCertStore(
                      hBinding,
                      (PWSTR)pszStoreName,
                      (PWSTR)pszPassword,
                      &pStoreHandle
                      );
        }

        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(sizeof(VECS_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;

    pStore->hBinding = hBinding;
    pStore->bOwnBinding = TRUE;

    pStore->pStoreHandle = pStoreHandle;

    pStore->pConnection = pConnection;
    pConnection = NULL;

    *ppStore = pStore;

cleanup:

    return dwError;

error:
    if (ppStore)
    {
        *ppStore = NULL;
    }
    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }
    if (pStoreHandle)
    {
        DWORD dwError2 = 0;

        DCETHREAD_TRY
        {
            dwError2 = VecsRpcCloseCertStore(hBinding, &pStoreHandle);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError2 = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }
    if (pConnection)
    {
        VmAfdFreeClientConnection (pConnection);
    }

    goto cleanup;
}

/**
 * @brief Enumerates certificate store names
 *
 * @param[in]     pszServerName      Host Server
 * @param[out]    ppszStoreNameArray Names of certificate stores
 * @param[in,out] pdwCount            Number of store names returned
 *
 * @return 0 on success
 */
DWORD
VecsEnumCertStoreA(
    PCSTR pszServerName,
    PSTR** ppszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR *pwszStoreArray = NULL;
    PSTR * pszStoreNameArray = NULL;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;

    if (!ppszStoreNameArray || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA (
                        pszServerName,
                        &pwszServerName
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    dwError = VecsEnumCertStoreW (
                  pwszServerName,
                  &pwszStoreArray,
                  &dwCount
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (dwCount)
    {
      dwError = VmAfdAllocateMemory (
                  sizeof (PSTR) * dwCount,
                  (PVOID *)&pszStoreNameArray
                  );
      BAIL_ON_VMAFD_ERROR (dwError);

      for (; dwIndex <dwCount; dwIndex++)
      {
          dwError = VmAfdAllocateStringAFromW (
                          pwszStoreArray[dwIndex],
                          &(pszStoreNameArray[dwIndex])
                          );
          BAIL_ON_VMAFD_ERROR (dwError);
      }
    }

    *ppszStoreNameArray = pszStoreNameArray;
    *pdwCount = dwCount;
cleanup:
    if (pwszStoreArray)
    {
        VmAfdFreeStringArrayW(pwszStoreArray, dwCount);
    }
    VMAFD_SAFE_FREE_MEMORY (pwszServerName);

    return dwError;
error:
    if (ppszStoreNameArray)
    {
        *ppszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pszStoreNameArray)
    {
    	VmAfdFreeStringArrayCountA(pszStoreNameArray, dwCount);
    }

    goto cleanup;
}

DWORD
VecsEnumCertStoreHA(
    PVMAFD_SERVER pServer,
    PSTR** ppszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    PWSTR *pwszStoreArray = NULL;
    PSTR * pszStoreNameArray = NULL;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;

    if (!ppszStoreNameArray || !pdwCount || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsEnumCertStoreHW (
                  pServer,
                  &pwszStoreArray,
                  &dwCount
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (dwCount)
    {
      dwError = VmAfdAllocateMemory (
                  sizeof (PSTR) * dwCount,
                  (PVOID *)&pszStoreNameArray
                  );
      BAIL_ON_VMAFD_ERROR (dwError);

      for (; dwIndex <dwCount; dwIndex++)
      {
          dwError = VmAfdAllocateStringAFromW (
                          pwszStoreArray[dwIndex],
                          &(pszStoreNameArray[dwIndex])
                          );
          BAIL_ON_VMAFD_ERROR (dwError);
      }
    }

    *ppszStoreNameArray = pszStoreNameArray;
    *pdwCount = dwCount;
cleanup:
    if (pwszStoreArray)
    {
        VmAfdFreeStringArrayW(pwszStoreArray, dwCount);
    }

    return dwError;
error:
    if (ppszStoreNameArray)
    {
        *ppszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pszStoreNameArray)
    {
        VmAfdFreeStringArrayCountA(pszStoreNameArray, dwCount);
    }

    goto cleanup;
}

/**
 * @brief Enumerates certificate store names
 *
 * @param[in]     ServerName          Host Server
 * @param[out]    ppwszStoreNameArray Names of certificate stores
 * @param[in,out] pdwCount            Number of store names returned
 *
 * @return 0 on success
 */
DWORD
VecsEnumCertStoreW(
    PCWSTR pwszServerName,
    PWSTR** pppwszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    handle_t hBinding = NULL;
    PWSTR pwszServerEndPoint = NULL;
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray = NULL;
    PWSTR* ppwszStoreNameArray = NULL;

    if (!pppwszStoreNameArray || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pwszServerName) ||
        VmAfdIsLocalHostW(pwszServerName)
       )
    {
        dwError = VecsLocalEnumCertStoreW(
                                          &ppwszStoreNameArray,
                                          &dwCount
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndPoint,
                        &hBinding
                        );
        BAIL_ON_VMAFD_ERROR (dwError);


        DCETHREAD_TRY
        {
            dwError = VecsRpcEnumCertStore(hBinding, &pCertStoreArray);
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR (dwError);

        if (pCertStoreArray->dwCount)
        {
            DWORD idx = 0;

            dwError = VmAfdAllocateMemory(
                        sizeof(PWSTR) * pCertStoreArray->dwCount,
                        (PVOID*) & ppwszStoreNameArray);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwCount = pCertStoreArray->dwCount;

            for (; idx < dwCount; idx++)
            {
                dwError = VmAfdAllocateStringW(
                            pCertStoreArray->ppwszStoreNames[idx],
                            &ppwszStoreNameArray[idx]);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *pppwszStoreNameArray = ppwszStoreNameArray;
    *pdwCount = dwCount;

cleanup:
    if (pCertStoreArray)
    {
        VecsRpcFreeCertStoreArray(pCertStoreArray);
    }
    if (hBinding)
    {
        VmAfdFreeBindingHandle (&hBinding);
    }

    return dwError;
error:
    if (pppwszStoreNameArray)
    {
        *pppwszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppwszStoreNameArray)
    {
        VmAfdFreeStringArrayW (ppwszStoreNameArray, dwCount);
    }

    goto cleanup;
}

DWORD
VecsEnumCertStoreHW(
    PVMAFD_SERVER pServer,
    PWSTR** pppwszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray = NULL;
    PWSTR* ppwszStoreNameArray = NULL;

    if (!pppwszStoreNameArray || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = VecsLocalEnumCertStoreW(
                      &ppwszStoreNameArray,
                      &dwCount);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcEnumCertStore(pServer->hBinding, &pCertStoreArray);
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR (dwError);

        if (pCertStoreArray->dwCount)
        {
            DWORD idx = 0;

            dwError = VmAfdAllocateMemory(
                        sizeof(PWSTR) * pCertStoreArray->dwCount,
                        (PVOID*) & ppwszStoreNameArray);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwCount = pCertStoreArray->dwCount;

            for (; idx < dwCount; idx++)
            {
                dwError = VmAfdAllocateStringW(
                            pCertStoreArray->ppwszStoreNames[idx],
                            &ppwszStoreNameArray[idx]);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *pppwszStoreNameArray = ppwszStoreNameArray;
    *pdwCount = dwCount;

cleanup:
    if (pCertStoreArray)
    {
        VecsRpcFreeCertStoreArray(pCertStoreArray);
    }

    return dwError;
error:
    if (pppwszStoreNameArray)
    {
        *pppwszStoreNameArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppwszStoreNameArray)
    {
        VmAfdFreeStringArrayW (ppwszStoreNameArray, dwCount);
    }

    goto cleanup;
}

/*
 * @brief Adds a certificate to the store.
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] entryType Type of entry
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszCertificate Certificate Content
 * @param[in,optional] pszPrivateKey Private Key
 * @param[in] bAutoRefresh Whether to automatically renew the certificate
 *
 * @return Returns 0 for success
 */
DWORD
VecsAddEntryA(
    PVECS_STORE pStore,
    CERT_ENTRY_TYPE entryType,
    PCSTR pszAlias,
    PCSTR pszCertificate,
    PCSTR pszPrivateKey,
    PCSTR pszPassword,
    BOOLEAN bAutoRefresh
    )
{
    DWORD dwError = 0;
    PWSTR pwszAlias = NULL;
    PWSTR pwszCertificate = NULL;
    PWSTR pwszPrivateKey = NULL;
    PWSTR pwszPassword = NULL;

    if (!pStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszPrivateKey)
    {
        dwError = VmAfdAllocateStringWFromA(pszPrivateKey, &pwszPrivateKey);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszAlias)
    {
        dwError = VmAfdAllocateStringWFromA(pszAlias, &pwszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszCertificate)
    {
       dwError = VmAfdAllocateStringWFromA (pszCertificate, &pwszCertificate);
       BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsAddEntryW(
                pStore,
                entryType,
                pwszAlias,
                pwszCertificate,
                pwszPrivateKey,
                pwszPassword,
                bAutoRefresh);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAlias);
    VMAFD_SAFE_FREE_MEMORY(pwszCertificate);
    VMAFD_SAFE_FREE_MEMORY(pwszPrivateKey);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    goto cleanup;
}

/*
 * @brief Adds a certificate to the store.
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] entryType Type of entry
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszCertificate Certificate Content
 * @param[in,optional] pszPrivateKey Private Key
 * @param[in] bAutoRefresh Whether to automatically renew the certificate
 *
 * @return Returns 0 for success
 */
DWORD
VecsAddEntryW(
    PVECS_STORE pStore,
    CERT_ENTRY_TYPE entryType,
    PCWSTR pszAlias,
    PCWSTR pszCertificate,
    PCWSTR pszPrivateKey,
    PCWSTR pszPassword,
    BOOLEAN bAutoRefresh
    )
{
    DWORD dwError = 0;
    PWSTR pszCanonicalCertPEM = NULL;
    PWSTR pszCanonicalKeyPEM = NULL;

    // Note: We intentionally keeps AutoRefresh off for client
    // VecsAddEntryA/W calls because certs added via them should
    // have it off. Otherwise, the certs will be purged by lotus
    // sync logic. This means we are intentionally ignoring the
    // bAutoRefresh parameter passed in.
    bAutoRefresh = 0;

    if (!pStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsValidateAddEntryInput (
                    entryType,
                    pszCertificate,
                    pszPrivateKey
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (
        entryType == CERT_ENTRY_TYPE_SECRET_KEY &&
        IsNullOrEmptyString (pszAlias)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsNullOrEmptyString(pszCertificate))
    {
        if (entryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST)
        {
            dwError = VecsValidateAndFormatCrl(
                                          pszCertificate,
                                          &pszCanonicalCertPEM
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);
        } else
        {
            dwError = VecsValidateAndFormatCert(
                                          pszCertificate,
                                          &pszCanonicalCertPEM
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);
        }
    }

    if (!IsNullOrEmptyString (pszPrivateKey))
    {
       if (entryType == CERT_ENTRY_TYPE_SECRET_KEY)
       {
          dwError = VmAfdAllocateStringW(
                                      pszPrivateKey,
                                      &pszCanonicalKeyPEM
                                      );
          BAIL_ON_VMAFD_ERROR (dwError);
       }
       else
       {
          dwError = VecsValidateAndFormatKey (
                                    pszPrivateKey,
                                    NULL,
                                    &pszCanonicalKeyPEM
                                  );
          BAIL_ON_VMAFD_ERROR (dwError);
       }
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalAddEntryW(
                                     pStore,
                                     entryType,
                                     pszAlias,
                                     pszCanonicalCertPEM,
                                     pszCanonicalKeyPEM,
                                     pszPassword,
                                     bAutoRefresh
                                    );
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcAddCertificate(
                    pStore->hBinding,
                    pStore->pStoreHandle,
                    entryType,
                    (PWSTR)pszAlias,
                    (PWSTR)pszCanonicalCertPEM,
                    (PWSTR)pszCanonicalKeyPEM,
                    (PWSTR)pszPassword,
                    bAutoRefresh
                    );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR (dwError)

cleanup:

    VMAFD_SAFE_FREE_MEMORY (pszCanonicalCertPEM);
    VMAFD_SAFE_FREE_MEMORY (pszCanonicalKeyPEM);

    return dwError;

error:

    goto cleanup;
}

/*
 * @brief Gets type of an entry in the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pType Type of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryTypeByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    CERT_ENTRY_TYPE *pType
    )
{
    DWORD dwError = 0;
    PWSTR pwszAlias = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pStore || !pType || IsNullOrEmptyString(pszAlias))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszAlias, &pwszAlias);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetEntryTypeByAliasW(
                    pStore,
                    pwszAlias,
                    &entryType);
    BAIL_ON_VMAFD_ERROR (dwError);

    *pType = entryType;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszAlias);

    return dwError;
error:
    if (pType)
    {
        *pType = CERT_ENTRY_TYPE_UNKNOWN;
    }
    goto cleanup;
}

/*
 * @brief Gets type of an entry in the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 * @param[out] pType Type of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryTypeByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    CERT_ENTRY_TYPE *pType
    )
{
    DWORD dwError = 0;
    UINT32 entryType = 0;
    CERT_ENTRY_TYPE cEntryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pStore || !pType || IsNullOrEmptyString (pwszAlias))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetEntryTypeByAliasW(
                                                pStore,
                                                pwszAlias,
                                                &cEntryType
                                               );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        DCETHREAD_TRY
        {
          dwError = VecsRpcGetEntryTypeByAlias(
                    pStore->hBinding,
                    pStore->pStoreHandle,
                    (PWSTR)pwszAlias,
                    &entryType
                    );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsGetEntryTypeFromInt(entryType, &cEntryType);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *pType = cEntryType;

cleanup:
    return dwError;

error:
    if (pType)
    {
        *pType = CERT_ENTRY_TYPE_UNKNOWN;
    }

    goto cleanup;
}

/*
 * @brief Gets date of an entry in the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pDate Date of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryDateByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PDWORD pdwDate
    )
{
    DWORD dwError = 0;
    PWSTR pwszAlias = NULL;
    DWORD dwDate = 0;

    if (IsNullOrEmptyString (pszAlias) ||
        !pStore ||
        !pdwDate
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA (
                    pszAlias,
                    &pwszAlias
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetEntryDateByAliasW(
                pStore,
                pwszAlias,
                &dwDate
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDate = dwDate;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszAlias);

    return dwError;
error:
    if (pdwDate)
    {
        *pdwDate = 0;
    }

    goto cleanup;
}

/*
 * @brief Gets date of an entry in the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 * @param[out] pDate Date of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryDateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias,
    PDWORD pdwDate
    )
{
    DWORD dwError = 0;
    DWORD dwDate = 0;

    if (IsNullOrEmptyString (pwszAlias) ||
        !pStore ||
        !pdwDate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetEntryDateByAliasW(
                                                pStore,
                                                pwszAlias,
                                                &dwDate
                                               );
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcGetEntryDateByAlias (
                  pStore->hBinding,
                  pStore->pStoreHandle,
                  (PWSTR) pwszAlias,
                  &dwDate
                  );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDate = dwDate;

cleanup:

    return dwError;
error:
    if (pdwDate)
    {
        *pdwDate = 0;
    }

    goto cleanup;
}

/*
 * @brief Gets an entry from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] infoLevel Level of info requied
 * @param[out] pEntry Entry Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_CERT_ENTRY_A *ppEntry
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pwEntry = NULL;
    PVECS_CERT_ENTRY_A pEntry = NULL;
    PWSTR pwszAlias = NULL;

    if (!pStore ||
        IsNullOrEmptyString (pszAlias) ||
        !ppEntry
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA (
                            pszAlias,
                            &pwszAlias
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetEntryByAliasW(
                          pStore,
                          pwszAlias,
                          infoLevel,
                          &pwEntry
                          );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwError = VecsAllocateCertEntryAFromW(
                            pwEntry,
                            &pEntry
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppEntry = pEntry;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszAlias);
    if (pwEntry)
    {
        VecsFreeCertEntryW(pwEntry);
    }

    return dwError;

error:
    if (ppEntry)
    {
        *ppEntry = NULL;
    }
    if (pEntry)
    {
        VecsFreeCertEntryA(pEntry);
    }

    goto cleanup;
}

/*
 * @brief Gets an entry from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] infoLevel Level of info requied
 * @param[out] pEntry Entry Content
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetEntryByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_CERT_ENTRY_W *ppEntry
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertArray = NULL;
    PVECS_CERT_ENTRY_W pEntry = {0};
    DWORD dwCount = 0;

    if (!pStore ||
        IsNullOrEmptyString (pszAlias) ||
        !ppEntry
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetEntryByAliasW(
                                            pStore,
                                            pszAlias,
                                            infoLevel,
                                            &pEntry
                                           );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    else
    {

        DCETHREAD_TRY
        {
            dwError = VecsRpcGetEntryByAlias(
                        pStore->hBinding,
                        pStore->pStoreHandle,
                        (PWSTR) pszAlias,
                        infoLevel,
                        &pCertArray
                        );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);

        if (!(pCertArray->dwCount))
        {
            dwError = ERROR_OBJECT_NOT_FOUND;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VecsAllocateCertArray(pCertArray, &pEntry, &dwCount);
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppEntry = pEntry;
cleanup:
    if (pCertArray)
    {
        VmAfdFreeCertArray (pCertArray);
    }

    return dwError;
error:
    if (ppEntry)
    {
        *ppEntry = NULL;
    }
    if (pEntry)
    {
        VecsFreeCertEntryW(pEntry);
    }


    goto cleanup;
}

/*
 * @brief Gets a certificate from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pszCertificate Certificate Content
 * @param[out,optional] pszPrivateKey Private Key
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetCertificateByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PSTR *ppszCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pwszAlias = NULL;
    PWSTR pwszCertificate = NULL;

    if (IsNullOrEmptyString(pszAlias) ||
        !ppszCertificate ||
        !pStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszAlias, &pwszAlias);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetCertificateByAliasW(
                  pStore,
                  pwszAlias,
                  &pwszCertificate
                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCertificate, ppszCertificate);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAlias);
    VMAFD_SAFE_FREE_MEMORY(pwszCertificate);

    return dwError;

error:
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }

    goto cleanup;
}

/*
 * @brief Gets a certificate from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[out] pszCertificate Certificate Content
 * @param[out,optional] pszPrivateKey Private Key
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetCertificateByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PWSTR *ppszCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pszCertificate = NULL;

    if (IsNullOrEmptyString(pszAlias) ||
        !pStore ||
        !ppszCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetCertificateByAliasW(
                                                  pStore,
                                                  pszAlias,
                                                  &pszCertificate
                                                 );
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcGetCertificateByAlias(
                      pStore->hBinding,
                      pStore->pStoreHandle,
                      (PWSTR) pszAlias,
                      &pszCertificate
                      );
        }

        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszCertificate)
    {
        dwError = VmAfdAllocateStringW(pszCertificate, ppszCertificate);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pszCertificate);

    return dwError;

error:
    if (ppszCertificate)
    {
      *ppszCertificate = NULL;
    }

    goto cleanup;
}


/*
 * @brief Gets a key from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszPassword Password
 * @param[out] pszPrivateKey PrivateKey entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetKeyByAliasA(
    PVECS_STORE pStore,
    PCSTR pszAlias,
    PCSTR pszPassword,
    PSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    PWSTR pwszAlias = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszPrivateKey = NULL;

    if (IsNullOrEmptyString(pszAlias) ||
        !pStore ||
        !ppszPrivateKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR_IF(dwError);
    }

    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA (pszAlias, &pwszAlias);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsGetKeyByAliasW(
                pStore,
                pwszAlias,
                pwszPassword,
                &pwszPrivateKey
                );
   BAIL_ON_VMAFD_ERROR(dwError);

   if (pwszPrivateKey)
   {
       dwError = VmAfdAllocateStringAFromW(pwszPrivateKey, ppszPrivateKey);
       BAIL_ON_VMAFD_ERROR(dwError);
   }


cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszAlias);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszPrivateKey);

    return dwError;
error:
    goto cleanup;
}

/*
 * @brief Gets a certificate from the store by alias
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 * @param[in] pszPassword Password
 * @param[out] pszPrivateKey PrivateKey entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsGetKeyByAliasW(
    PVECS_STORE pStore,
    PCWSTR pszAlias,
    PCWSTR pszPassword,
    PWSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    PWSTR pszPrivateKey = NULL;

    if (!pStore ||
        IsNullOrEmptyString(pszAlias) ||
        !ppszPrivateKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetKeyByAliasW (
                                           pStore,
                                           pszAlias,
                                           pszPassword,
                                           &pszPrivateKey
                                          );
    }

    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcGetPrivateKeyByAlias(
                      pStore->hBinding,
                      pStore->pStoreHandle,
                      (PWSTR) pszAlias,
                      (PWSTR) pszPassword,
                      &pszPrivateKey
                      );
        }

        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(pszPrivateKey, ppszPrivateKey);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszPrivateKey);

    return dwError;
error:
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }

    goto cleanup;
}

/*
 * @brief Returns number of entries in Store
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in,out] pdwSize Number of entries in Store
 *
 * @return Returns 0 for success
 */

DWORD
VecsGetEntryCount(
    PVECS_STORE pStore,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    if (!pStore || !pdwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetEntryCount(
                                         pStore,
                                         &dwSize
                                        );
    }
    else
    {
        DCETHREAD_TRY
        {
          dwError = VecsRpcGetEntryCount(
                    pStore->hBinding,
                    pStore->pStoreHandle,
                    &dwSize
                    );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR(dwError);
    *pdwSize = dwSize;

cleanup:

    return dwError;
error:
    if (pdwSize)
    {
      *pdwSize = 0;
    }

    goto cleanup;
}

/*
 * @brief Creates an enumeration handle
 *
 * @param[in] pStore Handle to the Certificate store
 * @param[in] dwEntryCount maximum number of entries returned
 * @param[out] ppEnumContext Enumeration Handle
 *
 * @return Returns 0 for success
 */
DWORD
VecsBeginEnumEntries(
    PVECS_STORE pStore,
    DWORD       dwEntryCount,
    ENTRY_INFO_LEVEL infoLevel,
    PVECS_ENUM_CONTEXT *ppEnumContext
    )
{
    DWORD dwError = 0;
    DWORD dwLimit = 0;
    vecs_entry_enum_handle_t pEnumHandle = NULL;
    PVECS_ENUM_CONTEXT pContext = NULL;

    if (!pStore || !ppEnumContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalBeginEnumEntries(
                                            pStore,
                                            dwEntryCount,
                                            infoLevel,
                                            (PBYTE *) &pEnumHandle,
                                            &dwLimit
                                           );
    }

    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcBeginEnumCerts(
                    pStore->hBinding,
                    pStore->pStoreHandle,
                    dwEntryCount,
                    infoLevel,
                    &dwLimit,
                    &pEnumHandle);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(sizeof(VECS_ENUM_CONTEXT), (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->pStore = VecsAcquireCertStore(pStore);
    pContext->pEnumHandle = pEnumHandle;
    pContext->dwLimit = dwLimit;

    *ppEnumContext = pContext;

cleanup:

    return dwError;

error:

    if (ppEnumContext)
    {
        *ppEnumContext = NULL;
    }
    if (pContext)
    {
        VecsFreeEnumContext(pContext);
    }
    if (pEnumHandle)
    {
        DWORD dwError2 = 0;

        DCETHREAD_TRY
        {
            dwError2 = VecsRpcEndEnumCerts(pStore->hBinding, &pEnumHandle);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError2 = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    goto cleanup;
}

/*
 * @brief Enumerates entries in a store
 *
 * @param[in] pEnumContext Enumeration Handle
 * @param[out] ppEntries Array of Entries
 * @param[in,out] pdwEntryCount Count of entries
 *
 * @return Returns 0 for success
 */
DWORD
VecsEnumEntriesA(
    PVECS_ENUM_CONTEXT pEnumContext,
    PVECS_CERT_ENTRY_A *ppEntries,
    PDWORD pdwEntryCount
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W ppwEntries = NULL;
    PVECS_CERT_ENTRY_A ppaEntries = NULL;
    DWORD dwEntryCount = 0;

    if (!ppEntries ||
        !pEnumContext ||
        !pdwEntryCount)
    {
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsEnumEntriesW(
                  pEnumContext,
                  &ppwEntries,
                  &dwEntryCount
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsAllocateCertEntryArrayAFromW(
                        ppwEntries,
                        dwEntryCount,
                        &ppaEntries
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppEntries = ppaEntries;
    *pdwEntryCount = dwEntryCount;

cleanup:
    if (ppwEntries)
    {
      VecsFreeCertEntryArrayW(ppwEntries, dwEntryCount);
    }
    return dwError;
error:
    if (ppEntries)
    {
        *ppEntries = NULL;
    }
    if (pdwEntryCount)
    {
        *pdwEntryCount = 0;
    }
    if (ppaEntries)
    {
        VecsFreeCertEntryArrayA(ppaEntries, dwEntryCount);
    }

    goto cleanup;
}

/*
 * @brief Enumerates Entries in a store
 *
 * @param[in] pEnumContext Enumeration Handle
 * @param[out] ppEntries Array of  Entries
 * @param[in,out] pdwEntryCount Count of entries
 *
 * @return Returns 0 for success
 */
DWORD
VecsEnumEntriesW(
    PVECS_ENUM_CONTEXT pEnumContext,
    PVECS_CERT_ENTRY_W *ppEntries,
    PDWORD pdwEntryCount
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pEntries = NULL;
    PVMAFD_CERT_ARRAY pCertArray_rpc = NULL;
    DWORD dwCount = 0;

    if (!pEnumContext || !ppEntries || !pdwEntryCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pEnumContext->pStore->hBinding)
    {
        dwError = VecsLocalEnumEntriesW (
                                          pEnumContext,
                                          &pCertArray_rpc
                                        );
    }

    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcEnumCerts(
                        pEnumContext->pStore->hBinding,
                        pEnumContext->pEnumHandle,
                        &pCertArray_rpc);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pCertArray_rpc->dwCount == 0)
    {
        dwError = ERROR_NO_MORE_ITEMS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsAllocateCertArray(pCertArray_rpc, &pEntries, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppEntries = pEntries;
    *pdwEntryCount = dwCount;

cleanup:

    if (pCertArray_rpc)
    {
        VmAfdFreeCertArray(pCertArray_rpc);
    }

    return dwError;

error:
    if (ppEntries)
    {
        *ppEntries = NULL;
    }
    if (pEntries)
    {
        VecsFreeCertEntryArrayW(pEntries,dwCount);
    }

     goto cleanup;
}

/*
 * @brief Closes an enumeration handle
 *
 * @param[in] pEnumContext Enumeration Handle
 *
 * @return Returns 0 for success
 */
DWORD
VecsEndEnumEntries(
    PVECS_ENUM_CONTEXT pEnumContext
    )
{
    DWORD dwError = 0;

    if (!pEnumContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VecsFreeEnumContext(pEnumContext);

error :

     return dwError;
}

/*
 * @brief Deletes a certificate from the store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pszAlias Alias of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteEntryA(
    PVECS_STORE pStore,
    PCSTR pszAlias
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszAlias = NULL;

    if (IsNullOrEmptyString(pszAlias) || pStore == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszAlias, &pwszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDeleteEntryW(pStore, pwszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszAlias);

     return dwError;

error:

     goto cleanup;
}

/*
 * @brief Deletes a certificate from the store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pStore Handle to the Certificate store
 * @param[in] pwszAlias Alias of the entry
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteEntryW(
    PVECS_STORE pStore,
    PCWSTR pwszAlias
    )
{
    DWORD dwError = 0;

    if (!pStore || IsNullOrEmptyString(pwszAlias))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalDeleteEntryW(
                                        pStore,
                                        pwszAlias
                                       );
    }

    else
    {
        DCETHREAD_TRY
        {
            dwError = VecsRpcDeleteCertificate(
                  pStore->hBinding,
                  pStore->pStoreHandle,
                  (PWSTR)pwszAlias
                  );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsSetPermissionA(
    PVECS_STORE pStore,
    PCSTR pszUserName,
    DWORD dwAccessMask
    )
{
    DWORD dwError = 0;

    PWSTR pwszUserName = NULL;

    if (!pStore ||
        IsNullOrEmptyString (pszUserName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA (
                                          pszUserName,
                                          &pwszUserName
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsSetPermissionW (
                                  pStore,
                                  pwszUserName,
                                  dwAccessMask
                                 );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszUserName);

    return dwError;

error:
    goto cleanup;
}


/*
 * @brief Sets Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsSetPermissionW(
    PVECS_STORE pStore,
    PCWSTR pszUserName,
    DWORD dwAccessMask
    )
{
    DWORD dwError = 0;

    if (!pStore ||
        IsNullOrEmptyString (pszUserName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalSetPermissionW (
                                            pStore,
                                            pszUserName,
                                            dwAccessMask
                                          );
    }

    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * @brief Revokes Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsRevokePermissionA(
    PVECS_STORE pStore,
    PCSTR pszUserName,
    DWORD dwAccessMask
    )
{
    DWORD dwError = 0;

    PWSTR pwszUserName = NULL;

    if (!pStore ||
        IsNullOrEmptyString (pszUserName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringWFromA (
                                          pszUserName,
                                          &pwszUserName
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsRevokePermissionW (
                                  pStore,
                                  pwszUserName,
                                  dwAccessMask
                                 );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pwszUserName);

    return dwError;

error:
    goto cleanup;
}


/*
 * @brief Revokes Permission for a store.
 *
 * @param[in] pStore Handle to the store
 * @param[in] pszUserName Name of the service/user
 * @param[in] dwAccessMask AccessMask indicating access
 *
 * @return Returns 0 for success
 */
DWORD
VecsRevokePermissionW(
    PVECS_STORE pStore,
    PCWSTR pszUserName,
    DWORD dwAccessMask
    )
{
    DWORD dwError = 0;

    if (!pStore ||
        IsNullOrEmptyString (pszUserName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pStore->hBinding)
    {
        dwError = VecsLocalRevokePermissionW (
                                            pStore,
                                            pszUserName,
                                            dwAccessMask
                                          );
    }

    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * @brief Gets Permission of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[out] ppszOwner Owner of the store
 * @param[out] dwUserCount Number of users who have access to the store
 * @param[out] ppStorePermissions Permissions of the users
 *
 * @return Returns 0 for success
 */

DWORD
VecsGetPermissionsA(
    PVECS_STORE pStore,
    PSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_A *ppStorePermissions
    )
{
    DWORD dwError = 0;
    PWSTR pwszOwner = NULL;
    PSTR pszOwner = NULL;
    DWORD dwUserCount = 0;
    PVECS_STORE_PERMISSION_W pStorePermissionsW = NULL;
    PVECS_STORE_PERMISSION_A pStorePermissionsA = NULL;


    if (!pStore ||
        !ppszOwner ||
        !ppStorePermissions
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsGetPermissionsW(
                              pStore,
                              &pwszOwner,
                              &dwUserCount,
                              &pStorePermissionsW
                              );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwError = VmAfdAllocateStringAFromW(
                                   pwszOwner,
                                   &pszOwner
                                   );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (dwUserCount)
    {
        dwError = VecsAllocateStorePermissionsAFromW(
                                  pStorePermissionsW,
                                  dwUserCount,
                                  &pStorePermissionsA
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppszOwner = pszOwner;

    *pdwUserCount = dwUserCount;

    *ppStorePermissions = pStorePermissionsA;

cleanup:
    if (pStorePermissionsW)
    {
        VecsFreeStorePermissionsArrayW(
                                    pStorePermissionsW,
                                    dwUserCount
                                    );
    }
    VMAFD_SAFE_FREE_MEMORY(pwszOwner);

    return dwError;

error:
    if (ppszOwner)
    {
        *ppszOwner = NULL;
    }
    if (pdwUserCount)
    {
        *pdwUserCount = 0;
    }
    if (ppStorePermissions)
    {
        *ppStorePermissions = NULL;
    }

    if (pStorePermissionsA)
    {
        VecsFreeStorePermissionsArrayA(
                                    pStorePermissionsA,
                                    dwUserCount
                                    );
    }
    VMAFD_SAFE_FREE_MEMORY (pszOwner);

    goto cleanup;
}


/*
 * @brief Gets Permission of a store.
 *
 * @param[in] pStore Handle to the store
 * @param[out] ppszOwner Owner of the store
 * @param[out] dwUserCount Number of users who have access to the store
 * @param[out] ppStorePermissions Permissions of the users
 *
 * @return Returns 0 for success
 */

DWORD
VecsGetPermissionsW(
    PVECS_STORE pStore,
    PWSTR *ppszOwner,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppStorePermissions
    )
{
    DWORD dwError = 0;
    PWSTR pwszOwner = NULL;
    DWORD dwUserCount = 0;
    PVECS_STORE_PERMISSION_W pStorePermissions = NULL;


    if (!pStore ||
        !ppszOwner ||
        !ppStorePermissions
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    if (!pStore->hBinding)
    {
        dwError = VecsLocalGetPermissionW (
                                            pStore,
                                            &pwszOwner,
                                            &dwUserCount,
                                            &pStorePermissions
                                          );
    }

    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }

    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszOwner = pwszOwner;

    *pdwUserCount = dwUserCount;

    *ppStorePermissions = pStorePermissions;

cleanup:

    return dwError;

error:

    if (ppszOwner)
    {
        *ppszOwner = NULL;
    }
    if (pdwUserCount)
    {
        *pdwUserCount = 0;
    }
    if (ppStorePermissions)
    {
        *ppStorePermissions = NULL;
    }

    if (pStorePermissions)
    {
        VecsFreeStorePermissionsArrayW(
                                    pStorePermissions,
                                    dwUserCount
                                    );
    }
    VMAFD_SAFE_FREE_MEMORY (pwszOwner);
    goto cleanup;
}


/*
 * @brief CLoses a certificate store.
 *
 * @param[in] Handle to the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsCloseCertStore(
    PVECS_STORE pStore
    )
{
    DWORD dwError = 0;

    if (pStore == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pStore)
    {
        VecsReleaseCertStore(pStore);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * @brief Deletes a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteCertStoreA(
    PCSTR pszServerName,
    PCSTR pszStoreName
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszStoreName = NULL;

    if (IsNullOrEmptyString(pszStoreName)){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (pszStoreName)
    {
        dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDeleteCertStoreW(pwszServerName, pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VecsDeleteCertStoreHA(
    PVMAFD_SERVER pServer,
    PCSTR pszStoreName
    )
{
    DWORD dwError = 0;
    PWSTR pwszStoreName = NULL;

    if (IsNullOrEmptyString(pszStoreName)){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (pszStoreName)
    {
        dwError = VmAfdAllocateStringWFromA(pszStoreName, &pwszStoreName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDeleteCertStoreHW(pServer, pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszStoreName);

    return dwError;

error:

    goto cleanup;
}

/*
 * @brief Deletes a certificate store.
 *
 * @param[in,optional] pszServerName Host server for Certificate Store
 * @param[in] pszStoreName Name of the Certificate store
 *
 * @return Returns 0 for success
 */
DWORD
VecsDeleteCertStoreW(
    PCWSTR pszServerName,
    PCWSTR pszStoreName
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PWSTR pszServerEndpoint = NULL;

    if (IsNullOrEmptyString(pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszServerName) ||
        VmAfdIsLocalHostW(pszServerName)
       )
    {
      dwError = VecsLocalDeleteCertStoreW(
                                          pszStoreName
                                         );
    }

    else
    {
        dwError = VmAfdCreateBindingHandleW(
                  pszServerName,
                  pszServerEndpoint,
                  &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
          dwError = VecsRpcDeleteCertStore(
                  hBinding,
                  (PWSTR)pszStoreName
                  );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;
}

DWORD
VecsDeleteCertStoreHW(
    PVMAFD_SERVER pServer,
    PCWSTR pszStoreName
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
      dwError = VecsLocalDeleteCertStoreW(
                                          pszStoreName
                                         );
    }
    else
    {
        DCETHREAD_TRY
        {
          dwError = VecsRpcDeleteCertStore(
                  pServer->hBinding,
                  (PWSTR)pszStoreName
                  );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

/*
 * @brief Frees an entry
 *
 * @param[in] pCertEntry  Entry
 *
 */
VOID
VecsFreeCertEntryA(
    PVECS_CERT_ENTRY_A pCertEntry
    )
{
    if (pCertEntry)
    {

        VMAFD_SAFE_FREE_MEMORY(pCertEntry->pszCertificate);
        VMAFD_SAFE_FREE_MEMORY(pCertEntry->pszAlias);
        VMAFD_SAFE_FREE_MEMORY(pCertEntry->pszKey);

        VmAfdFreeMemory(pCertEntry);
    }
}



/*
 * @brief Frees an entry
 *
 * @param[in] pCertEntry Entry
 *
 */
VOID
VecsFreeCertEntryW(
    PVECS_CERT_ENTRY_W pCertEntry
    )
{
    if (pCertEntry)
    {

       VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszCertificate);
       VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszAlias);
       VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszKey);

       VmAfdFreeMemory(pCertEntry);
    }
}


/*
 * @brief Frees an array of certificate entries
 *
 * @param[in] pCertEntryArray Array of Certificate Entries
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeCertEntryArrayA(
    PVECS_CERT_ENTRY_A pCertEntryArray,
    DWORD dwCount
    )
{
    if (pCertEntryArray)
    {
        DWORD iEntry = 0;

        for (; iEntry < dwCount; iEntry++)
        {
            PVECS_CERT_ENTRY_A pEntry = &pCertEntryArray[iEntry];

            VMAFD_SAFE_FREE_MEMORY(pEntry->pszCertificate);
            VMAFD_SAFE_FREE_MEMORY(pEntry->pszAlias);
            VMAFD_SAFE_FREE_MEMORY(pEntry->pszKey);
        }

        VmAfdFreeMemory(pCertEntryArray);
    }
}



/*
 * @brief Frees an array of certificate entries
 *
 * @param[in] pCertEntryArray Array of Certificate Entries
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */
VOID
VecsFreeCertEntryArrayW(
    PVECS_CERT_ENTRY_W pCertEntryArray,
    DWORD dwCount
    )
{
    if (pCertEntryArray)
    {
        DWORD iEntry = 0;

        for (; iEntry < dwCount; iEntry++)
        {
            PVECS_CERT_ENTRY_W pEntry = &pCertEntryArray[iEntry];

            VMAFD_SAFE_FREE_MEMORY(pEntry->pwszCertificate);
            VMAFD_SAFE_FREE_MEMORY(pEntry->pwszAlias);
            VMAFD_SAFE_FREE_MEMORY(pEntry->pwszKey);
        }

        VmAfdFreeMemory(pCertEntryArray);
    }
}

/*
 * @brief Frees an array of strings
 *
 * @param[in] ppszStringArray Array of strings
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStringArrayA (
    PSTR *pszStringArray,
    DWORD dwCount
    )
{
    if (pszStringArray && dwCount)
    {
        VmAfdFreeStringArrayCountA (
              pszStringArray,
              dwCount
            );
    }
}

/*
 * @brief Frees an array of strings
 *
 * @param[in] pwszStringArray Array of strings
 * @param[in] dwCount Count of certificate entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStringArrayW (
    PWSTR *pwszStringArray,
    DWORD dwCount
    )
{
    if (pwszStringArray && dwCount)
    {
        VmAfdFreeStringArrayW (
            pwszStringArray,
            dwCount
            );
    }
}


/*
 * @brief Frees an array of Store Permissions
 *
 * @param[in] pStorePermissions Array of Permissions
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStorePermissionsArrayA(
    PVECS_STORE_PERMISSION_A pStorePermissions,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;
    if (pStorePermissions && dwCount)
    {
        for (; dwIndex < dwCount; dwIndex++)
        {
            PVECS_STORE_PERMISSION_A pCursor = &pStorePermissions[dwIndex];

            VMAFD_SAFE_FREE_MEMORY (pCursor->pszUserName);
        }
    }

    VMAFD_SAFE_FREE_MEMORY (pStorePermissions);
}

/*
 * @brief Frees an array of Store Permissions
 *
 * @param[in] pStorePermissions Array of Permissions
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */

VOID
VecsFreeStorePermissionsArrayW(
    PVECS_STORE_PERMISSION_W pStorePermissions,
    DWORD dwCount
    )
{
    VmAfdFreeStorePermissionArray (pStorePermissions,dwCount);
}

static
DWORD
VecsValidateAddEntryInput (
    CERT_ENTRY_TYPE entryType,
    PCWSTR pwszCertificate,
    PCWSTR pwszPrivateKey
    )
{
    DWORD dwError = 0;

    switch (entryType)
    {
        case CERT_ENTRY_TYPE_PRIVATE_KEY:
          if (IsNullOrEmptyString(pwszCertificate) ||
              IsNullOrEmptyString(pwszPrivateKey)
             )
          {
              dwError = ERROR_INVALID_PARAMETER;
          }
          break;
        case CERT_ENTRY_TYPE_TRUSTED_CERT:
          if (IsNullOrEmptyString(pwszCertificate))
          {
              dwError = ERROR_INVALID_PARAMETER;
          }
          break;
        case CERT_ENTRY_TYPE_SECRET_KEY:
          if (IsNullOrEmptyString(pwszPrivateKey))
          {
              dwError = ERROR_INVALID_PARAMETER;
          }
          break;
        case CERT_ENTRY_TYPE_REVOKED_CERT_LIST:
          if (IsNullOrEmptyString(pwszCertificate))
          {
              dwError = ERROR_INVALID_PARAMETER;
          }
          break;
        default:
          dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}


static
DWORD
VecsGetEntryTypeFromInt (
    UINT32 uEntryType,
    CERT_ENTRY_TYPE *pEntryType
    )
{
    DWORD dwError = 0;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pEntryType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    switch (uEntryType)
    {
      case 1:
        entryType = CERT_ENTRY_TYPE_PRIVATE_KEY;
        break;
      case 2:
        entryType = CERT_ENTRY_TYPE_SECRET_KEY;
        break;
      case 3:
        entryType = CERT_ENTRY_TYPE_TRUSTED_CERT;
        break;
      default:
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
        break;
    }
    *pEntryType = entryType;
cleanup:
    return dwError;
error:
    if (pEntryType)
    {
        *pEntryType = CERT_ENTRY_TYPE_UNKNOWN;
    }

    goto cleanup;
}

static
VOID
VecsRpcFreeCertStoreArray(
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray
    )
{
    if (pCertStoreArray->ppwszStoreNames)
    {
        VmAfdRpcClientFreeStringArrayW(
                pCertStoreArray->ppwszStoreNames,
                pCertStoreArray->dwCount);
    }
    VmAfdRpcClientFreeMemory(pCertStoreArray);
}

static
DWORD
VecsAllocateCertEntryAFromW (
    PVECS_CERT_ENTRY_W pEntryW,
    PVECS_CERT_ENTRY_A* ppEntry
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_A pEntry = NULL;

    if (!pEntryW || !ppEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                    sizeof (VECS_CERT_ENTRY_A),
                    (PVOID *) &pEntry
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pEntryW->pwszCertificate)
    {
        dwError = VmAfdAllocateStringAFromW (
                      pEntryW->pwszCertificate,
                      &pEntry->pszCertificate
                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (pEntryW->pwszKey)
    {
        dwError = VmAfdAllocateStringAFromW (
                      pEntryW->pwszKey,
                      &pEntry->pszKey
                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (pEntryW->pwszAlias)
    {
        dwError = VmAfdAllocateStringAFromW (
                      pEntryW->pwszAlias,
                      &pEntry->pszAlias
                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pEntry->entryType = pEntryW->entryType;
    pEntry->dwDate = pEntryW->dwDate;

    *ppEntry = pEntry;
cleanup:
    return dwError;
error:
    if (ppEntry)
    {
        *ppEntry = NULL;
    }
    if (pEntry)
    {
        VecsFreeCertEntryA (pEntry);
    }

    goto cleanup;
}

static
DWORD
VecsAllocateCertEntryArrayAFromW(
    PVECS_CERT_ENTRY_W pEntriesW,
    DWORD dwCount,
    PVECS_CERT_ENTRY_A* ppEntries
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_A pEntries = NULL;
    DWORD dwIndex = 0;

    if (!pEntriesW || !ppEntries)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VECS_CERT_ENTRY_A) * dwCount,
                    (PVOID *) &pEntries
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    for (; dwIndex<dwCount; dwIndex++)
    {
        PVECS_CERT_ENTRY_W pCursor = &pEntriesW[dwIndex];
        PVECS_CERT_ENTRY_A pEntry = &pEntries[dwIndex];

        if (pCursor->pwszCertificate)
        {
            dwError = VmAfdAllocateStringAFromW(
                              pCursor->pwszCertificate,
                              &pEntry->pszCertificate
                              );
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (pCursor->pwszAlias)
        {
            dwError = VmAfdAllocateStringAFromW(
                            pCursor->pwszAlias,
                            &pEntry->pszAlias
                            );
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (pCursor->pwszKey)
        {
            dwError = VmAfdAllocateStringAFromW(
                            pCursor->pwszKey,
                            &pEntry->pszKey
                            );
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        pEntry->entryType = pCursor->entryType;
        pEntry->dwDate = pCursor->dwDate;
    }
    *ppEntries = pEntries;
cleanup:

    return dwError;
error:
    if (ppEntries)
    {
      *ppEntries = NULL;
    }
    if (pEntries)
    {
      VecsFreeCertEntryArrayA(pEntries, dwCount);
    }

    goto cleanup;
}


static
DWORD
VecsAllocateCertArray(
    PVMAFD_CERT_ARRAY pCertArray,
    PVECS_CERT_ENTRY_W* ppCertArray,
    PDWORD            pdwCount
    )
{
    DWORD dwError = 0;
    PVECS_CERT_ENTRY_W pEntries = NULL;
    DWORD iEntry = 0;

    if (!pCertArray || !pCertArray->dwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VECS_CERT_ENTRY_W) * pCertArray->dwCount,
                    (PVOID*)&pEntries);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; iEntry < pCertArray->dwCount; iEntry++)
    {
        PVMAFD_CERT_CONTAINER pCursor = &pCertArray->certificates[iEntry];
        PVECS_CERT_ENTRY_W pEntry = &pEntries[iEntry];

        if (pCursor->pAlias)
        {
            dwError = VmAfdAllocateStringW(pCursor->pAlias, &pEntry->pwszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (pCursor->pCert)
        {
            dwError = VmAfdAllocateStringW(
                            pCursor->pCert,
                            &pEntry->pwszCertificate);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (pCursor->pPassword)
        {
            dwError = VmAfdAllocateStringW(
                            pCursor->pPassword,
                            &pEntry->pwszKey);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        pEntry->entryType = pCursor->dwStoreType;
        pEntry->dwDate = pCursor->dwDate;
    }

    *ppCertArray = pEntries;
    *pdwCount = pCertArray->dwCount;

cleanup:

    return dwError;

error:

    *ppCertArray = NULL;
    *pdwCount = 0;

    if (pEntries)
    {
        VecsFreeCertEntryArrayW(pEntries, pCertArray->dwCount);
    }

    goto cleanup;
}


static
DWORD
VecsAllocateStorePermissionsAFromW(
    PVECS_STORE_PERMISSION_W pStorePermissionsW,
    DWORD dwUserCount ,
    PVECS_STORE_PERMISSION_A *ppStorePermissionsA
    )
{
    DWORD dwError = 0;
    PVECS_STORE_PERMISSION_A pStorePermissionsA = NULL;
    DWORD dwIndex = 0;

    if (!pStorePermissionsW ||
        !ppStorePermissionsA
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof (VECS_STORE_PERMISSION_A) *dwUserCount,
                            (PVOID *)&pStorePermissionsA
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    for (; dwIndex < dwUserCount; dwIndex++)
    {
        PVECS_STORE_PERMISSION_W pPermissionCursor =
                                    &pStorePermissionsW[dwIndex];
        PVECS_STORE_PERMISSION_A pPermissionCursorA =
                                    &pStorePermissionsA[dwIndex];

        if (pPermissionCursor->pszUserName)
        {
            dwError = VmAfdAllocateStringAFromW(
                                              pPermissionCursor->pszUserName,
                                              &pPermissionCursorA->pszUserName
                                              );
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        pPermissionCursorA->dwAccessMask = pPermissionCursor->dwAccessMask;
    }

    *ppStorePermissionsA = pStorePermissionsA;

cleanup:
    return dwError;

error:

    if (ppStorePermissionsA)
    {
        *ppStorePermissionsA = NULL;
    }

    if (pStorePermissionsA)
    {
        VecsFreeStorePermissionsArrayA(
                              pStorePermissionsA,
                              dwUserCount
                              );
    }

    goto cleanup;
}


static
VOID
VecsFreeEnumContext(
    PVECS_ENUM_CONTEXT pContext
    )
{
    if (pContext)
    {
        if (pContext->pEnumHandle)
        {
            DWORD dwError = 0;
            PVECS_STORE pStore = pContext->pStore;

            if (!pStore->hBinding)
            {
                dwError = VecsLocalEndEnumEntries(pContext);
            }

            else
            {
                DCETHREAD_TRY
                {
                    dwError = VecsRpcEndEnumCerts(
                                pStore->hBinding,
                                &pContext->pEnumHandle);
                }
                DCETHREAD_CATCH_ALL(THIS_CATCH)
                {
                    dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
                }
                DCETHREAD_ENDTRY;
            }
        }

        if (pContext->pStore)
        {
            VecsReleaseCertStore(pContext->pStore);
        }

        VmAfdFreeMemory(pContext);
    }
}

static
PVECS_STORE
VecsAcquireCertStore(
    PVECS_STORE pStore
    )
{
    if (pStore)
    {
        InterlockedIncrement(&pStore->refCount);
    }
    return pStore;
}


static
VOID
VecsReleaseCertStore(
    PVECS_STORE pStore
    )
{
    if (pStore && InterlockedDecrement(&pStore->refCount) == 0)
    {
        if (pStore->pServer)
        {
            VmAfdReleaseServer(pStore->pServer);
        }
        VecsFreeCertStore(pStore);
    }
}

static
VOID
VecsFreeCertStore(
    PVECS_STORE pStore
    )
{
    if (pStore)
    {
        DWORD dwError = 0;
        if (pStore->hBinding)
        {
            if (pStore->pStoreHandle)
            {
                DCETHREAD_TRY
                {
                    dwError = VecsRpcCloseCertStore(pStore->hBinding, &pStore->pStoreHandle);
                }
                DCETHREAD_CATCH_ALL(THIS_CATCH)
                {
                    dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
                }
                DCETHREAD_ENDTRY;
            }
            if (pStore->bOwnBinding && pStore->hBinding)
            {
                VmAfdFreeBindingHandle(&(pStore->hBinding));
            }
        }
        else if (pStore->pStoreHandle)
        {
            dwError = VecsLocalCloseCertStore(pStore);
        }
        if (pStore->pConnection)
        {
            VmAfdFreeClientConnection(pStore->pConnection);
        }
        VmAfdFreeMemory(pStore);
    }
}
