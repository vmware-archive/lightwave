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

#ifdef _WIN32
typedef HINSTANCE   VMW_LIB_HANDLE;
#else
#include <dlfcn.h>
typedef VOID*       VMW_LIB_HANDLE;
#endif

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

#ifdef _WIN32

#define VMAFD_VECS_CLIENT_LIBRARY   "\\libvmafdclient.dll"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_SOFTWARE_KEY_PATH
#define VMAFD_LIB_KEY               VMDIR_REG_KEY_INSTALL_PATH

#else

#define VMAFD_VECS_CLIENT_LIBRARY   "/lib64/libvmafdclient.so"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_KEY_ROOT
#define VMAFD_LIB_KEY               VMAFD_REG_KEY_PATH

#endif

static
DWORD
_VmDirOpenVecsLib(
    VMW_LIB_HANDLE*   pplibHandle
    )
{
    DWORD   dwError = 0;
    VMW_LIB_HANDLE plibHandle = NULL;
#ifdef _WIN32
    CHAR    pszRegLibPath[VMDIR_MAX_PATH_LEN] = WIN_SYSTEM32_PATH;
#else
    CHAR    pszRegLibPath[VMDIR_MAX_PATH_LEN] = {0};
#endif
    PSTR    pszVmafdName = NULL;
    PSTR    pszVmafdLibPath = NULL;

#ifndef _WIN32
    dwError = VmDirGetRegKeyValue( VMAFD_KEY_ROOT,
                                   VMAFD_LIB_KEY,
                                   pszRegLibPath,
                                   sizeof(pszRegLibPath)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    // find the first vmafd in path key "/usr/lib/vmware-vmafd/...."
    pszVmafdName = strstr(pszRegLibPath, VMAFD_NAME);
    if (pszVmafdName == NULL)
    {
        dwError = VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pszVmafdName[strlen(VMAFD_NAME)] = '\0';
    }
#endif

    // construct full path to libvmafdclient
    dwError = VmDirAllocateStringPrintf( &pszVmafdLibPath, "%s%s",pszRegLibPath, VMAFD_VECS_CLIENT_LIBRARY);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifdef _WIN32
    plibHandle = LoadLibrary(pszVmafdLibPath);
    if (plibHandle == NULL)
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "LoadLibrary %s failed, error code %d", pszVmafdLibPath, WSAGetLastError());
        dwError = VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
    }
#else
    plibHandle = dlopen(pszVmafdLibPath, RTLD_LAZY);
    if (plibHandle == NULL)
    {
         VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "dlopen %s library failed, error msg (%s)", pszVmafdLibPath, VDIR_SAFE_STRING(dlerror()));
         dlerror();    /* Clear any existing error */
         dwError = VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
    }
#endif
    BAIL_ON_VMDIR_ERROR(dwError);

    *pplibHandle = plibHandle;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszVmafdLibPath);

    return dwError;

error:
    goto cleanup;
}


static
VOID
_VmDirCloseVecsLib(
    VMW_LIB_HANDLE  plibHandle
    )
{
    if (plibHandle)
    {
#ifdef _WIN32
        FreeLibrary(plibHandle);
#else
        dlclose(plibHandle);
#endif
    }

    return;
}

static
#ifdef _WIN32
FARPROC WINAPI
#else
VOID*
#endif
_VmDirGetLibSym(
    VMW_LIB_HANDLE  plibHandle,
    PCSTR           pszFunctionName
    )
{
#ifdef _WIN32
    return GetProcAddress(plibHandle, pszFunctionName);
#else
    return dlsym(plibHandle, pszFunctionName);
#endif
}

static
DWORD
_VmDirGetSSLCert(
    VMW_LIB_HANDLE  plibHandle,
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

    if ( (fpOpenStore = (fpVecsOpenCertStoreA) _VmDirGetLibSym(plibHandle, FN_VECS_OPEN_CERT_STORE_A) ) == NULL
          ||
         (fpGetEntry = (fpVecsGetEntryByAliasA) _VmDirGetLibSym(plibHandle, FN_VECS_GET_ENTRY_BY_ALIAS_A) ) == NULL
          ||
         (fpGetKey = (fpVecsGetKeyByAliasA) _VmDirGetLibSym(plibHandle, FN_VECS_GET_KEY_BY_ALIAS_A) ) == NULL
          ||
         (fpCloseStore = (fpVecsCloseCertStore) _VmDirGetLibSym(plibHandle, FN_VECS_CLOSE_CERT_STORE) ) == NULL
          ||
         (fpFreeEntry = (fpVecsFreeCertEntryA) _VmDirGetLibSym(plibHandle, FN_VECS_FREE_ENTRY_A) ) == NULL
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
    DWORD           dwError = 0;
    VMW_LIB_HANDLE  plibHandle = NULL;
    PSTR            pszCert = NULL;
    PSTR            pszKey = NULL;

    dwError = _VmDirOpenVecsLib( &plibHandle );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetSSLCert( plibHandle, &pszCert, &pszKey );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszCert = pszCert; pszCert = NULL;
    *ppszKey  = pszKey;  pszKey = NULL;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Acquired SSL Cert from VECS");

cleanup:
    _VmDirCloseVecsLib( plibHandle );

    VMDIR_SAFE_FREE_MEMORY(pszCert);
    VMDIR_SAFE_FREE_MEMORY(pszKey);

    return dwError;

error:
    goto cleanup;
}
