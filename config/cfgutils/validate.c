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



#include "includes.h"

DWORD
VmwDeployValidateHostname(
    PCSTR pszHostname
    )
{
    DWORD dwError = 0;

    VMW_DEPLOY_LOG_DEBUG(
            "Validating hostname [%s]", 
            VMW_DEPLOY_SAFE_LOG_STRING(pszHostname));

    if (IsNullOrEmptyString(pszHostname) ||
        !strcmp(pszHostname, "localhost") ||
        !strcmp(pszHostname, "localhost.localdom"))
    {
        dwError = ERROR_INVALID_NETNAME;

        VMW_DEPLOY_LOG_ERROR(
            "Error : Invalid hostname [%s]", 
            VMW_DEPLOY_SAFE_LOG_STRING(pszHostname));
    }

    return dwError;
}

DWORD
VmwDeployValidateOrgUnit(
    PCSTR pszOrgUnit
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasSpecialChars = FALSE;

    VMW_DEPLOY_LOG_DEBUG(
            "Validating organizational unit [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pszOrgUnit));

    if (!IsNullOrEmptyString(pszOrgUnit))
    {
        PCSTR pszCursor = pszOrgUnit;

        while (*pszCursor && !bHasSpecialChars)
        {
            switch (*pszCursor)
            {
                case '!':
                case '@':
                case '#':
                case '$':
                case '%':
                case '^':
                case '&':
                case '*':
                case '[':
                case ']':
                     bHasSpecialChars = TRUE;
                     break;
                default:
                     pszCursor++;
                     break;
            }
        }
    }

    if (bHasSpecialChars)
    {
        VMW_DEPLOY_LOG_ERROR(
                "Organizational unit [%s] has invalid characters",
                VMW_DEPLOY_SAFE_LOG_STRING(pszOrgUnit));

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VmwDeployValidatePassword(
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    size_t iCh = 0;
    size_t nUpper = 0;
    size_t nLower = 0;
    size_t nDigit = 0;
    size_t nSpecial = 0;
    size_t sLen = 0;

    VMW_DEPLOY_LOG_DEBUG("Validating password");

    if (IsNullOrEmptyString(pszPassword) || (sLen = strlen(pszPassword)) < 8)
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    // We are looking for at least one upper case, one lower case, one digit and
    // one special case character. Added illegal chars check
    for (iCh = 0; iCh < sLen; iCh++)
    {
        int ch = pszPassword[iCh];

        if (isdigit(ch))
        {
            nDigit++;
        }
        else if (islower(ch))
        {
            nLower++;
        }
        else if (isupper(ch))
        {
            nUpper++;
        }
        else if (ispunct(ch))
        {
            nSpecial++;
        }
    }

    if (!nUpper || !nLower || !nDigit || !nSpecial)
    {
        VMW_DEPLOY_LOG_ERROR("Password complexity requirement not satisfied");

        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VmwDeployValidateSiteName(
    PCSTR pszSite
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasSpecialChars = FALSE;

    VMW_DEPLOY_LOG_DEBUG(
            "Validating site name [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pszSite));

    if (!IsNullOrEmptyString(pszSite))
    {
        PCSTR pszCursor = pszSite;

        while (*pszCursor && !bHasSpecialChars)
        {
            switch (*pszCursor)
            {
                case '!':
                case '@':
                case '#':
                case '$':
                case '%':
                case '^':
                case '&':
                case '*':
                case '[':
                case ']':
                     bHasSpecialChars = TRUE;
                     break;
                default:
                     pszCursor++;
                     break;
            }
        }
    }

    if (bHasSpecialChars)
    {
        VMW_DEPLOY_LOG_ERROR(
                "Site name [%s] has invalid characters",
                VMW_DEPLOY_SAFE_LOG_STRING(pszSite));

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VmwDeployGetPartnerDomain(
    PCSTR pszServer,
    PSTR* ppszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszVmDirDomain = NULL;
    PSTR  pszDomain = NULL;

    if (IsNullOrEmptyString(pszServer))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(pszServer, &pszVmDirDomain);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pszVmDirDomain, &pszDomain);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszDomain = pszDomain;

cleanup:

    if (pszVmDirDomain)
    {
        VmDirFreeMemory(pszVmDirDomain);
    }
 
    return dwError;

error:

    if (ppszDomain)
    {
        *ppszDomain = NULL;
    }

    goto cleanup;
}

DWORD
VmwDeployGetPartnerSiteName(
    PCSTR pszServer,
    PSTR* ppszSite
    )
{
    DWORD dwError = 0;
    PSTR  pszVmDirSite = NULL;
    PSTR  pszSite = NULL;

    if (IsNullOrEmptyString(pszServer))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmDirGetPartnerSiteName(pszServer, &pszVmDirSite);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pszVmDirSite, &pszSite);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszSite = pszSite;

cleanup:

    if (pszVmDirSite)
    {
        VmDirFreeMemory(pszVmDirSite);
    }

    return dwError;

error:

    if (ppszSite)
    {
        *ppszSite = NULL;
    }

    goto cleanup;
}

DWORD
VmwDeployValidatePartnerCredentials(
    PCSTR pszServer,
    PCSTR pszPassword,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    PCSTR pszUsername = VMW_ADMIN_NAME;
    PSTR  pszLdapURI = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    VMW_DEPLOY_LOG_INFO(
            "Validating credentials to partner [%s] at domain [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pszServer),
            VMW_DEPLOY_SAFE_LOG_STRING(pszDomain));
    if (IsNullOrEmptyString(pszServer) || !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    if (VmwDeployIsIPV6Address(pszServer))
    {
        dwError = VmwDeployAllocateStringPrintf(
    			    &pszLdapURI,
                    "ldaps://[%s]:636",
                    pszServer);
        BAIL_ON_DEPLOY_ERROR(dwError);

    }
    else
    {
        dwError = VmwDeployAllocateStringPrintf(
                    &pszLdapURI,
                    "ldaps://%s:636",
                    pszServer);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    dwError = VmDirConnectionOpen(
                    pszLdapURI,
                    pszDomain,
                    pszUsername,
                    pszPassword,
                    &pConnection);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }
    if (pszLdapURI)
    {
        VmwDeployFreeMemory(pszLdapURI);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployValidateDNSForwarders(
    PCSTR pszForwarders
    )
{
    DWORD  dwError = 0;
    PCSTR  pszDelim = ",";
    PCSTR  pszReadCursor = pszForwarders;

    VMW_DEPLOY_LOG_INFO(
            "Validating DNS Forwarders [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pszForwarders));

    if (IsNullOrEmptyString(pszForwarders))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = 0;
        size_t len_delim = 0;
        char szForwarder[128];

        len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (len_name > sizeof(szForwarder)-1)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);
            }

            strncpy(&szForwarder[0], pszReadCursor, len_name);
            szForwarder[len_name] = '\0';

            if (!VmwDeployIsIPAddress(szForwarder))
            {
               VMW_DEPLOY_LOG_ERROR("Error: An invalid DNS forwarder [%s] was specified", szForwarder);

               dwError = ERROR_INVALID_PARAMETER;
               BAIL_ON_DEPLOY_ERROR(dwError);
            }

            pszReadCursor += len_name;
        }

        len_delim = strspn(pszReadCursor, pszDelim);

        pszReadCursor += len_delim;
    }

error:

    return dwError;
}

