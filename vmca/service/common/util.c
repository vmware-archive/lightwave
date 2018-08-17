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

BOOLEAN
VMCAUtilIsValueIPAddr(
    PCSTR           pszValue
    )
{
    BOOLEAN         bIsIP = FALSE;

    if (!IsNullOrEmptyString(pszValue))
    {
        unsigned char buf[sizeof(struct in_addr)];

        bIsIP = (inet_pton(AF_INET, pszValue, &buf[0]) == 1);
    }

    return bIsIP;
}

/*
 * The IP Address in pszValue can be in either dot-decimal notation
 * or hexadecimal notation
 */
BOOLEAN
VMCAUtilIsValuePrivateOrLocalIPAddr(
    PSTR            pszValue
    )
{
    uint32_t        unIp = 0x0;

    if (!IsNullOrEmptyString(pszValue))
    {
        unIp = inet_network(pszValue);

        if (VMCA_IS_IP_IN_PRIVATE_NETWORK(unIp))
        {
            return TRUE;
        }
        else if (VMCA_IS_IP_IN_LOCAL_NETWORK(unIp))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}

DWORD
VMCAUtilIsValueFQDN(
    PCSTR           pszValue,
    PBOOLEAN        pbIsValid
    )
{
    DWORD           dwError = 0;
    DWORD           dwIdx = 0;
    PSTR            pszTempVal = NULL;
    PSTR            pszLabel = NULL;
    PSTR            pszNextTok = NULL;
    BOOLEAN         bIsValid = FALSE;
    int             count = 0;
    char            cTemp = 0;
    size_t          szLabelLen = 0;

    if (IsNullOrEmptyString(pszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAStringCountSubstring(
                        pszValue,
                        ".",
                        &count);
    BAIL_ON_VMCA_ERROR(dwError);
    if (count == 0)
    {
        bIsValid = FALSE;
        goto ret;
    }

    if (VMCAStringLenA(pszValue) > 255)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringPrintfA(
                            &pszTempVal,
                            pszValue);
    BAIL_ON_VMCA_ERROR(dwError);

    pszLabel = VMCAStringTokA(pszTempVal, ".", &pszNextTok);
    while (pszLabel)
    {
        szLabelLen = VMCAStringLenA(pszLabel);

        if (szLabelLen > 63)
        {
            bIsValid = FALSE;
            goto ret;
        }

        if (!isalpha(pszLabel[0]) &&
            !isalpha(pszLabel[szLabelLen - 1]) &&
            !isdigit(pszLabel[szLabelLen - 1]))
        {
            bIsValid = FALSE;
            goto ret;
        }

        while ((cTemp = *(pszLabel + dwIdx)) != '\0')
        {
            if (!isalpha(cTemp) &&
                !isdigit(cTemp) &&
                (cTemp != '-'))
            {
                bIsValid = FALSE;
                goto ret;
            }

            ++dwIdx;
        }

        pszLabel = VMCAStringTokA(NULL, ".", &pszNextTok);
    }

    bIsValid = TRUE;


ret:

    *pbIsValid = bIsValid;

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszTempVal);

    return dwError;

error:

    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

BOOLEAN
VMCAUtilDoesValueHaveWildcards(
    PCSTR            pszValue
    )
{
    DWORD           dwError = 0;
    int             count = 0;

    if (IsNullOrEmptyString(pszValue))
    {
        return FALSE;
    }

    dwError = VMCAStringCountSubstring(
                        pszValue,
                        "*",
                        &count);
    if (count != 0)
    {
        return TRUE;
    }

    return FALSE;
}

DWORD
VMCAUtilIsValueInWhitelist(
    PCSTR                           pszValue,
    PCSTR                           pszAuthUPN,
    PCSTR                           pcszRegValue,
    PBOOLEAN                        pbInWhitelist
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwNumWhitelistEntries = 0;
    DWORD                           dwIdx = 0;
    PVMW_CFG_CONNECTION             pConnection = NULL;
    PVMW_CFG_KEY                    pRootKey = NULL;
    PVMW_CFG_KEY                    pParamsKey = NULL;
    PSTR                            *ppszWhitelist = NULL;
    PSTR                            pszHostnameTemplate = NULL;
    PSTR                            pszHostname = NULL;
    PSTR                            pszHostFQDN = NULL;
    PSTR                            pszHostnameValue = NULL;
    PSTR                            pszDomain = NULL;
    PSTR                            pszNextTok = NULL;
    PSTR                            pszTempValue = NULL;
    PSTR                            pszTempValue2 = NULL;
    PSTR                            pszPtr = NULL;
    PCSTR                           pcszParamsKeyPath = VMCA_CONFIG_PARAMETER_KEY_PATH;
    BOOLEAN                         bInWhitelist = FALSE;
    int                             nCount = 0;

    if (IsNullOrEmptyString(pszValue) ||
        IsNullOrEmptyString(pcszRegValue) ||
        !pbInWhitelist)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pcszParamsKeyPath,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringArrayValue(
                    pParamsKey,
                    NULL,
                    pcszRegValue,
                    &dwNumWhitelistEntries,
                    &ppszWhitelist);
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        VMCA_LOG_INFO(
                "[%s,%d] (%s) Whitelist not found in registry.  Returning true",
                __FUNCTION__,
                __LINE__,
                pcszRegValue);

        dwError = 0;
        bInWhitelist = TRUE;
        goto ret;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    for (; dwIdx < dwNumWhitelistEntries; ++dwIdx)
    {
        dwError = VMCAStringCountSubstring(
                            ppszWhitelist[dwIdx],
                            VMCA_POLICY_HOSTNAME_TEMPLATE,
                            &nCount);
        BAIL_ON_VMCA_ERROR(dwError);

        if (!VMCAStringCompareA(VMCA_POLICY_MA_NAME, ppszWhitelist[dwIdx], FALSE) ||
            nCount)
        {
            dwError = VMCAAllocateStringA(
                            pszAuthUPN,
                            &pszTempValue);
            BAIL_ON_VMCA_ERROR(dwError);

            pszHostFQDN = VMCAStringTokA(pszTempValue, "@", &pszNextTok);
            if (!pszHostFQDN)
            {
                nCount = 0;
                VMCA_SAFE_FREE_STRINGA(pszTempValue);
                VMCA_SAFE_FREE_STRINGA(pszTempValue2);
                continue;
            }

            pszHostname = VMCAStringTokA(pszHostFQDN, ".", &pszNextTok);
            if (!pszHostname)
            {
                nCount = 0;
                VMCA_SAFE_FREE_STRINGA(pszTempValue);
                VMCA_SAFE_FREE_STRINGA(pszTempValue2);
                continue;
            }

            if (nCount) {
                dwError = VMCAAllocateStringA(
                                ppszWhitelist[dwIdx],
                                &pszTempValue2);
                BAIL_ON_VMCA_ERROR(dwError);

                pszPtr = strstr(ppszWhitelist[dwIdx], VMCA_POLICY_HOSTNAME_TEMPLATE);
                if (!pszPtr)
                {
                    nCount = 0;
                    VMCA_SAFE_FREE_STRINGA(pszTempValue);
                    VMCA_SAFE_FREE_STRINGA(pszTempValue2);
                    continue;
                }

                dwError = VMCAAllocateMemory(
                                    (strlen(pszTempValue2) - strlen(pszPtr)) * sizeof(char) + 1,
                                    (PVOID *)&pszHostnameTemplate);
                BAIL_ON_VMCA_ERROR(dwError);

                dwError = VMCAStringNCpyA(
                                    pszHostnameTemplate,
                                    VMCAStringLenA(pszTempValue2),
                                    pszTempValue2,
                                    (strlen(pszTempValue2) - strlen(pszPtr)));
                BAIL_ON_VMCA_ERROR(dwError);

                dwError = VMCAAllocateStringPrintfA(
                                    &pszHostnameValue,
                                    "%s%s",
                                    pszHostnameTemplate,
                                    pszHostname);
                BAIL_ON_VMCA_ERROR(dwError);
            }
            else
            {
                dwError = VMCAAllocateStringPrintfA(
                                    &pszHostnameValue,
                                    "%s",
                                    pszHostname);
                BAIL_ON_VMCA_ERROR(dwError);
            }


            if (!VMCAStringCompareA(pszHostnameValue, pszValue, FALSE))
            {
                bInWhitelist = TRUE;
                goto ret;
            }

            nCount = 0;
            VMCA_SAFE_FREE_STRINGA(pszTempValue);
            VMCA_SAFE_FREE_STRINGA(pszTempValue2);
        }

        if (!VMCAStringCompareA(VMCA_POLICY_MA_FQDN, ppszWhitelist[dwIdx], FALSE))
        {
            dwError = VMCAAllocateStringA(
                            pszAuthUPN,
                            &pszTempValue);
            BAIL_ON_VMCA_ERROR(dwError);

            pszHostFQDN = VMCAStringTokA(
                            pszTempValue,
                            "@",
                            &pszNextTok);
            if (!pszHostFQDN)
            {
                VMCA_SAFE_FREE_STRINGA(pszTempValue);
                continue;
            }

            if (!VMCAStringCompareA(pszHostFQDN, pszValue, FALSE))
            {
                bInWhitelist = TRUE;
                goto ret;
            }

            VMCA_SAFE_FREE_STRINGA(pszTempValue);
        }

        dwError = VMCAStringCountSubstring(
                            ppszWhitelist[dwIdx],
                            "*.",
                            &nCount);
        BAIL_ON_VMCA_ERROR(dwError);

        if (nCount)
        {
            dwError = VMCAAllocateStringA(
                            ppszWhitelist[dwIdx],
                            &pszTempValue);
            BAIL_ON_VMCA_ERROR(dwError);

            pszPtr = strstr(pszTempValue, "*.");
            if (pszPtr != pszTempValue)
            {
                // Wildcards can only be the first label
                nCount = 0;
                VMCA_SAFE_FREE_STRINGA(pszTempValue);
                VMCA_SAFE_FREE_STRINGA(pszDomain);
                continue;
            }

            dwError = VMCAAllocateStringA(
                                (pszTempValue + 2),
                                &pszDomain);
            BAIL_ON_VMCA_ERROR(dwError);

            nCount = 0;
            dwError = VMCAStringCountSubstring(
                                pszValue,
                                pszDomain,
                                &nCount);
            BAIL_ON_VMCA_ERROR(dwError);

            if (nCount)
            {
                bInWhitelist = TRUE;
                goto ret;
            }

            nCount = 0;
            VMCA_SAFE_FREE_STRINGA(pszTempValue);
            VMCA_SAFE_FREE_STRINGA(pszDomain);
        }

        if (!VMCAStringCompareA(pszValue, ppszWhitelist[dwIdx], FALSE))
        {
            bInWhitelist = TRUE;
            goto ret;
        }
    }

    bInWhitelist = FALSE;


ret:

    *pbInWhitelist = bInWhitelist;

cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmwConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmwConfigCloseConnection(pConnection);
    }
    VMCAFreeStringArrayA(ppszWhitelist, dwNumWhitelistEntries);
    VMCA_SAFE_FREE_STRINGA(pszHostnameTemplate);
    VMCA_SAFE_FREE_STRINGA(pszHostnameValue);
    VMCA_SAFE_FREE_STRINGA(pszTempValue);
    VMCA_SAFE_FREE_STRINGA(pszTempValue2);

    return dwError;

error:

    if (pbInWhitelist)
    {
        *pbInWhitelist = FALSE;
    }

    goto cleanup;
}
