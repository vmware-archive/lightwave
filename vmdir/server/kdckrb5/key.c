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

VOID
VmKdcZeroKey(
    PVMKDC_KEY pKey)
{
    if (pKey)
    {
        DWORD len = VMKDC_GET_LEN_DATA(pKey->data);
        PUCHAR ptr = VMKDC_GET_PTR_DATA(pKey->data);
        memset(ptr, '\0', len);
    }
}

VOID
VmKdcFreeKey(
    PVMKDC_KEY pKey)
{
    if (pKey)
    {
        VmKdcZeroKey(pKey);
        VMKDC_SAFE_FREE_DATA(pKey->data);
        VMKDC_SAFE_FREE_MEMORY(pKey);
    }
}

DWORD
VmKdcMakeKey(
    VMKDC_KEYTYPE type,
    DWORD kvno,
    PUCHAR contents,
    DWORD len,
    PVMKDC_KEY *ppRetKey)
{
    DWORD dwError = 0;
    PVMKDC_KEY pKey = NULL;

    BAIL_ON_VMKDC_INVALID_POINTER(contents, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(ppRetKey, dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KEY), (PVOID*)&pKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateData(contents, len, &pKey->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pKey->type = type;
    pKey->kvno = kvno;

    *ppRetKey = pKey;

error:
    if (dwError) {
        VMKDC_SAFE_FREE_KEY(pKey);
    }

    return dwError;
}

DWORD
VmKdcRandomKey(
    PVMKDC_CONTEXT pContext,
    VMKDC_KEYTYPE type,
    PVMKDC_KEY *ppRetKey)
{
    DWORD dwError = 0;
    PVMKDC_KEY pKey = NULL;
    krb5_keyblock krb5key = {0};
    int sts = 0;

    BAIL_ON_VMKDC_INVALID_POINTER(pContext, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(ppRetKey, dwError);

    sts = krb5_heim_generate_random_keyblock(
              pContext->pGlobals->pKrb5Ctx->ctx,
              type,
              &krb5key);
    if (sts)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcMakeKey(type,
                           0,
                           krb5key.keyvalue.data,
                           (DWORD) krb5key.keyvalue.length,
                           &pKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetKey = pKey;

error:
    if (dwError) {
        VMKDC_SAFE_FREE_KEY(pKey);
    }
    krb5_heim_free_keyblock_contents(
        pContext->pGlobals->pKrb5Ctx->ctx,
        &krb5key);

    return dwError;
}

DWORD
VmKdcCopyKey(
    PVMKDC_KEY src,
    PVMKDC_KEY *dst)
{
    DWORD dwError = 0;
    PVMKDC_KEY pKey = NULL;

    BAIL_ON_VMKDC_INVALID_POINTER(src, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(dst, dwError);

    dwError = VmKdcMakeKey(src->type,
                           src->kvno,
                           VMKDC_GET_PTR_DATA(src->data),
                           VMKDC_GET_LEN_DATA(src->data),
                           &pKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    *dst = pKey;

error:
    if (dwError) {
        VMKDC_SAFE_FREE_KEY(pKey);
    }

    return dwError;
}

VOID
VmKdcPrintKey(
    PVMKDC_KEY pKey)
{
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKey: type <%d>", pKey->type);
    VmKdcPrintData(pKey->data);
}


DWORD
VmKdcDecodeMasterKey(
    PBYTE asn1MasterKey,
    DWORD asn1MasterKeyLen,
    PVMKDC_KEY *ppMasterKey)
{
    DWORD dwError = 0;
    PVMKDC_KEY pRetMasterKey = NULL;
    KrbMKey heimMKey = {0};
    size_t heimMKeyLen = 0;
    int err = 0;

    err = decode_KrbMKey(asn1MasterKey, 
                         asn1MasterKeyLen,
                         &heimMKey,
                         &heimMKeyLen);
    if (err || heimMKeyLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KEY), (PVOID*)&pRetMasterKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    pRetMasterKey->kvno = heimMKey.kvno;
    pRetMasterKey->type = heimMKey.key.keytype;

    dwError = VmKdcAllocateData(
                  heimMKey.key.keyvalue.data,
                  (DWORD)heimMKey.key.keyvalue.length,
                  &pRetMasterKey->data);
    BAIL_ON_VMKDC_ERROR(dwError);
    
    *ppMasterKey = pRetMasterKey;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_KEY(pRetMasterKey);
    }
    free_KrbMKey(&heimMKey);

    return dwError;
}
