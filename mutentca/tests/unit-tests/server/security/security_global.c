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

LWCA_SECURITY_TEST_STATE _this_state = {0};

int
Test_LwCASecurity_GlobalLoad(
    VOID **state
    )
{
    int error = 0;

    _this_state.module = dlopen(
         "test-mutentcasecurity-plugin/.libs/mutentcasecuritytestplugin.so",
         RTLD_NOW);

    if (!_this_state.module)
    {
        error = 1;
        fail_msg("dlopen failed mutentca security tests: %s \n", dlerror());
        goto error;
    }

    *state = &_this_state;

error:
    return error;
}

int
Test_LwCASecurity_Global_Unload_Interface(
    VOID **state
    )
{
    int error = 0;
    PLWCA_SECURITY_TEST_STATE pState = *state;

    if (!pState || !pState->module)
    {
        error = EINVAL;
        goto error;
    }

    PFN_LWCA_SECURITY_UNLOAD_INTERFACE pFnUnloadInterface = NULL;
    pFnUnloadInterface = dlsym(pState->module, LWCA_FN_NAME_SECURITY_UNLOAD_INTERFACE);
    if (!pFnUnloadInterface)
    {
        fail_msg("dlsym failed LwCASecurityUnloadInterface: %s \n", dlerror());
        goto error;
    }

    if (pState->pInterface)
    {
        pFnUnloadInterface(pState->pInterface);
    }

error:
    return error;
}

int
Test_LwCASecurity_GlobalUnload(
    VOID **state
    )
{
    int error = 0;
    PLWCA_SECURITY_TEST_STATE pState = *state;

    if (Test_LwCASecurity_Global_Unload_Interface(state))
    {
        fail_msg("dlsym failed LwCASecurityUnloadInterface: %s \n", dlerror());
        goto error;
    }

error:
    if (pState && pState->module)
    {
        dlclose(pState->module);
    }

    return error;
}
