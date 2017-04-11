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

#define VMKDC_DEFAULT_KVNO 1

typedef struct _PROG_ARGS
{
    char *upnName;
    char *pwd;
    int randPwdLen;
} PROG_ARGS;


int ldap_syslog = 0;
#ifndef _WIN32
int slap_debug = 1;
#endif

void print_hex(PBYTE data, DWORD len)
{
    DWORD i = 0;

    for (i=0; i<len; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
}


void
usage(char *argv0, char *msg)
{
    printf("usage: %s [--upnname principal --pwd passwd] [--randpwd-len len]", argv0);

    if (msg)
    {
        printf("%s\n", msg);
    }
    exit(1);
}

VOID
parseArgsFree(PROG_ARGS *args)
{
    VMDIR_SAFE_FREE_MEMORY(args->upnName);
    VMDIR_SAFE_FREE_MEMORY(args->pwd);
}

void
parseArgs(int argc, char *argv[], PROG_ARGS *args)
{
    int i;

    i = 1;
    while (i<argc)
    {
        if (strcmp("--randpwd-len", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--randpwd-len missing argument");
            }
            args->randPwdLen = atoi(argv[i]);
            i++;
        }
        else if (strcmp("--upnname", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--upnname missing argument");
            }
            args->upnName = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--pwd", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                usage(argv[0], "--pwd missing argument");
            }
            args->pwd = strdup(argv[i]);
            i++;
        }
        else if (strcmp("--help", argv[i]) == 0 ||
                 strcmp("-h", argv[i]) == 0)
        {
            usage(argv[0], NULL);
        }
        else
        {
            usage(argv[0], "unknown argument provided");
        }
    }
}


int main(int argc, char *argv[])
{
    DWORD dwError = 0;
    PBYTE pMasterKey = NULL;
    DWORD masterKeyLen = 0;
    PBYTE pEncMasterKey = NULL;
    DWORD encMasterKeyLen = 0;
    PBYTE pUpnKeys = NULL;
    DWORD upnKeysLen = 0;
    PSTR upnName = NULL;
    PSTR krbtgtName = NULL;
    PSTR password = NULL;
    PROG_ARGS args = {0};
    PSTR pRandPwd = NULL;
    DWORD randPwdLen = 32;
    PVMDIR_KEYTAB_HANDLE pKeyTab = NULL;

    parseArgs(argc, argv, &args);
    password = args.pwd;
    upnName = args.upnName;

    dwError = VmKdcGenerateMasterKey(
                  &pMasterKey,
                  &masterKeyLen,
                  &pEncMasterKey,
                  &encMasterKeyLen);

    if (dwError)
    {
        fprintf(stderr, "ERROR: Failed to generate master key!\n");
        return 0;
    }

    if (masterKeyLen > 0)
    {
        printf("created master key len=%d\n", masterKeyLen);
        print_hex(pMasterKey, masterKeyLen);
    }

    if (encMasterKeyLen > 0)
    {
        printf("created encrypted master key len=%d\n", encMasterKeyLen);
        print_hex(pEncMasterKey, encMasterKeyLen);
    }
    if (args.randPwdLen)
    {
        randPwdLen = args.randPwdLen;
    }


    dwError = VmKdcGenerateRandomPassword(
                  randPwdLen,
                  &pRandPwd);
    printf("created random password <%s>\n", pRandPwd);
    krbtgtName = "krbtgt@VMWARE.COM";
    dwError = VmKdcStringToKeysEncrypt(
                      krbtgtName,
                      pRandPwd,
                      pMasterKey,
                      masterKeyLen,
                      VMKDC_DEFAULT_KVNO,
                      &pUpnKeys,
                      &upnKeysLen);
    printf("\nCreated krbtgt keys len=%d for %s\n", upnKeysLen, krbtgtName);
    print_hex(pUpnKeys, upnKeysLen);

    if (upnName && password)
    {
        dwError = VmKdcStringToKeysEncrypt(
                      upnName,
                      password,
                      pMasterKey,
                      masterKeyLen,
                      VMKDC_DEFAULT_KVNO,
                      &pUpnKeys,
                      &upnKeysLen);
        BAIL_ON_VMDIR_ERROR(dwError);
        if (upnKeysLen > 0)
        {
            printf("\nCreated encrypted keys len=%d for %s\n", upnKeysLen, upnName);
            print_hex(pUpnKeys, upnKeysLen);
        }
        dwError = VmDirKeyTabOpen("test-encrypted.keytab", "a", &pKeyTab);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirKeyTabWriteKeys(pKeyTab,
                                       upnName,
                                       pUpnKeys,
                                       upnKeysLen,
                                       pMasterKey,
                                       masterKeyLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirKeyTabClose(pKeyTab);
        printf("\nCreated keytab file %s from encrypted keys\n", "test-encrypted.keytab");

        VMDIR_SAFE_FREE_MEMORY(pUpnKeys);

        dwError = VmKdcStringToKeys(
                      upnName,
                      password,
                      &pUpnKeys,
                      &upnKeysLen);
        BAIL_ON_VMDIR_ERROR(dwError);
        if (upnKeysLen > 0)
        {
            printf("\nCreated keys len=%d for %s\n", upnKeysLen, upnName);
            print_hex(pUpnKeys, upnKeysLen);
        }

        dwError = VmDirKeyTabOpen("test-unencrypted.keytab", "a", &pKeyTab);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirKeyTabWriteKeys(pKeyTab,
                                       upnName,
                                       pUpnKeys,
                                       upnKeysLen,
                                       NULL,
                                       0);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirKeyTabClose(pKeyTab);
        printf("\nCreated keytab file %s from encrypted keys\n", "test-unencrypted.keytab");

        VMDIR_SAFE_FREE_MEMORY(pUpnKeys);
    }
    VMDIR_SAFE_FREE_MEMORY(pMasterKey);
    VMDIR_SAFE_FREE_MEMORY(pRandPwd);
    VMDIR_SAFE_FREE_MEMORY(pEncMasterKey);
    parseArgsFree(&args);
error:

    return 0;
}
