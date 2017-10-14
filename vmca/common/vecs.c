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

/*
 *
 * Filename: vecs.c
 *
 * Abstract:
 *
 * VECS integration to get SSL cert
 *
 */

#include "includes.h"

//#include "../../vmafd/include/public/vmafdtypes.h"
//#include "../../vmafd/include/public/vmafd.h"
//#include "../../vmafd/include/public/vecsclient.h"

#define BAIL_ON_VECS_ERROR(dwError)                                 \
    if (dwError)                                                    \
    {                                                               \
        VMCA_LOG_ERROR("[%s,%d], VECS error code %d",__FILE__, __LINE__, dwError); \
        dwError = VMCA_UNKNOW_ERROR; \
        goto vecs_error;                                            \
    }

#define FN_VECS_OPEN_CERT_STORE_A           "VecsOpenCertStoreA"
#define FN_VECS_GET_ENTRY_BY_ALIAS_A        "VecsGetEntryByAliasA"
#define FN_VECS_GET_KEY_BY_ALIAS_A          "VecsGetKeyByAliasA"
#define FN_VECS_CLOSE_CERT_STORE            "VecsCloseCertStore"
#define FN_VECS_FREE_ENTRY_A                "VecsFreeCertEntryA"

#define MACHINE_CERT_STORE_NAME             "MACHINE_SSL_CERT"
#define MACHINE_CERT_ALIAS                  "__MACHINE_CERT"

typedef DWORD   (*fpVecsOpenCertStoreA)     ( PCSTR,PCSTR, PCSTR, PVECS_STORE* );
typedef DWORD   (*fpVecsGetEntryByAliasA)   ( PVECS_STORE, PCSTR, ENTRY_INFO_LEVEL, PVECS_CERT_ENTRY_A* );
typedef DWORD   (*fpVecsGetKeyByAliasA)     ( PVECS_STORE, PCSTR, PCSTR, PSTR* );
typedef DWORD   (*fpVecsCloseCertStore)     ( PVECS_STORE );
typedef VOID    (*fpVecsFreeCertEntryA)     ( PVECS_CERT_ENTRY_A );

static
DWORD
_VMCAGetSSLCert(
    VMCA_LIB_HANDLE plibHandle,
    PSTR*           ppszCert,
    PSTR*           ppszKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;
    PVECS_STORE         pVECSStore = NULL;
    PVECS_CERT_ENTRY_A  pCertEntry = NULL;

    if (plibHandle == NULL || ppszCert == NULL || ppszKey == NULL)
    {
        dwError = VMCA_ARGUMENT_ERROR;
        goto cleanup;
    }

    fpVecsOpenCertStoreA    fpOpenStore = NULL;
    fpVecsGetEntryByAliasA  fpGetEntry = NULL;
    fpVecsGetKeyByAliasA    fpGetKey = NULL;
    fpVecsCloseCertStore    fpCloseStore = NULL;
    fpVecsFreeCertEntryA    fpFreeEntry = NULL;

    if ( (fpOpenStore = (fpVecsOpenCertStoreA) VMCAGetLibSym(plibHandle, FN_VECS_OPEN_CERT_STORE_A) ) == NULL
          ||
         (fpGetEntry = (fpVecsGetEntryByAliasA) VMCAGetLibSym(plibHandle, FN_VECS_GET_ENTRY_BY_ALIAS_A) ) == NULL
          ||
         (fpGetKey = (fpVecsGetKeyByAliasA) VMCAGetLibSym(plibHandle, FN_VECS_GET_KEY_BY_ALIAS_A) ) == NULL
          ||
         (fpCloseStore = (fpVecsCloseCertStore) VMCAGetLibSym(plibHandle, FN_VECS_CLOSE_CERT_STORE) ) == NULL
          ||
         (fpFreeEntry = (fpVecsFreeCertEntryA) VMCAGetLibSym(plibHandle, FN_VECS_FREE_ENTRY_A) ) == NULL
       )
    {
#ifdef _WIN32
        VMCA_LOG_ERROR("VECS sym lookup failed, %d", WSAGetLastError());
#else
        VMCA_LOG_ERROR("VECS sym lookup failed, %s", VMCA_SAFE_STRING(dlerror()));
#endif
        dwError = VMCA_UNKNOW_ERROR;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = (*fpOpenStore)( "localhost", MACHINE_CERT_STORE_NAME, NULL, &pVECSStore );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetEntry)( pVECSStore, MACHINE_CERT_ALIAS, ENTRY_INFO_LEVEL_2, &pCertEntry );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetKey)( pVECSStore, MACHINE_CERT_ALIAS, NULL, &pszKey );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VMCAAllocateStringA( pCertEntry->pszCertificate, &pszCert );
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
    VMCA_SAFE_FREE_MEMORY(pszCert);
    VMCA_SAFE_FREE_MEMORY(pszKey);

    VMCA_LOG_ERROR("%s failed, error (%u)", __FUNCTION__, dwError);

    goto cleanup;

vecs_error:
    goto cleanup;
}


DWORD
VMCAGetVecsMachineCert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    )
{
    DWORD            dwError = 0;
    VMCA_LIB_HANDLE  plibHandle = NULL;
    PSTR             pszCert = NULL;
    PSTR             pszKey = NULL;

    if (ppszCert == NULL || ppszKey == NULL)
    {
        dwError = VMCA_ARGUMENT_ERROR;
        goto cleanup;
    }

    dwError = VMCAOpenVmAfdClientLib( &plibHandle );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = _VMCAGetSSLCert( plibHandle, &pszCert, &pszKey );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszCert = pszCert;
    *ppszKey  = pszKey;

    VMCA_LOG_INFO("Acquired SSL Cert from VECS");

cleanup:
    VMCACloseLibrary( plibHandle );

    return dwError;

error:
    *ppszCert = NULL;
    *ppszKey = NULL;
    VMCA_SAFE_FREE_MEMORY(pszCert);
    VMCA_SAFE_FREE_MEMORY(pszKey);

    goto cleanup;
}


