#include "includes.h"

DWORD
VecsInsertCertificate(PVECS_DB_CERTIFICATE_ENTRY pEntry)
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pContext = NULL;

    ENTER_LOG();
    dwError = VecsDbCreateContext(&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbAddCertificate(pContext,pEntry);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    if (pContext != NULL)
    {
        VecsDbReleaseContext(pContext);
    }
    EXIT_LOG();
    return dwError;
}

DWORD
VecsGetCertificatebyAlias(
    PCWSTR pwszAlias,
    PDWORD pdwCount,
    PVMAFD_CERT_CONTAINER *pContainer
)
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PVECS_DB_CONTEXT pContext = NULL;
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;
    ENTER_LOG();

    if (IsNullOrEmptyString(pwszAlias))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsDbCreateContext(&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError =  VecsDbQueryCertificateByAlias(
                   pContext,
                   &pCertEntryArray,
                   &dwCount,
                   pwszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(dwCount == 0)
    {
        // nothing to do, not cert found
        *pdwCount = dwCount;
        dwError = 0;
        goto cleanup;
    }

    // There is more than one certificate
    // with the same alias Flag an Error
    if(dwCount > 1)
    {
        dwError = VECS_UNIQUE_ALIAS_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(sizeof(VMAFD_CERT_CONTAINER),
                                  (PVOID*)&pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCertContainer->dwStoreType = pCertEntryArray[0].dwStoreType;

    if (pCertEntryArray[0].pPrivateKey != NULL )
    {
        dwError = VmAfdAllocateStringW(
                  (PCWSTR)pCertEntryArray[0].pPrivateKey,
                  &pCertContainer->pPrivateKey);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pCertEntryArray[0].pCertBlob != NULL)
    {
        dwError = VmAfdAllocateStringW(
                  (PCWSTR) pCertEntryArray[0].pCertBlob,
                  &pCertContainer->pCert);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pContainer = pCertContainer;
    *pdwCount = dwCount;

cleanup:
    if (pContext != NULL)
    {
        VecsDbReleaseContext(pContext);
        pContext = NULL;
    }

    if (pCertEntryArray)
    {
        VecsDbFreeCertEntryArray(pCertEntryArray, dwCount);
    }
    EXIT_LOG();
    return dwError;

error:

    *pContainer = NULL;
    *pdwCount = 0;

    if (pCertContainer)
    {
        VecsSrvFreeCertContainer(pCertContainer);
    }

    goto cleanup;
}

DWORD
VecsAddCertificateInternal(
    UINT32 dwStoreType,
    PWSTR pwszAlias,
    PWSTR pwszCertificate,
    PWSTR pwszPrivateKey,
    UINT32 uAutoRefresh
    )
{
    DWORD dwError = 0;
    VECS_DB_CERTIFICATE_ENTRY Entry = { 0 };
    PSTR pszMachineAlias = NULL;
    DWORD dwCertCount = 0;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;

    size_t nCertLen = 0;
    size_t nKeyLen = 0;

    if(IsNullOrEmptyString(pwszAlias) ||
       IsNullOrEmptyString(pwszCertificate))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // If it is not trusted Roots, we better have Private key,
    // Otherwise reject the Add Cert Call.
    if (dwStoreType != CERTIFICATE_STORE_TYPE_TRUSTED_ROOTS)
    {
        if (IsNullOrEmptyString(pwszPrivateKey))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = VmAfdAllocateStringAFromW(pwszAlias,&pszMachineAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    // if user tries to give us a certificate called
    // __MACHINE_CERT reject it
    if(VmAfdStringCompareA( pszMachineAlias,
                                VECS_MACHINE_CERT_ALIAS, FALSE) == 0)
    {
        dwError = VECS_UNIQUE_ALIAS_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Favoring Java, and being more restrictive here
    // Do not allow duplicate aliases at all.

    dwError = VecsGetCertificatebyAlias(pwszAlias,
                                        &dwCertCount,
                                        &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (dwCertCount != 0) {
        dwError = VECS_UNIQUE_ALIAS_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW((PCWSTR)pwszCertificate, &nCertLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwStoreType != CERTIFICATE_STORE_TYPE_TRUSTED_ROOTS)
    {
        dwError = VmAfdGetStringLengthW((PCWSTR)pwszPrivateKey, &nKeyLen);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        pwszPrivateKey = NULL;
    }

    Entry.pwszAlias = pwszAlias;
    Entry.pCertBlob = (PBYTE) pwszCertificate;
    Entry.dwCertSize = nCertLen * sizeof(wchar16_t);
    Entry.dwStoreType = dwStoreType;
    Entry.pPrivateKey = (PBYTE) pwszPrivateKey;
    Entry.dwKeySize = nKeyLen  * sizeof(wchar16_t);

    dwError = VecsInsertCertificate(&Entry);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMachineAlias);

    VecsSrvFreeCertContainer(pCertContainer);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VecsAddCertificateInternal failed. Error(%u)",
                              dwError);
    goto cleanup;
}


DWORD
VecsDeleteCertificateByAlias(PCWSTR pwszAlias)
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pContext = NULL;
    ENTER_LOG();

    dwError = VecsDbCreateContext(&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbDeleteCertificate(pContext, pwszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    if (pContext != NULL)
    {
        VecsDbReleaseContext(pContext);
        pContext = NULL;
    }
    EXIT_LOG();
    return dwError;
}


DWORD
VecsEnumCertificates(
    DWORD dwStartIndex,
    DWORD dwNumCertificates,
    CERTIFICATE_STORE_TYPE dwStatus,
    PVMAFD_CERT_ARRAY* ppCertArray
)
{

    DWORD dwError = 0;
    PVECS_DB_CONTEXT pContext = NULL;
    PVMAFD_CERT_ARRAY pCertEntryArray = NULL;
    PVECS_DB_CERTIFICATE_ENTRY  pDbCertEntryArray = NULL;
    DWORD dwCount = 0;
    DWORD uCounter = 0;
    ENTER_LOG();

    dwError = VecsDbCreateContext(&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);


    // TODO: add status filter
    dwError = VecsDbQueryCertificatesPaged(
                  pContext,
                  dwStartIndex,
                  dwNumCertificates,
                  dwStatus,
                  &pDbCertEntryArray,
                  &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);


    // Allocate Memory for the Array
    dwError = VmAfdAllocateMemory(sizeof(VMAFD_CERT_ARRAY),
                                 (PVOID*)&pCertEntryArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    memset(pCertEntryArray, 0, sizeof(VMAFD_CERT_ARRAY));
    pCertEntryArray->dwCount = dwCount;

    dwError = VmAfdAllocateMemory(sizeof(VMAFD_CERT_CONTAINER) * (dwCount + 1) ,
                                 (PVOID*)&pCertEntryArray->certificates);
    BAIL_ON_VMAFD_ERROR(dwError);

    memset(pCertEntryArray->certificates, 0, sizeof(VMAFD_CERT_CONTAINER) * (dwCount + 1 ));



    // Allocate and Copy each String

    for( uCounter = 0 ; uCounter < dwCount; uCounter ++)
    {
        if (pDbCertEntryArray[uCounter].pCertBlob != NULL)
        {
            dwError = VmAfdAllocateStringW(
                          (PWSTR) pDbCertEntryArray[uCounter].pCertBlob,
                          &pCertEntryArray->certificates[uCounter].pCert);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pDbCertEntryArray[uCounter].pwszAlias != NULL)
        {
            dwError = VmAfdAllocateStringW(
                          pDbCertEntryArray[uCounter].pwszAlias,
                          &pCertEntryArray->certificates[uCounter].pAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pCertEntryArray->certificates[uCounter].dwStoreType =
            pDbCertEntryArray[uCounter].dwStoreType;

        if (pDbCertEntryArray[uCounter].pwszPassword != NULL)
        {
            dwError = VmAfdAllocateStringW(
                          pDbCertEntryArray[uCounter].pwszPassword,
                          &pCertEntryArray->certificates[uCounter].pPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
        }


        if (pDbCertEntryArray[uCounter].pPrivateKey != NULL)
        {
            dwError = VmAfdAllocateStringW(
                          (PWSTR) pDbCertEntryArray[uCounter].pPrivateKey,
                          &pCertEntryArray->certificates[uCounter].pPrivateKey);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

    }
    *ppCertArray = pCertEntryArray;


cleanup :
    if (pDbCertEntryArray)
    {
        VecsDbFreeCertEntryArray(pDbCertEntryArray, dwCount);
    }


    if (pContext != NULL)
    {
        VecsDbReleaseContext(pContext);
        pContext = NULL;
    }

    EXIT_LOG();
    return dwError;

error:
    if (pCertEntryArray != NULL)
    {
        VecsFreeCertArray(pCertEntryArray);
    }

    goto cleanup;
}

#ifdef _WIN32
DWORD
VecsGetWindowsFileCount(
    PSTR pszDirPath,
    DWORD *pdwCount
)
{

    DWORD dwError = 0;

    DWORD dwFileCount = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd = { 0 };
    PSTR pszFileSearchPattern = NULL;
    PWSTR pwszSearchPattern = NULL;

    if ( IsNullOrEmptyString(pszDirPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAVsnprintf(&pszFileSearchPattern,"%s\\*", pszDirPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA( pszFileSearchPattern, &pwszSearchPattern);
    BAIL_ON_VMAFD_ERROR(dwError);

    hFind = FindFirstFile(pwszSearchPattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE )
    {
        dwError = 0;
        *pdwCount = 0;
        goto cleanup;
    }

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }
        else
        {
            dwFileCount++;
        }
    }
    while (FindNextFile(hFind, &ffd) != 0);

    // Let the user know if we encountered some sort of Error
    dwError = GetLastError();
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwCount = dwFileCount;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszFileSearchPattern);
    VMAFD_SAFE_FREE_MEMORY(pwszSearchPattern);

    if(hFind != INVALID_HANDLE_VALUE )
    {
        FindClose(hFind);
    }
    return dwError;

error :

    goto cleanup;

}


DWORD
VecsGetWindowsCRLData(
    PSTR    pszDirPath,
    DWORD   dwCount,
    PVMAFD_CRL_FILE_METADATA* ppMetaData,
    PDWORD pdwAvailable
)
{
    DWORD dwError = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd = { 0 };
    PSTR pszCRLFileName = NULL;
    X509_CRL *pCRL = NULL;
    PVMAFD_CRL_FILE_METADATA pMetaData = NULL;
    PSTR pszFileSearchPattern = NULL;
    PWSTR pwszSearchPattern = NULL;
    DWORD nIndex = 0;

    if (!ppMetaData || IsNullOrEmptyString(pszDirPath) || !dwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CRL_FILE_METADATA) * dwCount,
                    (PVOID*)&pMetaData);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAVsnprintf(&pszFileSearchPattern,"%s\\*", pszDirPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszFileSearchPattern, &pwszSearchPattern);
    BAIL_ON_VMAFD_ERROR(dwError);


    hFind = FindFirstFile(pwszSearchPattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE )
    {
        dwError = 0;
        goto cleanup;
    }

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }
        else
        {
            PVMAFD_CRL_FILE_METADATA pCursor = &pMetaData[nIndex];

            dwError = VmAfdAllocateStringW(
                            ffd.cFileName,
                            &pCursor->pwszCrlFileName);
            BAIL_ON_VMAFD_ERROR(dwError);

            VMAFD_SAFE_FREE_MEMORY(pszCRLFileName);

            dwError = VmAfdAllocateStringAVsnprintf(
                            &pszCRLFileName,
                            "%s\\%S",
                            pszDirPath,
                            ffd.cFileName);
            BAIL_ON_VMAFD_ERROR(dwError);

            if (pCRL != NULL)
            {
                VmAfdCloseCRL(pCRL);
                pCRL = NULL;
            }

            dwError = VmAfdOpenCRL( pszCRLFileName, &pCRL );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLVersion( pCRL, (DWORD *)&pCursor->dwCRLNumber);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLName(pCRL,&pCursor->pwszIssuerName);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLLastUpdate(pCRL, &pCursor->pwszLastUpdate);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLNextUpdate(pCRL, &pCursor->pwszNextUpdate);
            BAIL_ON_VMAFD_ERROR(dwError);

            nIndex++;
        }
    }
    while ((FindNextFile(hFind, &ffd) != 0) &&  (nIndex < dwCount));

    *ppMetaData = pMetaData;
    *pdwAvailable = nIndex;

cleanup:
    if (pCRL != NULL)
    {
        VmAfdCloseCRL(pCRL);
    }
    if(hFind != INVALID_HANDLE_VALUE )
    {
        FindClose(hFind);
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSearchPattern);
    VMAFD_SAFE_FREE_MEMORY(pszFileSearchPattern);
    VMAFD_SAFE_FREE_MEMORY(pszCRLFileName);

    return dwError;

error :

    *ppMetaData = NULL;
    *pdwAvailable = 0;

    if (pMetaData)
    {
        VecsSrvFreeCRLMetaDataArray(pMetaData, dwCount);
    }

    goto cleanup;
}
#endif

#ifndef _WIN32
DWORD
VecsGetLinuxFileCount(
    PSTR pszDirPath,
    DWORD *pdwCount
)
{
    DWORD dwError = 0;
    DWORD dwFileCount = 0;

    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;

    if ( IsNullOrEmptyString(pszDirPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pDir = opendir(pszDirPath);
    if(pDir == NULL)
    {
        dwError = errno;
    }
    BAIL_ON_VMAFD_ERROR(dwError);
    while((pEntry = readdir(pDir)) != NULL)
    {
        if ( pEntry->d_type == DT_REG)
        {
            dwFileCount ++;
        }
    }

    *pdwCount = dwFileCount;
cleanup:
    if(pDir != NULL)
    {
        closedir(pDir);
    }
    return dwError;

error :
    goto cleanup;
}

DWORD
VecsGetLinuxCRLData(
    PSTR    pCRLDir,
    DWORD   dwCount,
    PVMAFD_CRL_FILE_METADATA* ppMetaData,
    PDWORD pdwAvailable
    )
{
    DWORD dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    X509_CRL *pCRL = NULL;
    int nIndex = 0;
    PSTR pszCRLFilePath = NULL;
    PVMAFD_CRL_FILE_METADATA pMetaData = NULL;

    pDir = opendir(pCRLDir);
    if (pDir == NULL)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CRL_FILE_METADATA) * dwCount,
                    (PVOID*)&pMetaData);
    BAIL_ON_VMAFD_ERROR(dwError);

    while(((pEntry = readdir(pDir)) != NULL) && (nIndex < dwCount))
    {
        if ( pEntry->d_type == DT_REG)
        {
            struct stat statbuf = {0};

            PVMAFD_CRL_FILE_METADATA pCursor = &pMetaData[nIndex];

            dwError = VmAfdAllocateStringWFromA(
                            pEntry->d_name,
                            &pCursor->pwszCrlFileName);
            BAIL_ON_VMAFD_ERROR(dwError);

            VMAFD_SAFE_FREE_MEMORY(pszCRLFilePath);

            dwError = VmAfdAllocateStringPrintf(
                            &pszCRLFilePath,
                            "%s/%s",
                            pCRLDir,
                            pEntry->d_name);
            BAIL_ON_VMAFD_ERROR(dwError);

            if (stat(pszCRLFilePath, &statbuf) < 0)
            {
                dwError = LwErrnoToWin32Error(errno);
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            if (statbuf.st_size == 0)
            {
                continue;
            }

            if (pCRL != NULL)
            {
                VmAfdCloseCRL(pCRL);
                pCRL = NULL;
            }

            dwError = VmAfdOpenCRL(pszCRLFilePath, &pCRL );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLVersion(pCRL,(DWORD *)&pCursor->dwCRLNumber);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLName(pCRL, &pCursor->pwszIssuerName);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLLastUpdate(pCRL, &pCursor->pwszLastUpdate);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdGetCRLNextUpdate(pCRL, &pCursor->pwszNextUpdate);
            BAIL_ON_VMAFD_ERROR(dwError);

            nIndex ++;
        }
    }

    *ppMetaData = pMetaData;
    *pdwAvailable = nIndex;

cleanup:

    if(pDir != NULL)
    {
        closedir(pDir);
    }
    if (pCRL != NULL)
    {
        VmAfdCloseCRL(pCRL);
    }
    VMAFD_SAFE_FREE_MEMORY(pszCRLFilePath);

    return dwError;

error :

    *ppMetaData = NULL;
    *pdwAvailable = 0;

    if (pMetaData)
    {
        VecsSrvFreeCRLMetaDataArray(pMetaData, dwCount);
    }

    goto cleanup;
}

#endif


DWORD
VecsGetFileCount(PSTR pszCRLPath, DWORD *pdwCount)
{
#ifdef _WIN32
    return VecsGetWindowsFileCount(pszCRLPath, pdwCount);
#else
    return VecsGetLinuxFileCount(pszCRLPath, pdwCount);
#endif
}

DWORD
VecsSrvRpcAllocateCertStoreArray(
    PWSTR* ppwszStoreNames,
    DWORD  dwCount,
    PVMAFD_CERT_STORE_ARRAY* ppCertStoreArray
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray = NULL;

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(VMAFD_CERT_STORE_ARRAY),
                    (PVOID*)&pCertStoreArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
          dwError = VmAfdRpcServerAllocateStringArrayW (
                        dwCount,
                        (PCWSTR*) ppwszStoreNames,
                        &pCertStoreArray->ppwszStoreNames
                        );
          BAIL_ON_VMAFD_ERROR (dwError);

          pCertStoreArray->dwCount = dwCount;
    }

    *ppCertStoreArray = pCertStoreArray;

cleanup:

    return dwError;

error:

    *ppCertStoreArray = NULL;

    if (pCertStoreArray)
    {
        VecsSrvRpcFreeCertStoreArray(pCertStoreArray);
    }

    goto cleanup;
}

VOID
VecsSrvRpcFreeCertStoreArray(
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray
    )
{
    if (pCertStoreArray->ppwszStoreNames)
    {
        VmAfdRpcServerFreeStringArrayW(
                pCertStoreArray->ppwszStoreNames,
                pCertStoreArray->dwCount);
    }

    VmAfdRpcServerFreeMemory(pCertStoreArray);
}

DWORD
VecsRpcAllocateCertArray(
    PVMAFD_CERT_ARRAY  pSrc,
    PVMAFD_CERT_ARRAY* ppDst
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pDst = NULL;

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(VMAFD_CERT_ARRAY),
                    (PVOID*)&pDst);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pSrc->dwCount > 0)
    {
        DWORD iCert = 0;

        dwError = VmAfdRpcServerAllocateMemory(
                        sizeof(VMAFD_CERT_CONTAINER) * pSrc->dwCount,
                        (PVOID*)&pDst->certificates);
        BAIL_ON_VMAFD_ERROR(dwError);

        pDst->dwCount = pSrc->dwCount;

        for (; iCert < pSrc->dwCount; iCert++)
        {
            PVMAFD_CERT_CONTAINER pSrcCert = &pSrc->certificates[iCert];
            PVMAFD_CERT_CONTAINER pDstCert = &pDst->certificates[iCert];

            pDstCert->dwStoreType = pSrcCert->dwStoreType;
            pDstCert->dwDate = pSrcCert->dwDate;

            if (pSrcCert->pAlias)
            {
                dwError = VmAfdRpcServerAllocateStringW(
                                pSrcCert->pAlias,
                                &pDstCert->pAlias);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            if (pSrcCert->pCert)
            {
                dwError = VmAfdRpcServerAllocateStringW(
                                pSrcCert->pCert,
                                &pDstCert->pCert);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            if (pSrcCert->pPassword)
            {
                dwError = VmAfdRpcServerAllocateStringW(
                                pSrcCert->pPassword,
                                &pDstCert->pPassword);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            if (pSrcCert->pPrivateKey)
            {
                dwError = VmAfdRpcServerAllocateStringW(
                                pSrcCert->pPrivateKey,
                                &pDstCert->pPrivateKey);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *ppDst = pDst;

cleanup:

    return dwError;

error:

    *ppDst = NULL;

    if (pDst)
    {
        VecsSrvRpcFreeCertArray(pDst);
    }

    goto cleanup;
}

VOID
VecsSrvRpcFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    )
{
    if (pArray != NULL)
    {
        if (pArray->certificates)
        {
            DWORD i = 0;

            for (i = 0; i < pArray->dwCount; i++)
            {
                PVMAFD_CERT_CONTAINER pContainer = &pArray->certificates[i];

                if (pContainer->pPrivateKey != NULL)
                {
                    VmAfdRpcServerFreeMemory(pContainer->pPrivateKey);
                }
                if (pContainer->pCert != NULL)
                {
                    VmAfdRpcServerFreeMemory(pContainer->pCert);
                }
                if (pContainer->pAlias != NULL)
                {
                    VmAfdRpcServerFreeMemory(pContainer->pAlias);
                }
                if (pContainer->pPassword != NULL)
                {
                    VmAfdRpcServerFreeMemory(pContainer->pPassword);
                }
            }

            VmAfdRpcServerFreeMemory(pArray->certificates);
        }

        VmAfdRpcServerFreeMemory(pArray);
    }
}

VOID
VecsSrvFreeCertContainer(
    PVMAFD_CERT_CONTAINER pContainer
    )
{
    if (pContainer != NULL)
    {
        VMAFD_SAFE_FREE_MEMORY(pContainer->pPrivateKey);
        VMAFD_SAFE_FREE_MEMORY(pContainer->pCert);
        VMAFD_SAFE_FREE_MEMORY(pContainer->pAlias);
        VMAFD_SAFE_FREE_MEMORY(pContainer->pPassword);
        VmAfdFreeMemory(pContainer);
    }
}

VOID
VecsSrvRpcFreeCRLData(
    PVMAFD_CRL_DATA pCRLData
    )
{
    if (pCRLData)
    {
        if (pCRLData->buffer)
        {
            VmAfdRpcServerFreeMemory(pCRLData->buffer);
        }
        VmAfdRpcServerFreeMemory(pCRLData);
    }
}

DWORD
VmAfdSrvRpcAllocateCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER  pSrcData,
    PVMAFD_CRL_METADATA_CONTAINER* ppDstData
    )
{
    DWORD dwError = 0;
    PVMAFD_CRL_METADATA_CONTAINER pDstData = NULL;

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(VMAFD_CRL_METADATA_CONTAINER),
                    (PVOID*)&pDstData);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pSrcData->MetaData)
    {
        pDstData->dwCount = pSrcData->dwCount;

        if (pSrcData->dwCount > 0)
        {
            DWORD i = 0;

            dwError = VmAfdRpcServerAllocateMemory(
                           sizeof(VMAFD_CRL_FILE_METADATA) * pSrcData->dwCount,
                           (PVOID*)&pDstData->MetaData);
            BAIL_ON_VMAFD_ERROR(dwError);

            for (; i < pSrcData->dwCount; i++)
            {
                PVMAFD_CRL_FILE_METADATA pDataSrc = &pSrcData->MetaData[i];
                PVMAFD_CRL_FILE_METADATA pDataDst = &pDstData->MetaData[i];

                memcpy(
                    &pDataDst->bAuthID[0],
                    &pDataSrc->bAuthID[0],
                    sizeof(pDataSrc->bAuthID));

                pDataDst->dwCRLNumber = pDataSrc->dwCRLNumber;
                pDataDst->dwSize = pDataSrc->dwSize;

                if (pDataSrc->pwszCrlFileName)
                {
                    dwError = VmAfdRpcServerAllocateStringW(
                                    pDataSrc->pwszCrlFileName,
                                    &pDataDst->pwszCrlFileName);
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                if (pDataSrc->pwszIssuerName)
                {
                    dwError = VmAfdRpcServerAllocateStringW(
                                    pDataSrc->pwszIssuerName,
                                    &pDataDst->pwszIssuerName);
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                if (pDataSrc->pwszLastUpdate)
                {
                    dwError = VmAfdRpcServerAllocateStringW(
                                    pDataSrc->pwszLastUpdate,
                                    &pDataDst->pwszLastUpdate);
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                if (pDataSrc->pwszNextUpdate)
                {
                    dwError = VmAfdRpcServerAllocateStringW(
                                    pDataSrc->pwszNextUpdate,
                                    &pDataDst->pwszNextUpdate);
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
            }
        }
    }

    *ppDstData = pDstData;

cleanup:

    return dwError;

error:

    *ppDstData = NULL;

    if (pDstData)
    {
        VecsSrvRpcFreeCRLContainer(pDstData);
    }

    goto cleanup;
}

VOID
VecsSrvRpcFreeCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER pContainer
    )
{
    if (pContainer)
    {
        if (pContainer->MetaData)
        {
            DWORD i = 0;

            for (; i < pContainer->dwCount; i++)
            {
                PVMAFD_CRL_FILE_METADATA pMetaData = &pContainer->MetaData[i];

                if (pMetaData->pwszCrlFileName)
                {
                    VmAfdRpcServerFreeMemory(pMetaData->pwszCrlFileName);
                }
                if (pMetaData->pwszIssuerName)
                {
                    VmAfdRpcServerFreeMemory(pMetaData->pwszIssuerName);
                }
                if (pMetaData->pwszLastUpdate)
                {
                    VmAfdRpcServerFreeMemory(pMetaData->pwszLastUpdate);
                }
                if (pMetaData->pwszNextUpdate)
                {
                    VmAfdRpcServerFreeMemory(pMetaData->pwszNextUpdate);
                }
            }

            VmAfdRpcServerFreeMemory(pContainer->MetaData);
        }

        VmAfdRpcServerFreeMemory(pContainer);
    }
}

VOID
VecsSrvFreeCRLContainer(
    PVMAFD_CRL_METADATA_CONTAINER pContainer
    )
{
    if (pContainer)
    {
        if (pContainer->MetaData)
        {
            DWORD i = 0;

            for (; i < pContainer->dwCount; i++)
            {
                PVMAFD_CRL_FILE_METADATA pMetaData = &pContainer->MetaData[i];

                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszCrlFileName);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszIssuerName);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszLastUpdate);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszNextUpdate);
            }

            VmAfdFreeMemory(pContainer->MetaData);
        }

        VmAfdFreeMemory(pContainer);
    }
}

VOID
VecsSrvFreeCRLArray(
    PVMAFD_CRL_METADATA_CONTAINER pCRLArray,
    DWORD dwSize)
{
    if (pCRLArray)
    {
        if (pCRLArray->MetaData)
        {
            DWORD i = 0;

            for (; i < dwSize; i++)
            {
                PVMAFD_CRL_FILE_METADATA pMetaData = &pCRLArray->MetaData[i];

                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszCrlFileName);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszIssuerName);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszLastUpdate);
                VMAFD_SAFE_FREE_MEMORY(pMetaData->pwszNextUpdate);
            }

            VmAfdFreeMemory(pCRLArray->MetaData);
        }

        VmAfdFreeMemory(pCRLArray);
    }
}

VOID
VecsSrvFreeCRLMetaDataArray(
    PVMAFD_CRL_FILE_METADATA pMetaData,
    DWORD                    dwCount
    )
{
    if (pMetaData)
    {
        DWORD i = 0;

        for (; i < dwCount; i++)
        {
            PVMAFD_CRL_FILE_METADATA pCursor = &pMetaData[i];

            VMAFD_SAFE_FREE_MEMORY(pCursor->pwszCrlFileName);
            VMAFD_SAFE_FREE_MEMORY(pCursor->pwszIssuerName);
            VMAFD_SAFE_FREE_MEMORY(pCursor->pwszLastUpdate);
            VMAFD_SAFE_FREE_MEMORY(pCursor->pwszNextUpdate);
        }

        VmAfdFreeMemory(pMetaData);
    }
}

VOID
VecsSrvFreeCRLData(
    PVMAFD_CRL_DATA pCRLData
    )
{
    if (pCRLData)
    {
        if (pCRLData->buffer)
        {
            VmAfdFreeMemory(pCRLData->buffer);
        }
        VmAfdFreeMemory(pCRLData);
    }
}

