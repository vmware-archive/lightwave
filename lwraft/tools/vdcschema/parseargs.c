/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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

static
DWORD
_DeleteArgAt(
    PSTR*   ppszArgs,
    DWORD   dwDelIdx
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;

    if (!ppszArgs)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_SAFE_FREE_MEMORY(ppszArgs[dwDelIdx]);

    for (i = dwDelIdx; ppszArgs[i+1]; i++)
    {
        ppszArgs[i] = ppszArgs[i+1];
        ppszArgs[i+1] = NULL;
    }

error:
    return dwError;
}

static
DWORD
_ParseConn(
    PSTR*               ppszArgs,
    PVDC_SCHEMA_CONN*   ppConn
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR*   ppszArg = NULL;
    PVDC_SCHEMA_CONN   pConn = NULL;

    if (!ppszArgs || !ppConn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaConnInit(&pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    i = 1;
    while (ppszArgs[i])
    {
        if (VmDirStringCompareA("--domain", ppszArgs[i], TRUE) == 0)
        {
            ppszArg = &pConn->pszDomain;
        }
        else if (VmDirStringCompareA("--host", ppszArgs[i], TRUE) == 0)
        {
            ppszArg = &pConn->pszHostName;
        }
        else if (VmDirStringCompareA("--login", ppszArgs[i], TRUE) == 0)
        {
            ppszArg = &pConn->pszUserName;
        }
        else if (VmDirStringCompareA("--passwd", ppszArgs[i], TRUE) == 0)
        {
            ppszArg = &pConn->pszPassword;
        }
        else
        {
            i++;
            continue;
        }

        if (!ppszArgs[i+1])
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateStringA(ppszArgs[i+1], ppszArg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _DeleteArgAt(ppszArgs, i+1);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _DeleteArgAt(ppszArgs, i);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaConnValidateAndSetDefault(pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConn = pConn;

cleanup:
    return dwError;

error:
    VdcSchemaFreeConn(pConn);
    goto cleanup;
}

static
DWORD
_ParseOpParam(
    PSTR*                   ppszArgs,
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    )
{
    DWORD   dwError = 0;
    PVDC_SCHEMA_OP_PARAM    pOpParam = NULL;

    if (!ppszArgs || !ppOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaOpParamInit(&pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA("get-supported-syntaxes", ppszArgs[0], TRUE) == 0)
    {
        pOpParam->opCode = OP_GET_SUPPORTED_SYNTAXES;
    }
    else if (VmDirStringCompareA("patch-schema-defs", ppszArgs[0], TRUE) == 0)
    {
        pOpParam->opCode = OP_PATCH_SCHEMA_DEFS;
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _DeleteArgAt(ppszArgs, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (ppszArgs[0])
    {
        if (pOpParam->opCode == OP_PATCH_SCHEMA_DEFS &&
            VmDirStringCompareA("--file", ppszArgs[0], TRUE) == 0 &&
            ppszArgs[1])
        {
            dwError = VmDirAllocateStringA(ppszArgs[1], &pOpParam->pszFileName);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = _DeleteArgAt(ppszArgs, 1);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = _DeleteArgAt(ppszArgs, 0);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (pOpParam->opCode == OP_PATCH_SCHEMA_DEFS &&
                 VmDirStringCompareA("--dryrun", ppszArgs[0], TRUE) == 0)
        {
            pOpParam->bDryrun = TRUE;

            dwError = _DeleteArgAt(ppszArgs, 0);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // Unrecognizable argument detected
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppOpParam = pOpParam;

cleanup:
    return dwError;

error:
    VdcSchemaFreeOpParam(pOpParam);
    goto cleanup;
}

DWORD
VdcSchemaParseArgs(
    int                     argc,
    char*                   argv[],
    PVDC_SCHEMA_CONN*       ppConn,
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    )
{
    DWORD   dwError = 0;
    int     i = 0;
    PSTR*   ppszArgs = NULL;
    PVDC_SCHEMA_CONN        pConn = NULL;
    PVDC_SCHEMA_OP_PARAM    pOpParam = NULL;

    if (argc < 4 || !ppConn || !ppOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 1; i < argc; i++)
    {
        dwError = VmDirAllocateStringA(argv[i], &ppszArgs[i-1]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _ParseConn(ppszArgs, &pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _ParseOpParam(ppszArgs, &pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConn = pConn;
    *ppOpParam = pOpParam;

cleanup:
    VmDirFreeStrArray(ppszArgs);
    return dwError;

error:
    VdcSchemaFreeConn(pConn);
    VdcSchemaFreeOpParam(pOpParam);
    VdcSchemaShowUsage();
    goto cleanup;
}

VOID
VdcSchemaShowUsage(
    VOID
    )
{
    printf( "Usage: vdcschema { arguments }\n"
            "\n"
            "Arguments:\n"
            "\n"
            "\tget-supported-syntaxes\n"
            "\t                   --domain <domain-name>\n"
            "\t                 [ --host   <host-name> ]\n"
            "\t                 [ --login  <user-name> ]\n"
            "\t                 [ --passwd <password> ]\n"
            "\n"
            "\tpatch-schema-defs\n"
            "\t                   --file   <filename>\n"
            "\t                   --domain <domain-name>\n"
            "\t                 [ --host   <host-name> ]\n"
            "\t                 [ --login  <user-name> ]\n"
            "\t                 [ --passwd <password> ]\n"
            "\t                 [ --dryrun ]\n");
}
