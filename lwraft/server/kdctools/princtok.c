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

static void
hexStringToBinary(char *hexStr, int len, unsigned char **ppBinaryBuf)
{
    int i = 0;
    unsigned char *binaryBuf = NULL;
    unsigned char twoHexChars[3];

    binaryBuf = (unsigned char *) calloc(1, len);
    if (!binaryBuf)
    {
        return;
    }

    for (i=0; i<len; i++)
    {
        strncpy(twoHexChars, &hexStr[i*2], 2);
        twoHexChars[2] = '\0';
        binaryBuf[i] = (unsigned char) strtoul(twoHexChars, NULL, 16);
    }
    *ppBinaryBuf = binaryBuf;
}

/* Missing convenience routine, should be in enckey.c */
static
DWORD
VmKdcMakeEncKey(
    VMKDC_KEYTYPE type,
    DWORD kvno,
    PUCHAR contents,
    DWORD len,
    PVMKDC_ENCKEY *ppRetEncKey)
{
    DWORD dwError = 0;
    PVMKDC_ENCKEY pEncKey = NULL;

    BAIL_ON_VMKDC_INVALID_POINTER(contents, dwError);
    BAIL_ON_VMKDC_INVALID_POINTER(ppRetEncKey, dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCKEY), (PVOID*)&pEncKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCDATA), (PVOID*)&pEncKey->encdata);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateData(contents, len, &pEncKey->encdata->data);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEncKey->encdata->kvno = kvno;
    pEncKey->keytype = type;
    *ppRetEncKey = pEncKey;

error:
    if (dwError) {
        VMKDC_SAFE_FREE_ENCKEY(pEncKey);
    }

    return dwError;
}

DWORD tokenizeLine(
    char *line,
    char **ppRetPrincName,
    PVMKDC_KEYSET *ppRetKeySet)
{
    char *ptr = NULL;
    char *tok_r_ptr = NULL;
    char *line_tmp = NULL;
    char *princName = NULL;
    PVMKDC_KEYSET pKeySet = NULL;
    PVMKDC_ENCKEY pEncKeyValue = NULL;
    int tokenCount = 0;
    int tagListLen = 0;
    int tagListCnt = 0;
    int numKeyData = 0;
    int numKeyDataCnt = 0;
    int keyDataVer = 0;
    int keyDataVerCnt = 0;
    int keyVersionNumber = 0;
    int keyType = 0;
    int keyDataLen = 0;
    int keyIndex = 0;
    unsigned char *keyData = NULL;
    DWORD dwError = 0;

    line_tmp = strdup(line);
    if (!line_tmp)
    {
        dwError = ENOMEM;
        goto error;
    }
    ptr = strtok_r(line_tmp, "\t", &tok_r_ptr);
    if (strcmp(ptr, "princ") != 0)
    {
        dwError = EINVAL;
        goto error;
    }
    pKeySet = calloc(1, sizeof(*pKeySet));
    if (!pKeySet)
    {
        dwError = ENOMEM;
        goto error;
    }

    ptr = strtok_r(NULL, "\t", &tok_r_ptr);
    while (ptr)
    {
        tokenCount++;
        switch (tokenCount)
        {
          case 3:
            tagListLen = atoi(ptr);
            break;
          case 4:
            numKeyData = atoi(ptr);
            /* Array of key values parsed by this line */
            pKeySet->encKeys = (PVMKDC_ENCKEY *) calloc(2*numKeyData, sizeof(PVMKDC_ENCKEY));
            if (!pKeySet->encKeys)
            {
                dwError = ENOMEM;
                goto error;
            }
            break;
          case 6:
            princName = strdup(ptr);
            if (!princName)
            {
                dwError = ENOMEM;
                goto error;
            }
            break;

          /* Tagged data, which contains tagListLen entries */
          case 15:
            break;
          case 16:
            break;
          case 17:
            tagListCnt++;
            if (tagListCnt < tagListLen)
            {
                /* Enter case 15 again until tagListLen is reached */
                tokenCount = 14;
            }
            break;

            /* Outer loop: Key data, which contains numKeyData entries */
            case 18:
              keyDataVer = atoi(ptr);
              keyDataVerCnt = 0;
              break;

            case 19:
              keyVersionNumber = atoi(ptr);
              break;

            /* Inner loop: This is a loop of entries keyDataVer */
            case 20:
              keyType = atoi(ptr);
              break;

            case 21:
              keyDataLen = atoi(ptr);
              break;

            case 22: /* keyData */
              keyDataVerCnt++;
              if (keyDataLen > 0)
              {
                  hexStringToBinary(ptr, keyDataLen, &keyData);
                  if (!keyData)
                  {
                      dwError = ENOMEM;
                      goto error;
                  }
                  dwError = VmKdcMakeEncKey(
                                keyType,
                                keyVersionNumber,
                                keyData + 2,
                                keyDataLen - 2,
                                &pEncKeyValue);
                  free(keyData);
                  keyData = NULL;
                  if (dwError)
                  {
                      goto error;
                  }
                  pKeySet->encKeys[keyIndex++] = pEncKeyValue;
                  pKeySet->numKeys = keyIndex;
              }
              if (keyDataVerCnt < keyDataVer)
              {
                tokenCount = 19;  // set to case 20; inner loop
              }
              else
              {
                  numKeyDataCnt++;
                  if (numKeyDataCnt < numKeyData)
                  {
                    tokenCount = 17; // set to case 18; outer loop
                  }
              }
              break;
        }
        ptr = strtok_r(NULL, "\t", &tok_r_ptr);
    }
    *ppRetPrincName = princName;
    *ppRetKeySet = pKeySet;

error:
    if (dwError)
    {
        if (princName)
        {
            free(princName);
        }
        VmKdcFreeKeySet(pKeySet);
    }

    if (line_tmp)
    {
        free(line_tmp);
    }
    if (keyData)
    {
        free(keyData);
    }
    return dwError;
}

DWORD VmKdcGetUpnKeysMitDb(
    char *upn,
    char *dumpFile,
    char **ppRetPrincName,
    PVMKDC_KEYSET *ppRetKeySet)
{
    DWORD dwError = 0;
    FILE *fp = NULL;
    char *line = NULL;
    char *princName = NULL;
    size_t upnLen = strlen(upn);
    int found = 0;
    PVMKDC_KEYSET keySet = NULL;

    fp = fopen(dumpFile, "r");
    if (!fp)
    {
        dwError = EINVAL;
        goto error;
    }
    
    line = fgets_long(fp);
    while (line)
    {
        if (princName)
        {
            free(princName);
        }
        dwError = tokenizeLine(line, &princName, &keySet);
        if (dwError == ENOMEM)
        {
            goto error;
        }
        else if (dwError == EINVAL)
        {
            if (line)
            {
                free(line);
                line = NULL;
            }
            line = fgets_long(fp);
            continue;
        }

        if (strncmp(upn, princName, upnLen) == 0)
        {
            found = 1;
            break;
        }
        else
        {
            if (princName)
            {
                free(princName);
                princName = NULL;
            }
            VMKDC_SAFE_FREE_KEYSET(keySet);
        }
        if (line)
        {
            free(line);
            line = NULL;
        }
        line = fgets_long(fp);
    }

    if (found)
    {
        *ppRetKeySet = keySet;
        if (ppRetPrincName)
        {
            *ppRetPrincName = princName;
            princName = NULL;
        }
    }

error:
    if (line)
    {
        free(line);
    }
    if (fp)
    {
        fclose(fp);
    }
    if (!found)
    {
        VMKDC_SAFE_FREE_KEYSET(keySet);
    }
    if (princName)
    {
        free(princName);
    }
    return dwError;
}
    

