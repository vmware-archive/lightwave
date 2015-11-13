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


DWORD
VmKdcParseKeyTabOpen(
    PSTR ktName,
    PSTR ktOpenMode, // "r", "rw"
    PVMKDC_KEYTAB_HANDLE *ppKeyTab)
{
    DWORD dwError = 0;
    FILE *ktfp = NULL;
    PVMKDC_KEYTAB_HANDLE pKeyTab = NULL;
    CHAR *ktOpenModeStr = NULL;
    int ktMode = 0;
    int sts = 0;
    int16_t size_16 = 0;
    int16_t stashVersion = 0;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KEYTAB_HANDLE), (PVOID*)&pKeyTab);
    BAIL_ON_VMKDC_ERROR(dwError);

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
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* Initially open file for read/write to obtain keytab version info */
    ktfp = fopen(ktName, "rb");
    if (!ktfp)
    {
        if (ktMode == 1)
        {
            dwError = EINVAL;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        ktfp = fopen(ktName, "w+b");
    }
    if (!ktfp)
    {
        dwError = EINVAL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    sts = (int) fread(&size_16, 1, sizeof(size_16), ktfp);
    if (ktMode == 1 && sts < sizeof(size_16))
    {
        /* Opened bogus keytab file for read, as length must be at least 2 */
        dwError = EINVAL;
        BAIL_ON_VMKDC_ERROR(dwError);
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
            BAIL_ON_VMKDC_ERROR(dwError);
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
            BAIL_ON_VMKDC_ERROR(sts);
        }
    }
     
    /* Reopen file using intended mode */
    fclose(ktfp);
    ktfp = fopen(ktName, ktOpenModeStr);
    if (!ktfp)
    {
        dwError = EINVAL;
        BAIL_ON_VMKDC_ERROR(dwError);
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
VmKdcParseKeyTabRewind(
    PVMKDC_KEYTAB_HANDLE pKeyTab)
{
    DWORD dwError = 0;

    if (!pKeyTab || !pKeyTab->ktfp)
    {
        dwError = EINVAL;  // TBD: Not correct error status here
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    fseek(pKeyTab->ktfp, pKeyTab->ktOffset, SEEK_SET);

error:
    return dwError;
}


VOID
VmKdcParseKeyTabClose(
    PVMKDC_KEYTAB_HANDLE pKeyTab)
{
    if (pKeyTab)
    {
        if (pKeyTab->ktfp)
        {
            fclose(pKeyTab->ktfp);
        }
        VMKDC_SAFE_FREE_MEMORY(pKeyTab);
    }
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
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        *pTotalRecordData = newRecordData;
    }

    memcpy(&totalRecordData[usedRecordData], inData, inDataLen);
    usedRecordData += inDataLen;
    *pUsedRecordData = usedRecordData;

error:
    return dwError;
}

DWORD
VmKdcParseKeyTabWrite(
    PVMKDC_KEYTAB_HANDLE pKt,
    PVMKDC_MIT_KEYTAB_FILE pKtEntry)
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

    dwError = VmKdcAllocateMemory(
                  allocRecordData,
                  (PVOID*)&totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Will back fill this value once record is completely filled */
    usedRecordData = sizeof(size_32);

    /* Number of principal name components */
    size_16 = htons(pKtEntry->nameComponentsLen);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Realm size */
    len = (DWORD) strlen(pKtEntry->realm);
    size_16 = (int16_t) len;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Realm value */
    dwError = _RecordDataAppend(pKtEntry->realm,
                                len,
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

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
        BAIL_ON_VMKDC_ERROR(dwError);

        /* Format the actual name component, without the trailing NULL terminator */
        dwError = _RecordDataAppend(pKtEntry->nameComponents[i],
                                    len,
                                    &allocRecordData,
                                    &usedRecordData,
                                    &totalRecordData);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* Principal type */
    size_32 = htonl(pKtEntry->princType);
    dwError = _RecordDataAppend(&size_32,
                                sizeof(size_32),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Timestamp */
    size_32 = htonl(pKtEntry->timeStamp);
    dwError = _RecordDataAppend(&size_32,
                                sizeof(size_32),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Key version  number */
    size_8 = (uint8_t) pKtEntry->key->kvno;
    dwError = _RecordDataAppend(&size_8,
                                sizeof(size_8),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* key type */
    size_16 = (int16_t) pKtEntry->key->type;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* key length */
    len = VMKDC_GET_LEN_DATA(pKtEntry->key->data);
    size_16 = (int16_t) len;
    size_16 = htons(size_16);
    dwError = _RecordDataAppend(&size_16,
                                sizeof(size_16),
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* key data */
    dwError = _RecordDataAppend(VMKDC_GET_PTR_DATA(pKtEntry->key->data),
                                len,
                                &allocRecordData,
                                &usedRecordData,
                                &totalRecordData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* 
     * Back patch the total record length at the start of the buffer, not 
     * including the leading length 
     */
    size_32 = htonl((int32_t) (usedRecordData - sizeof(size_32)));
    memcpy(totalRecordData, &size_32, sizeof(size_32));

    /* Write this record to file */
    len = (DWORD) fwrite(totalRecordData, 1, usedRecordData, pKt->ktfp);
    if (len != usedRecordData)
    {
        /* I/O Error */
        dwError = ERROR_IO;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    VMKDC_SAFE_FREE_MEMORY(totalRecordData);
    return dwError;
}


DWORD
VmKdcParseKeyTabRead(
    PVMKDC_KEYTAB_HANDLE pKt,
    PVMKDC_MIT_KEYTAB_FILE *ppRetData)
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
    PVMKDC_MIT_KEYTAB_FILE pKtEntry = NULL;
    PVMKDC_KEY pKey = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_MIT_KEYTAB_FILE), (PVOID*)&pKtEntry);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Total stash record len */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR((sts != sizeof(size_32)) ? (dwError = EINVAL) : 0);

    entrySize = ntohl(size_32);

    /* Number of principal name components */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);
    
    nameCompCnt = ntohs(size_16);

    /* Realm size */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);

    princSize = ntohs(size_16);

    /* Realm value */
    realm = calloc(sizeof(char), princSize+1);
    sts = fread(realm, sizeof(char), princSize, pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != princSize ? (dwError = EINVAL) : 0);
    

    /* Array of name components */
    nameComponents = (char **) calloc(nameCompCnt, sizeof(char *));
    for (i=0; i<nameCompCnt; i++)
    {
        sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
        BAIL_ON_VMKDC_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);

        princSize = ntohs(size_16);
        nameComponent = calloc(sizeof(char), princSize+1);
        sts = fread(nameComponent, sizeof(char), princSize, pKt->ktfp);
        BAIL_ON_VMKDC_ERROR(sts != princSize ? (dwError = EINVAL) : 0);
        nameComponents[i] = nameComponent;
    }

    /* Principal type */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_32) ? (dwError = EINVAL) : 0);
    princType = ntohl(size_32);

    /* Timestamp */
    sts = fread(&size_32, 1, sizeof(size_32), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_32) ? (dwError = EINVAL) : 0);
    timeStamp = ntohl(size_32);

    /* Key version  number */
    sts = fread(&size_8, 1, sizeof(size_8), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_8) ? (dwError = EINVAL) : 0);
    kvno = size_8;
  
    /* key type */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);
    keyType = ntohs(size_16);

    /* key length */
    sts = fread(&size_16, 1, sizeof(size_16), pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != sizeof(size_16) ? (dwError = EINVAL) : 0);
    keyLength = ntohs(size_16);

    key = calloc(keyLength, sizeof(unsigned char));
    sts = fread(key, sizeof(unsigned char), keyLength, pKt->ktfp);
    BAIL_ON_VMKDC_ERROR(sts != keyLength ? (dwError = EINVAL) : 0);

    dwError = VmKdcMakeKey(
                  keyType,
                  kvno,
                  key,
                  keyLength,
                  &pKey);
    BAIL_ON_VMKDC_ERROR(dwError);
    pKtEntry->entrySize = entrySize;
    pKtEntry->realm = realm;
    pKtEntry->nameComponents = nameComponents;
    pKtEntry->nameComponentsLen = nameCompCnt;
    pKtEntry->princType = princType;
    pKtEntry->timeStamp = timeStamp;
    pKtEntry->key = pKey;
    *ppRetData = pKtEntry;

error:
    if (key)
    {
        free(key);
        key = NULL;
    }
    return dwError;
}

VOID
VmKdcParseKeyTabFreeEntry(
    PVMKDC_MIT_KEYTAB_FILE pKtEntry)
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
        VMKDC_SAFE_FREE_KEY(pKtEntry->key);
        VMKDC_SAFE_FREE_MEMORY(pKtEntry);
    }
}
