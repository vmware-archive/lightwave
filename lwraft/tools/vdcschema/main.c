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

int VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;
    PSTR    pszErrorMessage = NULL;
    PVDC_SCHEMA_CONN        pConn = NULL;
    PVDC_SCHEMA_OP_PARAM    pOpParam = NULL;

    dwError = VmDirLogInitialize(
            NULL, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaParseArgs(argc, argv, &pConn, &pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaConnOpen(pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pOpParam->opCode == OP_GET_SUPPORTED_SYNTAXES)
    {
        dwError = VdcSchemaOpGetSupportedSyntaxes(pConn, pOpParam);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pOpParam->opCode == OP_PATCH_SCHEMA_DEFS)
    {
        dwError = VdcSchemaOpPatchSchemaDefs(pConn, pOpParam);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VdcSchemaFreeConn(pConn);
    VdcSchemaFreeOpParam(pOpParam);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("\nError %d - %s\n", dwError, pszErrorMessage);
    goto cleanup;
}


#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszArgs);
    }
    return dwError;
}

#else

int main(int argc, char* argv[])
{
    return VmDirMain(argc, argv);
}

#endif
