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

/*
 * Module Name: Directory ldap head
 *
 * Filename: vesc.c
 *
 * Abstract:
 *
 * VECS integration to get SSL cert
 *
 */

#include "includes.h"

// WARNING, WARNING, WARNING. It is awkward to get VECS headers via source tree structure.
#include "../../../vmafd/include/public/vmafdtypes.h"
#include "../../../vmafd/include/public/vmafd.h"
#include "../../../vmafd/include/public/vecsclient.h"

#define BAIL_ON_VECS_ERROR(dwError)                                 \
    if (dwError)                                                    \
    {                                                               \
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "[%s,%d], VECS error code %d",__FILE__, __LINE__, dwError); \
        dwError = VMDIR_ERROR_GENERIC;                              \
        goto vecs_error;                                            \
    }

#define FN_VECS_OPEN_CERT_STORE_A           "VecsOpenCertStoreA"
#define FN_VECS_GET_ENTRY_BY_ALIAS_A        "VecsGetEntryByAliasA"
#define FN_VECS_GET_KEY_BY_ALIAS_A          "VecsGetKeyByAliasA"
#define FN_VECS_CLOSE_CERT_STORE            "VecsCloseCertStore"
#define FN_VECS_FREE_ENTRY_A                "VecsFreeCertEntryA"

#define MACIHNE_CERT_STORE_NAME             "MACHINE_SSL_CERT"
#define MACHINE_CERT_ALIAS                  "__MACHINE_CERT"

typedef DWORD   (*fpVecsOpenCertStoreA)     ( PCSTR,PCSTR, PCSTR, PVECS_STORE* );
typedef DWORD   (*fpVecsGetEntryByAliasA)   ( PVECS_STORE, PCSTR, ENTRY_INFO_LEVEL, PVECS_CERT_ENTRY_A* );
typedef DWORD   (*fpVecsGetKeyByAliasA)     ( PVECS_STORE, PCSTR, PCSTR, PSTR* );
typedef DWORD   (*fpVecsCloseCertStore)     ( PVECS_STORE );
typedef VOID    (*fpVecsFreeCertEntryA)     ( PVECS_CERT_ENTRY_A );

static
DWORD
_VmDirGetSSLCert(
    VMDIR_LIB_HANDLE  plibHandle,
    PSTR*           ppszCert,
    PSTR*           ppszKey
    )
{
    DWORD   dwError = 0;
    PSTR    pszCert = NULL;
    PSTR    pszKey = NULL;
    PVECS_STORE         pVECSStore = NULL;
    PVECS_CERT_ENTRY_A  pCertEntry = NULL;

    fpVecsOpenCertStoreA    fpOpenStore = NULL;
    fpVecsGetEntryByAliasA  fpGetEntry = NULL;
    fpVecsGetKeyByAliasA    fpGetKey = NULL;
    fpVecsCloseCertStore    fpCloseStore = NULL;
    fpVecsFreeCertEntryA    fpFreeEntry = NULL;

    if ( (fpOpenStore = (fpVecsOpenCertStoreA) VmDirGetLibSym(plibHandle, FN_VECS_OPEN_CERT_STORE_A) ) == NULL
          ||
         (fpGetEntry = (fpVecsGetEntryByAliasA) VmDirGetLibSym(plibHandle, FN_VECS_GET_ENTRY_BY_ALIAS_A) ) == NULL
          ||
         (fpGetKey = (fpVecsGetKeyByAliasA) VmDirGetLibSym(plibHandle, FN_VECS_GET_KEY_BY_ALIAS_A) ) == NULL
          ||
         (fpCloseStore = (fpVecsCloseCertStore) VmDirGetLibSym(plibHandle, FN_VECS_CLOSE_CERT_STORE) ) == NULL
          ||
         (fpFreeEntry = (fpVecsFreeCertEntryA) VmDirGetLibSym(plibHandle, FN_VECS_FREE_ENTRY_A) ) == NULL
       )
    {
#ifdef _WIN32
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VECS sym lookup failed, %d", WSAGetLastError());
#else
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VECS sym lookup failed, %s", VDIR_SAFE_STRING(dlerror()));
#endif
        dwError = VMDIR_ERROR_NOT_FOUND;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = (*fpOpenStore)( "localhost", MACIHNE_CERT_STORE_NAME, NULL, &pVECSStore );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetEntry)( pVECSStore, MACHINE_CERT_ALIAS, ENTRY_INFO_LEVEL_2, &pCertEntry );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = (*fpGetKey)( pVECSStore, MACHINE_CERT_ALIAS, NULL, &pszKey );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VmDirAllocateStringA( pCertEntry->pszCertificate, &pszCert );
    BAIL_ON_VECS_ERROR(dwError);

    *ppszCert = pszCert;  pszCert = NULL;
    *ppszKey  = pszKey;   pszKey = NULL;

cleanup:

    if ( fpFreeEntry && pCertEntry )
    {
        (*fpFreeEntry)(pCertEntry);
    }

    if ( fpCloseStore && pVECSStore )
    {
        (*fpCloseStore)(pVECSStore);
    }

    VMDIR_SAFE_FREE_MEMORY(pszCert);
    VMDIR_SAFE_FREE_MEMORY(pszKey);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error (%u)", __FUNCTION__, dwError);

    goto cleanup;

vecs_error:
    goto cleanup;
}

DWORD
VmDirGetVecsMachineCert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    )
{
    DWORD             dwError = 0;
    VMDIR_LIB_HANDLE  plibHandle = NULL;
    PSTR              pszCert = NULL;
    PSTR              pszKey = NULL;

    dwError = VmDirOpenVmAfdClientLib( &plibHandle );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetSSLCert( plibHandle, &pszCert, &pszKey );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszCert = pszCert; pszCert = NULL;
    *ppszKey  = pszKey;  pszKey = NULL;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Acquired SSL Cert from VECS");

cleanup:
    VmDirCloseLibrary( plibHandle );

    VMDIR_SAFE_FREE_MEMORY(pszCert);
    VMDIR_SAFE_FREE_MEMORY(pszKey);

    return dwError;

error:
    goto cleanup;
}
