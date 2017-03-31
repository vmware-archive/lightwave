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

VOID
FreeCLStateContent(
    PCOMMAND_LINE_STATE pState
    )
{
    if (pState)
    {
        VMDIR_SAFE_FREE_MEMORY(pState->pszHostName);
        VMDIR_SAFE_FREE_MEMORY(pState->pszLoginUserUPN);
        VMDIR_SAFE_FREE_MEMORY(pState->pszUserUPN);
        VMDIR_SECURE_FREE_STRINGA(pState->pszLoginPassword);
        VMDIR_SECURE_FREE_STRINGA(pState->pszNewPassword);
    }
}
