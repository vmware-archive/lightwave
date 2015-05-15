#include "includes.h"

DWORD
VecsPEMFiletoX509(
    PCSTR pszPath,
    X509** ppX509Cert
    )
{
    DWORD dwError = 0;
    FILE* fp = NULL;
    size_t fileSize = 0;
    PSTR   pszCertificate = NULL;
    PSTR   pszCursor = NULL;
    size_t bytesToRead = 0;
    X509 *pCert = NULL;

    if (IsNullOrEmptyString(pszPath) || !ppX509Cert)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetFileSize(pszPath, &fileSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (fileSize == 0)
    {
        dwError = ERROR_FILE_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : How to limit the max file size?

    dwError = VmAfdAllocateMemory(fileSize + 1, (PVOID*)&pszCertificate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenFilePath(pszPath, "rb", &fp);
    BAIL_ON_VMAFD_ERROR(dwError);

    bytesToRead = fileSize;
    pszCursor = pszCertificate;

    while (!feof(fp) && bytesToRead > 0)
    {
        size_t bytesRead = 0;

        if ((bytesRead = fread(pszCursor, 1, bytesToRead, fp)) == 0)
        {
#ifndef _WIN32
            dwError = LwErrnoToWin32Error(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszCursor += bytesRead;
        bytesToRead -= bytesRead;
    }

    dwError = VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppX509Cert = pCert;

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    VMAFD_SAFE_FREE_MEMORY(pszCertificate);

    return dwError;

error:

    if (ppX509Cert)
    {
        *ppX509Cert = NULL;
    }

    goto cleanup;
}

DWORD
VecsPEMToX509(
    PSTR pCertificate,
    X509 **ppX509Cert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    BIO *pBioMem = NULL;

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if ( pBioMem == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCert  = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pCert  == NULL)
    {
        dwError = VECS_CERT_IO_FAILURE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppX509Cert = pCert;

cleanup :

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :

    *ppX509Cert = NULL;

    if (pCert != NULL)
    {
       X509_free(pCert);
    }

    goto cleanup;
}

DWORD
VecsHashToAlias(
    unsigned long nSubjectHash,
    PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    BIO *b64Bio = NULL;
    BIO *bMem = NULL;
    BUF_MEM *bPtr = NULL;
    char buff[8] = { 0 };
    PSTR pszAlias = NULL;

    b64Bio = BIO_new(BIO_f_base64());
    if ( b64Bio == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bMem = BIO_new(BIO_s_mem());
    if ( bMem == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    b64Bio = BIO_push(b64Bio, bMem);
    if ( b64Bio == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    BIO_write(b64Bio, (PVOID)&nSubjectHash, sizeof(nSubjectHash));
    dwError = BIO_flush(b64Bio);
    if ( dwError != 1)
    {
        dwError = VECS_CERT_IO_FAILURE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    dwError = 0;

    BIO_get_mem_ptr(b64Bio, &bPtr);

    if (((bPtr->length  - 1) < sizeof(buff)))
    {
        memcpy(buff, bPtr->data, bPtr->length - 1);

        dwError = VmAfdAllocateStringA(buff, &pszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszAlias = pszAlias;

cleanup:

    if (b64Bio)
    {
        BIO_free_all(b64Bio);
    }

    return dwError;

error:

    *ppszAlias = NULL;

    goto cleanup;
}

DWORD
VecsComputeCertHash_MD5(
    PSTR  pszCertificate,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR pszHash = NULL;

    if (!pszCertificate || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCertSubjectHash(pCert, TRUE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

DWORD
VecsComputeCertHash_SHA_1(
    PSTR pszCertificate,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR  pszHash = NULL;

    if (!pszCertificate || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCertSubjectHash(pCert, FALSE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}



