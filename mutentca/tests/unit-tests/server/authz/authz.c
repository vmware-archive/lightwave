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

#define LWCA_TEST_AUTHZ_INVALID_PATH "{\"pluginPath\":\"dummyplugin.so\"}"

VOID
Test_LwCAAuthZInitialize_NoPlugin_Valid(
    VOID        **state
    )
{
    DWORD       dwError = 0;

    dwError = LwCAAuthZInitialize(NULL);
    assert_int_equal(dwError, 0);

    LwCAAuthZDestroy();
}

VOID
Test_LwCAAuthZInitialize_PluginPath_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = { 0 };
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;

    pJsonConfig = json_loads(LWCA_TEST_AUTHZ_INVALID_PATH, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonConfig);

    dwError = LwCAAuthZInitialize(pJsonConfig);
    assert_int_equal(dwError, LWCA_ERROR_CANNOT_LOAD_LIBRARY);

    LwCAAuthZDestroy();
    LwCAJsonCleanupObject(pJsonConfig);
}
