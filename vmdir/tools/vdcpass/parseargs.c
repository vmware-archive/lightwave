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
 * Module Name: vdcpass
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcpass argument parsing functions
 *
 */

#include "includes.h"

static
VOID
ShowUsage(
    PVOID pvContext
    )
{
    printf(
      "Usage: vdcpass -h <hostname> -u <user UPN> -w <user password> -W <new password>\n"
      "    [-U <user UPN for password change>]\n"
      "Note: change password needs administrator privilege.\n");
}

static
DWORD
HandleServerParameterCallback(
    PVOID pvContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

    pContext->pszHostName = pValue;

    return VMDIR_SUCCESS;
}

static
DWORD
HandleUserNameParameterCallback(
    PVOID pvContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

    pContext->pszLoginUserUPN = pValue;

    return VMDIR_SUCCESS;
}

static
DWORD
HandlePasswordParameterCallback(
    PVOID pvContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

    pContext->pszLoginPassword = pValue;

    return VMDIR_SUCCESS;
}

static
DWORD
HandleNewPassParameterCallback(
    PVOID pvContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

    pContext->pszNewPassword = pValue;

    return VMDIR_SUCCESS;
}

static
DWORD
HandleNewLoginParameterCallback(
    PVOID pvContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

    pContext->pszUserUPN = pValue;

    return VMDIR_SUCCESS;
}

static
DWORD
PostValidationRoutine(
    PVOID pvContext
    )
{
     PCOMMAND_LINE_STATE pContext = (PCOMMAND_LINE_STATE)pvContext;

     //
     // These parameters are all required.
     //
     if (pContext->pszHostName == NULL ||
         pContext->pszLoginUserUPN == NULL ||
         pContext->pszNewPassword == NULL)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    return VMDIR_SUCCESS;
}

VMDIR_COMMAND_LINE_OPTIONS CommandLineOptions =
{
    ShowUsage,
    PostValidationRoutine,
    {
        {'h', "host", CL_STRING_PARAMETER, HandleServerParameterCallback},
        {'u', "username", CL_STRING_PARAMETER, HandleUserNameParameterCallback},
        {'U', "newuser", CL_STRING_PARAMETER, HandleNewLoginParameterCallback},
        {'w', "password", CL_STRING_PARAMETER, HandlePasswordParameterCallback},
        {'W', "newpass", CL_STRING_PARAMETER, HandleNewPassParameterCallback},
        {0, 0, 0, 0}
    }
};
