/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_TestAnonymousSearchWorks(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectDn,
    DWORD dwExpectedError
    )
{
    DWORD dwError = 0;
    LDAPMessage* pResult = NULL;
    PCSTR ppszAttrs[] = { NULL };

    dwError = ldap_search_ext_s(
                pState->pLdAnonymous,
                pszObjectDn,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                TRUE,
                NULL,
                NULL,
                NULL,
                0,
                &pResult);
    TestAssertEquals(dwError, dwExpectedError);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssert(ldap_count_entries(pState->pLdAnonymous, pResult) == 1);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
TestAnonymousSearch(
    PVMDIR_TEST_STATE pState
    )
{
    (VOID)_TestAnonymousSearchWorks(pState, DSE_ROOT_DN, 0);
    (VOID)_TestAnonymousSearchWorks(pState, PERSISTED_DSE_ROOT_DN, LDAP_INSUFFICIENT_ACCESS);
    (VOID)_TestAnonymousSearchWorks(pState, SCHEMA_NAMING_CONTEXT_DN, LDAP_INSUFFICIENT_ACCESS);
    (VOID)_TestAnonymousSearchWorks(pState, SUB_SCHEMA_SUB_ENTRY_DN, 0);

    return 0;
}
