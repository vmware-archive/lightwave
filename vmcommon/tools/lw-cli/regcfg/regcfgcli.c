/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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

typedef enum
{
    PARSE_MODE_OPEN = 0,
    PARSE_MODE_GET_KEY,
    PARSE_MODE_SET_KEY,
    PARSE_MODE_SET_KEY_VALUE,
    PARSE_MODE_DELETE_KEY,
    PARSE_MODE_MERGE_FILE,
    PARSE_MODE_MERGE_FILE_FROM,
    PARSE_MODE_MERGE_FILE_INTO,
    PARSE_MODE_DONE,
} PARSE_MODE;

typedef struct _LW_CLI_REGCFG_PARAMS
{
    PARSE_MODE   mode;
    PSTR         pszKey;
    PSTR         pszValue;
    PSTR         pszFileFrom;
    PSTR         pszFileInto;
} LW_CLI_REGCFG_PARAMS, *PLW_CLI_REGCFG_PARAMS;

static
DWORD
ParseArgs(
    int   argc,
    char* argv[],
    PLW_CLI_REGCFG_PARAMS pSetupParams
    );

static
VOID
ShowUsage(
    VOID
    );

static
DWORD
_CLIRegCfgHandleKey(
        PLW_CLI_REGCFG_PARAMS pSetupParams
        )
{
    DWORD       dwError = 0;
    DWORD       dwCnt = 0;
    PSTR        pszLocalKey = NULL;
    CHAR        keyBuf[VM_SIZE_512] = {0};
    size_t      keyBufSize = sizeof(keyBuf);
    BOOLEAN     bReadOnly = (pSetupParams->mode == PARSE_MODE_GET_KEY);

    if (VmStringStartsWithA(pSetupParams->pszKey, "/vmdir/", FALSE))
    {
        dwError = VmRegConfigAddFile(VMREGCONFIG_VMDIR_REG_CONFIG_FILE, bReadOnly);
    }
    else if (VmStringStartsWithA(pSetupParams->pszKey, "/vmafd/", FALSE))
    {
        dwError = VmRegConfigAddFile(VMREGCONFIG_VMAFD_REG_CONFIG_FILE, bReadOnly);
    }
    else if (VmStringStartsWithA(pSetupParams->pszKey, "/vmca/", FALSE))
    {
        dwError = VmRegConfigAddFile(VMREGCONFIG_VMCA_REG_CONFIG_FILE, bReadOnly);
    }
    else if (VmStringStartsWithA(pSetupParams->pszKey, "/vmsts/", FALSE))
    {
        dwError = VmRegConfigAddFile(VMREGCONFIG_VMSTS_REG_CONFIG_FILE, bReadOnly);
    }
    else
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringPrintf(
                    &pszLocalKey,
                    "%s%s",
                    VM_REGCONFIG_TOP_KEY_PATH,
                    pSetupParams->pszKey);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; pszLocalKey[dwCnt] != '\0'; dwCnt++)
    {
            if (pszLocalKey[dwCnt] == '/')
            {
                pszLocalKey[dwCnt] = VM_REGCONFIG_KEY_SEPARATOR;
            }
    }

    switch (pSetupParams->mode)
    {
    case PARSE_MODE_GET_KEY:
        dwError = VmRegConfigGetKeyA(
                        pszLocalKey,
                        keyBuf,
                        &keyBufSize);
        if (dwError)
        {
            printf("get key (%s) failed, error (%d)\n", pSetupParams->pszKey, dwError);
        }
        else
        {
            printf("%s\n", keyBuf);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);

        break;

    case PARSE_MODE_SET_KEY:
        dwError = VmRegConfigSetKeyA(
                        pszLocalKey,
                        pSetupParams->pszValue,
                        VmStringLenA(pSetupParams->pszValue));
        if (dwError)
        {
            printf("set key (%s) failed, error (%d)\n", pSetupParams->pszKey, dwError);
        }
        else
        {
            printf("key (%s) new value set\n", pSetupParams->pszKey);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);

        break;

    case PARSE_MODE_DELETE_KEY:
        dwError = VmRegConfigDeleteKeyA(pszLocalKey);
        if (dwError)
        {
            printf("delete key (%s) failed, error (%d)\n", pSetupParams->pszKey, dwError);
        }
        else
        {
            printf("key (%s) deleted\n", pSetupParams->pszKey);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);

        break;

    default:
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;;
        break;
        }

error:
    return dwError;
}

int
CLIRegCfg(
    int argc,
    char* argv[])
{
    DWORD dwError = 0;
    LW_CLI_REGCFG_PARAMS setupParams = {0};

    if (argc == 0 || argv[0] == NULL || !strcmp(argv[0], "--help"))
    {
        ShowUsage();
        goto cleanup;
    }

    setlocale(LC_ALL, "");

    dwError = ParseArgs(argc, argv, &setupParams);
    if (dwError)
    {
        ShowUsage();
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmRegConfigInit();
    BAIL_ON_VM_COMMON_ERROR(dwError);

    switch (setupParams.mode)
    {
    case PARSE_MODE_GET_KEY:
    case PARSE_MODE_SET_KEY:
    case PARSE_MODE_DELETE_KEY:
            dwError = _CLIRegCfgHandleKey(&setupParams);
            break;

    case PARSE_MODE_MERGE_FILE:
            dwError = VmRegConfigMergeFile(
                    setupParams.pszFileFrom,
                    setupParams.pszFileInto);
            if (dwError)
            {
                printf("Merge file (%s) into (%s) failed, error (%d)\n",
                        setupParams.pszFileFrom,
                        setupParams.pszFileInto,
                        dwError);
            }
            else
            {
                printf("Merge file (%s) into (%s) successfully\n",
                        setupParams.pszFileFrom,
                        setupParams.pszFileInto);
            }
            break;

    default:
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            break;
    }
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    VmRegConfigFree();
    VM_COMMON_SAFE_FREE_MEMORY(setupParams.pszFileFrom);
    VM_COMMON_SAFE_FREE_MEMORY(setupParams.pszFileInto);
    VM_COMMON_SAFE_FREE_MEMORY(setupParams.pszKey);
    VM_COMMON_SAFE_FREE_MEMORY(setupParams.pszValue);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
ParseArgs(
    int   argc,
    char* argv[],
    PLW_CLI_REGCFG_PARAMS pSetupParams
    )
{
    DWORD dwError  = 0;
    PARSE_MODE     parseMode = PARSE_MODE_OPEN;
    PSTR  pszKey   = NULL;
    PSTR  pszKeyValue   = NULL;
    PSTR  pszFileFrom   = NULL;
    PSTR  pszFileInto   = NULL;
    PSTR  pszLocalStr   = NULL;

    int iArg = 0;

    for (; iArg < argc; iArg++)
    {
        char* pszArg = argv[iArg];

        switch (parseMode)
        {
        case PARSE_MODE_OPEN:
            if (!strcmp(pszArg, "get-key"))
            {
                parseMode = PARSE_MODE_GET_KEY;
            }
            else if (!strcmp(pszArg, "set-key"))
            {
                parseMode = PARSE_MODE_SET_KEY;
            }
            else if (!strcmp(pszArg, "delete-key"))
            {
                parseMode = PARSE_MODE_DELETE_KEY;
            }
            else if (!strcmp(pszArg, "merge-file"))
            {
                parseMode = PARSE_MODE_MERGE_FILE_FROM;
            }

            break;

        case PARSE_MODE_GET_KEY:
            if (pszKey)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszKey = pszArg;
            pSetupParams->mode = PARSE_MODE_GET_KEY;
            parseMode = PARSE_MODE_DONE;

            break;

        case PARSE_MODE_SET_KEY:
            if (pszKey)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszKey = pszArg;
            pSetupParams->mode = PARSE_MODE_SET_KEY;
            parseMode = PARSE_MODE_SET_KEY_VALUE;

            break;

        case PARSE_MODE_SET_KEY_VALUE:
            if (pszKeyValue)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszKeyValue = pszArg;
            parseMode = PARSE_MODE_DONE;

            break;

        case PARSE_MODE_DELETE_KEY:
            if (pszKey)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszKey = pszArg;
            pSetupParams->mode = PARSE_MODE_DELETE_KEY;
            parseMode = PARSE_MODE_DONE;

            break;

        case PARSE_MODE_MERGE_FILE_FROM:
            if (pszFileFrom)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszFileFrom = pszArg;
            pSetupParams->mode = PARSE_MODE_MERGE_FILE_INTO;
            parseMode = PARSE_MODE_MERGE_FILE_INTO;

            break;

        case PARSE_MODE_MERGE_FILE_INTO:
            if (pszFileInto)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
            }

            pszFileInto = pszArg;
            pSetupParams->mode = PARSE_MODE_MERGE_FILE;
            parseMode = PARSE_MODE_DONE;

            break;

        default:
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);

            break;
        }
    }

    if (parseMode != PARSE_MODE_DONE)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (pszKey)
    {
        dwError = VmAllocateStringA(pszKey, &pszLocalStr);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pSetupParams->pszKey = pszLocalStr;
        pszLocalStr = NULL;
    }

    if (pszKeyValue)
    {
        dwError = VmAllocateStringA(pszKeyValue, &pszLocalStr);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pSetupParams->pszValue = pszLocalStr;
        pszLocalStr = NULL;
    }

    if (pszFileFrom)
    {
        dwError = VmAllocateStringA(pszFileFrom, &pszLocalStr);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pSetupParams->pszFileFrom = pszLocalStr;
        pszLocalStr = NULL;
    }

    if (pszFileInto)
    {
        dwError = VmAllocateStringA(pszFileInto, &pszLocalStr);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pSetupParams->pszFileInto = pszLocalStr;
        pszLocalStr = NULL;
    }


cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocalStr);
    goto cleanup;
}

static
VOID
ShowUsage(
    VOID
    )
{
    PSTR pszUsageText =
           "Usage: lw-cli regcfg get-key keyname\n"
           "                     set-key keyname value\n"
           "                     delete-key keyname\n"
           "                     merge-file from-file into-file\n\n"
           "    keyname should be full path form such as: /vmafd/description\n";
    printf("%s", pszUsageText);
}
