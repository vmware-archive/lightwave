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

#if defined(_WIN32) && !defined(HEIMDAL_CONFIG)
// krb5-crypto/includes.h needs this to work.
#include <krb5-crypto/config.h>
#endif

#include <krb5-crypto/includes.h>
#include <asn1/kerberos_db.h>

/* TBD: FIXME - Only generating master key with VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96  */
#define VMKDC_MASTER_KEYBLOCK_ET ENCTYPE_AES256_CTS_HMAC_SHA1_96
#define VMKDC_DEFAULT_KVNO 1

typedef struct _VMDIR_KRBKEY {
    krb5_keyblock heimKey;
} VMDIR_KRBKEY, *PVMDIR_KRBKEY;

static VOID
_VmKdcParsePrincipalHeimdalFree(
    krb5_principal principal);

static DWORD
_VmKdcParsePrincipalHeimdal(
    PCSTR upnName,
    krb5_principal *pRetUpnPrincipal);

static DWORD
_VmKdcAsn1EncodeStringToKeys(
    krb5_keyblock *keyBlockArray,
    DWORD keyBlockArrayLen,
    DWORD kvno,
    PBYTE *ppAsn1Keys,
    PDWORD asn1KeysLen);

static DWORD
_VmKdcEncryptKey(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    krb5_data *key);

static DWORD
_VmKdcDecryptKey(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    krb5_data *key);

static DWORD
_VmKdcStringToKeyEncrypt(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    int encType,
    char *password,
    krb5_principal upnPrincipal,
    krb5_keyblock *keyBlock);

static DWORD
VmDirKeyTabMakeEntry(
    PCSTR pszUpnName,
    int kvno,
    krb5_keyblock *key,
    PVMDIR_KEYTAB_ENTRY *ppRetKeyTabEntry);

static VOID
VmDirKeyTabFreeEntry(
    PVMDIR_KEYTAB_ENTRY pKtEntry);

DWORD
VmKdcGenerateMasterKey(
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen,
    PBYTE *ppEncMasterKey,
    PDWORD pEncMasterKeyLen)
{
    DWORD dwError = 0;
    krb5_error_code err = 0;
    krb5_keyblock keyBlock = {0};
    krb5_keyblock keyBlockClear = {0};
    krb5_context krb5Context;
    ssize_t asn1_masterkey_len = 0;
    ssize_t asn1_encmasterkey_len = 0;
    ssize_t len = 0;
    PBYTE asn1_masterkey = NULL;
    PBYTE asn1_encmasterkey = NULL;
    KrbMKey inMasterKey = {0};
    KrbMKey inEncMasterKey = {0};
    krb5_crypto cryptoContext = NULL;

    err = krb5_heim_init_context(&krb5Context);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = krb5_heim_generate_random_keyblock(
              krb5Context,
              VMKDC_MASTER_KEYBLOCK_ET,
              &keyBlock);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Initialize crypto context to encrypt master key with itself */
    err = krb5_heim_crypto_init(
              krb5Context,
              &keyBlock,
              0, // encryption type argument
              &cryptoContext);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Master key NOT encrypted in itself */
    keyBlockClear = keyBlock;
    err = krb5_heim_data_copy(&keyBlockClear.keyvalue,
                         keyBlock.keyvalue.data,
                         keyBlock.keyvalue.length);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Encrypt keyBlock (master key) in its own key */
    dwError = _VmKdcEncryptKey(
                  krb5Context,
                  cryptoContext,
                  &keyBlock.keyvalue);
    BAIL_ON_VMDIR_ERROR(dwError);

    inEncMasterKey.kvno = VMKDC_DEFAULT_KVNO;
    inEncMasterKey.key.keytype = keyBlock.keytype;
    inEncMasterKey.key.keyvalue = keyBlock.keyvalue;
    ASN1_MALLOC_ENCODE(KrbMKey,
                       asn1_encmasterkey,
                       len,
                       &inEncMasterKey,
                       &asn1_encmasterkey_len,
                       err);
    if (err == 0 && asn1_encmasterkey_len > 0)
    {
        *ppEncMasterKey = asn1_encmasterkey;
        *pEncMasterKeyLen = (DWORD) asn1_encmasterkey_len;
    }

    inMasterKey.kvno = VMKDC_DEFAULT_KVNO;
    inMasterKey.key.keytype = keyBlockClear.keytype;
    inMasterKey.key.keyvalue = keyBlockClear.keyvalue;
    ASN1_MALLOC_ENCODE(KrbMKey,
                       asn1_masterkey,
                       len,
                       &inMasterKey,
                       &asn1_masterkey_len,
                       err);
    if (err == 0 && asn1_masterkey_len > 0)
    {
        *ppMasterKey = asn1_masterkey;
        *pMasterKeyLen = (DWORD) asn1_masterkey_len;
    }

error:

    krb5_heim_free_keyblock_contents(krb5Context, &keyBlock);
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlockClear);
    if (cryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, cryptoContext);
    }
    krb5_heim_free_context(krb5Context);
    return dwError;
}


static VOID
_VmKdcParsePrincipalHeimdalFree(
    krb5_principal principal)
{
    DWORD i = 0;
    if (principal)
    {
        VMDIR_SAFE_FREE_STRINGA(principal->realm);
        for (i=0; i<principal->name.name_string.len; i++)
        {
            VMDIR_SAFE_FREE_STRINGA(principal->name.name_string.val[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(principal->name.name_string.val);
        VMDIR_SAFE_FREE_MEMORY(principal);
    }
}

static DWORD
_VmKdcParsePrincipalHeimdal(
    PCSTR pPrincipalName,
    krb5_principal *ppRetPrincipal)
{
    DWORD dwError = 0;
    krb5_principal pPrincipal = NULL;
    heim_general_string *val = NULL;
    PSTR pRealm = NULL;
    PSTR pNameComp = NULL;
    PSTR p = NULL;
    PSTR q = NULL;
    PSTR pNewPrincipalName = NULL;
    int count = 0;
    size_t dwLen = 0;
    BOOLEAN bAtFound = FALSE;
    CHAR cSave = '\0';

    if (pPrincipalName == NULL ||
        *pPrincipalName == '/' ||
        *pPrincipalName == '@' ||
        *pPrincipalName == '\0')
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwLen = strlen(pPrincipalName) + 1;
    dwError = VmDirAllocateMemory(sizeof(CHAR) + dwLen, (PVOID*)&pNewPrincipalName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(pNewPrincipalName,
                              sizeof(CHAR) + dwLen,
                              (const PVOID)pPrincipalName,
                              dwLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNewPrincipalName[dwLen] = '\0';

    dwError = VmDirAllocateMemory(sizeof(*pPrincipal), (PVOID*)&pPrincipal);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* count the number of name components */
    p = (PSTR) pNewPrincipalName;
    do
    {
        p++;
        if (*p == '/' || *p == '@' || *p == '\0')
        {
            count++;
        }
    }
    while (*p && *p != '@');

    /* allocate an array for the name components */
    dwError = VmDirAllocateMemory(
                  sizeof(heim_general_string *) * count,
                  (PVOID*)&val);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* allocate and copy the name components */
    count = 0;
    p = (PSTR) pNewPrincipalName;
    q = p;
    do
    {
        p++;
        if (*p == '/' || *p == '@' || *p == '\0')
        {
            dwLen = p - q;
            cSave = q[dwLen];
            q[dwLen] = '\0';
            dwError = VmDirAllocateStringA(
                          q,
                          &pNameComp);
            BAIL_ON_VMDIR_ERROR(dwError);
            val[count] = pNameComp;
            q[dwLen] = cSave;

            count++;
            if (*p == '@')
            {
                bAtFound = TRUE;
                break;
            }

            q = p + 1;
        }
    }
    while (*p && *p != '@');

    if (bAtFound)
    {
        /* allocate and copy the realm */
        dwError = VmDirAllocateStringA(
                      p+1,
                      &pRealm);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pPrincipal->name.name_type = KRB5_NT_PRINCIPAL;
    pPrincipal->name.name_string.len = count;
    pPrincipal->name.name_string.val = val;
    pPrincipal->realm = pRealm;

    *ppRetPrincipal = pPrincipal;

error:
    if (dwError)
    {
        _VmKdcParsePrincipalHeimdalFree(pPrincipal);
    }
    VMDIR_SAFE_FREE_MEMORY(pNewPrincipalName);

    return dwError;
}

static DWORD
_VmKdcAsn1EncodeStringToKeys(
    krb5_keyblock *keyBlockArray,
    DWORD keyBlockArrayLen,
    DWORD kvno,
    PBYTE *ppAsn1Keys,
    PDWORD asn1KeysLen)
{
    DWORD dwError = 0;
    PBYTE asn1_keyset = NULL;
    ssize_t asn1_keyset_len = 0;
    krb5_error_code err = 0;
    ssize_t len = 0;
    KrbKeySet inKeySet = {0};
    KrbKey *krbKeyArray = NULL;
    DWORD i = 0;

    inKeySet.attribute_major_vno = 1;
    inKeySet.attribute_minor_vno = 0;
    inKeySet.kvno = kvno;
    inKeySet.mkvno = NULL; // Optional, but should match current MKVNO

    dwError = VmDirAllocateMemory(sizeof(*krbKeyArray) * keyBlockArrayLen,
                                  (PVOID*)&krbKeyArray);
    BAIL_ON_VMDIR_ERROR(dwError);


    for (i=0; i<keyBlockArrayLen; i++)
    {
        krbKeyArray[i].key.keytype = keyBlockArray[i].keytype;
        krbKeyArray[i].key.keyvalue = keyBlockArray[i].keyvalue;
    }
    inKeySet.keys.val = krbKeyArray;
    inKeySet.keys.len = keyBlockArrayLen;

    ASN1_MALLOC_ENCODE(KrbKeySet,
                       asn1_keyset,
                       len,
                       &inKeySet,
                       &asn1_keyset_len,
                       err);
    if (err == 0 && asn1_keyset_len > 0)
    {
        *ppAsn1Keys = asn1_keyset;
        *asn1KeysLen = (DWORD) asn1_keyset_len;
    }

error:
    VMDIR_SAFE_FREE_MEMORY(krbKeyArray);
    return dwError;
}

/*
 * Helper function to encrypt key in-place using key in crypto context
 */
static DWORD
_VmKdcEncryptKey(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    krb5_data *key)
{
    DWORD dwError = 0;
    krb5_data encryptedKey = {0};
    krb5_error_code err = 0;

    err = krb5_heim_encrypt(krb5Context,
             cryptoContext,
             0, // key usage
             key->data,
             key->length,
             &encryptedKey);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_SAFE_FREE_MEMORY(key->data);
    *key = encryptedKey;

error:
    return dwError;
}

/*
 * Helper function to decrypt key in-place using key in crypto context
 */
static DWORD
_VmKdcDecryptKey(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    krb5_data *key)
{
    DWORD dwError = 0;
    krb5_data plainKey = {0};
    krb5_error_code err = 0;

    err = krb5_heim_decrypt(krb5Context,
             cryptoContext,
             0, // key usage
             key->data,
             key->length,
             &plainKey);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_SAFE_FREE_MEMORY(key->data);
    *key = plainKey;

error:
    return dwError;
}

static DWORD
_VmKdcStringToKeyEncrypt(
    krb5_context krb5Context,
    krb5_crypto cryptoContext,
    int encType,
    char *password,
    krb5_principal upnPrincipal,
    krb5_keyblock *keyBlock)
{
    DWORD dwError = 0;
    krb5_error_code err = 0;

    err = krb5_heim_string_to_key(
              krb5Context,
              encType,
              (const char *)password,
              upnPrincipal,
              keyBlock);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmKdcEncryptKey(
                  krb5Context,
                  cryptoContext,
                  &keyBlock->keyvalue);
    BAIL_ON_VMDIR_ERROR(dwError);
error:
    return dwError;
}

DWORD
VmKdcStringToKeysEncrypt(
    PSTR upnName,
    PSTR password,
    PBYTE pKey,
    DWORD keyLen,
    DWORD kvno,
    PBYTE *ppUpnKeys,
    PDWORD pUpnKeysLen)
{
    KrbMKey kM = {0};
    krb5_keyblock masterKeyBlock = {0};
    size_t kMlen = 0;
    krb5_crypto cryptoContext = NULL;
    DWORD dwError = 0;
    krb5_principal upnPrincipal = {0};
    krb5_error_code err = 0;
    krb5_context krb5Context = {0};
    krb5_keyblock keyBlocks[2] = {{0}};
    PBYTE pAsn1Keys = NULL;
    DWORD asn1KeysLen = 0;

    err = decode_KrbMKey(pKey, keyLen, &kM, &kMlen);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = krb5_heim_init_context(&krb5Context);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    masterKeyBlock.keytype = kM.key.keytype;
    masterKeyBlock.keyvalue = kM.key.keyvalue;
    err = krb5_heim_crypto_init(
              krb5Context,
              &masterKeyBlock,
              0, // encryption type argument
              &cryptoContext);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmKdcParsePrincipalHeimdal(
                  upnName,
                  &upnPrincipal);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmKdcStringToKeyEncrypt(
                   krb5Context,
                   cryptoContext,
                   ENCTYPE_AES256_CTS_HMAC_SHA1_96,
                   password,
                   upnPrincipal,
                   &keyBlocks[0]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmKdcStringToKeyEncrypt(
                   krb5Context,
                   cryptoContext,
                   ENCTYPE_ARCFOUR_HMAC,
                   password,
                   upnPrincipal,
                   &keyBlocks[1]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmKdcAsn1EncodeStringToKeys(
                  keyBlocks,
                  2,
                  kvno,
                  &pAsn1Keys,
                  &asn1KeysLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppUpnKeys = pAsn1Keys;
    *pUpnKeysLen = asn1KeysLen;

error:
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[0]);
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[1]);
    free_KrbMKey(&kM);
    if (cryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, cryptoContext);
    }
    krb5_heim_free_context(krb5Context);
    _VmKdcParsePrincipalHeimdalFree(upnPrincipal);
    return dwError;
}


DWORD
VmKdcStringToKeys(
    PSTR upnName,
    PSTR password,
    PBYTE *ppUpnKeys,
    PDWORD pUpnKeysLen)
{
    DWORD dwError = 0;
    krb5_principal upnPrincipal;
    krb5_error_code err = 0;
    krb5_context krb5Context;
    krb5_keyblock keyBlocks[2];
    PBYTE pAsn1Keys = NULL;
    DWORD asn1KeysLen = 0;

    err = krb5_heim_init_context(&krb5Context);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmKdcParsePrincipalHeimdal(
                  upnName,
                  &upnPrincipal);
    BAIL_ON_VMDIR_ERROR(dwError);

    err = krb5_heim_string_to_key(
              krb5Context,
              ENCTYPE_AES256_CTS_HMAC_SHA1_96,
              (const char *)password,
              upnPrincipal,
              &keyBlocks[0]);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = krb5_heim_string_to_key(
              krb5Context,
              ENCTYPE_ARCFOUR_HMAC,
              (const char *)password,
              upnPrincipal,
              &keyBlocks[1]);

    dwError = _VmKdcAsn1EncodeStringToKeys(
                  keyBlocks,
                  2,
                  VMKDC_DEFAULT_KVNO, /* TBD: Get real kvno */
                  &pAsn1Keys,
                  &asn1KeysLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppUpnKeys = pAsn1Keys;
    *pUpnKeysLen = asn1KeysLen;

error:
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[0]);
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[1]);
    krb5_heim_free_context(krb5Context);
    _VmKdcParsePrincipalHeimdalFree(upnPrincipal);
    return dwError;
}

DWORD
VmKdcGenerateRandomPassword(
    DWORD pwdLen,
    PSTR *ppRandPwd)
{
    DWORD dwError = 0;
    PSTR pRandPwdStr = NULL;
    UCHAR pRandPwd[32] = {0};
    DWORD i = 0;
    DWORD j = 0;
    CHAR c = '\0';
    CHAR cPrev = '\0';


    dwError = VmDirAllocateMemory(sizeof(CHAR) * pwdLen + 1,
                                  (PVOID*)&pRandPwdStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    krb5_heim_generate_random_block(pRandPwd, 32);
    for (i=0, j=0; i<pwdLen; j++)
    {
        if (j >= 32)
        {
            j = 0;
            krb5_heim_generate_random_block(pRandPwd, 32);
        }

        c = pRandPwd[j] & 0x7f;
        if (isprint((int) c))
        {
            /* Prevent random passwords with consecutive duplicate characters */
            if (cPrev != c)
            {
                pRandPwdStr[i++] = c;
                cPrev = c;
            }
        }
    }
    pRandPwdStr[i] = '\0';
    *ppRandPwd = pRandPwdStr;

error:
    if (dwError)
    {
        VMDIR_SAFE_FREE_MEMORY(pRandPwdStr);
    }
    return dwError;
}

static DWORD
_RecordDataAppend(PVOID inData,
                  DWORD inDataLen,
                  PDWORD pAllocRecordData,
                  PDWORD pUsedRecordData,
                  PBYTE *pTotalRecordData)
{
    DWORD dwError = 0;
    DWORD allocRecordData = *pAllocRecordData;
    DWORD usedRecordData = *pUsedRecordData;
    PBYTE totalRecordData = *pTotalRecordData;
    PBYTE newRecordData = NULL;

    /* Insure there is enough space in buffer before appending data */
    if ((usedRecordData + inDataLen) > allocRecordData)
    {
        allocRecordData *= 2;
        /* Must realloc more space here */
        newRecordData = calloc(sizeof(BYTE), allocRecordData);
        if (!newRecordData)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        *pTotalRecordData = newRecordData;
    }

    dwError = VmDirCopyMemory(&totalRecordData[usedRecordData],
                              inDataLen,
                              inData,
                              inDataLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    usedRecordData += inDataLen;
    *pUsedRecordData = usedRecordData;

error:
    return dwError;
}

DWORD
VmDirKeyTabMakeRecord(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY pKtEntry,
    PBYTE* ppKtRecord,
    PDWORD  pdwRecordLen)
{
    DWORD dwError = 0;
    int16_t size_16;
    int32_t size_32;
    uint8_t size_8;
    PBYTE totalRecordData = NULL;
    DWORD allocRecordData = 1024;
    DWORD usedRecordData = 0;
    DWORD len = 0;
    size_t i = 0;

    dwError = VmDirAllocateMemory(
                  allocRecordData,
                  (PVOID*)&totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Will back fill this value once record is completely filled */
    usedRecordData = sizeof(size_32);

    /* Number of principal name components */
    size_16 = htons(pKtEntry->nameComponentsLen);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Realm size */
    len = (DWORD) strlen(pKtEntry->realm);
    size_16 = (int16_t) len;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Realm value */
    dwError = _RecordDataAppend(pKtEntry->realm,
                                len,
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Array of name components */
    for (i=0; i < pKtEntry->nameComponentsLen; i++)
    {
        /* Length of name component in network byte order */
        len = (DWORD) strlen(pKtEntry->nameComponents[i]);
        size_16 = (int16_t) len;
        size_16 = htons(size_16);
        dwError = _RecordDataAppend(&size_16,
                                    sizeof(size_16),
                                    &allocRecordData,
                                    &usedRecordData,
                                    &totalRecordData);
        BAIL_ON_VMDIR_ERROR(dwError);

        /* Format the actual name component, without the trailing NULL terminator */
        dwError = _RecordDataAppend(pKtEntry->nameComponents[i],
                                    len,
                                    &allocRecordData,
                                    &usedRecordData,
                                    &totalRecordData);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Principal type */
    size_32 = htonl(pKtEntry->princType);
    dwError = _RecordDataAppend(&size_32,
                                sizeof(size_32),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Timestamp */
    size_32 = htonl(pKtEntry->timeStamp);
    dwError = _RecordDataAppend(&size_32,
                                sizeof(size_32),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Key version  number */
    size_8 = (uint8_t) pKtEntry->kvno;
    dwError = _RecordDataAppend(&size_8,
                                sizeof(size_8),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* key type */
    size_16 = (int16_t) pKtEntry->key->heimKey.keytype;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* key length */
    len = (DWORD) pKtEntry->key->heimKey.keyvalue.length;
    size_16 = (int16_t) len;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* key data */
    dwError = _RecordDataAppend(pKtEntry->key->heimKey.keyvalue.data,
                                len,
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * Back patch the total record length at the start of the buffer, not
     * including the leading length
     */
    size_32 = htonl((int32_t) (usedRecordData - sizeof(size_32)));
    dwError = VmDirCopyMemory(totalRecordData,
                              sizeof(size_32),
                              &size_32,
                              sizeof(size_32));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppKtRecord = totalRecordData;
    *pdwRecordLen = usedRecordData;

error:
    if (dwError)
    {
        VMDIR_SAFE_FREE_MEMORY(totalRecordData);
    }

    return dwError;
}

DWORD
VmDirKeyTabWrite(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY pKtEntry)
{
    DWORD dwError = 0;
    PBYTE pKtRecord = NULL;
    DWORD dwRecordLen = 0;
    DWORD len = 0;

    dwError = VmDirKeyTabMakeRecord(
                      pKt,
                      pKtEntry,
                      &pKtRecord,
                      &dwRecordLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Write record to the file */
    len = (DWORD) fwrite(pKtRecord, 1, dwRecordLen, pKt->ktfp);
    if (len != dwRecordLen)
    {
        /* I/O Error */
        dwError = ERROR_IO;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:

    VMDIR_SAFE_FREE_MEMORY(pKtRecord);

    return dwError;
}


static VOID
VmDirKeyTabFreeEntry(
    PVMDIR_KEYTAB_ENTRY pKtEntry)
{
    int i = 0;

    if (pKtEntry) {
        if (pKtEntry->realm)
        {
            free(pKtEntry->realm);
            pKtEntry->realm = NULL;
        }
        if (pKtEntry->nameComponents)
        {
            for (i=0; i<pKtEntry->nameComponentsLen; i++)
            {
                if (pKtEntry->nameComponents[i])
                {
                    free(pKtEntry->nameComponents[i]);
                    pKtEntry->nameComponents[i] = NULL;
                }
            }
            free(pKtEntry->nameComponents);
            pKtEntry->nameComponents = NULL;
        }
        if (pKtEntry->key)
        {
            if (pKtEntry->key->heimKey.keyvalue.data)
            {
                free(pKtEntry->key->heimKey.keyvalue.data);
            }
            VMDIR_SAFE_FREE_MEMORY(pKtEntry->key);
        }
        VMDIR_SAFE_FREE_MEMORY(pKtEntry);
    }
}

static DWORD
VmDirKeyTabMakeEntry(
    PCSTR pszUpnName,
    int kvno,
    krb5_keyblock *key,
    PVMDIR_KEYTAB_ENTRY *ppRetKeyTabEntry)
{
    DWORD dwError = 0;
    PVMDIR_KEYTAB_ENTRY pKeyTabEntry = NULL;
    krb5_principal upnPrincipal = {0};
    int err = 0;
    int i = 0;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KEYTAB_ENTRY),
                                  (PVOID*)&pKeyTabEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmKdcParsePrincipalHeimdal(pszUpnName, &upnPrincipal);
    BAIL_ON_VMDIR_ERROR(dwError);

    pKeyTabEntry->princType = upnPrincipal->name.name_type;

    dwError = VmDirAllocateStringA(upnPrincipal->realm,
                                   &pKeyTabEntry->realm);
    BAIL_ON_VMDIR_ERROR(dwError);

    pKeyTabEntry->nameComponentsLen = upnPrincipal->name.name_string.len;

    dwError = VmDirAllocateMemory(sizeof(char *) * pKeyTabEntry->nameComponentsLen,
                                  (PVOID*)&pKeyTabEntry->nameComponents);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<pKeyTabEntry->nameComponentsLen; i++)
    {
        dwError = VmDirAllocateStringA(upnPrincipal->name.name_string.val[i],
                                       &pKeyTabEntry->nameComponents[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pKeyTabEntry->timeStamp = (int)time(NULL);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KRBKEY), (PVOID*)&pKeyTabEntry->key);
    BAIL_ON_VMDIR_ERROR(dwError);

    pKeyTabEntry->key->heimKey.keytype = key->keytype;
    pKeyTabEntry->kvno = kvno;
    err = krb5_heim_data_copy(
               &pKeyTabEntry->key->heimKey.keyvalue,
               key->keyvalue.data,
               key->keyvalue.length);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppRetKeyTabEntry = pKeyTabEntry;

error:
    if (dwError)
    {
        VmDirKeyTabFreeEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
    }
    _VmKdcParsePrincipalHeimdalFree(upnPrincipal);

    return dwError;
}

DWORD
VmDirKeyTabWriteKeys(
    PVMDIR_KEYTAB_HANDLE pKeyTab,
    PCSTR pszUpnName,
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    PBYTE pMasterKey,
    DWORD masterKeyLen)
{
    DWORD dwError = 0;
    KrbKeySet keyset = {0};
    size_t keysetLen = 0;
    KrbMKey mKey = {0};
    size_t mKeyLen = 0;
    int err = 0;
    DWORD i = 0;
    PVMDIR_KEYTAB_ENTRY pKeyTabEntry = NULL;
    krb5_context krb5Context = {0};
    BOOLEAN krb5ContextInitialized = 0;
    krb5_crypto cryptoContext = NULL;
    krb5_keyblock masterKeyBlock = {0};

    /* Decode the principal keys */
    err = decode_KrbKeySet(pUpnKeys, upnKeysLen, &keyset, &keysetLen);
    if (err || keysetLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* if a master key is supplied, decode it now */
    if (pMasterKey)
    {
        err = decode_KrbMKey(pMasterKey, masterKeyLen, &mKey, &mKeyLen);
        if (err || mKeyLen <= 0)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        err = krb5_heim_init_context(&krb5Context);
        if (err)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        krb5ContextInitialized = 1;

        masterKeyBlock.keytype = mKey.key.keytype;
        masterKeyBlock.keyvalue = mKey.key.keyvalue;
        err = krb5_heim_crypto_init(
                  krb5Context,
                  &masterKeyBlock,
                  0, // encryption type argument
                  &cryptoContext);
        if (err)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i=0; i<keyset.keys.len; i++)
    {
        if (masterKeyLen)
        {
            dwError = _VmKdcDecryptKey(krb5Context,
                                       cryptoContext,
                                       &keyset.keys.val[i].key.keyvalue);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        VmDirKeyTabFreeEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
        /* make a new keytab entry */
        dwError = VmDirKeyTabMakeEntry(pszUpnName,
                                       keyset.kvno,
                                       (krb5_keyblock *)&keyset.keys.val[i].key,
                                       &pKeyTabEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        /* write the keytab entry to the file */
        dwError = VmDirKeyTabWrite(pKeyTab, pKeyTabEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    if (keysetLen)
    {
        free_KrbKeySet(&keyset);
    }
    if (mKeyLen)
    {
        free_KrbMKey(&mKey);
    }
    if (cryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, cryptoContext);
    }
    if (krb5ContextInitialized)
    {
        krb5_heim_free_context(krb5Context);
    }
    VmDirKeyTabFreeEntry(pKeyTabEntry);

    return dwError;
}

DWORD
VmDirKeyTabWriteKeysBlob(
    PVMDIR_KEYTAB_HANDLE pKeyTab,
    PCSTR pszUpnName,
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    PBYTE pMasterKey,
    DWORD masterKeyLen,
    PBYTE *ppBlob,
    PDWORD pdwBlobLen)
{
    DWORD dwError = 0;
    KrbKeySet keyset = {0};
    size_t keysetLen = 0;
    KrbMKey mKey = {0};
    size_t mKeyLen = 0;
    int err = 0;
    DWORD i = 0;
    PVMDIR_KEYTAB_ENTRY pKeyTabEntry = NULL;
    krb5_context krb5Context = {0};
    BOOLEAN krb5ContextInitialized = 0;
    krb5_crypto cryptoContext = NULL;
    krb5_keyblock masterKeyBlock = {0};
    PBYTE pKeyTabRecord = NULL;
    DWORD dwKeyTabRecordLen = 0;
    PBYTE pBlob = NULL;
    DWORD dwBlobLen = 0;

    /* Decode the principal keys */
    err = decode_KrbKeySet(pUpnKeys, upnKeysLen, &keyset, &keysetLen);
    if (err || keysetLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* if a master key is supplied, decode it now */
    if (pMasterKey)
    {
        err = decode_KrbMKey(pMasterKey, masterKeyLen, &mKey, &mKeyLen);
        if (err || mKeyLen <= 0)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        err = krb5_heim_init_context(&krb5Context);
        if (err)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        krb5ContextInitialized = 1;

        masterKeyBlock.keytype = mKey.key.keytype;
        masterKeyBlock.keyvalue = mKey.key.keyvalue;
        err = krb5_heim_crypto_init(
                  krb5Context,
                  &masterKeyBlock,
                  0, // encryption type argument
                  &cryptoContext);
        if (err)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i=0; i<keyset.keys.len; i++)
    {
        if (masterKeyLen)
        {
            dwError = _VmKdcDecryptKey(krb5Context,
                                       cryptoContext,
                                       &keyset.keys.val[i].key.keyvalue);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        VmDirKeyTabFreeEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
        /* make a new keytab entry */
        dwError = VmDirKeyTabMakeEntry(pszUpnName,
                                       keyset.kvno,
                                       (krb5_keyblock *)&keyset.keys.val[i].key,
                                       &pKeyTabEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pKeyTabRecord);
        /* make a record from the keytab entry */
        dwError = VmDirKeyTabMakeRecord(pKeyTab,
                                        pKeyTabEntry,
                                        &pKeyTabRecord,
                                        &dwKeyTabRecordLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        /* append the record to the return blob */
        dwError = VmDirReallocateMemory(pBlob,
                                        (PVOID*)&pBlob,
                                        dwBlobLen + dwKeyTabRecordLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory(pBlob + dwBlobLen,
                                  dwKeyTabRecordLen,
                                  pKeyTabRecord,
                                  dwKeyTabRecordLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwBlobLen += dwKeyTabRecordLen;
    }

    *ppBlob = pBlob;
    *pdwBlobLen = dwBlobLen;

error:

    if (dwError)
    {
        VMDIR_SAFE_FREE_MEMORY(pBlob);
    }
    if (keysetLen)
    {
        free_KrbKeySet(&keyset);
    }
    if (mKeyLen)
    {
        free_KrbMKey(&mKey);
    }
    if (cryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, cryptoContext);
    }
    if (krb5ContextInitialized)
    {
        krb5_heim_free_context(krb5Context);
    }
    VmDirKeyTabFreeEntry(pKeyTabEntry);
    VMDIR_SAFE_FREE_MEMORY(pKeyTabRecord);

    return dwError;
}

DWORD
VmDirMigrateUserKey(
    PBYTE pOldUpnKeys,
    DWORD oldUpnKeysLen,
    PBYTE pOldMasterKey,
    DWORD oldMasterKeyLen,
    PBYTE pNewMasterKey,
    DWORD newMasterKeyLen,
    PBYTE* ppNewUpnKeys,
    PDWORD pNewUpnKeysLen)
{
    DWORD       dwError = 0;
    int         err = 0;
    DWORD       i = 0;
    ssize_t len = 0;

    KrbKeySet   keyset = {0};
    size_t      keysetLen = 0;
    KrbMKey     oldMKey = {0};
    size_t      oldMKeyLen = 0;
    KrbMKey     newMKey = {0};
    size_t      newMKeyLen = 0;
    PBYTE       new_keyset = NULL;
    ssize_t     new_keyset_len = 0;

    krb5_context    krb5Context = {0};
    BOOLEAN         krb5ContextInitialized = 0;
    krb5_keyblock   oldMasterKeyBlock = {0};
    krb5_keyblock   newMasterKeyBlock = {0};
    krb5_crypto     oldCryptoContext = NULL;
    krb5_crypto     newCryptoContext = NULL;

    if (!pOldUpnKeys || oldUpnKeysLen==0 || !pOldMasterKey || oldMasterKeyLen==0 ||
            !pNewMasterKey || newMasterKeyLen==0 ||!ppNewUpnKeys ||!pNewUpnKeysLen)
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Decode the principal keys */
    err = decode_KrbKeySet(pOldUpnKeys, oldUpnKeysLen, &keyset, &keysetLen);
    if (err || keysetLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* decode old and new master keys */
    err = decode_KrbMKey(pOldMasterKey, oldMasterKeyLen, &oldMKey, &oldMKeyLen);
    if (err || oldMKeyLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = decode_KrbMKey(pNewMasterKey, newMasterKeyLen, &newMKey, &newMKeyLen);
    if (err || newMKeyLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = krb5_heim_init_context(&krb5Context);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    krb5ContextInitialized = 1;

    oldMasterKeyBlock.keytype = oldMKey.key.keytype;
    oldMasterKeyBlock.keyvalue = oldMKey.key.keyvalue;

    newMasterKeyBlock.keytype = newMKey.key.keytype;
    newMasterKeyBlock.keyvalue = newMKey.key.keyvalue;

    err = krb5_heim_crypto_init(
              krb5Context,
              &oldMasterKeyBlock,
              0, // encryption type argument
              &oldCryptoContext);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    err = krb5_heim_crypto_init(
              krb5Context,
              &newMasterKeyBlock,
              0, // encryption type argument
              &newCryptoContext);
    if (err)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i<keyset.keys.len; i++)
    {
        dwError = _VmKdcDecryptKey(krb5Context,
                                   oldCryptoContext,
                                   &keyset.keys.val[i].key.keyvalue);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmKdcEncryptKey(krb5Context,
                                   newCryptoContext,
                                   &keyset.keys.val[i].key.keyvalue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ASN1_MALLOC_ENCODE(KrbKeySet,
                       new_keyset,
                       len,
                       &keyset,
                       &new_keyset_len,
                       err);
    if (err == 0 && new_keyset_len > 0)
    {
        *ppNewUpnKeys = new_keyset;
        *pNewUpnKeysLen = (DWORD) new_keyset_len;
    }

error:
    if (keysetLen)
    {
        free_KrbKeySet(&keyset);
    }
    if (oldMKeyLen)
    {
        free_KrbMKey(&oldMKey);
    }
    if (newMKeyLen)
    {
        free_KrbMKey(&newMKey);
    }
    if (oldCryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, oldCryptoContext);
    }
    if (newCryptoContext)
    {
        krb5_heim_crypto_destroy(krb5Context, newCryptoContext);
    }

    if (krb5ContextInitialized)
    {
        krb5_heim_free_context(krb5Context);
    }

    return dwError;
}

DWORD
VmDirKeySetGetKvno(
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    DWORD *kvno)
{
    DWORD       dwError = 0;
    int         err = 0;
    KrbKeySet   keyset = {0};
    size_t      keysetLen = 0;

    err = decode_KrbKeySet(pUpnKeys, upnKeysLen, &keyset, &keysetLen);
    if (err || keysetLen <= 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *kvno = keyset.kvno;

error:
    if (err == 0)
    {
        free_KrbKeySet(&keyset);
    }
    return dwError;
}
