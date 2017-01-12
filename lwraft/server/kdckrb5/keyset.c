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
VmKdcFreeEncKey(
    PVMKDC_ENCKEY pEncKey)
{
    if (pEncKey)
    {
        VMKDC_SAFE_FREE_ENCDATA(pEncKey->encdata);
        VMKDC_SAFE_FREE_MEMORY(pEncKey);
    }
}

VOID
VmKdcFreeKeySet(
    PVMKDC_KEYSET pKeySet)
{
    DWORD i = 0;

    if (pKeySet)
    {
        if (pKeySet->encKeys)
        {
            for (i=0; i<pKeySet->numKeys; i++)
            {
                VMKDC_SAFE_FREE_ENCKEY(pKeySet->encKeys[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pKeySet->encKeys);
        }
        if (pKeySet->keys)
        {
            for (i=0; i<pKeySet->numKeys; i++)
            {
                VMKDC_SAFE_FREE_KEY(pKeySet->keys[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pKeySet->keys);
        }
        if (pKeySet->salts)
        {
            for (i=0; i<pKeySet->numKeys; i++)
            {
                VMKDC_SAFE_FREE_SALT(pKeySet->salts[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pKeySet->salts);
        }
        VMKDC_SAFE_FREE_MEMORY(pKeySet);
    }
}

VOID
VmKdcFreeSalt(
    PVMKDC_SALT pSalt)
{
    if (pSalt)
    {
        VMKDC_SAFE_FREE_DATA(pSalt->data);
        VMKDC_SAFE_FREE_MEMORY(pSalt);
    }
}

DWORD
VmKdcMakeSalt(
    VMKDC_SALTTYPE type,
    DWORD length,
    PUCHAR data,
    PVMKDC_SALT *ppRetSalt)
{
    DWORD dwError = 0;
    PVMKDC_SALT pSalt = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_SALT),
                                  (PVOID*)&pSalt);
    BAIL_ON_VMKDC_ERROR(dwError);

    pSalt->type = type;

    if (data)
    {
        dwError = VmKdcAllocateData(data, length, &pSalt->data);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppRetSalt = pSalt;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_SALT(pSalt);
    }

    return dwError;
}

DWORD
VmKdcDecodeKeySet(
    PVMKDC_DATA pData,
    PVMKDC_KEYSET *ppRetKeySet)
{
    DWORD dwError = 0;
    PVMKDC_KEYSET pKeySet = NULL;
    KrbKeySet heimKeySet = {0};
    size_t heimKeySetLen = 0;
    PUCHAR reqBufPtr = NULL;
    DWORD  reqBufLen = 0;
    DWORD i = 0;
    unsigned char *keyData = NULL;
    int keyLen = 0;
    DWORD keyType = 0;
    DWORD kvno = 0;
    KrbSalt *heimSalt = NULL;

    reqBufPtr = VMKDC_GET_PTR_DATA(pData);
    reqBufLen = VMKDC_GET_LEN_DATA(pData);

    /*
     * Decode the ASN.1 data.
     */
    decode_KrbKeySet(reqBufPtr, reqBufLen, &heimKeySet, &heimKeySetLen);
    if (heimKeySetLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KEYSET), (PVOID*)&pKeySet);
    BAIL_ON_VMKDC_ERROR(dwError);

    pKeySet->numKeys = heimKeySet.keys.len;
    pKeySet->kvno = heimKeySet.kvno;

    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_ENCKEY) * pKeySet->numKeys,
                                  (PVOID*)&pKeySet->encKeys);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_SALT) * pKeySet->numKeys,
                                  (PVOID*)&pKeySet->salts);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pKeySet->numKeys; i++)
    {
        dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCKEY),
                                      (PVOID*)&pKeySet->encKeys[i]);
        BAIL_ON_VMKDC_ERROR(dwError);

        keyType = heimKeySet.keys.val[i].key.keytype;
        kvno = heimKeySet.kvno;
        keyData = (unsigned char *)heimKeySet.keys.val[i].key.keyvalue.data;
        keyLen = (DWORD)heimKeySet.keys.val[i].key.keyvalue.length;
        dwError = VmKdcMakeEncData(keyType,
                                   kvno,
                                   keyData,
                                   keyLen,
                                   &pKeySet->encKeys[i]->encdata);
        BAIL_ON_VMKDC_ERROR(dwError);

        pKeySet->encKeys[i]->keytype = heimKeySet.keys.val[i].key.keytype;

        heimSalt = heimKeySet.keys.val[i].salt;
        if (heimSalt && heimSalt->salt)
        {
            dwError = VmKdcMakeSalt(heimSalt->type,
                                    (DWORD)heimSalt->salt->length,
                                    heimSalt->salt->data,
                                    &pKeySet->salts[i]);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    *ppRetKeySet = pKeySet;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_KEYSET(pKeySet);
    }
    free_KrbKeySet(&heimKeySet);

    return dwError;
}

DWORD
VmKdcDecryptKeySet(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    PVMKDC_KEYSET pKeySet)
{
    DWORD dwError = 0;
    DWORD dwError2 = 0;
    PVMKDC_DATA pData = NULL;
    DWORD i = 0;
    DWORD j = 0;

    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_KEY) * pKeySet->numKeys,
                                  (PVOID*)&pKeySet->keys);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pKeySet->numKeys; i++)
    {
        dwError2 = VmKdcDecryptEncData(pContext,
                                       pKey,
                                       (VMKDC_KEY_USAGE) 0,
                                       pKeySet->encKeys[i]->encdata,
                                       &pData);

        if (dwError2 == 0)
        {
            dwError = VmKdcMakeKey(pKeySet->encKeys[j]->keytype,
                                   pKeySet->kvno,
                                   VMKDC_GET_PTR_DATA(pData),
                                   VMKDC_GET_LEN_DATA(pData),
                                   &pKeySet->keys[j]);
            j++;
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_DATA(pData);
    }
    pKeySet->numKeys = j;

error:
    VMKDC_SAFE_FREE_DATA(pData);

    return dwError;
}

VOID
VmKdcPrintKeySet(
    PVMKDC_KEYSET pKeySet)
{
    DWORD i = 0;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKeySet: numKeys <%d>", pKeySet->numKeys);
    for (i=0; i<pKeySet->numKeys; i++)
    {
        VmKdcPrintEncData(pKeySet->encKeys[i]->encdata);
        VmKdcPrintKey(pKeySet->keys[i]);
    }
}
