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
 * Filename: addprinc.c
 *
 * Abstract:
 *
 * vmkdc_admin addprinc module
 *
 */

#include "includes.h"

DWORD
VmKdcAdminAddPrinc(int argc, char *argv[], PROG_ARGS *args)
{
    DWORD   dwError = 0;
    PCSTR   pszPassword = NULL;
    PSTR    pszUsername = NULL;
    PSTR    pszUPNName = NULL;
    BOOLEAN bRandKey = FALSE;
    PSTR    p = NULL;
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    PVMKDC_CONTEXT pContext = NULL;

    bRandKey = args->randKey;
    pszPassword = args->password;

    if (!bRandKey && !pszPassword)
    {
        ShowUsage(VmKdc_argv0, "no password or random key option specified");
    }

    if ( argc != 1 || IsNullOrEmptyString(argv[0]) ||
        (IsNullOrEmptyString(pszPassword) && !bRandKey ) )
    {
        ShowUsage(VmKdc_argv0, "Wrong number of principals specified");
    }

    dwError = VmKdcAdminInitContext(&pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Get the UPN name by parsing/unparsing the input name */
    dwError = VmKdcParsePrincipalName(pContext, argv[0], &pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcUnparsePrincipalName(pPrincipal, &pszUPNName);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Get the username by copying the UPN name and removing the realm */
    dwError = VmKdcAllocateStringA(pszUPNName, &pszUsername);
    BAIL_ON_VMKDC_ERROR(dwError);

    p = strchr(pszUsername, '@');
    if (p)
    {
        *p = '\0';
    }

    /* Create the user in vmdir */
    dwError = VmDirCreateUser((PSTR)pszUsername,
                              (PSTR)pszPassword,
                              pszUPNName,
                              bRandKey);
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);
    VMKDC_SAFE_FREE_STRINGA(pszUPNName);
    if (pContext)
    {
        VmKdcAdminDestroyContext(pContext);
        pContext = NULL;
    }

    return dwError;
}
