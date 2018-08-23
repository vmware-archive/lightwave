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
VMCAPolicyCNValidate(
    PCSTR                           pszPKCS10Request,
    PVMCA_REQ_CONTEXT               pReqContext,
    PBOOLEAN                        pbIsValid
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwNumCNs = 0;
    DWORD                           dwIdx = 0;
    PSTR                            *ppszCNEntries = NULL;
    BOOLEAN                         bIsFQDN = FALSE;
    BOOLEAN                         bInWhitelist = FALSE;

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pReqContext ||
        !pbIsValid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAOpenSSLGetValuesFromSubjectName(
                            pszPKCS10Request,
                            VMCA_OPENSSL_NID_CN,
                            &dwNumCNs,
                            &ppszCNEntries);
    BAIL_ON_VMCA_ERROR(dwError);

    for (; dwIdx < dwNumCNs; ++dwIdx)
    {
        if (VMCAUtilIsValueIPAddr(ppszCNEntries[dwIdx]))
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] CN policy violation: CN value is IP Address (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszCNEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAUtilIsValueFQDN(
                        ppszCNEntries[dwIdx],
                        &bIsFQDN);
        BAIL_ON_VMCA_ERROR(dwError);

        if (bIsFQDN)
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] CN policy violation: CN value is FQDN (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszCNEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (VMCAUtilDoesValueHaveWildcards(ppszCNEntries[dwIdx]))
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] CN policy violation: CN value contains Wildcards (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszCNEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAUtilIsValueInWhitelist(
                                ppszCNEntries[dwIdx],
                                pReqContext->pszAuthPrincipal,
                                VMCA_REG_KEY_CN_WHITELIST,
                                &bInWhitelist);
        BAIL_ON_VMCA_ERROR(dwError);

        if (!bInWhitelist)
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] CN policy violation: CN value is not in whitelist (%s). UPN (%s)",
                    __FUNCTION__,
                    __LINE__,
                    ppszCNEntries[dwIdx],
                    pReqContext->pszAuthPrincipal);

            dwError = VMCA_POLICY_VALIDATION_ERROR;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *pbIsValid = TRUE;


cleanup:

    VMCAFreeStringArrayA(ppszCNEntries, dwNumCNs);

    return dwError;

error:

    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}
