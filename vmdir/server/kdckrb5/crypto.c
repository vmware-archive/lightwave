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

struct _VMKDC_KRB5_CONTEXT
{
    krb5_context ctx;
};

struct _VMKDC_CRYPTO_CONTEXT
{
    krb5_crypto ctx;
};

VOID
VmKdcDestroyKrb5(
    PVMKDC_KRB5_CONTEXT pKrb5)
{
    if (pKrb5)
    {
        krb5_heim_free_context(pKrb5->ctx);
        VmKdcFreeMemory(pKrb5);
        pKrb5 = NULL;
    }
}

DWORD
VmKdcInitKrb5(
    PVMKDC_KRB5_CONTEXT *ppRetKrb5)
{
    DWORD dwError = 0;
    PVMKDC_KRB5_CONTEXT pKrb5;
    int sts = 0;

    dwError = VmKdcAllocateMemory(sizeof(*pKrb5), (PVOID*)&pKrb5);
    BAIL_ON_VMKDC_ERROR(dwError);

    sts = krb5_heim_init_context(&pKrb5->ctx);
    if (sts)
    {
        dwError = ERROR_ALLOC_KRB5_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    *ppRetKrb5 = pKrb5;
    return dwError;

}

VOID
VmKdcDestroyCrypto(
    PVMKDC_CRYPTO pCrypto)
{
    if (pCrypto)
    {
        if (pCrypto->krb5Crypto)
        {
            krb5_heim_crypto_destroy(pCrypto->krb5Ctx->ctx, pCrypto->krb5Crypto->ctx);
            VmKdcFreeMemory(pCrypto->krb5Crypto);
            pCrypto->krb5Crypto = NULL;
        }
        VmKdcFreeMemory(pCrypto);
        pCrypto = NULL;
    }
}

DWORD
VmKdcInitCrypto(
    PVMKDC_KRB5_CONTEXT pKrb5,
    PVMKDC_KEY pKey,
    PVMKDC_CRYPTO *ppRetCrypto
    )
{
    DWORD dwError = 0;
    PVMKDC_CRYPTO pCrypto = NULL;
    int sts = 0;
    krb5_keyblock krb5Key = {0};

    dwError = VmKdcAllocateMemory(sizeof(*pCrypto), (PVOID*)&pCrypto);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_CRYPTO_CONTEXT), (PVOID*)&pCrypto->krb5Crypto);
    BAIL_ON_VMKDC_ERROR(dwError);

    krb5Key.keytype = pKey->type;
    krb5Key.keyvalue.length = VMKDC_GET_LEN_DATA(pKey->data);
    krb5Key.keyvalue.data = VMKDC_GET_PTR_DATA(pKey->data);

    pCrypto->krb5Ctx = pKrb5;
    sts = krb5_heim_crypto_init(
              pCrypto->krb5Ctx->ctx, 
              &krb5Key, 
              0, // encryption type argument
              &pCrypto->krb5Crypto->ctx);
    if (sts)
    {
        dwError = ERROR_ALLOC_KRB5_CRYPTO_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (dwError == 0)
    {
        *ppRetCrypto = pCrypto;
    }
    return dwError;
}

DWORD
VmKdcCryptoEncrypt(
    IN PVMKDC_CRYPTO pCrypto,
    IN VMKDC_KEY_USAGE keyUsage,
    IN PVMKDC_DATA pPlainText,
    OUT PVMKDC_DATA *ppVmKdcCipherText)
{
    DWORD dwError = 0;
    PVMKDC_DATA pVmKdcCipherText = NULL;
    krb5_data krb5CipherText = {0};
    int err = 0;

    err = krb5_heim_encrypt(
              pCrypto->krb5Ctx->ctx,
              pCrypto->krb5Crypto->ctx,
              keyUsage,
              VMKDC_GET_PTR_DATA(pPlainText),
              VMKDC_GET_LEN_DATA(pPlainText),
              &krb5CipherText);
    if (err)
    {
        dwError = ERROR_CRYPTO_ENCRYPT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(
                  krb5CipherText.data,
                  (DWORD) krb5CipherText.length,
                  &pVmKdcCipherText);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppVmKdcCipherText = pVmKdcCipherText;
    
error:
    krb5_heim_data_free(&krb5CipherText);
    return dwError;
}

DWORD
VmKdcCryptoDecrypt(
    IN PVMKDC_CRYPTO pCrypto,
    IN VMKDC_KEY_USAGE keyUsage,
    IN PVMKDC_DATA pCipherText,
    OUT PVMKDC_DATA *ppVmKdcPlainText)
{
    DWORD dwError = 0;
    PVMKDC_DATA pVmKdcPlainText = NULL;
    krb5_data krb5PlainText = {0};
    int err = 0;

    err = krb5_heim_decrypt(
              pCrypto->krb5Ctx->ctx,
              pCrypto->krb5Crypto->ctx,
              keyUsage,
              VMKDC_GET_PTR_DATA(pCipherText),
              VMKDC_GET_LEN_DATA(pCipherText),
              &krb5PlainText);
    if (err)
    {
        dwError = ERROR_CRYPTO_ENCRYPT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(
                  krb5PlainText.data,
                  (DWORD) krb5PlainText.length,
                  &pVmKdcPlainText);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppVmKdcPlainText = pVmKdcPlainText;

error:
    if (krb5PlainText.data)
    {
        krb5_heim_data_free(&krb5PlainText);
    }
    return dwError;
}
