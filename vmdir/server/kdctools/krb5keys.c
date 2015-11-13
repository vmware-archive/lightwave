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
#include <asn1/kerberos_db.h>

#define VMKDC_DEFAULT_KVNO 1


#if 0
DWORD
VmKdcGenerateMasterKey(
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen)
{
    DWORD dwError = 0;
    krb5_error_code err = 0;
    krb5_keyblock keyBlock = {0};
    krb5_context krb5Context;
    ssize_t asn1_masterkey_len = 0;
    int len = 0;
    PBYTE asn1_masterkey = NULL;
    KrbMKey inMasterKey = {0};

    err = krb5_heim_init_context(&krb5Context);
    if (err)
    {
        dwError = ERROR_ALLOC_KRB5_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }


    /* TBD: FIXME - Only use VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96 for now */
    err = krb5_heim_generate_random_keyblock(krb5Context,
                              VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96,
                              &keyBlock);
    if (err)
    {
        dwError = ERROR_ALLOC_KRB5_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    inMasterKey.kvno = VMKDC_DEFAULT_KVNO;
    inMasterKey.key.keytype = keyBlock.keytype;
    inMasterKey.key.keyvalue = keyBlock.keyvalue;

    ASN1_MALLOC_ENCODE(KrbMKey, 
                       asn1_masterkey, 
                       len, 
                       &inMasterKey, 
                       &asn1_masterkey_len, 
                       err);
    if (asn1_masterkey_len > 0)
    {
        *ppMasterKey = asn1_masterkey;
        *pMasterKeyLen = (DWORD) asn1_masterkey_len;
    }

error:

    krb5_heim_free_keyblock_contents(krb5Context, &keyBlock);
    krb5_heim_free_context(krb5Context);
    return dwError;
}
#endif

static VOID
_VmKdcParsePrincipalHeimdalFree(
    krb5_principal principal)
{
    DWORD i = 0;
    if (principal)
    {
        VMKDC_SAFE_FREE_MEMORY(principal->realm);
        for (i=0; i< principal->name.name_string.len; i++)
        {
            VMKDC_SAFE_FREE_MEMORY(principal->name.name_string.val[i]);
        }
        VMKDC_SAFE_FREE_MEMORY(principal->name.name_string.val);
        VMKDC_SAFE_FREE_MEMORY(principal);
    }
}

static DWORD
_VmKdcParsePrincipalHeimdal(
    PSTR upnName,
    krb5_principal *pRetUpnPrincipal)
{
    DWORD dwError = 0;
    krb5_principal upnPrincipal = NULL;
    PSTR pAt = NULL;
    PSTR realmComp = NULL;
    PSTR nameComp = NULL;
    DWORD len = 0;

    dwError = VmKdcAllocateMemory(sizeof(*upnPrincipal),
                                 (PVOID*)&upnPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    pAt = strrchr(upnName, '@');
    if (pAt)
    {
        len = pAt - upnName;
        dwError = VmKdcAllocateMemory(len + 1,
                                     (PVOID*)&nameComp);
        BAIL_ON_VMKDC_ERROR(dwError);
        strncat(nameComp, upnName, len);
        nameComp[len] = '\0';

        pAt++;
        dwError = VmKdcAllocateMemory(
                      sizeof(heim_general_string *) * 1,
                      (PVOID*)&upnPrincipal->name.name_string.val);
        BAIL_ON_VMKDC_ERROR(dwError);

        len = strlen(pAt);
        dwError = VmKdcAllocateMemory(len + 1,
                                     (PVOID*)&realmComp);
        BAIL_ON_VMKDC_ERROR(dwError);
        strncat(realmComp, pAt, len);
        realmComp[len] = '\0';

        upnPrincipal->realm = realmComp;
        upnPrincipal->name.name_type = VMKDC_NT_PRINCIPAL;
        upnPrincipal->name.name_string.len = 1;
        upnPrincipal->name.name_string.val[0] = nameComp;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *pRetUpnPrincipal = upnPrincipal;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_MEMORY(upnPrincipal);
        VMKDC_SAFE_FREE_MEMORY(pAt);
        VMKDC_SAFE_FREE_MEMORY(realmComp);
        VMKDC_SAFE_FREE_MEMORY(nameComp);
    }
    return dwError;
}


static DWORD
_VmKdcAsn1EncodeStringToKeys(
    krb5_keyblock *keyBlockArray,
    DWORD keyBlockArrayLen,
    PBYTE *ppAsn1Keys,
    PDWORD asn1KeysLen)
{
    DWORD dwError = 0;
    PBYTE asn1_keyset = NULL;
    ssize_t asn1_keyset_len = 0;
    krb5_error_code err = 0;
    int len = 0;
    KrbKeySet inKeySet = {0};
    KrbKey *krbKeyArray = NULL;
    DWORD i = 0;
   
    inKeySet.attribute_major_vno = 1;
    inKeySet.attribute_minor_vno = 0;
    inKeySet.kvno = VMKDC_DEFAULT_KVNO;
    inKeySet.mkvno = NULL; // Optional, but should match current MKVNO

    dwError = VmKdcAllocateMemory(sizeof(*krbKeyArray) * keyBlockArrayLen,
                                  (PVOID*)&krbKeyArray);
    BAIL_ON_VMKDC_ERROR(dwError);


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
        *asn1KeysLen = asn1_keyset_len;
    }

error:
    VMKDC_SAFE_FREE_MEMORY(krbKeyArray);
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
        dwError = ERROR_ALLOC_KRB5_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = _VmKdcParsePrincipalHeimdal(
                  upnName,
                  &upnPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    err = krb5_heim_string_to_key(
              krb5Context,
              VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96,
              (const char *)password,
              upnPrincipal,
              &keyBlocks[0]);
    if (err)
    {
        dwError = ERROR_ALLOC_KRB5_CONTEXT;
        BAIL_ON_VMKDC_ERROR(dwError);
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
                  &pAsn1Keys,
                  &asn1KeysLen);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppUpnKeys = pAsn1Keys;
    *pUpnKeysLen = asn1KeysLen;

error:
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[0]);
    krb5_heim_free_keyblock_contents(krb5Context, &keyBlocks[1]);
    krb5_heim_free_context(krb5Context);
    _VmKdcParsePrincipalHeimdalFree(upnPrincipal);
    return dwError;
}
