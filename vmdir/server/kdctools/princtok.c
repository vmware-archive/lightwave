#include "includes.h"

static void
hexStringToBinary(char *hexStr, int len, unsigned char **ppBinaryBuf)
{
    int i = 0;
    unsigned char *binaryBuf = NULL;
    unsigned char twoHexChars[3];

    binaryBuf = (unsigned char *) calloc(1, len);

    for (i=0; i<len; i++)
    {
        strncpy(twoHexChars, &hexStr[i*2], 2);
        twoHexChars[2] = '\0';
        binaryBuf[i] = (unsigned char) strtoul(twoHexChars, NULL, 16);
    }
    *ppBinaryBuf = binaryBuf;
}


void keyEntryFree(PKEY_ENTRY pKeyEntry)
{
    int i = 0;

    if (pKeyEntry)
    {
        for (i=0; i<pKeyEntry->numKeys; i++)
        {
            VmKdcFreeKey(pKeyEntry->keys[i]);
        }
        free(pKeyEntry->princName);
        free(pKeyEntry->keys);
        free(pKeyEntry);
    }
}

void tokenizeLine(char *line, PKEY_ENTRY *ppRetKeyEntry)
{
    char *ptr = NULL;
    char *princName = NULL;
    PKEY_ENTRY pKeyEntry = NULL;
    PVMKDC_KEY pKeyValue = NULL;
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


    ptr = strtok(line, "\t");
    if (strcmp(ptr, "princ") != 0)
    {
        return;
    }
    pKeyEntry = calloc(1, sizeof(*pKeyEntry));

    ptr = strtok(NULL, "\t");
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
            pKeyEntry->keys = (PVMKDC_KEY *) calloc(2*numKeyData, sizeof(PVMKDC_KEY));
            break;
          case 6:
            princName = strdup(ptr);
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
                  dwError = VmKdcMakeKey(
                                keyType,
                                keyVersionNumber,
                                keyData,
                                keyDataLen,
                                &pKeyValue);
//                  pKeyValue = (PVMKDC_KEY) calloc(1, sizeof(*pKeyValue));
                  pKeyEntry->keys[keyIndex++] = pKeyValue;
//                  pKeyValue->kvno = keyVersionNumber;
                  pKeyEntry->numKeys = keyIndex;
//                  pKeyValue->keyType = keyType;
//                  pKeyValue->keyLen = keyDataLen;
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
        ptr = strtok(NULL, "\t");
    }
    pKeyEntry->princName = princName;
    *ppRetKeyEntry = pKeyEntry;
}
