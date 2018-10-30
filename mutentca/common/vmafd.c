/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#define BAIL_ON_VECS_ERROR(dwError)                                 \
    if (dwError)                                                    \
    {                                                               \
        dwError = LWCA_UNKNOWN_ERROR; \
        LWCA_LOG_ERROR("[%s,%d], VECS error code %d",__FILE__, __LINE__, dwError); \
        goto vecs_error;                                            \
    }

#define FN_VECS_OPEN_CERT_STORE_A                   "VecsOpenCertStoreA"
#define FN_VECS_GET_ENTRY_BY_ALIAS_A                "VecsGetEntryByAliasA"
#define FN_VECS_GET_KEY_BY_ALIAS_A                  "VecsGetKeyByAliasA"
#define FN_VECS_CLOSE_CERT_STORE                    "VecsCloseCertStore"
#define FN_VECS_FREE_ENTRY_A                        "VecsFreeCertEntryA"
#define FN_VMAFD_GET_DC_NAME_A                      "VmAfdGetDCNameA"
#define FN_VMAFD_GET_DOMAIN_NAME_A                  "VmAfdGetDomainNameA"

typedef DWORD   (*fpVecsOpenCertStoreA)             ( PCSTR,PCSTR, PCSTR, PVECS_STORE* );
typedef DWORD   (*fpVecsGetEntryByAliasA)           ( PVECS_STORE, PCSTR, ENTRY_INFO_LEVEL, PVECS_CERT_ENTRY_A* );
typedef DWORD   (*fpVecsGetKeyByAliasA)             ( PVECS_STORE, PCSTR, PCSTR, PSTR* );
typedef DWORD   (*fpVecsCloseCertStore)             ( PVECS_STORE );
typedef VOID    (*fpVecsFreeCertEntryA)             ( PVECS_CERT_ENTRY_A );
typedef DWORD   (*fpVmAfdGetDCNameA)                ( PCSTR, PSTR* );
typedef DWORD   (*fpVmAfdGetDomainNameA)            ( PCSTR, PSTR* );


static
DWORD
_LwCAGetSSLCert(
    LWCA_LIB_HANDLE plibHandle,
    PSTR*           ppszCert,
    PSTR*           ppszKey,
    PSTR            pszVecsStore,
    PSTR            pszVecsAlias
    );

static
DWORD
_LwCAGetVecsCert(
    PLWCA_CERTIFICATE   *ppszCert,
    PSTR                *ppszKey,
    PSTR                pszVecsStore,
    PSTR                pszVecsAlias
    );

static
DWORD
_LwCAGetVmAfdDCName(
    PSTR        *ppszDCName
    );

static
DWORD
_LwCAGetVmAfdDomainName(
    PSTR        *ppszDomainName
    );


DWORD
LwCAGetVecsMachineCert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;

    if (!ppszCert || !ppszKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetVecsCert(&pszCert,
                               &pszKey,
                               LWCA_MACHINE_CERT_STORE_NAME,
                               LWCA_MACHINE_CERT_ALIAS
                               );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCert = pszCert;
    *ppszKey = pszKey;

cleanup:
    return dwError;

error:
    if (ppszCert)
    {
        *ppszCert = NULL;
    }
    if (ppszKey)
    {
        *ppszKey = NULL;
    }

    goto cleanup;
}

DWORD
LwCAGetVecsMutentCACert(
    PLWCA_CERTIFICATE   *ppszCert,
    PSTR                *ppszKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;

    if (!ppszCert || !ppszKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetVecsCert(&pszCert,
                               &pszKey,
                               LWCA_MUTENTCA_STORE_NAME,
                               LWCA_MUTENTCA_ALIAS
                              );
    BAIL_ON_LWCA_ERROR(dwError);
    *ppszCert = pszCert;
    *ppszKey = pszKey;

cleanup:
    return dwError;

error:
    if (ppszCert)
    {
        *ppszCert = NULL;
    }
    if (ppszKey)
    {
        *ppszKey = NULL;
    }

    goto cleanup;
}

DWORD
LwCAOpenVmAfdClientLib(
    LWCA_LIB_HANDLE*   pplibHandle
    )
{
    DWORD   dwError = 0;
    CHAR    pszRegLibPath[LWCA_MAX_PATH_LEN] = {0};
    PSTR    pszVmafdLibPath = NULL;
    LWCA_LIB_HANDLE    plibHandle = NULL;

#ifdef LIGHTWAVE_BUILD

    dwError = LwCAStringNCpyA(
            pszRegLibPath,
            LWCA_MAX_PATH_LEN,
            MUTENTCA_LIB_DIR,
            LWCA_MAX_PATH_LEN);
    BAIL_ON_LWCA_ERROR(dwError);

#else

    PSTR pszVmafdName = NULL;

    dwError = LwCAGetRegKeyValue(
            VMAFD_KEY_ROOT,
            VMAFD_LIB_KEY,
            pszRegLibPath,
            sizeof(pszRegLibPath) - 1);
    BAIL_ON_LWCA_ERROR(dwError);

    // find the first vmafd in path key "/usr/lib/vmware-vmafd/...."
    pszVmafdName = strstr(pszRegLibPath, VMAFD_NAME);

    dwError = pszVmafdName ? 0 : LWCA_ERROR_NO_FILE_OR_DIRECTORY;
    BAIL_ON_LWCA_ERROR(dwError);

    pszVmafdName[strlen(VMAFD_NAME)] = '\0';

#endif

    // construct full path to libvmafdclient
    dwError = LwCAAllocateStringPrintfA(
            &pszVmafdLibPath,
            "%s%s",
            pszRegLibPath,
            VMAFD_VECS_CLIENT_LIBRARY);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCALoadLibrary(pszVmafdLibPath, &plibHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    *pplibHandle = plibHandle;

cleanup:
    LWCA_SAFE_FREE_MEMORY(pszVmafdLibPath);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAGetDCName(
    PSTR        *ppszDCName
    )
{
    DWORD       dwError = 0;
    PSTR        pszDCName = NULL;

    if (!ppszDCName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetVmAfdDCName(&pszDCName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDCName = pszDCName;


cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszDCName);
    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }

    goto cleanup;
}

DWORD
LwCAGetDomainName(
    PSTR        *ppszDomainName
    )
{
    DWORD       dwError = 0;
    PSTR        pszDomainName = NULL;

    if (!ppszDomainName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetVmAfdDomainName(&pszDomainName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszDomainName = pszDomainName;


cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszDomainName);
    if (ppszDomainName)
    {
        *ppszDomainName = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAGetSSLCert(
    LWCA_LIB_HANDLE plibHandle,
    PSTR*           ppszCert,
    PSTR*           ppszKey,
    PSTR            pszVecsStore,
    PSTR            pszVecsAlias
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;
    PVECS_STORE         pVECSStore = NULL;
    PVECS_CERT_ENTRY_A  pCertEntry = NULL;

    if (plibHandle == NULL || ppszCert == NULL || ppszKey == NULL)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    fpVecsOpenCertStoreA    fpOpenStore = NULL;
    fpVecsGetEntryByAliasA  fpGetEntry = NULL;
    fpVecsGetKeyByAliasA    fpGetKey = NULL;
    fpVecsCloseCertStore    fpCloseStore = NULL;
    fpVecsFreeCertEntryA    fpFreeEntry = NULL;

    if ( (fpOpenStore = (fpVecsOpenCertStoreA)LwCAGetLibSym(
                             plibHandle,
                             FN_VECS_OPEN_CERT_STORE_A)) == NULL
          ||
         (fpGetEntry = (fpVecsGetEntryByAliasA)LwCAGetLibSym(
                              plibHandle,
                              FN_VECS_GET_ENTRY_BY_ALIAS_A)) == NULL
          ||
         (fpGetKey = (fpVecsGetKeyByAliasA)LwCAGetLibSym(
                              plibHandle,
                              FN_VECS_GET_KEY_BY_ALIAS_A)) == NULL
          ||
         (fpCloseStore = (fpVecsCloseCertStore)LwCAGetLibSym(
                              plibHandle,
                              FN_VECS_CLOSE_CERT_STORE)) == NULL
          ||
         (fpFreeEntry = (fpVecsFreeCertEntryA)LwCAGetLibSym(
                              plibHandle,
                              FN_VECS_FREE_ENTRY_A)) == NULL
       )
    {
        LWCA_LOG_ERROR("VECS sym lookup failed, %s",
                       LWCA_SAFE_STRING(dlerror())
                       );
        dwError = LWCA_UNKNOWN_ERROR;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = (*fpOpenStore)("localhost", pszVecsStore, NULL, &pVECSStore);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetEntry)(pVECSStore,
                            pszVecsAlias,
                            ENTRY_INFO_LEVEL_2,
                            &pCertEntry
                            );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetKey)(pVECSStore, pszVecsAlias, NULL, &pszKey);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = LwCAAllocateStringA(pCertEntry->pszCertificate, &pszCert);
    BAIL_ON_VECS_ERROR(dwError);

    *ppszCert = pszCert;
    *ppszKey  = pszKey;

cleanup:

    if ( fpFreeEntry && pCertEntry )
    {
        (*fpFreeEntry)(pCertEntry);
    }

    if ( fpCloseStore && pVECSStore )
    {
        (*fpCloseStore)(pVECSStore);
    }

    return dwError;

error:
    *ppszCert = NULL;
    *ppszKey = NULL;
    LWCA_SAFE_FREE_MEMORY(pszCert);
    LWCA_SAFE_FREE_MEMORY(pszKey);

    LWCA_LOG_ERROR("%s failed, error (%u)", __FUNCTION__, dwError);

    goto cleanup;

vecs_error:
    goto cleanup;
}

static
DWORD
_LwCAGetVecsCert(
    PLWCA_CERTIFICATE   *ppszCert,
    PSTR                *ppszKey,
    PSTR                pszVecsStore,
    PSTR                pszVecsAlias
    )
{
    DWORD               dwError = 0;
    LWCA_LIB_HANDLE     plibHandle = NULL;
    PSTR                pszCert = NULL;
    PSTR                pszKey = NULL;

    if (!ppszCert || !ppszKey || !pszVecsStore || !pszVecsAlias)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAOpenVmAfdClientLib(&plibHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetSSLCert(plibHandle,
                              &pszCert,
                              &pszKey,
                              pszVecsStore,
                              pszVecsAlias);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCert = pszCert;
    *ppszKey = pszKey;

    LWCA_LOG_INFO("Acquired Certs from VECS");

cleanup:
    LwCACloseLibrary(plibHandle);
    return dwError;

error:
    if (ppszCert)
    {
        *ppszCert = NULL;
    }
    if (ppszKey)
    {
        *ppszKey = NULL;
    }
    LWCA_SAFE_FREE_MEMORY(pszCert);
    LWCA_SAFE_FREE_MEMORY(pszKey);

    goto cleanup;
}

static
DWORD
_LwCAGetVmAfdDCName(
    PSTR                *ppszDCName
    )
{
    DWORD               dwError = 0;
    DWORD               dwVmAfdError = 0;
    LWCA_LIB_HANDLE     pVmAfdLibHandle = NULL;
    fpVmAfdGetDCNameA   fpGetDCName = NULL;
    PSTR                pszDCName = NULL;

    if (!ppszDCName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAOpenVmAfdClientLib(&pVmAfdLibHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    fpGetDCName = (fpVmAfdGetDCNameA)LwCAGetLibSym(pVmAfdLibHandle, FN_VMAFD_GET_DC_NAME_A);
    if (!fpGetDCName)
    {
        LWCA_LOG_ERROR(
                "[%s,%d] Failed to lookup libvmafdclient symbol (%s) with (%s)",
                __FUNCTION__,
                __LINE__,
                FN_VMAFD_GET_DC_NAME_A,
                LWCA_SAFE_STRING(dlerror()));

        dwError = LWCA_ERROR_DLL_SYMBOL_NOTFOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwVmAfdError = fpGetDCName(NULL, &pszDCName);
    if (dwVmAfdError)
    {
        LWCA_LOG_ERROR(
                "[%s,%d] Failed to get DC Name from libvmafdclient (%d)",
                __FUNCTION__,
                __LINE__,
                dwVmAfdError);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_VMAFD_UNAVAILABLE);
    }

    *ppszDCName = pszDCName;


cleanup:

    LwCACloseLibrary(pVmAfdLibHandle);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszDCName);
    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAGetVmAfdDomainName(
    PSTR        *ppszDomainName
    )
{
    DWORD               dwError = 0;
    DWORD               dwVmAfdError = 0;
    LWCA_LIB_HANDLE     pVmAfdLibHandle = NULL;
    fpVmAfdGetDCNameA   fpGetDomainName = NULL;
    PSTR                pszDomainName = NULL;

    if (!ppszDomainName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAOpenVmAfdClientLib(&pVmAfdLibHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    fpGetDomainName = (fpVmAfdGetDomainNameA)LwCAGetLibSym(pVmAfdLibHandle, FN_VMAFD_GET_DOMAIN_NAME_A);
    if (!fpGetDomainName)
    {
        LWCA_LOG_ERROR(
                "[%s,%d] Failed to lookup libvmafdclient symbol (%s) with (%s)",
                __FUNCTION__,
                __LINE__,
                FN_VMAFD_GET_DOMAIN_NAME_A,
                LWCA_SAFE_STRING(dlerror()));

        dwError = LWCA_ERROR_DLL_SYMBOL_NOTFOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwVmAfdError = fpGetDomainName(NULL, &pszDomainName);
    if (dwVmAfdError)
    {
        LWCA_LOG_ERROR(
                "[%s,%d] Failed to get Domain Name from libvmafdclient (%d)",
                __FUNCTION__,
                __LINE__,
                dwVmAfdError);
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_VMAFD_UNAVAILABLE);
    }

    *ppszDomainName = pszDomainName;


cleanup:

    LwCACloseLibrary(pVmAfdLibHandle);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszDomainName);
    if (ppszDomainName)
    {
        *ppszDomainName = NULL;
    }

    goto cleanup;
}
