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



/*
 * Module Name: vmkdc_admin
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vmkdc_admin main module entry point
 *
 */

#include "includes.h"

#define VMKDC_ADMIN_DEFAULT_REALM           "VSPHERE.LOCAL"
#ifndef _WIN32
#define VMKDC_CONFIG_PARAMETER_KEY_PATH     "Services\\vmafd\\Parameters"
#define VMKDC_REG_KEY_DEFAULT_REALM         "DomainName"
#else
#define VMKDC_CONFIG_PARAMETER_KEY_PATH     _T("SYSTEM\\CurrentControlSet\\Services\\VMWareAfdService\\Parameters")
#define VMKDC_REG_KEY_DEFAULT_REALM         _T("DomainName")
#endif
#define VMKDC_MAX_CONFIG_VALUE_LENGTH       255

VMKDC_GLOBALS gVmkdcGlobals = {0};
char *VmKdc_argv0 = NULL;

VOID
ShowUsage(
    PSTR argv0,
    PSTR msg)
{
    char usage[] = "Usage: %s addprinc [-p <password> | -r] <princ> \n"\
                   "       %s ktadd [-k <keytab>] <princ>\n";
    if (msg)
    {
        printf("%s\n", msg);
    }
    printf(usage, argv0, argv0);
    exit(1);
}

VOID
parseArgs(
    int argc,
    char *argv[],
    PROG_ARGS *args,
    int *params)
{
    int i;

    i = 1;
    while (i<argc && argv[i][0] == '-')
    {
        if (strcmp("-p", argv[i]) == 0 ||
            strcmp("--password", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                ShowUsage(VmKdc_argv0, "-p password missing");
            }
            args->password = strdup(argv[i]);
            i++;
        }
        else if (strcmp("-r", argv[i]) == 0 ||
            strcmp("--randkey", argv[i]) == 0)
        {
            i++;
            args->randKey = TRUE;
        }
        else if (strcmp("-k", argv[i]) == 0 ||
            strcmp("--keytab", argv[i]) == 0)
        {
            i++;
            if (i >= argc)
            {
                ShowUsage(VmKdc_argv0, "-k keytab file name missing");
            }
            args->keytab = strdup(argv[i]);
            i++;
        }
        else
        {
            ShowUsage(VmKdc_argv0, "Unknown option");
        }
    }

    if (i<argc)
    {
        *params = i;
    }
}

typedef enum {
    VMKDC_ADMIN_COMMAND_ADDPRINC=1,
    VMKDC_ADMIN_COMMAND_KTADD,
} VMKDC_ADMIN_COMMANDS;

static
DWORD
VmKdcRegGetDefaultRealm(
    PCHAR *ppszDefaultRealm)
{
    DWORD dwError = 0;
    char szValue[VMKDC_MAX_CONFIG_VALUE_LENGTH] = {0};
    DWORD dwValueSize = sizeof(szValue);
    PCHAR pszDefaultRealm = NULL;
    HKEY hKey = NULL;
#ifndef _WIN32
    HANDLE hConnection = NULL;
#endif

#ifndef _WIN32
    dwError = RegOpenServer(&hConnection);
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

    dwError = RegOpenKeyExA(
#ifndef _WIN32
                hConnection,
                NULL,
                HKEY_THIS_MACHINE,
#else
                HKEY_LOCAL_MACHINE,
                NULL,
#endif
                0,
                KEY_READ,
                &hKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = RegGetValueA(
#ifndef _WIN32
                           hConnection,
#endif
                           hKey,
                           VMKDC_CONFIG_PARAMETER_KEY_PATH,
                           VMKDC_REG_KEY_DEFAULT_REALM,
                           RRF_RT_REG_SZ,
                           NULL,
                           szValue,
                           &dwValueSize);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateStringA(szValue,
                                   &pszDefaultRealm);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppszDefaultRealm = pszDefaultRealm;

error:

    if (hKey)
    {
        dwError = RegCloseKey(
#ifndef _WIN32
                           hConnection,
#endif
                           hKey);
    }

#ifndef _WIN32
    if (hConnection)
    {
        RegCloseServer(hConnection);
    }
#endif

    return dwError;
}

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, TCHAR *targv[])
#endif
{
    DWORD   dwError = 0;
    PROG_ARGS args = {0};
    int params = 0;
    VMKDC_ADMIN_COMMANDS cmd;

#ifdef _WIN32

    char** allocArgv = NULL;
    PSTR* argv = NULL;
    char pathsep = '\\';

#ifdef UNICODE
    dwError = VmKdcAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMKDC_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

#else
    char pathsep = '/';
#endif

    dwError = VmKdcRegGetDefaultRealm(&gVmkdcGlobals.pszDefaultRealm);
    if (dwError)
    {
        dwError = VmKdcAllocateStringA(VMKDC_ADMIN_DEFAULT_REALM,
                                       &gVmkdcGlobals.pszDefaultRealm);
    }
    BAIL_ON_VMKDC_ERROR(dwError);

    VmKdc_argv0 = strrchr(argv[0], pathsep);
    if (VmKdc_argv0)
    {
        VmKdc_argv0++;
    }
    else
    {
        VmKdc_argv0 = argv[0];
    }

    if (argc < 2)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (VmKdcStringCompareA("addprinc",
                            argv[1], TRUE) == 0)
    {
        cmd = VMKDC_ADMIN_COMMAND_ADDPRINC;
    } 
    else if (VmKdcStringCompareA("ktadd",
                                  argv[1], TRUE) == 0)
    {
        cmd = VMKDC_ADMIN_COMMAND_KTADD;
    }
    else
    {
        ShowUsage(VmKdc_argv0, "Unknown command");
    }

    parseArgs(argc-1, &argv[1], &args, &params);
    if (params == 0)
    {
        ShowUsage(VmKdc_argv0, "No UPN name specified");
    }
    params += 1; // Passed in argv[1]

    switch (cmd)
    {
      case VMKDC_ADMIN_COMMAND_ADDPRINC:
        dwError = VmKdcAdminAddPrinc(argc-params, &argv[params], &args);
        BAIL_ON_VMKDC_ERROR(dwError);
        break;

      case VMKDC_ADMIN_COMMAND_KTADD:
        dwError = VmKdcAdminKtAdd(argc-params, &argv[params], &args);
        BAIL_ON_VMKDC_ERROR(dwError);
        break;

      default:
        ShowUsage(VmKdc_argv0, "Unknown command");
        break;
    }

error:
    if (dwError == ERROR_INVALID_PARAMETER)
    {
        ShowUsage(VmKdc_argv0, NULL);
    }
    else if (dwError)
    {
        printf("command failed, dwError=%d\n", dwError);
    }

    VmKdcDestroyKrb5(gVmkdcGlobals.pKrb5Ctx);
    gVmkdcGlobals.pKrb5Ctx = NULL;

    VMKDC_SAFE_FREE_STRINGA(gVmkdcGlobals.pszDefaultRealm);

    VmKdcDestroyKrb5(gVmkdcGlobals.pKrb5Ctx);
    gVmkdcGlobals.pKrb5Ctx = NULL;

#ifdef _WIN32
    VmKdcDeallocateArgsA(argc, allocArgv);
    allocArgv = NULL;
#endif

    return dwError;
}

VOID
VmKdcAdminDestroyContext(
    PVMKDC_CONTEXT pContext)
{
    if (pContext)
    {
        if (pContext->pGlobals)
        {
            if (pContext->pGlobals->pKrb5Ctx)
            {
                VmKdcDestroyKrb5(pContext->pGlobals->pKrb5Ctx);
                pContext->pGlobals->pKrb5Ctx = NULL;
            }
            VMKDC_SAFE_FREE_STRINGA(pContext->pGlobals->pszDefaultRealm);
        }
        VMKDC_SAFE_FREE_MEMORY(pContext);
    }
}

DWORD
VmKdcAdminInitContext(
    PVMKDC_CONTEXT *ppRetContext)
{
    DWORD dwError = 0;
    PVMKDC_CONTEXT pContext = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_CONTEXT),
                                  (PVOID*)&pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    pContext->pGlobals = &gVmkdcGlobals;
    pContext->pRequest = NULL;

    dwError = VmKdcInitKrb5(&pContext->pGlobals->pKrb5Ctx);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (!pContext->pGlobals->pszDefaultRealm)
    {
        dwError = VmKdcAllocateStringA(VMKDC_ADMIN_DEFAULT_REALM,
                                       &pContext->pGlobals->pszDefaultRealm);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppRetContext = pContext;

error:
    if (dwError)
    {
        VmKdcAdminDestroyContext(pContext);
        pContext = NULL;
    }

    return dwError;
}
