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

#include "includes.h"

static
VOID
_Create_ReqCtx_FullPower_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    );

static
VOID
_Create_ReqCtx_CAAdmin_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    );

static
VOID
_Create_ReqCtx_CAOperator_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    );

static
VOID
_Create_ReqCtx_Regular_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    );


int
Test_LwCAAuthZLW_Setup(
    VOID        **state
    )
{
    DWORD       dwError = 0;

    dwError = LwCAAuthZInitialize(NULL);
    assert_int_equal(dwError, 0);

    return 0;
}

int
Test_LwCAAuthZLW_Teardown(
    VOID        **state
    )
{
    LwCAAuthZDestroy();

    return 0;
}


VOID
Test_LwCAAuthZLWCheckCACreate_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CA Creation with a user who belongs to both CAAdmins and CAOperators
    _Create_ReqCtx_FullPower_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_CREATE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CA Creation with a user who belongs to CAAdmins
    _Create_ReqCtx_CAAdmin_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_CREATE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CA Creation with a user who belongs to CAOperators
    _Create_ReqCtx_CAOperator_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_CREATE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCACreate_InValid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CA Creation fails with a regular user
    _Create_ReqCtx_Regular_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_CREATE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCARevoke_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CA Revocation with a user who belongs to both CAAdmins and CAOperators
    _Create_ReqCtx_FullPower_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_REVOKE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CA Revocation with a user who belongs to CAAdmins
    _Create_ReqCtx_CAAdmin_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_REVOKE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCARevoke_InValid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CA Revocation with a user who belongs to CAOperators
    _Create_ReqCtx_CAOperator_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_REVOKE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CA Revocation fails with a regular user
    _Create_ReqCtx_Regular_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CA_REVOKE_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCSR_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CSR with a user who's requesting from the same CA as their tenant
    _Create_ReqCtx_Regular_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CSR_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCSR_InValid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CSR fails with a user who's requesting from a CA that is not their tenant's
    _Create_ReqCtx_Regular_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example2.com",
                        pX509Request,
                        LWCA_AUTHZ_CSR_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCRL_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CRL with a user who belongs to both CAAdmins and CAOperators
    _Create_ReqCtx_FullPower_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CRL_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CRL with a user who belongs to CAAdmins
    _Create_ReqCtx_CAAdmin_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CRL_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_true(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}

VOID
Test_LwCAAuthZLWCheckCRL_InValid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    X509_REQ                *pX509Request = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthorized = FALSE;

    pX509Request = X509_REQ_new();
    assert_non_null(pX509Request);

    // Test CRL with a user who belongs to CAOperators
    _Create_ReqCtx_CAOperator_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CRL_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    // Test CRL fails with a regular user
    _Create_ReqCtx_Regular_User(&pReqCtx);
    dwError = LwCAAuthZCheckAccess(
                        pReqCtx,
                        "example.com",
                        pX509Request,
                        LWCA_AUTHZ_CRL_PERMISSION,
                        &bAuthorized);
    assert_int_equal(dwError, 0);
    assert_false(bAuthorized);
    LwCARequestContextFree(pReqCtx);

    X509_REQ_free(pX509Request);
}


static
VOID
_Create_ReqCtx_FullPower_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    )
{
    DWORD                   dwError = 0;
    PSTR                    *ppszGroups = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAAllocateMemory(3 * sizeof(PSTR), (PVOID *)&ppszGroups);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("CAAdmins", &ppszGroups[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup", &ppszGroups[1]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("CAOperators", &ppszGroups[2]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(ppszGroups, 3, &pReqCtx->pBindUPNGroups);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx->pBindUPNGroups);

    dwError = LwCAAllocateStringA("user@example.com", &pReqCtx->pszBindUPN);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("example.com", &pReqCtx->pszBindUPNTenant);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(ppszGroups[0]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[1]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[2]);
    LWCA_SAFE_FREE_MEMORY(ppszGroups);

    *ppReqCtx = pReqCtx;
}

static
VOID
_Create_ReqCtx_CAAdmin_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    )
{
    DWORD                   dwError = 0;
    PSTR                    *ppszGroups = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAAllocateMemory(2 * sizeof(PSTR), (PVOID *)&ppszGroups);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("CAAdmins", &ppszGroups[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup", &ppszGroups[1]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(ppszGroups, 2, &pReqCtx->pBindUPNGroups);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx->pBindUPNGroups);

    dwError = LwCAAllocateStringA("user@example.com", &pReqCtx->pszBindUPN);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("example.com", &pReqCtx->pszBindUPNTenant);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(ppszGroups[0]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[1]);
    LWCA_SAFE_FREE_MEMORY(ppszGroups);

    *ppReqCtx = pReqCtx;
}

static
VOID
_Create_ReqCtx_CAOperator_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    )
{
    DWORD                   dwError = 0;
    PSTR                    *ppszGroups = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAAllocateMemory(2 * sizeof(PSTR), (PVOID *)&ppszGroups);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup", &ppszGroups[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("CAOperators", &ppszGroups[1]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(ppszGroups, 2, &pReqCtx->pBindUPNGroups);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx->pBindUPNGroups);

    dwError = LwCAAllocateStringA("user@example.com", &pReqCtx->pszBindUPN);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("example.com", &pReqCtx->pszBindUPNTenant);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(ppszGroups[0]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[1]);
    LWCA_SAFE_FREE_MEMORY(ppszGroups);

    *ppReqCtx = pReqCtx;
}

static
VOID
_Create_ReqCtx_Regular_User(
    PLWCA_REQ_CONTEXT       *ppReqCtx
    )
{
    DWORD                   dwError = 0;
    PSTR                    *ppszGroups = NULL;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAAllocateMemory(3 * sizeof(PSTR), (PVOID *)&ppszGroups);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup1", &ppszGroups[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup2", &ppszGroups[1]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("DummyGroup3", &ppszGroups[2]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(ppszGroups, 3, &pReqCtx->pBindUPNGroups);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx->pBindUPNGroups);

    dwError = LwCAAllocateStringA("user@example.com", &pReqCtx->pszBindUPN);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("example.com", &pReqCtx->pszBindUPNTenant);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(ppszGroups[0]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[1]);
    LWCA_SAFE_FREE_STRINGA(ppszGroups[2]);
    LWCA_SAFE_FREE_MEMORY(ppszGroups);

    *ppReqCtx = pReqCtx;
}
