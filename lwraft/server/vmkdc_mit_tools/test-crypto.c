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
#if 0
#include <vmkrb5/types.h>
#include <vmkrb5/structs.h>
#include <vmkrb5/prototypes.h>


#define DWORD int
#include "parsekt.h"
#include "fgetsl.h"
#include "princtok.h"
#endif

#define SEARCH_PRINC_NAME "K/M";

int ldap_syslog = 0;
int slap_debug = 0;
VMKDC_GLOBALS gVmkdcGlobals = {};

typedef struct _PROG_ARGS
{
    char *keytabFile;
    char *princDbFile;
    char *princName;
    char *plainText;
} PROG_ARGS;


void
usage(char *argv0, char *msg)
{
    printf("usage: %s --keytab-file ktfile [--princdb-file princ_file] [--princ-name princ] [--plaintext \"string\"] [--help | -h]\n", argv0);

    if (msg)
    {
        printf("%s\n", msg);
    }
    exit(1);
}

void
parseArgs(int argc, char *argv[], PROG_ARGS *args)
{
    int i;

    i = 1;
    while (i<argc)
    {
        if (strcmp("--keytab-file", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--keytab-file missing argument");
            }
            args->keytabFile = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--princdb-file", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--princdb-file missing argument");
            }
            args->princDbFile = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--princ-name", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--princ-name missing argument");
            }
            args->princName = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--plaintext", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--plaintext missing argument");
            }
            args->plainText = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]))
        {
            i++;
            usage(argv[0], NULL);
        }
        else
        {
            usage(argv[0], argv[i]);
        }
    }
}


DWORD
encryptDecryptText(
    PVMKDC_CRYPTO krb5Crypto,
    char *plainText)
{
    DWORD dwError = 0;
    PVMKDC_DATA plainTextData = NULL;
    PVMKDC_DATA cipherTextData = NULL;
    PVMKDC_DATA decryptedTextData = NULL;
    int i;
    int len;
    unsigned char *data;

    dwError = VmKdcAllocateDataString(plainText, &plainTextData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCryptoEncrypt(
                krb5Crypto,
                0, // key usage
                plainTextData,
                &cipherTextData);
    BAIL_ON_VMKDC_ERROR(dwError);

    data = VMKDC_GET_PTR_DATA(cipherTextData);
    len = VMKDC_GET_LEN_DATA(cipherTextData);

    printf("\nEncrypted result: length=%d\n", len);
    for (i=0; i<len; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
    printf("\n");


    dwError = VmKdcCryptoDecrypt(
                  krb5Crypto,
                  0,
                  cipherTextData,
                  &decryptedTextData);
    BAIL_ON_VMKDC_ERROR(dwError);
    len = VMKDC_GET_LEN_DATA(decryptedTextData);
    data = VMKDC_GET_PTR_DATA(decryptedTextData);
    printf("Decrypted result: length=%d <%.*s>\n", len, len, (char *) data);

error:
    return dwError;
}


DWORD
decryptPrincKey(PVMKDC_CRYPTO krb5Crypto,
                PVMKDC_KEY princKey)
{
    DWORD dwError = 0;
    PVMKDC_DATA encKey = {0};
    PVMKDC_DATA decKey = {0};
    unsigned char *cipherData;
    int i;
    int len;

    dwError = VmKdcAllocateData(
                  VMKDC_GET_PTR_DATA(princKey->data),
                  VMKDC_GET_LEN_DATA(princKey->data),
                  &encKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCryptoDecrypt(
                  krb5Crypto,
                  0,
                  encKey,
                  &decKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    cipherData = VMKDC_GET_PTR_DATA(decKey);
    len = VMKDC_GET_LEN_DATA(decKey);
    printf("Decrypted key: len=%d\n", (int) len);
    for (i=0; i<len; i++)
    {
        printf("%02x", cipherData[i]);
    }
    printf("\n");

error:
    if (dwError)
    {
        printf("decryptPrincKey: failed %d\n", dwError);
    }
    return dwError;
}

int main(int argc, char *argv[])
{
    DWORD dwError = 0;
    PVMKDC_KEYSET dbEntry = NULL;
    FILE *indb = NULL;
    char *line = NULL;
    int sts = 0;
    int i;
    unsigned char *cipherData;
    PROG_ARGS progArgs = {0};
    char *keytabFile = NULL;
    char *princDbFile = NULL;
    char *plainText = NULL;
    char *searchPrincName = SEARCH_PRINC_NAME;
    char *princName = NULL;
    int bSearchPrincName = FALSE;
    PVMKDC_MIT_KEYTAB_FILE ktData = NULL;
    PVMKDC_KRB5_CONTEXT krb5Ctx = NULL;
    PVMKDC_KEY vmkdcMasterKey = NULL;
    PVMKDC_KEY princKey = NULL;
    PVMKDC_CRYPTO krb5Crypto = NULL;
    PVMKDC_KEYTAB_HANDLE pKtHandle = NULL;

    dwError = VmKdcInitKrb5(&gVmkdcGlobals.pKrb5Ctx);
    BAIL_ON_VMKDC_ERROR(dwError);

    parseArgs(argc, argv, &progArgs);
    if (!progArgs.keytabFile)
    {
        usage(argv[0], "No keytab file specified");
    }
    keytabFile = progArgs.keytabFile;

    if (progArgs.princDbFile)
    {
        princDbFile = progArgs.princDbFile;
    }
    if (progArgs.princName)
    {
        searchPrincName = progArgs.princName;
    }
    if (progArgs.plainText)
    {
        plainText = progArgs.plainText;
    }

    if (princDbFile)
    {
        indb = fopen(princDbFile, "rb");
        if (!indb)
        {
            perror("fopen(indb)");
            return 1;
        }
    }

    dwError = VmKdcParseKeyTabOpen(
                  keytabFile,
                  "r",
                  &pKtHandle);
    BAIL_ON_VMKDC_ERROR(dwError);

    sts = VmKdcParseKeyTabRead(pKtHandle, &ktData);
    if (sts)
    {
        printf("VmKDcParseKeyTabFile: failed %d\n", sts);
        return 1;
    }
    printf("Master Key: keytype=%d keylen=%d\n", 
           ktData->key->type, VMKDC_GET_LEN_DATA(ktData->key->data));

    cipherData = (unsigned char *) VMKDC_GET_PTR_DATA(ktData->key->data);
    for (i=0; i<VMKDC_GET_LEN_DATA(ktData->key->data); i++)
    {
        printf("%02x", cipherData[i]);
    }
    printf("\n");
    printf("\n");

    dwError = VmKdcInitKrb5(&krb5Ctx);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcMakeKey(
                  ktData->key->type,
                  1,
                  VMKDC_GET_PTR_DATA(ktData->key->data),
                  VMKDC_GET_LEN_DATA(ktData->key->data),
                  &vmkdcMasterKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcInitCrypto(krb5Ctx, vmkdcMasterKey, &krb5Crypto);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (indb)
    {
        line = fgets_long(indb);
        while (line)
        {
            if (princName)
            {
                free(princName);
                princName = NULL;
            }
            tokenizeLine(line, &princName, &dbEntry);
            free(line);
            if (dbEntry)
            {
                if (strncmp(searchPrincName, 
                            princName,
                            strlen(searchPrincName)) == 0)
                {
                    dwError = VmKdcMakeKey(
                                  dbEntry->encKeys[0]->keytype,
                                  1,
#if 1
                                  VMKDC_GET_PTR_DATA(dbEntry->encKeys[0]->encdata->data),
                                  VMKDC_GET_LEN_DATA(dbEntry->encKeys[0]->encdata->data),
#else
                                  VMKDC_GET_PTR_DATA(dbEntry->encKeys[0]->encdata->data) + 2,
                                  VMKDC_GET_LEN_DATA(dbEntry->encKeys[0]->encdata->data) - 2,
#endif
                                  &princKey);
                    BAIL_ON_VMKDC_ERROR(dwError);
                    printf("Found %s keyType=%d keyLen=%d\n", 
                           princName,
                           dbEntry->encKeys[0]->keytype, 
                           VMKDC_GET_LEN_DATA(dbEntry->encKeys[0]->encdata->data));
                    bSearchPrincName = TRUE;
                    decryptPrincKey(krb5Crypto, princKey);
                }
            }
            line = fgets_long(indb);
        }
        if (!bSearchPrincName)
        {
            printf("WARNING: Principal not found %s\n", searchPrincName);
        }
    }

    if (plainText)
    {
        dwError = encryptDecryptText(krb5Crypto, plainText);
    }

error:
    VmKdcParseKeyTabClose(pKtHandle);
    if (princName)
    {
        free(princName);
    }

    if (indb)
    {
        fclose(indb);
    }
    return 0;
}
