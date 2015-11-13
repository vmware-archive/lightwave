/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : certutil.c
 *
 * Abstract :
 *
 */
#include "includes.h"

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

