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

#define LWCA_SERVER "lightwave.ip"
#define LWCA_DOMAIN "lightwave.local"

VOID
LwCAAuthTokenGetHOTK_Valid(
    VOID **state
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;

    will_return(__wrap_LwCAGetVecsMutentCACert, 0);
    will_return(__wrap_OidcClientBuild, 0);
    will_return(__wrap_OidcClientAcquireTokensBySolutionUserCredentials, 0);

    dwError = LwCAGetAccessToken(
        LWCA_SERVER,
        LWCA_DOMAIN,
        &pszToken
        );

    assert_int_equal(dwError, 0);
    assert_non_null(pszToken);

    LWCA_SAFE_FREE_STRINGA(pszToken);

}

VOID
LwCAAuthTokenGetHOTK_InvalidInput(
    void **state
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;

    dwError = LwCAGetAccessToken(
        "",
        NULL,
        &pszToken
        );
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pszToken);
}
