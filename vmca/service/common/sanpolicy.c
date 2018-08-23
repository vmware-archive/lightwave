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
VMCAPolicySANValidate(
    PCSTR                           pszPKCS10Request,
    PVMCA_REQ_CONTEXT               pReqContext,
    PBOOLEAN                        pbIsValid
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwNumDNSSANs = 0;
    DWORD                           dwNumIPSANs = 0;
    DWORD                           dwIdx = 0;
    PSTR                            *ppszSANDNSEntries = NULL;
    PSTR                            *ppszSANIPEntries = NULL;
    BOOLEAN                         bInWhitelist = FALSE;

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pReqContext ||
        !pbIsValid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAOpenSSLGetSANEntries(
                        pszPKCS10Request,
                        &dwNumDNSSANs,
                        &ppszSANDNSEntries,
                        &dwNumIPSANs,
                        &ppszSANIPEntries);
    BAIL_ON_VMCA_ERROR(dwError);

    for (; dwIdx < dwNumIPSANs; ++dwIdx)
    {
        if (!VMCAUtilIsValuePrivateOrLocalIPAddr(ppszSANIPEntries[dwIdx]))
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] SAN policy violation: SAN IP value is a public address (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszSANIPEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    for (dwIdx = 0; dwIdx < dwNumDNSSANs; ++dwIdx)
    {
        dwError = VMCAUtilIsValueInWhitelist(
                                ppszSANDNSEntries[dwIdx],
                                pReqContext->pszAuthPrincipal,
                                VMCA_REG_KEY_SAN_WHITELIST,
                                &bInWhitelist);
        BAIL_ON_VMCA_ERROR(dwError);

        if (!bInWhitelist)
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] SAN policy violation: SAN value is not in whitelist (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszSANDNSEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *pbIsValid = TRUE;


cleanup:

    VMCAFreeStringArrayA(ppszSANDNSEntries, dwNumDNSSANs);
    VMCAFreeStringArrayA(ppszSANIPEntries, dwNumIPSANs);

    return dwError;

error:

    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}
