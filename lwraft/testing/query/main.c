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
 * Module Name: vmdir_test_query
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * query testing main module entry point
 *
 */

#include "includes.h"

static
DWORD
VmDirQueryParseArgs(
    int   argc,
    char* argv[],
    PVMDIR_QUERY_ARGS* ppArgs
    );

static
VOID
VmDirFreeArgs(
    PVMDIR_QUERY_ARGS pArgs
    );

int main(int argc, char* argv[])
{
    DWORD   dwError = 0;

    const int ldapVer = LDAP_VERSION3;

    PVMDIR_QUERY_ARGS pArgs = NULL;
    PSTR              pszLdapURL = NULL;
    LDAP*             pLd = NULL;
    BerValue          ldapBindPwd = {0};
    LDAPMessage*      pResult = NULL;
    PSTR              pszDN = NULL;

    dwError = VmDirQueryParseArgs(argc, argv, &pArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                    &pszLdapURL,
                    "ldap://%s",
                    pArgs->pszHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 0
    dwError = ldap_initialize(&pLd, pszLdapURL);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
    pLd = ldap_open(pArgs->pszHostname, 389);
    if (!pLd)
    {
        dwError = VMDIR_ERROR_SERVER_DOWN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
#endif

    dwError = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapBindPwd.bv_val = pArgs->pszPassword;
    ldapBindPwd.bv_len = strlen(pArgs->pszPassword);

#if 0
    dwError = ldap_sasl_bind_s(
                        pLd,
                        pArgs->pszBindDN,
                        LDAP_SASL_SIMPLE,
                        &ldapBindPwd,
                        NULL,
                        NULL,
                        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
    dwError = ldap_bind_s(
                        pLd,
                        pArgs->pszBindDN,
                        pArgs->pszPassword,
                        LDAP_AUTH_SIMPLE);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

#if 0
    dwError = ldap_search_ext_s(
                        pLd,
                        pArgs->pszBaseDN,
                        LDAP_SCOPE_SUBTREE,
                        pArgs->pszFilter,
                        NULL,
                        TRUE,
                        NULL,                       // server ctrls
                        NULL,                       // client ctrls
                        NULL,                       // timeout
                        -1,                         // size limit,
                        &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
    dwError = ldap_search_s(
                        pLd,
                        pArgs->pszBaseDN,
                        LDAP_SCOPE_SUBTREE,
                        pArgs->pszFilter,
                        NULL,
                        TRUE,
                        &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

        for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
        {
            if (pszDN)
            {
                ldap_memfree(pszDN);
                pszDN = NULL;
            }

            pszDN = ldap_get_dn(pLd, pEntry);

            if (IsNullOrEmptyString(pszDN))
            {
                dwError = VMDIR_ERROR_INVALID_DN;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            fprintf(stdout, "DN : %s\n", pszDN);
        }
    }

cleanup:

   if (pArgs)
   {
       VmDirFreeArgs(pArgs);
   }

   VMDIR_SAFE_FREE_MEMORY(pszLdapURL);

   if (pResult)
   {
       ldap_msgfree(pResult);
   }

   if (pszDN)
   {
       ldap_memfree(pszDN);
   }

   if (pLd)
   {
       ldap_unbind_ext_s(pLd, NULL, NULL);
   }

   return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDirQueryParseArgs(
    int   argc,
    char* argv[],
    PVMDIR_QUERY_ARGS* ppArgs
    )
{
    DWORD dwError = 0;
    DWORD iArg = 0;
    PVMDIR_QUERY_ARGS pArgs = NULL;
    enum ParseMode
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_HOSTNAME,
        PARSE_MODE_BIND_DN,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_BASE_DN,
        PARSE_MODE_FILTER
    } parseMode = PARSE_MODE_OPEN;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_QUERY_ARGS), (PVOID*)&pArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iArg = 1; iArg < argc; iArg++)
    {
        PCSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_OPEN :

                if (!strcmp(pszArg, "--host"))
                {
                    parseMode = PARSE_MODE_HOSTNAME;
                }
                else if (!strcmp(pszArg, "--bind-dn"))
                {
                    parseMode = PARSE_MODE_BIND_DN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--base-dn"))
                {
                    parseMode = PARSE_MODE_BASE_DN;
                }
                else if (!strcmp(pszArg, "--filter"))
                {
                    parseMode = PARSE_MODE_FILTER;
                }
                else
                {
                    dwError = VMDIR_ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                break;

            case PARSE_MODE_HOSTNAME:

                VMDIR_SAFE_FREE_MEMORY(pArgs->pszHostname);

                dwError = VmDirAllocateStringA(pszArg, &pArgs->pszHostname);
                BAIL_ON_VMDIR_ERROR(dwError);

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_BIND_DN:

                VMDIR_SAFE_FREE_MEMORY(pArgs->pszBindDN);

                dwError = VmDirAllocateStringA(pszArg, &pArgs->pszBindDN);
                BAIL_ON_VMDIR_ERROR(dwError);

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PASSWORD:

                VMDIR_SAFE_FREE_MEMORY(pArgs->pszPassword);

                dwError = VmDirAllocateStringA(pszArg, &pArgs->pszPassword);
                BAIL_ON_VMDIR_ERROR(dwError);

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_BASE_DN:

                VMDIR_SAFE_FREE_MEMORY(pArgs->pszBaseDN);

                dwError = VmDirAllocateStringA(pszArg, &pArgs->pszBaseDN);
                BAIL_ON_VMDIR_ERROR(dwError);

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_FILTER:

                VMDIR_SAFE_FREE_MEMORY(pArgs->pszFilter);

                dwError = VmDirAllocateStringA(pszArg, &pArgs->pszFilter);
                BAIL_ON_VMDIR_ERROR(dwError);

                parseMode = PARSE_MODE_OPEN;

                break;

            default:

                break;
        }
    }

    if (IsNullOrEmptyString(pArgs->pszHostname) ||
        IsNullOrEmptyString(pArgs->pszBindDN) ||
        IsNullOrEmptyString(pArgs->pszPassword) ||
        IsNullOrEmptyString(pArgs->pszBaseDN) ||
        IsNullOrEmptyString(pArgs->pszFilter))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

   *ppArgs = pArgs; 

cleanup:

    return dwError;

error:
    
    *ppArgs = NULL;

    if (pArgs)
    {
        VmDirFreeArgs(pArgs);
    }

    goto cleanup;
}

static
VOID
VmDirFreeArgs(
    PVMDIR_QUERY_ARGS pArgs
    )
{
    if (pArgs)
    {
        VMDIR_SAFE_FREE_MEMORY(pArgs->pszHostname);
        VMDIR_SAFE_FREE_MEMORY(pArgs->pszBindDN);
        VMDIR_SAFE_FREE_MEMORY(pArgs->pszPassword);
        VMDIR_SAFE_FREE_MEMORY(pArgs->pszBaseDN);
        VMDIR_SAFE_FREE_MEMORY(pArgs->pszFilter);

        VmDirFreeMemory(pArgs);
    }
}
