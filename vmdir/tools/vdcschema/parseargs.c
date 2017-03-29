/*
 * Copyright © 2016-2017 VMware, Inc.  All Rights Reserved.
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
VdcSchemaParseArgs(
    int                     argc,
    char*                   argv[],
    PVDC_SCHEMA_CONN*       ppConn,
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    )
{
    DWORD   dwError = 0;
    PVDC_SCHEMA_CONN        pConn = NULL;
    PVDC_SCHEMA_OP_PARAM    pOpParam = NULL;

    if (argc < 4 || !ppConn || !ppOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaConnInit(&pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaOpParamInit(&pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA("get-supported-syntaxes", argv[1], TRUE) == 0)
    {
        VMDIR_COMMAND_LINE_OPTION options[] =
        {
                {0, "domain",   CL_STRING_PARAMETER,    &pConn->pszDomain},
                {0, "host",     CL_STRING_PARAMETER,    &pConn->pszHostName},
                {0, "login",    CL_STRING_PARAMETER,    &pConn->pszUserName},
                {0, "passwd",   CL_STRING_PARAMETER,    &pConn->pszPassword},
                {0, 0, 0, 0}
        };

        pOpParam->opCode = OP_GET_SUPPORTED_SYNTAXES;

        dwError = VmDirParseArguments(options, NULL, argc - 1, argv + 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA("patch-schema-defs", argv[1], TRUE) == 0)
    {
        VMDIR_COMMAND_LINE_OPTION options[] =
        {
                {0, "domain",   CL_STRING_PARAMETER,    &pConn->pszDomain},
                {0, "host",     CL_STRING_PARAMETER,    &pConn->pszHostName},
                {0, "login",    CL_STRING_PARAMETER,    &pConn->pszUserName},
                {0, "passwd",   CL_STRING_PARAMETER,    &pConn->pszPassword},
                {0, "file",     CL_STRING_PARAMETER,    &pOpParam->pszFileName},
                {0, "dryrun",   CL_NO_PARAMETER,        &pOpParam->bDryrun},
                {0, 0, 0, 0}
        };

        pOpParam->opCode = OP_PATCH_SCHEMA_DEFS;

        dwError = VmDirParseArguments(options, NULL, argc - 1, argv + 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA("get-schema-repl-status", argv[1], TRUE) == 0)
    {
        VMDIR_COMMAND_LINE_OPTION options[] =
        {
                {0, "domain",   CL_STRING_PARAMETER,    &pConn->pszDomain},
                {0, "host",     CL_STRING_PARAMETER,    &pConn->pszHostName},
                {0, "login",    CL_STRING_PARAMETER,    &pConn->pszUserName},
                {0, "passwd",   CL_STRING_PARAMETER,    &pConn->pszPassword},
                {0, "verbose",  CL_NO_PARAMETER,        &pOpParam->bVerbose},
                {0, "timeout",  CL_INTEGER_PARAMETER,   &pOpParam->iTimeout},
                {0, 0, 0, 0}
        };

        pOpParam->opCode = OP_GET_SCHEMA_REPL_STATUS;

        dwError = VmDirParseArguments(options, NULL, argc - 1, argv + 1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaConnValidateAndSetDefault(pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaOpParamValidate(pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConn = pConn;
    *ppOpParam = pOpParam;

cleanup:
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
            "\t                 [ --dryrun ]\n"
            "\n"
            "\tget-schema-repl-status\n"
            "\t                   --domain <domain-name>\n"
            "\t                 [ --host   <host-name> ]\n"
            "\t                 [ --login  <user-name> ]\n"
            "\t                 [ --passwd <password> ]\n"
            "\t                 [ --verbose ]\n"
            "\t                 [ --timeout <seconds> ]\n");
}
