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
 * Module Name:
 *
 *        test-ldap-bind.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        LDAP tests
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ldap.h>

#define BAIL_ON_ERROR(dwError) \
    do { \
       if (dwError) {  \
           goto error; \
       } \
    } while(0)

int main(int argc, char *argv[])
{
    DWORD dwError = 0;
    LDAP* pLd = NULL;
    CHAR  szURI[512];
    const int ldapVer = LDAP_VERSION3;
    PCSTR pszFilter = "(&(objectclass=user)(sAMAccountName=administrator))";
    PCSTR pszBaseSearchDN = "";
    PSTR  ppszAttrs[] = { "sAMAccountName", NULL };
    LDAPMessage* pSearchRes = NULL;
    BOOLEAN bIDMInitialized = FALSE;
    PWSTR pwstrUser = NULL;
    PWSTR pwstrDomain = NULL;
    PWSTR pwstrPassword = NULL;

    if (argc < 2)
    {
        printf("Usage: test-ldapbind <ldap-server-hostname> [user] [domain] [password]\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    snprintf(szURI, 512, "ldap://%s:3268", argv[1]);

    dwError = IDMInitialize();
    BAIL_ON_ERROR(dwError);

    bIDMInitialized = TRUE;

    dwError = ldap_initialize(&pLd, szURI);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    BAIL_ON_ERROR(dwError);

    if(argc > 2) {
        dwError = LwRtlWC16StringAllocateFromCString(&pwstrUser, argv[2]);
        BAIL_ON_ERROR(dwError);
    }

    if(argc > 3) {
        dwError = LwRtlWC16StringAllocateFromCString(&pwstrDomain, argv[3]);
        BAIL_ON_ERROR(dwError);
    }

    if(argc > 4) {
        dwError = LwRtlWC16StringAllocateFromCString(&pwstrPassword, argv[4]);
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMLdapSaslBind(pLd, pwstrUser, pwstrDomain, pwstrPassword);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszBaseSearchDN,
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    ppszAttrs,
                    FALSE,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &pSearchRes);
    BAIL_ON_ERROR(dwError);

    printf("Found %d users\n", ldap_count_entries(pLd, pSearchRes));

cleanup:

    printf("IDMLdapSaslBind returns %d\n", dwError);
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    if (bIDMInitialized)
    {
        IDMShutdown();
    }

    return dwError;

error:

    goto cleanup;
}
