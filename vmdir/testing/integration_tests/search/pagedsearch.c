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

#define USER_COUNT 20

int
ldap_parse_page_control(
    LDAP *pLd,
    LDAPControl **ctrls,
    ber_int_t *countp,
    struct berval **cookiep
    );

DWORD
SendPagedSearch(
    LDAP *pLd,
    DWORD dwPageSize,
    PCSTR pszBase,
    PCSTR pszFilter,
    struct berval **ppbvCookie, /* IN/OUT/OPTIONAL*/
    PVMDIR_STRING_LIST pStringList,
    BOOL *pbCompleted /* OUT */
    )
{
    DWORD dwError = 0;
    int bMorePages, l_errcode=0;
    struct berval *pbvCookie = *ppbvCookie;
    char pagingCriticality = 'T';
    ber_int_t totalCount;
    LDAPControl *pageControl=NULL, *M_controls[2] = { NULL, NULL }, **returnedControls = NULL;
    LDAPMessage *pResult, *pEntry;

    dwError = ldap_create_page_control(pLd, dwPageSize, pbvCookie, pagingCriticality, &pageControl);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Insert the control into a list to be passed to the search.     */
    M_controls[0] = pageControl;

    /* Search for entries in the directory using the parmeters.       */
    dwError = ldap_search_ext_s(pLd, pszBase, 2, pszFilter, NULL, 0, M_controls, NULL, NULL, 0, &pResult);
    if (dwError != LDAP_SUCCESS && dwError != LDAP_PARTIAL_RESULTS)
    {
       BAIL_ON_VMDIR_ERROR(dwError);
    }

    /* Parse the results to retrieve the contols being returned.      */
    dwError = ldap_parse_result(pLd, pResult, &l_errcode, NULL, NULL, NULL, &returnedControls, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pbvCookie != NULL)
    {
        ber_bvfree(pbvCookie);
        pbvCookie = NULL;
    }

    /* Parse the page control returned to get the pbvCookie and          */
    /* determine whether there are more pages.                        */
    dwError = ldap_parse_page_control(pLd, returnedControls, &totalCount, &pbvCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Determine if the pbvCookie is not empty, indicating there are more pages for these search parameters. */
    if (pbvCookie && pbvCookie->bv_val != NULL && (strlen(pbvCookie->bv_val) > 0))
    {
        bMorePages = TRUE;
    }
    else
    {
        bMorePages = FALSE;
    }

    /* Cleanup the controls used. */
    if (returnedControls != NULL)
    {
        ldap_controls_free(returnedControls);
        returnedControls = NULL;
    }

    M_controls[0] = NULL;
    ldap_control_free(pageControl);
    pageControl = NULL;

    if (ldap_count_entries(pLd, pResult) == 0)
    {
        bMorePages = FALSE;
    }
    else
    {
        for (pEntry = ldap_first_entry(pLd, pResult);
             pEntry != NULL;
             pEntry = ldap_next_entry(pLd, pEntry))
        {
            dwError = VmDirStringListAddStrClone(
                        ldap_get_dn(pLd, pEntry),
                        pStringList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    ldap_msgfree(pResult);

    if (!bMorePages)
    {
        ber_bvfree(pbvCookie);
        pbvCookie = NULL;
    }

   *ppbvCookie = pbvCookie;
   *pbCompleted = !bMorePages;

cleanup:
    return dwError;
error:
    goto cleanup;
}

/*
 * This creates an ACL that models the normal, system default ACL. We need to
 * construct this because we want an ACL that doesn't give the limited test user
 * access to the entries we'll create under the testcontainer, but the
 * testcontainer itself is ACL'ed to give the limited user access and entries
 * created in this container will inherit this ACL (unless we explicitly
 * specify one).
 */
DWORD CreateSystemAcl(
    PVMDIR_TEST_STATE pState,
    PSTR *ppszAcl
    )
{
    DWORD dwError = 0;
    PSTR pszDomainSid = NULL;
    PSTR pszAcl = NULL;
    PSTR pszInternalUserSid = NULL;

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszAcl,
                "O:%s-500G:BAD:(A;;RPWP;;;S-1-7-32-666)(A;;GXNRNWGXCCDCRPWP;;;BA)(A;;GXNRNWGXCCDCRPWP;;;%s-500)",
                pszDomainSid,
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAcl = pszAcl;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    VMDIR_SAFE_FREE_STRINGA(pszInternalUserSid);
    return dwError;
error:
    VMDIR_SAFE_FREE_STRINGA(pszAcl);
    goto cleanup;
}

DWORD
PagedSearchTestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    CHAR szName[14] = {0};
    PSTR pszSystemAcl = NULL;
    PSTR pszAclToApply = NULL;

    dwError = CreateSystemAcl(pState, &pszSystemAcl);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < USER_COUNT; ++dwIndex)
    {
        /*
         * We create USER_COUNT users. The first half  will be half users that
         * we have access to and half users we don't. The second half will be
         * entirely users we don't have access to.
         */
        if (dwIndex % 2 == 0 && dwIndex < USER_COUNT / 2)
        {
            pszAclToApply = NULL;
        }
        else
        {
            pszAclToApply = pszSystemAcl;
        }

        sprintf(szName, "testpsuser%d", dwIndex);
        dwError = VmDirTestCreateUser(pState, VmDirTestGetTestContainerCn(pState), szName, pszAclToApply);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSystemAcl);
    return dwError;
error:
    goto cleanup;
}

DWORD
PagedSearchTestCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    CHAR szName[14] = {0};

    for (dwIndex = 0; dwIndex < USER_COUNT; ++dwIndex)
    {
        sprintf(szName, "testpsuser%d", dwIndex);
        dwError = VmDirTestDeleteUser(pState, VmDirTestGetTestContainerCn(pState), szName);
        TestAssertEquals(dwError, 0);
    }

    return 0;
}

DWORD
TestThatWeFindAllEntries(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    struct berval *pbvCookie = NULL;
    DWORD dwTotalCount = 0;
    DWORD dwPageSize = 2;
    BOOL bCompleted = FALSE;
    PVMDIR_STRING_LIST pStringList = NULL;
    PSTR pszUsersDN = NULL;

    dwError = VmDirStringListInitialize(&pStringList, dwPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetTestContainerDn(pState, &pszUsersDN);
    BAIL_ON_VMDIR_ERROR(dwError)

    while (!bCompleted)
    {
        VmDirStringListFreeContent(pStringList);
        dwError = SendPagedSearch(
                    pState->pLd,
                    dwPageSize,
                    pszUsersDN,
                    "(&(cn=testpsuser*)(objectClass=user))",
                    &pbvCookie,
                    pStringList,
                    &bCompleted);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssert(pStringList->dwCount <= dwPageSize);

        dwTotalCount += pStringList->dwCount;
    }

    TestAssertEquals(dwTotalCount, USER_COUNT);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUsersDN);
    VmDirStringListFree(pStringList);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestLongRunningSearchSucceeds(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    struct berval *pbvCookie = NULL;
    DWORD dwTotalCount = 0;
    BOOL bCompleted = FALSE;
    DWORD dwPageSize = 2; // TODO
    PSTR pszUsersDN = NULL;
    PVMDIR_STRING_LIST pStringList = NULL;
    DWORD dwLoopCount = 0;

    dwError = VmDirStringListInitialize(&pStringList, dwPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetTestContainerDn(pState, &pszUsersDN);
    BAIL_ON_VMDIR_ERROR(dwError)

    while (!bCompleted)
    {
        VmDirStringListFreeContent(pStringList);

        dwError = SendPagedSearch(
                    pState->pLd,
                    dwPageSize,
                    pszUsersDN,
                    // TESTPSUSER is upper-case to force normalization.
                    "(&(cn=TESTPSUSER*)(objectClass=user))",
                    &pbvCookie,
                    pStringList,
                    &bCompleted);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssert(pStringList->dwCount <= dwPageSize);

        dwTotalCount += pStringList->dwCount;

        VmDirSleep(30 * 1000);

        dwLoopCount++;
    }

    TestAssertEquals(dwLoopCount, 11);
    TestAssertEquals(dwTotalCount, USER_COUNT);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUsersDN);
    VmDirStringListFree(pStringList);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestThatWeDontCrashWithStaleCookie(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    struct berval *pbvCookie = NULL;
    BOOL bCompleted = FALSE;
    DWORD dwPageSize = 2;
    PSTR pszUsersDN = NULL;
    PVMDIR_STRING_LIST pStringList = NULL;

    dwError = VmDirStringListInitialize(&pStringList, dwPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetTestContainerDn(pState, &pszUsersDN);
    BAIL_ON_VMDIR_ERROR(dwError)

    while (!bCompleted)
    {
        dwError = SendPagedSearch(
                    pState->pLd,
                    dwPageSize,
                    pszUsersDN,
                    "(&(cn=testpsuser*)(objectClass=user))",
                    &pbvCookie,
                    pStringList,
                    &bCompleted);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssert(pStringList->dwCount <= dwPageSize);

        VmDirStringListFreeContent(pStringList);
    }

    //
    // Send one more search with a "stale" cookie. This search will succeed,
    // as our behavior in the face of an invalid cookie is to just go back to
    // a normal search. We're mostly interested in making sure that vmdird
    // doesn't crash.
    //
    VmDirStringListFreeContent(pStringList);
    dwError = SendPagedSearch(
                    pState->pLd,
                    USER_COUNT,
                    pszUsersDN,
                    "(&(cn=testpsuser*)(objectClass=user))",
                    &pbvCookie,
                    pStringList,
                    &bCompleted);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(pStringList->dwCount, USER_COUNT);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUsersDN);
    VmDirStringListFree(pStringList);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestPagedSearch(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = TestThatWeFindAllEntries(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestLongRunningSearchSucceeds(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestThatWeDontCrashWithStaleCookie(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    printf("paged search test failed with error 0n%d\n", dwError);
    goto cleanup;
}
