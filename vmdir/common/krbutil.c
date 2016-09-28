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
#include <krb5-crypto/includes.h>

/* TBD: FIXME - Only generating master key with VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96  */
#define VMKDC_MASTER_KEYBLOCK_ET ENCTYPE_AES256_CTS_HMAC_SHA1_96
#define VMKDC_DEFAULT_KVNO 1

typedef struct _VMDIR_KRBKEY {
    krb5_keyblock heimKey;
} VMDIR_KRBKEY, *PVMDIR_KRBKEY;

/*
 * NOTE: REMOVE THIS CODE WHEN A SHARED VMKRBLIB IS CREATED!
 *
 * The KeyTab API functionality is used by both vmdir and vmkdc.
 * This should be refactored into a shared component to eliminate
 * duplicated code.
 */

DWORD
VmDirKeyTabOpen(
    PSTR ktName,
    PSTR ktOpenMode, // "r", "rw"
    PVMDIR_KEYTAB_HANDLE *ppKeyTab)
{
    DWORD dwError = 0;
    FILE *ktfp = NULL;
    PVMDIR_KEYTAB_HANDLE pKeyTab = NULL;
    CHAR *ktOpenModeStr = NULL;
    int ktMode = 0;
    int sts = 0;
    int16_t size_16 = 0;
    int16_t stashVersion = 0;

    //Workaround only. Once "w" is supported, this code should be removed
    if (!strcmp(ktOpenMode, "w"))
    {
        remove(ktName);
        ktOpenMode = "a";
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KEYTAB_HANDLE), (PVOID*)&pKeyTab);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!strcmp(ktOpenMode, "r"))
    {
        ktOpenModeStr = "rb";
        ktMode = 1;
    }
    else if (!strcmp(ktOpenMode, "w"))
    {
        ktOpenModeStr = "rw+b";
        ktMode = 2;
    }
    else if (!strcmp(ktOpenMode, "a"))
    {
        ktOpenModeStr = "ab";
        ktMode = 3;
    }
    else
    {
        dwError = EINVAL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Initially open file for read/write to obtain keytab version info */
    ktfp = fopen(ktName, "rb");
    if (!ktfp)
    {
        if (ktMode == 1)
        {
            dwError = EINVAL;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        ktfp = fopen(ktName, "w+b");
    }
    if (!ktfp)
    {
        dwError = EINVAL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    sts = (int) fread(&size_16, 1, sizeof(size_16), ktfp);
    if (ktMode == 1 && sts < sizeof(size_16))
    {
        /* Opened bogus keytab file for read, as length must be at least 2 */
        dwError = EINVAL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Opened for write/append and there is no header yet */
    if (ktMode != 1 && sts == 0)
    {
        size_16 = 0x0502;
        size_16 = htons(size_16);
        sts = (int) fwrite(&size_16, 1, sizeof(size_16), ktfp);
        if (sts != sizeof(size_16))
        {
            dwError = ERROR_IO;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    /* File size could be 0, when opened for read/write */
    if (sts >= sizeof(size_16))
    {
        /* stash file version */
        stashVersion = ntohs(size_16);
        if (stashVersion != 0x0502)
        {
            sts = EINVAL;
            BAIL_ON_VMDIR_ERROR(sts);
        }
    }

    /* Reopen file using intended mode */
    fclose(ktfp);
    ktfp = fopen(ktName, ktOpenModeStr);
    if (!ktfp)
    {
        dwError = EINVAL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    fseek(ktfp, sizeof(size_16), SEEK_SET);

    pKeyTab->ktOffset = sizeof(size_16);
    pKeyTab->ktType = 1; // Only support file KT for now
    pKeyTab->ktfp = ktfp;
    pKeyTab->ktMode = ktMode;

    *ppKeyTab = pKeyTab;
error:
    return dwError;
}

DWORD
VmDirKeyTabRewind(
    PVMDIR_KEYTAB_HANDLE pKeyTab)
{
    DWORD dwError = 0;

    if (!pKeyTab || !pKeyTab->ktfp)
    {
        dwError = EINVAL;  // TBD: Not correct error status here
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    fseek(pKeyTab->ktfp, pKeyTab->ktOffset, SEEK_SET);

error:
    return dwError;
}

VOID
VmDirKeyTabClose(
    PVMDIR_KEYTAB_HANDLE pKeyTab)
{
    if (pKeyTab)
    {
        if (pKeyTab->ktfp)
        {
            fclose(pKeyTab->ktfp);
        }
        VMDIR_SAFE_FREE_MEMORY(pKeyTab);
    }
}

DWORD
VmDirKeyTabRead(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY *ppRetData)
{
    DWORD dwError = 0;
    int16_t size_16;
    int32_t size_32;
    uint8_t size_8;
    int entrySize;
    int princSize;
    int princType;
    int nameCompCnt;
    int kvno;
    int keyType;
    int keyLength;
    int timeStamp;
    unsigned char *key = NULL;
    char **nameComponents = NULL;
    char *realm = NULL;
    char *nameComponent = NULL;
    ssize_t sts;
    int i;
    PVMDIR_KEYTAB_ENTRY pKtEntry = NULL;
    PVMDIR_KRBKEY pKey = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KEYTAB_ENTRY), (PVOID*)&pKtEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Total stash record len */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR((sts != sizeof(size_32)) ? (dwError = EINVAL) : 0);

    entrySize = ntohl(size_32);

    /* Number of principal name components */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);

    nameCompCnt = ntohs(size_16);

    /* Realm size */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);

    princSize = ntohs(size_16);

    /* Realm value */
    dwError = VmDirAllocateMemory(princSize+1, (PVOID)&realm);
    BAIL_ON_VMDIR_ERROR(dwError);

    sts = fread(realm, sizeof(char), princSize, pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != princSize ? (dwError = EINVAL) : 0);


    /* Array of name components */
    dwError = VmDirAllocateMemory(nameCompCnt * sizeof(char*), (PVOID)&nameComponents);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<nameCompCnt; i++)
    {
        sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
        BAIL_ON_VMDIR_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);

        princSize = ntohs(size_16);
        dwError = VmDirAllocateMemory(princSize+1, (PVOID)&nameComponent);
        BAIL_ON_VMDIR_ERROR(dwError);

        sts = fread(nameComponent, sizeof(char), princSize, pKt->ktfp);
        BAIL_ON_VMDIR_ERROR(sts != princSize ? (dwError = EINVAL) : 0);
        nameComponents[i] = nameComponent;
    }

    /* Principal type */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_32) ? (dwError = EINVAL) : 0);
    princType = ntohl(size_32);

    /* Timestamp */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_32) ? (dwError = EINVAL) : 0);
    timeStamp = ntohl(size_32);

    /* Key version  number */
    sts = fread(&size_8, 1, sizeof(size_8), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_8) ? (dwError = EINVAL) : 0);
    kvno = size_8;

    /* key type */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);
    keyType = ntohs(size_16);

    /* key length */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);
    keyLength = ntohs(size_16);

    dwError = VmDirAllocateMemory(keyLength * sizeof(unsigned char), (PVOID)&key);
    BAIL_ON_VMDIR_ERROR(dwError);

    sts = fread(key, sizeof(unsigned char), keyLength, pKt->ktfp);
    BAIL_ON_VMDIR_ERROR(sts != keyLength ? (dwError = EINVAL) : 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KRBKEY), (PVOID*)&pKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    pKey->heimKey.keytype = keyType;
    pKey->heimKey.keyvalue.data = key;
    pKey->heimKey.keyvalue.length = keyLength;

    pKtEntry->entrySize = entrySize;
    pKtEntry->realm = realm;
    pKtEntry->nameComponents = nameComponents;
    pKtEntry->nameComponentsLen = nameCompCnt;
    pKtEntry->princType = princType;
    pKtEntry->timeStamp = timeStamp;
    pKtEntry->kvno = kvno;
    pKtEntry->key = pKey;
    *ppRetData = pKtEntry;

cleanup:
    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pKey);
    VMDIR_SAFE_FREE_MEMORY(key);
    if (nameComponents)
    {
        for (i=0; i<nameCompCnt; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(nameComponents[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(nameComponents);
    }

    VMDIR_SAFE_FREE_MEMORY(realm);
    VMDIR_SAFE_FREE_MEMORY(pKtEntry);

    goto cleanup;
}
