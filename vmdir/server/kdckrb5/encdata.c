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
VmKdcFreeEncData(
    PVMKDC_ENCDATA pEncData)
{
    if (pEncData)
    {
        VMKDC_SAFE_FREE_DATA(pEncData->data);
        VMKDC_SAFE_FREE_MEMORY(pEncData);
    }
}

DWORD
VmKdcEncryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_DATA pInData,
    PVMKDC_ENCDATA *ppRetEncData)
{
    DWORD dwError = 0;
    PVMKDC_ENCDATA pEncData = NULL;
    PVMKDC_CRYPTO pCrypto = NULL;

    dwError = VmKdcInitCrypto(pContext->pGlobals->pKrb5Ctx,
                              pKey,
                              &pCrypto);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCDATA), (PVOID*)&pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCryptoEncrypt(pCrypto,
                                 keyUsage,
                                 pInData,
                                 &pEncData->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEncData->kvno = 0;
    pEncData->type = pKey->type;

    *ppRetEncData = pEncData;

error:
    if (pCrypto)
    {
        VmKdcDestroyCrypto(pCrypto);
        pCrypto = NULL;
    }
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCDATA(pEncData);
    }
    return dwError;
}

DWORD
VmKdcDecryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_ENCDATA pEncData,
    PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pData = NULL;
    PVMKDC_CRYPTO pCrypto = NULL;

    dwError = VmKdcInitCrypto(pContext->pGlobals->pKrb5Ctx,
                              pKey,
                              &pCrypto);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCryptoDecrypt(pCrypto,
                                 keyUsage,
                                 pEncData->data,
                                 &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pData;

error:
    if (pCrypto)
    {
        VmKdcDestroyCrypto(pCrypto);
        pCrypto = NULL;
    }
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pData);
    }
    return dwError;
}

DWORD
VmKdcMakeEncData(
    VMKDC_ENCTYPE type,
    DWORD kvno,
    PUCHAR contents,
    DWORD length,
    PVMKDC_ENCDATA *ppRetEncData)
{
    DWORD dwError = 0;
    PVMKDC_ENCDATA pEncData = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCDATA), (PVOID*)&pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateData(contents, length, &pEncData->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEncData->type = type;
    pEncData->kvno = kvno;

    *ppRetEncData = pEncData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCDATA(pEncData);
    }
    return dwError;
}

DWORD
VmKdcDecodeEncData(
    PVMKDC_DATA pData,
    PVMKDC_ENCDATA *ppRetEncData)
{
    DWORD dwError = 0;
    PVMKDC_ENCDATA pEncData = NULL;
    EncryptedData heimEncData = {0};
    size_t heimEncDataLen = 0;
    PUCHAR encDataBufPtr = NULL;
    DWORD  encDataBufLen = 0;

    encDataBufPtr = VMKDC_GET_PTR_DATA(pData);
    encDataBufLen = VMKDC_GET_LEN_DATA(pData);

    /*
     * Decode the ASN.1 data.
     */
    decode_EncryptedData(encDataBufPtr, encDataBufLen, &heimEncData, &heimEncDataLen);
    if (heimEncDataLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcMakeEncData(heimEncData.etype,
                               0,
                               heimEncData.cipher.data,
                               (DWORD)heimEncData.cipher.length,
                               &pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetEncData = pEncData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCDATA(pEncData);
    }
    free_EncryptedData(&heimEncData);

    return dwError;
}

DWORD
VmKdcCopyEncData(
    PVMKDC_ENCDATA pEncData,
    PVMKDC_ENCDATA *ppRetEncData)
{
    DWORD dwError = 0;
    PVMKDC_ENCDATA pNewEncData = NULL;

    dwError = VmKdcMakeEncData(pEncData->type,
                               pEncData->kvno,
                               VMKDC_GET_PTR_DATA(pEncData->data),
                               VMKDC_GET_LEN_DATA(pEncData->data),
                               &pNewEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetEncData = pNewEncData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCDATA(pNewEncData);
    }

    return dwError;
}

VOID
VmKdcPrintEncData(
    PVMKDC_ENCDATA pEncData)
{
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEncData: type <%d>", pEncData->type);
    VmKdcPrintData(pEncData->data);
}
