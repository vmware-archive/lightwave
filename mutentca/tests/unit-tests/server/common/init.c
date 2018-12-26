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

#define LWCA_CONFIG_VALID_JSON "{\"host\":\"localhost\", \
                          \"config\":{\"name\":\"LightwaveCA\"}}"

#define LWCA_CONFIG_INVALID_JSON "{}"

int
Test_LwCASrvInitTests_Valid_Setup(
    VOID **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = { 0 };
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;

    pJsonConfig = json_loads(LWCA_CONFIG_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonConfig);

    dwError =  LwCASrvInitCtx(pJsonConfig);
    assert_int_equal(dwError, 0);

    LwCAJsonCleanupObject(pJsonConfig);

    return 0;
}

int
Test_LwCASrvInitTests_Valid_Teardown(
    VOID **state
    )
{
    LwCASrvFreeCtx();

    return 0;
}

int
Test_LwCASrvInitTests_Invalid_Setup(
    VOID **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = { 0 };
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;

    pJsonConfig = json_loads(LWCA_CONFIG_INVALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonConfig);

    dwError =  LwCASrvInitCtx(pJsonConfig);
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);

    LwCAJsonCleanupObject(pJsonConfig);

    return 0;
}

int
Test_LwCASrvInitTests_Invalid_Teardown(
    VOID **state
    )
{
    LwCASrvFreeCtx();

    return 0;
}

VOID
Test_LwCAGetRootCAId_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszRootCAId = NULL;

    dwError = LwCAGetRootCAId(&pszRootCAId);
    assert_int_equal(dwError, 0);

    assert_string_equal(pszRootCAId, "LightwaveCA");

    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
}

VOID
Test_LwCAGetRootCAId_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszRootCAId = NULL;

    dwError = LwCAGetRootCAId(&pszRootCAId);
    assert_int_equal(dwError, LWCA_INIT_CA_FAILED);

    assert_null(pszRootCAId);

    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
}

VOID
Test_LwCAGetCAEndpoint_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszRootCAId = NULL;
    PSTR pszUri1 = NULL;
    PSTR pszUri2 = NULL;

    dwError = LwCAGetRootCAId(&pszRootCAId);
    assert_int_equal(dwError, 0);

    dwError = LwCAGetCAEndpoint(pszRootCAId, &pszUri1);
    assert_int_equal(dwError, 0);

#ifdef REST_ENABLED
    assert_string_equal(pszUri1, "https://localhost:7878/v1/mutentca/root");
#endif

    dwError = LwCAGetCAEndpoint("Test_CA_ID_1", &pszUri2);
    assert_int_equal(dwError, 0);

#ifdef REST_ENABLED
    assert_string_equal(pszUri2, "https://localhost:7878/v1/mutentca/intermediate/Test_CA_ID_1");
#endif

    LWCA_SAFE_FREE_STRINGA(pszRootCAId);
    LWCA_SAFE_FREE_STRINGA(pszUri1);
    LWCA_SAFE_FREE_STRINGA(pszUri2);
}

VOID
Test_LwCAGetCAEndpoint_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszUri = NULL;

    dwError = LwCAGetCAEndpoint("Test_CA_ID_1", &pszUri);
    assert_int_equal(dwError, LWCA_INIT_CA_FAILED);

    assert_null(pszUri);

    dwError = LwCAGetCAEndpoint(NULL, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    LWCA_SAFE_FREE_STRINGA(pszUri);
}
