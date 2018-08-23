/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include <includes.h>

DWORD
VMCAOpenLocalLdapServer(
    PVMCA_LDAP_CONTEXT* pLd
    )
{
    DWORD dwError = 0;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDN = NULL;
    PSTR pszUPN = NULL;
    PVMCA_LDAP_CONTEXT pContext = NULL;

    if (pLd == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCASrvGetMachineAccountInfoA(
                &pszAccount,
                &pszDN,
                &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                &pszUPN,
                "%s@%s",
                pszAccount,
                pszDN);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCALdapConnect(
                    "localhost",
                    0, /* use default port */
                    pszUPN,
                    pszPassword,
                    &pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    *pLd = pContext;
cleanup:
    VMCA_SAFE_FREE_STRINGA(pszUPN);
    VMCA_SAFE_FREE_STRINGA(pszDN);
    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszPassword);
    return dwError;

error:
    goto cleanup;
}
