/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : vecsserviceapi.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
VOID
VecsSrvFreeCertStore(
        PVECS_SERV_STORE pStore
        );

static
VOID
VecsSrvFreeEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    );

static
BOOL
VecsIsFlushableEntry(
    CERT_ENTRY_TYPE cEntryType,
    DWORD           dwStoreId
    );

static
DWORD
VecsGetFlushedFileFullPath(
    PCSTR pszCAPath,
    PCSTR pszFileName,
    PSTR* ppszFullPath
    );

static
DWORD
VecsSrvFlushRoot_MD5(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvFlushRoot_SHA_1(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvFlushCrl_MD5(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvFlushCrl_SHA_1(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsCheckCertOnDisk(
    PCSTR    pszAlias,
    PCSTR    pszCAPath,
    PCSTR    pszFilename,
    LONG     maxIndex,
    BOOLEAN  bCrl,
    LONG*    pNextAvailable,
    PSTR*    ppszMatchedFile
    );

static
DWORD
VecsDeleteFileWithRetry(
    PCSTR pszPath,
    int nRetry
    );

static
DWORD
VecsWriteCleanupFlagFile(
    PCSTR pszCAPath,
    PCSTR pszSrcPath
    );

static
DWORD
VecsSrvWriteCertOrCrlToDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bIsCrl,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvWriteRootToDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvWriteCrlToDisk(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bLogOnDuplicate
    );

static
DWORD
VecsSrvDeleteRootFromDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bCrl
    );

static
DWORD
VecsSrvWriteCertStringToDisk(
    PCSTR pszCertificate,
    PCSTR pszFilePath,
    BOOL bLogOnDuplicate,
    int mode
    );

static
DWORD
VecsSrvUnflushCertificate(
    PVECS_SERV_STORE pStore,
    PSTR pszCert
    );

static
DWORD
VecsSrvUnflushMachineSslCertificate(
    PVECS_SERV_STORE pStore
    );

static
DWORD
VecsSrvGetVpxDocRootPath(
    PSTR* ppszDocRoot
    );

static
DWORD
VecsSrvGetMachineSslPathA(
    PSTR* ppszFilePath
    );

static
DWORD
VecsSrvFlushCertsFromDB(
    PWSTR pwszStoreName,
    CERT_ENTRY_TYPE entryType
    );

DWORD
VecsSrvCreateCertStore(
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_SERV_STORE *ppStore
        )
{
    DWORD dwError = 0;
    DWORD dwStore = 0;
    PVECS_SERV_STORE pStore = NULL;

    dwError = VecsDbCreateCertStore(
                    pszStoreName,
                    pszPassword
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbGetCertStore(
                    pszStoreName,
                    pszPassword,
                    &dwStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(sizeof(VECS_SERV_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;
    pStore->dwStoreId = dwStore;

    *ppStore = pStore;


cleanup:

    return dwError;

error:
    *ppStore = NULL;

    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }
   /* if (dwError == SQLITE_CONSTRAINT)
    {
        dwError = ERROR_ALREADY_EXISTS;
    }*/

    goto cleanup;
}

DWORD
VecsSrvOpenCertStore(
        PCWSTR pszStoreName,
        PCWSTR pszPassword,
        PVECS_SERV_STORE *ppStore
        )
{
    DWORD dwError = 0;
    DWORD dwStore = 0;
    PVECS_SERV_STORE pStore = NULL;

    dwError = VecsDbGetCertStore(
                    pszStoreName,
                    pszPassword,
                    &dwStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(sizeof(VECS_SERV_STORE), (PVOID *)&pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStore->refCount = 1;
    pStore->dwStoreId = dwStore;

    *ppStore = pStore;

cleanup:

    return dwError;

error:
    *ppStore = NULL;
    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }

    goto cleanup;
}

DWORD
VecsSrvCloseCertStore(
        PVECS_SERV_STORE pStore
        )
{
    DWORD dwError = 0;
    VecsSrvReleaseCertStore(pStore);
    return dwError;
}

DWORD
VecsSrvEnumCertStore(
    PWSTR ** ppszStoreNameArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PWSTR * pszStoreNameArray = NULL;

    dwError = VecsDbEnumCertStore(
                &pszStoreNameArray,
                &dwCount
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszStoreNameArray = pszStoreNameArray;
    *pdwCount = dwCount;

cleanup:

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
        VmAfdFreeStringArrayW(pszStoreNameArray, dwCount);
    }

    goto cleanup;
}

DWORD
VecsSrvDeleteCertStore(
    PCWSTR pwszStoreName
    )
{
    return VecsDbDeleteCertStore(pwszStoreName);
}

DWORD
VecsSrvAllocateCertEnumContext(
    PVECS_SERV_STORE        pStore,
    DWORD                   dwMaxCount,
    ENTRY_INFO_LEVEL        infoLevel,
    PVECS_SRV_ENUM_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PVECS_SRV_ENUM_CONTEXT pContext = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(VECS_SRV_ENUM_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->refCount = 1;

    pContext->pStore = VecsSrvAcquireCertStore(pStore);

    if (!dwMaxCount || (dwMaxCount > 256))
    {
        pContext->dwLimit = 256;
    }
    else
    {
        pContext->dwLimit = dwMaxCount;
    }
    pContext->infoLevel = infoLevel;

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    if (ppContext)
    {
        *ppContext = NULL;
    }

    goto cleanup;
}

DWORD
VecsSrvEnumCertsInternal(
    PVECS_SRV_ENUM_CONTEXT pContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;

    dwError = VecsSrvEnumCerts(pContext, &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->dwIndex += pCertContainer->dwCount;

    *ppCertContainer = pCertContainer;

cleanup:

    return dwError;

error:

    *ppCertContainer = NULL;

    goto cleanup;
}

DWORD
VecsSrvEnumCerts(
    PVECS_SRV_ENUM_CONTEXT pContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;

    switch (pContext->infoLevel)
    {
        case ENTRY_INFO_LEVEL_1:
          dwError = VecsDbEnumInfoLevel1(
                        pContext->pStore->dwStoreId,
                        pContext->dwIndex,
                        pContext->dwLimit,
                        &pCertContainer
                        );
          BAIL_ON_VMAFD_ERROR (dwError);
          break;

        case ENTRY_INFO_LEVEL_2:
          dwError = VecsDbEnumInfoLevel2(
                      pContext->pStore->dwStoreId,
                      pContext->dwIndex,
                      pContext->dwLimit,
                      &pCertContainer
                      );
          BAIL_ON_VMAFD_ERROR(dwError);
          break;

       default:
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppCertContainer = pCertContainer;

cleanup:

    return dwError;
error:
    if (ppCertContainer)
    {
      *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VmAfdFreeCertArray(pCertContainer);
    }

    goto cleanup;
}

DWORD
VecsSrvGetEntryCount(
    PVECS_SERV_STORE pStore,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;

    if (!pStore || !pdwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbGetEntriesCount(
                    pStore->dwStoreId,
                    &dwCount
                    );
    BAIL_ON_VMAFD_ERROR (dwError);
    *pdwSize = dwCount;

cleanup:

    return dwError;
error:
    if (pdwSize)
    {
      *pdwSize = 0;
    }

    goto cleanup;
}

DWORD
VecsSrvGetCertificateByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PWSTR *ppszCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pszCertificate = NULL;

    dwError = VecsDbGetCertificateByAlias(
                        pStore->dwStoreId,
                        pszAliasName,
                        &pszCertificate
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringW(pszCertificate, ppszCertificate);
    BAIL_ON_VMAFD_ERROR (dwError);

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

DWORD
VecsSrvGetPrivateKeyByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PWSTR pszPassword,
    PWSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    PWSTR pszPrivateKey = NULL;
    PWSTR pszDbPrivateKey = NULL;
    CERT_ENTRY_TYPE cEntryType;

    dwError = VecsDbGetPrivateKeyByAlias(
                        pStore->dwStoreId,
                        pszAliasName,
                        pszPassword,
                        &pszDbPrivateKey
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetEntryTypeByAlias(
                                pStore,
                                pszAliasName,
                                &cEntryType);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cEntryType != CERT_ENTRY_TYPE_ENCRYPTED_PRIVATE_KEY)
    {
        dwError = VmAfdAllocateStringW(pszDbPrivateKey, &pszPrivateKey);
    }
    else
    {
        if (IsNullOrEmptyString(pszPassword))
        {
            dwError = ERROR_WRONG_PASSWORD;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VecsDecryptAndFormatKey (
                                pszDbPrivateKey,
                                pszPassword,
                                &pszPrivateKey
                                );
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPrivateKey = pszPrivateKey;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDbPrivateKey);
    return dwError;
error:
    if (*ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszPrivateKey);
    goto cleanup;
}

DWORD
VecsSrvAddCertificate(
    PVECS_SERV_STORE pStore,
    CERT_ENTRY_TYPE cEntryType,
    PWSTR pszAliasName,
    PWSTR pszCertificate,
    PWSTR pszPrivateKey,
    PWSTR pszPassword,
    BOOLEAN bAutoRefresh
    )
{
    DWORD dwError = 0;
    DWORD dwErrorAddToDb = 0;
    PSTR  pszAlias = NULL;
    PWSTR pszAliasToUse = NULL;
    PWSTR pszCanonicalCertPEM = NULL;
    PWSTR pszCanonicalKeyPEM = NULL;

    VmAfdLog(VMAFD_DEBUG_DEBUG, "[%s,%d] Adding cert. Entry type: %d",
        __FILE__, __LINE__, cEntryType);

    dwError = VecsSrvValidateAddEntryInput (
                                         cEntryType,
                                         pszCertificate,
                                         pszPrivateKey
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (
          cEntryType == CERT_ENTRY_TYPE_SECRET_KEY &&
          IsNullOrEmptyString (pszAliasName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszAliasName))
    {
        dwError = VecsComputeCertAliasW(
                                        pszCertificate,
                                        &pszAliasToUse
                                       );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        dwError = VmAfdAllocateStringW(
                                        pszAliasName,
                                        &pszAliasToUse
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsNullOrEmptyString (pszCertificate))
    {
        if (cEntryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST)
        {
            dwError = VecsValidateAndFormatCrl(
                                          pszCertificate,
                                          &pszCanonicalCertPEM
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        else
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
        if (cEntryType == CERT_ENTRY_TYPE_SECRET_KEY)
        {
            dwError = VmAfdAllocateStringW(
                                           pszPrivateKey,
                                           &pszCanonicalKeyPEM
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        else
        {
            dwError = VecsValidateCertKeyPair(
                                    pszCertificate,
                                    pszPrivateKey
                                    );
            BAIL_ON_VMAFD_ERROR (dwError);

            dwError = VecsValidateAndFormatKey(
                                    pszPrivateKey,
                                    pszPassword,
                                    &pszCanonicalKeyPEM
                                  );
            BAIL_ON_VMAFD_ERROR (dwError);
            if (pszPassword)
            {
                cEntryType = CERT_ENTRY_TYPE_ENCRYPTED_PRIVATE_KEY;
            }
        }
    }

    dwErrorAddToDb = VecsDbAddCert(
                  pStore->dwStoreId,
                  cEntryType,
                  pszAliasToUse,
                  pszCanonicalCertPEM,
                  pszCanonicalKeyPEM,
                  NULL,
                  bAutoRefresh
                  );
    if (dwErrorAddToDb != ERROR_ALREADY_EXISTS)
    {
        dwError = dwErrorAddToDb;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pszAliasToUse, &pszAlias);
    BAIL_ON_VMAFD_ERROR (dwError);
    VmAfdLog(VMAFD_DEBUG_ANY, "Added cert to VECS DB: %s", pszAlias);

    if (cEntryType == CERT_ENTRY_TYPE_TRUSTED_CERT
            && pStore->dwStoreId == VECS_TRUSTED_ROOT_STORE_ID)
    {
        dwError = VecsSrvFlushRootCertificate(pStore, pszCanonicalCertPEM,TRUE);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else if (cEntryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST
            && pStore->dwStoreId == VECS_CRL_STORE_ID)
    {
        dwError = VecsSrvFlushCrl(pStore, pszCanonicalCertPEM,TRUE);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
#ifndef _WIN32
    else if (dwErrorAddToDb == ERROR_SUCCESS &&
             pStore->dwStoreId == VECS_MACHINE_SSL_STORE_ID &&
             0 == VmAfdStringCompareA(
                    pszAlias,
                    VECS_MACHINE_CERT_ALIAS,
                    TRUE))
    {
        (DWORD)VecsSrvFlushMachineSslCertificate(
                            pStore,
                            pszCanonicalCertPEM,
                            pszCanonicalKeyPEM,
                            TRUE
                            );
    }
#endif

    dwError = dwErrorAddToDb;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszAlias);
    VMAFD_SAFE_FREE_MEMORY(pszCanonicalKeyPEM);
    VMAFD_SAFE_FREE_MEMORY(pszCanonicalCertPEM);
    VMAFD_SAFE_FREE_MEMORY(pszAliasToUse);
    return dwError;
error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvAddCertificate returning %u", dwError);
    goto cleanup;
}

DWORD
VecsSrvGetEntryTypeByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    CERT_ENTRY_TYPE *pEntryType
    )
{
    DWORD dwError = 0;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    dwError = VecsDbGetEntryTypeByAlias(
                  pStore->dwStoreId,
                  pszAliasName,
                  &entryType
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

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

DWORD
VecsSrvGetEntryDateByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    PDWORD pdwDate
    )
{
    DWORD dwError = 0;
    DWORD dwDate = 0;

    dwError = VecsDbGetEntryDateByAlias(
                  pStore->dwStoreId,
                  pszAliasName,
                  &dwDate
                  );
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

DWORD
VecsSrvGetEntryByAlias(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName,
    ENTRY_INFO_LEVEL infoLevel,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;

    switch (infoLevel)
    {
        case ENTRY_INFO_LEVEL_1:
          dwError = VecsDbGetEntryByAliasInfoLevel1(
                          pStore->dwStoreId,
                          pszAliasName,
                          &pCertContainer
                          );
          break;
        case ENTRY_INFO_LEVEL_2:
          dwError = VecsDbGetEntryByAliasInfoLevel2(
                        pStore->dwStoreId,
                        pszAliasName,
                        &pCertContainer
                        );
          break;
        default:
            dwError = ERROR_INVALID_PARAMETER;
            break;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppCertContainer = pCertContainer;

cleanup:

    return dwError;

error:
    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VecsFreeCertArray (pCertContainer);
    }

    goto cleanup;
}

DWORD
VecsSrvDeleteCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszAliasName
    )
{
    DWORD dwError = 0;
    PSTR  pszAlias = NULL;
    PSTR  pszCert = NULL;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_2;
    PVMAFD_CERT_ARRAY pCertArray = NULL;

    if (!pStore || !pszAliasName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Get the cert so that we have it after it's deleted from VECS DB
    dwError = VecsSrvGetEntryByAlias(pStore, pszAliasName, infoLevel, &pCertArray);
    if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pCertArray || pCertArray->dwCount == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else if (pCertArray->dwCount > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (VecsIsFlushableEntry(
                      pCertArray->certificates[0].dwStoreType,
                      pStore->dwStoreId
                      )
       )
    {
        dwError = VmAfdAllocateStringAFromW(
            (PWSTR)pCertArray->certificates[0].pCert,
            &pszCert);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbDeleteCert(
                    pStore->dwStoreId,
                    pszAliasName
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    VmAfdAllocateStringAFromW(pszAliasName, &pszAlias);

#ifndef _WIN32
    if (pStore->dwStoreId == VECS_MACHINE_SSL_STORE_ID &&
        0 == VmAfdStringCompareA(
                    pszAlias,
                    VECS_MACHINE_CERT_ALIAS,
                    TRUE)
       )
    {
        (DWORD)VecsSrvUnflushMachineSslCertificate(pStore);
    }
#endif

    VmAfdLog(VMAFD_DEBUG_ANY,
        "VecsSrvDeleteCertificate: Deleted cert (alias %s) from store %u",
        pszAlias, pStore->dwStoreId);

    if (!IsNullOrEmptyString(pszCert))
    {
      dwError = VecsSrvUnflushCertificate(pStore, pszCert);
      BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszAlias);
    VMAFD_SAFE_FREE_MEMORY(pszCert);

    if (pCertArray)
    {
        VecsFreeCertArray(pCertArray);
    }
    return dwError;
error:

    goto cleanup;
}



PVECS_SERV_STORE
VecsSrvAcquireCertStore(
    PVECS_SERV_STORE pStore
    )
{
    if (pStore)
    {
        InterlockedIncrement(&pStore->refCount);
    }

    return pStore;
}

VOID
VecsSrvReleaseCertStore(
    PVECS_SERV_STORE pStore
    )
{
    if (pStore && InterlockedDecrement(&pStore->refCount)== 0)
    {
        VecsSrvFreeCertStore (pStore);
    }
}


PVECS_SRV_ENUM_CONTEXT
VecsSrvAcquireEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    )
{
    if (pContext)
    {
        InterlockedIncrement(&pContext->refCount);
    }
    return pContext;
}

VOID
VecsSrvReleaseEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    )
{
    if (pContext && InterlockedDecrement(&pContext->refCount) == 0)
    {
        VecsSrvFreeEnumContext(pContext);
    }
}

DWORD
VecsSrvValidateEntryType(
    UINT32 uInputEntryType,
    CERT_ENTRY_TYPE *pOutputEntryType
    )
{
    DWORD dwError = 0;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    if (!pOutputEntryType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    switch (uInputEntryType)
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
      case 4:
        entryType = CERT_ENTRY_TYPE_REVOKED_CERT_LIST;
        break;
      default:
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
        break;
    }

    *pOutputEntryType = entryType;
cleanup:
    return dwError;
error:
    if (pOutputEntryType)
    {
      *pOutputEntryType = CERT_ENTRY_TYPE_UNKNOWN;
    }
    goto cleanup;
}

DWORD
VecsSrvValidateInfoLevel(
    UINT32 uInfoLevel,
    ENTRY_INFO_LEVEL *pInfoLevel
    )
{
    DWORD dwError = 0;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;

    switch (uInfoLevel)
    {
      case 1:
        infoLevel = ENTRY_INFO_LEVEL_1;
        break;
      case 2:
        infoLevel = ENTRY_INFO_LEVEL_2;
        break;
      default:
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
    }

    *pInfoLevel = infoLevel;
cleanup:
    return dwError;
error:
    *pInfoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
    goto cleanup;
}

DWORD
VecsSrvValidateAddEntryInput(
    CERT_ENTRY_TYPE entryType,
    PCWSTR pwszCertificate,
    PCWSTR pwszPrivateKey
    )
{
    DWORD dwError = 0;

    if (entryType == CERT_ENTRY_TYPE_PRIVATE_KEY &&
        (IsNullOrEmptyString(pwszCertificate) ||
         IsNullOrEmptyString(pwszPrivateKey))
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (entryType == CERT_ENTRY_TYPE_TRUSTED_CERT &&
        IsNullOrEmptyString (pwszCertificate)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (entryType == CERT_ENTRY_TYPE_SECRET_KEY &&
        IsNullOrEmptyString (pwszPrivateKey)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VecsSrvFlushRootCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PWSTR pwszCAPath = NULL;
    PSTR  pszCAPath = NULL;
    PSTR  pszDownloadPath = NULL;
    PSTR  pszCert = NULL;

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pszCanonicalCertPEM, &pszCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Flush to CAPath
    dwError = VecsSrvFlushRoot_SHA_1(pszCert, pszCAPath, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushRoot_MD5(pszCert, pszCAPath, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetVpxDocRootPath(&pszDownloadPath);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvFlushRootCertificate "
            "Failed to get download directory, %u", dwError);
        dwError = 0;
        goto cleanup;
    }

    // Flush to docRoot directory for download
    dwError = VecsSrvFlushRoot_SHA_1(pszCert, pszDownloadPath, bLogOnDuplicate);
    if (dwError)
    {
        if (bLogOnDuplicate)
        {
            VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvFlushRootCertificate "
            "Failed to flush trusted root to download directory, %u", dwError);
        }
        dwError = 0;
        goto cleanup;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszDownloadPath);
    VMAFD_SAFE_FREE_MEMORY(pszCert);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvFlushRootCertificate - %u", dwError);
    goto cleanup;
}

DWORD
VecsSrvFlushMachineSslCertificate(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    PWSTR pszCanonicalKeyPEM,
    BOOL  bLogOnError
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pszMachineSslPath = NULL;
    PSTR  pszCert = NULL;
    PSTR  pszKey = NULL;
    PSTR  pszSslCertPath = NULL;
    PSTR  pszSslKeyPath = NULL;
    PSTR  pszSslCertFingerPrint = NULL;
    PSTR  pszFileCertFingerPrint = NULL;
    BOOLEAN bExists = FALSE;

    if (!pStore ||
        pStore->dwStoreId != VECS_MACHINE_SSL_STORE_ID ||
        !pszCanonicalCertPEM ||
        !pszCanonicalKeyPEM
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvGetMachineSslPathA(&pszMachineSslPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                        &pszSslCertPath,
                        "%s%smachine-ssl.crt",
                        pszMachineSslPath,
                        VMAFD_PATH_SEPARATOR_STR
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pszCanonicalCertPEM, &pszCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdFileExists(pszSslCertPath, &bExists);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bExists)
    {
        dwError = VecsComputeCertAliasA(
                                    pszCert,
                                    &pszSslCertFingerPrint
                                    );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsComputeCertAliasFile(
                                    pszSslCertPath,
                                    &pszFileCertFingerPrint
                                    );
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!VmAfdStringCompareA(pszSslCertFingerPrint, pszFileCertFingerPrint, FALSE))
        {
            goto cleanup;
        }
    }

    dwError = VecsSrvWriteCertStringToDisk(pszCert, pszSslCertPath, bLogOnError, 0644);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                        &pszSslKeyPath,
                        "%s%smachine-ssl.key",
                        pszMachineSslPath,
                        VMAFD_PATH_SEPARATOR_STR
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pszCanonicalKeyPEM, &pszKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteCertStringToDisk(pszKey, pszSslKeyPath, bLogOnError, 0600);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMachineSslPath);
    VMAFD_SAFE_FREE_MEMORY(pszSslCertPath);
    VMAFD_SAFE_FREE_MEMORY(pszCert);
    VMAFD_SAFE_FREE_MEMORY(pszSslKeyPath);
    VMAFD_SAFE_FREE_MEMORY(pszKey);
    VMAFD_SAFE_FREE_MEMORY(pszSslCertFingerPrint);
    VMAFD_SAFE_FREE_MEMORY(pszFileCertFingerPrint);
    return dwError;

error:
    if (bLogOnError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, 
             "VecsSrvFlushMachineSslCertificate returning %u", dwError);
    }
    goto cleanup;
}

DWORD
VecsSrvUnflushMachineSslCertificate(
    PVECS_SERV_STORE pStore
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pszMachineSslPath = NULL;
    PSTR  pszSslCertPath = NULL;
    PSTR  pszSslKeyPath = NULL;
    int   nRetry = VECS_DEL_RETRY_MAX;

    if (!pStore ||
        pStore->dwStoreId != VECS_MACHINE_SSL_STORE_ID
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvGetMachineSslPathA(&pszMachineSslPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                        &pszSslCertPath,
                        "%s%smachine-ssl.crt",
                        pszMachineSslPath,
                        VMAFD_PATH_SEPARATOR_STR
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDeleteFileWithRetry(pszSslCertPath, nRetry);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                        &pszSslKeyPath,
                        "%s%smachine-ssl.key",
                        pszMachineSslPath,
                        VMAFD_PATH_SEPARATOR_STR);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDeleteFileWithRetry(pszSslKeyPath, nRetry);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMachineSslPath);
    VMAFD_SAFE_FREE_MEMORY(pszSslCertPath);
    VMAFD_SAFE_FREE_MEMORY(pszSslKeyPath);
     return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, 
             "VecsSrvUnflushMachineSslCertificate returning %u", dwError);
    goto cleanup;
}

DWORD
VecsSrvFlushCrl(
    PVECS_SERV_STORE pStore,
    PWSTR pszCanonicalCertPEM,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PWSTR pwszCAPath = NULL;
    PSTR  pszCAPath = NULL;
    PSTR  pszDownloadPath = NULL;
    PSTR  pszCrl = NULL;

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pszCanonicalCertPEM, &pszCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Flush to CAPath
    dwError = VecsSrvFlushCrl_SHA_1(pszCrl, pszCAPath, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushCrl_MD5(pszCrl, pszCAPath, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetVpxDocRootPath(&pszDownloadPath);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvFlushCrl "
            "Failed to get download directory, %u", dwError);
        dwError = 0;
        goto cleanup;
    }

    // Flush to docRoot directory for download
    dwError = VecsSrvFlushCrl_SHA_1(pszCrl, pszDownloadPath, bLogOnDuplicate);
    if (dwError)
    {
        if (bLogOnDuplicate)
        {
            VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvFlushCrl "
                "Failed to flush CRL to download directory, %u", dwError);
        }
        dwError = 0;
        goto cleanup;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszDownloadPath);
    VMAFD_SAFE_FREE_MEMORY(pszCrl);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    return dwError;
error:

    goto cleanup;
}

DWORD
VecsFillVacantFileSlot(
    PCSTR pszPath
    )
{
    DWORD dwError = 0;
    PSTR pCursor = NULL;
    PSTR pFileStart = NULL;
    PSTR pszEnd = NULL;
    PSTR pszFilename = NULL;
    PSTR pszMaxIndexFile = NULL;
    PSTR pszCAPath = NULL;
    PWSTR pwszCAPath = NULL;
    int maxIndex = 0;
    int index = 0;
    DWORD dwFileNameLen = 0;
    DWORD dwCAPathLength = 0;
    BOOLEAN bCrl = FALSE;
    BOOLEAN bExist = FALSE;

    if (IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VmAfdLog(VMAFD_DEBUG_DEBUG,
            "VecsFillVacantFileSlot: vacant slot %s.", pszPath);

    dwError = VmAfdFileExists(pszPath, &bExist);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bExist)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
            "VecsFillVacantFileSlot: cert file: %s exists so return",
            pszPath);
        goto cleanup;
    }

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwCAPathLength = VmAfdStringLenA(pszCAPath);

    if (VmAfdStringLenA(pszPath) < dwCAPathLength + 2)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCursor = VmAfdStringChrA(pszPath, '.');
    if (pCursor == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pFileStart = (PSTR)(pszPath) + dwCAPathLength;
    if (
#ifndef _WIN32
        *pFileStart == '/'
#else
        *pFileStart == '\\'
#endif
        )
    {
        pFileStart++;
    }

    dwFileNameLen = pCursor - pFileStart;
    if (dwFileNameLen == 0)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory((dwFileNameLen + 1) * sizeof(CHAR),
                (PVOID*)&pszFilename);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringNCpyA(pszFilename, dwFileNameLen + 1,
                pFileStart, dwFileNameLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Extension name must exist
    if (VmAfdStringLenA(pFileStart) < dwFileNameLen + 2)
    {
        VmAfdLog(VMAFD_DEBUG_DEBUG,
                "VecsFillVacantFileSlot: missing extension name in %s",
                pFileStart);
        dwError = ERROR_BAD_FILE_TYPE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bCrl = *(pCursor + 1) == 'r';

    dwError = VmAfdFindFileIndex(pszCAPath, pszFilename, bCrl, &maxIndex);
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        VmAfdLog(VMAFD_DEBUG_DEBUG,
            "VecsFillVacantFileSlot: No more file of same hash name, return.");
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_DEBUG,
            "VecsFillVacantFileSlot: max index for %s is %u",
            pszFilename, maxIndex);

    index = strtol(bCrl ? pCursor + 2 : pCursor + 1, &pszEnd, 10);
    if (!pszEnd || (*pszEnd != '\0'))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (index < maxIndex)
    {
        dwError = VmAfdAllocateStringPrintf(
                        &pszMaxIndexFile,
                        bCrl ? "%s%s%s.r%ld" : "%s%s%s.%ld",
                        pszCAPath,
                        VMAFD_PATH_SEPARATOR_STR,
                        pszFilename,
                        maxIndex);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdCopyFile(pszMaxIndexFile, pszPath);
        BAIL_ON_VMAFD_ERROR(dwError);

        VmAfdLog(VMAFD_DEBUG_ANY,
            "VecsFillVacantFileSlot: copied %s to %s", pszMaxIndexFile, pszPath);

        dwError = VecsDeleteFileWithRetry(pszMaxIndexFile, VECS_DEL_RETRY_MAX);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VecsFillVacantFileSlot(pszMaxIndexFile);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = VecsWriteCleanupFlagFile(pszCAPath, pszMaxIndexFile);
        }
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMaxIndexFile);
    VMAFD_SAFE_FREE_MEMORY(pszFilename);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_DEBUG,
        "VecsFillVacantFileSlot: returning %u", dwError);
    goto cleanup;
}

DWORD
VecsSrvFlushSSLCertFromDB(
    BOOL bLogOnError
    )
{
    DWORD dwError = 0;
    WCHAR pwszMachineSSLStoreName[] = SYSTEM_CERT_STORE_NAME_W;
    WCHAR pwszMachineSSLCertAlias[] = VECS_MACHINE_CERT_ALIAS_W;
    PVECS_SERV_STORE pStore = NULL;
    PVMAFD_CERT_ARRAY pSSLCertificate = NULL;
    PWSTR pwszSSLKey = NULL;

    dwError = VecsSrvOpenCertStore(
                    pwszMachineSSLStoreName,
                    NULL,
                    &pStore
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetEntryByAlias(
                                pStore,
                                pwszMachineSSLCertAlias,
                                ENTRY_INFO_LEVEL_2,
                                &pSSLCertificate
                                );

    if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        //There is nothing in the DB to flush
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetPrivateKeyByAlias(
                                      pStore,
                                      pwszMachineSSLCertAlias,
                                      NULL,
                                      &pwszSSLKey
                                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushMachineSslCertificate(
                                        pStore,
                                        pSSLCertificate->certificates[0].pCert,
                                        pwszSSLKey,
                                        bLogOnError
                                        );
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:

    if (pSSLCertificate)
    {
        VecsFreeCertArray(pSSLCertificate);
    }
    VMAFD_SAFE_FREE_MEMORY(pwszSSLKey);
    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
VecsSrvFlushCertsToDisk(
    VOID
    )
{
    DWORD dwError = 0;
    WCHAR trustedRootsStoreName[] = TRUSTED_ROOTS_STORE_NAME_W;
    WCHAR crlStoreName[] = CRL_STORE_NAME_W;

    dwError = VecsSrvFlushCertsFromDB(
                              trustedRootsStoreName,
                              CERT_ENTRY_TYPE_TRUSTED_CERT
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushCertsFromDB(
                              crlStoreName,
                              CERT_ENTRY_TYPE_REVOKED_CERT_LIST
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushSSLCertFromDB(TRUE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
VOID
VecsSrvFreeCertStore(
    PVECS_SERV_STORE pStore
    )
{
    if (pStore)
    {
        VMAFD_SAFE_FREE_MEMORY(pStore);
    }
}

static
VOID
VecsSrvFreeEnumContext(
    PVECS_SRV_ENUM_CONTEXT pContext
    )
{
    if (pContext->pStore)
    {
        VecsSrvReleaseCertStore(pContext->pStore);
    }
    VmAfdFreeMemory(pContext);
}

static
BOOL
VecsIsFlushableEntry(
    CERT_ENTRY_TYPE cEntryType,
    DWORD           dwStoreId
    )
{
    BOOL bToFlush = FALSE;

    if (cEntryType == CERT_ENTRY_TYPE_TRUSTED_CERT &&
        dwStoreId == VECS_TRUSTED_ROOT_STORE_ID)
    {
        bToFlush = TRUE;
    }
    else if(cEntryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST &&
         dwStoreId == VECS_CRL_STORE_ID)
    {
        bToFlush = TRUE;
    }

    return bToFlush;
}


static
DWORD
VecsSrvFlushRoot_MD5(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCertHash_MD5((PSTR)pszCertificate, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteRootToDisk(
                            pszCertificate,
                            pszCAPath,
                            pszHash,
                            bLogOnDuplicate
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsSrvFlushRoot_SHA_1(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCertHash_SHA_1((PSTR)pszCertificate, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteRootToDisk(
                          pszCertificate,
                          pszCAPath,
                          pszHash,
                          bLogOnDuplicate
                          );
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsSrvFlushCrl_MD5(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCrlAuthorityHash_MD5((PSTR)pszCrl, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteCrlToDisk(pszCrl, pszCAPath, pszHash, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsSrvFlushCrl_SHA_1(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCrlAuthorityHash_SHA_1((PSTR)pszCrl, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteCrlToDisk(pszCrl, pszCAPath, pszHash, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsCheckCertOnDisk(
    PCSTR    pszAlias,
    PCSTR    pszCAPath,
    PCSTR    pszFilename,
    LONG     maxIndex,
    BOOLEAN  bCrl,
    LONG*    pNextAvailable,
    PSTR*    ppszMatchedFile
    )
{
    DWORD dwError = 0;
    LONG  index = 0;
    PSTR  pszMatchedFile = FALSE;
    PSTR  pszPath = NULL;
    PSTR  pszAliasOther = NULL;
    BOOLEAN bExist = FALSE;
    LONG nextAvailable = -1;

    // Note : maxIndex starts from 0
    for (; !pszMatchedFile && (index <= maxIndex); index++)
    {
        VMAFD_SAFE_FREE_MEMORY(pszPath);

        dwError = VmAfdAllocateStringPrintf(
                        &pszPath,
                        bCrl ? "%s%s%s.r%ld" : "%s%s%s.%ld",
                        pszCAPath,
                        VMAFD_PATH_SEPARATOR_STR,
                        pszFilename,
                        index);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdFileExists(pszPath, &bExist);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!bExist)
        {
            VmAfdLog(VMAFD_DEBUG_DEBUG,
                "VecsCheckCertOnDisk: found vacant index: %d", index);
            // Allow vacant spot.
            // However we do patch vacant spot after deleting a file
            // by moving the max index file to fill the vacant
            if (nextAvailable < 0)
            {
                nextAvailable = index;
            }
            continue;
        }

        VMAFD_SAFE_FREE_MEMORY(pszAliasOther);

        if (bCrl)
        {
            dwError = VecsComputeCrlAliasFromFile(pszPath, &pszAliasOther);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = VecsComputeCertAliasFile(pszPath, &pszAliasOther);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (!VmAfdStringCompareA(pszAlias, pszAliasOther, FALSE))
        {
            pszMatchedFile = pszPath;
            pszPath = NULL;
        }
    }

    if (nextAvailable < 0)
    {
        nextAvailable = maxIndex + 1;
    }

    *pNextAvailable = nextAvailable;
    *ppszMatchedFile = pszMatchedFile;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPath);
    VMAFD_SAFE_FREE_MEMORY(pszAliasOther);

    return dwError;

error:

    *ppszMatchedFile = FALSE;

    goto cleanup;
}

static
DWORD
VecsDeleteFileWithRetry(
    PCSTR pszPath,
    int nRetry
    )
{
    DWORD dwError = 0;
    while (nRetry > 0)
    {
        dwError = VmAfdDeleteFile(pszPath);
        if (dwError == ERROR_SUCCESS)
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                "VecsDeleteFileWithRetry: successfully deleted cert file: %s",
                pszPath);
            break;
        }
        VmAfdLog(VMAFD_DEBUG_ANY,
            "VecsDeleteFileWithRetry: cannot delete cert file: %s, will retry.",
            pszPath);
        VmAfdSleep(VECS_DEL_RETRY_INTV);
        --nRetry;
    }

    return dwError;
}

static
DWORD
VecsWriteCleanupFlagFile(
    PCSTR pszCAPath,
    PCSTR pszSrcPath
    )
{
    DWORD dwError = 0;
    PSTR  pszFlagFile = NULL;
    PSTR  pszFlagFilePath = NULL;

    // Write an indicator for fetch thread to clean up later
    dwError = VmAfdAllocateStringPrintf(&pszFlagFile, "%s%s",
        VECS_DEL_FILE_PREFIX, pszSrcPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsGetFlushedFileFullPath(pszCAPath,
        pszFlagFile, &pszFlagFilePath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSaveStringToFile(pszFlagFilePath, "");
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_DEBUG, "Wrote a flag file for future clean up of %s.",
                pszSrcPath);
#ifndef _WIN32
    if(chmod(pszFlagFilePath,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
#endif
cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszFlagFilePath);
    VMAFD_SAFE_FREE_MEMORY(pszFlagFile);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VecsSrvDeleteRootFromDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bCrl
    )
{
    DWORD dwError = 0;
    int   nRetry = VECS_DEL_RETRY_MAX;
    LONG  maxIndex = -1;
    LONG  vacantIndex = -1;
    PSTR  pszAlias = NULL;
    PSTR  pszMatchedFile = NULL;

    dwError = VmAfdFindFileIndex(pszCAPath, pszFilename, bCrl, &maxIndex);
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        VmAfdLog(VMAFD_DEBUG_DEBUG,
            "VecsSrvDeleteRootFromDisk: no match for name hash %s, return.",
            pszFilename);
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bCrl)
    {
        dwError = VecsComputeCrlAliasA((PSTR)pszCertificate, &pszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VecsComputeCertAliasA((PSTR)pszCertificate, &pszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsCheckCertOnDisk(
                    pszAlias,
                    pszCAPath,
                    pszFilename,
                    maxIndex,
                    bCrl,
                    &vacantIndex,
                    &pszMatchedFile);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszMatchedFile)
    {
        VmAfdLog(VMAFD_DEBUG_DEBUG, "VecsSrvDeleteRootFromDisk: cert not found.");
        goto cleanup;
    }

    dwError = VecsDeleteFileWithRetry(pszMatchedFile, nRetry);
    if (dwError != ERROR_SUCCESS)
    {
        // Write an indicator for fetch thread to clean up later
        VecsWriteCleanupFlagFile(pszCAPath, pszMatchedFile);
        // Reset error because we will clean up later.
        dwError = 0;
    }
    else
    {
        // Index compacting issues should not abort cert deletion
        VecsFillVacantFileSlot(pszMatchedFile);
    }

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszMatchedFile);
    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR,
        "VecsSrvDeleteRootFromDisk: returning %u.", dwError);
    goto cleanup;
}

static
DWORD
VecsSrvWriteCertStringToDisk(
    PCSTR pszCertificate,
    PCSTR pszFilePath,
    BOOL bLogOnDuplicate,
    int mode
)
{
    DWORD dwError = 0;
    FILE* pFile = NULL;
    size_t bytesToWrite = 0;
    PCSTR pszCursor = NULL;
    size_t len = 0;

    dwError = VmAfdOpenFilePath(pszFilePath, "w", &pFile, mode);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    len = strlen(pszCertificate);
    bytesToWrite = len;
    pszCursor = pszCertificate;

    while (bytesToWrite > 0)
    {
        size_t bytesWritten = 0;

        if ((bytesWritten = fwrite(pszCursor, 1, len, pFile)) == 0)
        {
#ifndef _WIN32
            dwError = LwErrnoToWin32Error(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszCursor += bytesWritten;
        bytesToWrite -= bytesWritten;
    }

    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if (bLogOnDuplicate)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
            "VecsSrvWriteCertStringToDisk: successfully written file %s.",
            pszFilePath);
    }

cleanup:
    return dwError;

error:
    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if (bLogOnDuplicate)
    {
        VmAfdLog(
              VMAFD_DEBUG_ANY,
              "VecsSrvWriteCertStringToDisk: returning %u.", dwError
              );
    }

    goto cleanup;
}

static
DWORD
VecsSrvWriteCertOrCrlToDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bCrl,
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    LONG  maxIndex = -1;
    LONG  nextAvailable = -1;
    PSTR  pszPath = NULL;
    PSTR  pszAlias = NULL;
    PSTR  pszMatchedFile = NULL;

    dwError = VmAfdFindFileIndex(pszCAPath, pszFilename, bCrl, &maxIndex);
    if (dwError != ERROR_FILE_NOT_FOUND)
    {
        BAIL_ON_VMAFD_ERROR(dwError);

        VmAfdLog(VMAFD_DEBUG_DEBUG,
            "VecsSrvWriteCertOrCrlToDisk: found files of same name hash %s.",
            pszFilename);

        if (bCrl)
        {
            dwError = VecsComputeCrlAliasA((PSTR)pszCertificate, &pszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = VecsComputeCertAliasA((PSTR)pszCertificate, &pszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VecsCheckCertOnDisk(
                        pszAlias,
                        pszCAPath,
                        pszFilename,
                        maxIndex,
                        bCrl,
                        &nextAvailable,
                        &pszMatchedFile);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (pszMatchedFile)
        {
            if (bLogOnDuplicate)
            {
                VmAfdLog(VMAFD_DEBUG_ANY,
                  "VecsSrvWriteCertOrCrlToDisk: cert/crl already exists as %s"
                  ", so will not write again.", pszMatchedFile);
            }
            goto cleanup;
        }
    }

    if (nextAvailable < 0)
    {
        nextAvailable = maxIndex + 1;
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszPath,
                    bCrl ? "%s%s%s.r%ld" : "%s%s%s.%ld",
                    pszCAPath,
                    VMAFD_PATH_SEPARATOR_STR,
                    pszFilename,
                    nextAvailable);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvWriteCertStringToDisk(pszCertificate, pszPath, bLogOnDuplicate, 0644);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPath);
    VMAFD_SAFE_FREE_MEMORY(pszAlias);
    VMAFD_SAFE_FREE_MEMORY(pszMatchedFile);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
VecsSrvWriteRootToDisk(
    PCSTR pszCert,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bLogOnDuplicate
    )
{
    return VecsSrvWriteCertOrCrlToDisk(
                                pszCert,
                                pszCAPath,
                                pszFilename,
                                FALSE,
                                bLogOnDuplicate
                                );
}

static
DWORD
VecsSrvWriteCrlToDisk(
    PCSTR pszCrl,
    PCSTR pszCAPath,
    PCSTR pszFilename,
    BOOLEAN bLogOnDuplicate
    )
{
    return VecsSrvWriteCertOrCrlToDisk(
                                  pszCrl,
                                  pszCAPath,
                                  pszFilename,
                                  TRUE,
                                  FALSE
                                  );
}

static
DWORD
VecsGetFlushedFileFullPath(
    PCSTR pszCAPath,
    PCSTR pszFileName,
    PSTR* ppszFullPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszPath = NULL;

    dwError = VmAfdAllocateStringPrintf(
                    &pszPath,
                    "%s%s%s",
                    pszCAPath,
                    VMAFD_PATH_SEPARATOR_STR,
                    pszFileName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszFullPath = pszPath;

cleanup:
    return dwError;

error :
    VMAFD_SAFE_FREE_MEMORY(pszPath);
    if (ppszFullPath)
    {
        ppszFullPath = NULL;
    }
    goto cleanup;
}

static
DWORD
VecsSrvUnflushCertificate(
    PVECS_SERV_STORE pStore,
    PSTR pszCert
    )
{
    DWORD dwError = 0;

    PWSTR pwszCAPath = NULL;
    PSTR  pszCAPath = NULL;
    PSTR  pszDownloadPath = NULL;
    PSTR  pszHashMD5  = NULL;
    PSTR  pszHashSHA1 = NULL;
    BOOLEAN bCrl = FALSE;

    if (!pStore || IsNullOrEmptyString(pszCert))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    bCrl = pStore->dwStoreId == VECS_CRL_STORE_ID;
    if (bCrl)
    {
        dwError = VecsComputeCrlAuthorityHash_MD5(pszCert, &pszHashMD5);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VecsComputeCertHash_MD5(pszCert, &pszHashMD5);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvDeleteRootFromDisk(pszCert, pszCAPath, pszHashMD5, bCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bCrl)
    {
        dwError = VecsComputeCrlAuthorityHash_SHA_1(pszCert, &pszHashSHA1);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VecsComputeCertHash_SHA_1(pszCert, &pszHashSHA1);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvDeleteRootFromDisk(pszCert, pszCAPath, pszHashSHA1, bCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvGetVpxDocRootPath(&pszDownloadPath);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvUnflushCertificate "
            "Failed to get download directory, %u", dwError);
        dwError = 0;
        goto cleanup;
    }

    dwError = VecsSrvDeleteRootFromDisk(pszCert, pszDownloadPath, pszHashSHA1, bCrl);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvUnflushCertificate "
            "Failed to delete %s from download directory, %u",
            bCrl ? "crl" : "trusted root", dwError);
        dwError = 0;
        goto cleanup;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDownloadPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszHashMD5);
    VMAFD_SAFE_FREE_MEMORY(pszHashSHA1);
    return dwError;
error:

    goto cleanup;
}

static
DWORD
VecsSrvGetVpxDocRootPath(
    PSTR* ppszDocRoot
    )
{
    DWORD dwError = 0;
    PSTR  pszTempPath = VMAFD_TRUSTED_ROOT_DOWNLOAD_PATH;
    PSTR  pszDownloadPath = NULL;
#ifdef _WIN32
    DWORD dwDownloadPathLength = 0;
#endif

    if (!ppszDocRoot)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifdef _WIN32
    dwDownloadPathLength = ExpandEnvironmentStringsA(pszTempPath, NULL, 0);
    dwError = VmAfdAllocateMemory(
                    dwDownloadPathLength * sizeof(CHAR),
                    &pszDownloadPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ExpandEnvironmentStringsA(pszTempPath,
                pszDownloadPath, dwDownloadPathLength) == 0)
    {
        dwError = GetLastError();
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvGetVpxDocRootPath "
            "unable to expand config path, error %u.", dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!VmAfdStringCompareA(pszTempPath, pszDownloadPath, FALSE))
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvGetVpxDocRootPath "
            "Environment variable for config path not defined.");
        dwError = ERROR_BAD_CONFIGURATION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszDocRoot = pszDownloadPath;
#else
    dwError = VmAfdAllocateStringA(pszTempPath, &pszDownloadPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDocRoot = pszDownloadPath;

#endif

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszDownloadPath);
    if (ppszDocRoot)
    {
        *ppszDocRoot = NULL;
    }

    goto cleanup;
}

static
DWORD
VecsSrvGetMachineSslPathA(
    PSTR* ppszFilePath
    )
{
    DWORD dwError = 0;
    PSTR  pszTempPath = VMAFD_MACHINE_SSL_PATH;
    PSTR  pszMachineSslFilePath = NULL;

#ifdef _WIN32
    DWORD dwSslPathLength = 0;

    dwSslPathLength = ExpandEnvironmentStringsA(pszTempPath, NULL, 0);
    dwError = VmAfdAllocateMemory(
                    dwSslPathLength * sizeof(CHAR),
                    &pszMachineSslFilePath);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ExpandEnvironmentStringsA(pszTempPath,
                pszMachineSslFilePath, dwSslPathLength) == 0)
    {
        dwError = GetLastError();
        VmAfdLog(VMAFD_DEBUG_ERROR, "VecsSrvGetMachineSslPathA "
            "unable to expand config path, error %u.", dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszFilePath = pszMachineSslFilePath;
#else
    dwError = VmAfdAllocateStringA(pszTempPath, &pszMachineSslFilePath);
    BAIL_ON_VMAFD_ERROR(dwError);
    *ppszFilePath = pszMachineSslFilePath;
#endif

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszMachineSslFilePath);
    if (ppszFilePath)
    {
        *ppszFilePath = NULL;
    }

    goto cleanup;
}

static
DWORD
VecsSrvFlushCertsFromDB(
    PWSTR pwszStoreName,
    CERT_ENTRY_TYPE entryType
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVECS_SERV_STORE pStore = NULL;
    PVECS_SRV_ENUM_CONTEXT pEnumContext = NULL;
    PVMAFD_CERT_ARRAY pVecsCertContainer = NULL;

    if (IsNullOrEmptyString(pwszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvOpenCertStore(
                    pwszStoreName,
                    NULL,
                    &pStore
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvAllocateCertEnumContext(
                    pStore,
                    0, /* default */
                    ENTRY_INFO_LEVEL_2,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvEnumCerts(pEnumContext, &pVecsCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (entryType == CERT_ENTRY_TYPE_TRUSTED_CERT)
    {
        for (; dwIndex < pVecsCertContainer->dwCount; ++dwIndex)
        {
            dwError = VecsSrvFlushRootCertificate(
                                       pStore,
                                       pVecsCertContainer->certificates[dwIndex].pCert,
                                       FALSE
                                       );
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    else if (entryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST)
    {
        for (; dwIndex < pVecsCertContainer->dwCount; ++dwIndex)
        {
            dwError = VecsSrvFlushCrl(
                                 pStore,
                                 pVecsCertContainer->certificates[dwIndex].pCert,
                                 FALSE
                                 );
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

cleanup:

    if (pEnumContext)
    {
        VecsSrvReleaseEnumContext(pEnumContext);
    }
    if (pVecsCertContainer)
    {
        VecsFreeCertArray(pVecsCertContainer);
    }
    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }
    return dwError;
error:

    goto cleanup;
}


