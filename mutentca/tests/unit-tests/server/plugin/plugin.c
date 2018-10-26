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

#define TESTPLUGIN_VALID_PATH           "./test-plugins/testplugin-valid/.libs/testpluginvalid.so"
#define TESTPLUGIN_NOENTRY_PATH         "./test-plugins/testplugin-noentry/.libs/testpluginnoentry.so"
#define TESTPLUGIN_INVALIDVTABLE_PATH   "./test-plugins/testplugin-invalidvtable/.libs/testplugininvalidvtable.so"

#define TESTPLUGIN_VALID_MESSAGE        "Hello from testpluginvalid!"
#define TESTPLUGIN_INVALIDVTABLE_PFN    "get_message"

static
DWORD
_LwCAPluginAllocateTestVTable(
    PLWCA_PLUGIN_VTABLE     *ppvTable
    );

static
VOID
_LwCAPluginFreeTestVTable(
    PLWCA_PLUGIN_VTABLE     pvTable
    );


VOID
LwCAPluginInitialize_Valid(
    void                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_PLUGIN_HANDLE     pTestPluginHdl = NULL;
    PLWCA_PLUGIN_VTABLE     pTestPluginVTable = NULL;

    dwError = _LwCAPluginAllocateTestVTable(&pTestPluginVTable);
    assert_int_equal(dwError, 0);
    assert_non_null(pTestPluginVTable);

    dwError = LwCAPluginInitialize(
                        TESTPLUGIN_VALID_PATH,
                        (PVOID)pTestPluginVTable,
                        &pTestPluginHdl);
    assert_int_equal(dwError, 0);
    assert_non_null(pTestPluginHdl);
    assert_non_null(pTestPluginVTable->pfnGetNumber);
    assert_non_null(pTestPluginVTable->pfnGetMessage);
    assert_int_equal(pTestPluginVTable->pfnGetNumber(), 0xCA2018);
    assert_string_equal(pTestPluginVTable->pfnGetMessage(), TESTPLUGIN_VALID_MESSAGE);

    LwCAPluginDeinitialize(pTestPluginHdl);
    _LwCAPluginFreeTestVTable(pTestPluginVTable);
}

VOID
LwCAPluginInitialize_NoEntryPoint(
    void                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_PLUGIN_HANDLE     pTestPluginHdl = NULL;
    PLWCA_PLUGIN_VTABLE     pTestPluginVTable = NULL;

    dwError = _LwCAPluginAllocateTestVTable(&pTestPluginVTable);
    assert_int_equal(dwError, 0);
    assert_non_null(pTestPluginVTable);

    dwError = LwCAPluginInitialize(
                        TESTPLUGIN_NOENTRY_PATH,
                        (PVOID)pTestPluginVTable,
                        &pTestPluginHdl);
    assert_int_equal(dwError, LWCA_ERROR_DLL_SYMBOL_NOTFOUND);
    assert_null(pTestPluginHdl);
    assert_null(pTestPluginVTable->pfnGetNumber);
    assert_null(pTestPluginVTable->pfnGetMessage);

    LwCAPluginDeinitialize(pTestPluginHdl);
    _LwCAPluginFreeTestVTable(pTestPluginVTable);
}

VOID
LwCAPluginInitialize_InvalidVTable(
    void                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_PLUGIN_HANDLE     pTestPluginHdl = NULL;
    PLWCA_PLUGIN_VTABLE     pTestPluginVTable = NULL;

    dwError = _LwCAPluginAllocateTestVTable(&pTestPluginVTable);
    assert_int_equal(dwError, 0);
    assert_non_null(pTestPluginVTable);

    dwError = LwCAPluginInitialize(
                        TESTPLUGIN_INVALIDVTABLE_PATH,
                        (PVOID)pTestPluginVTable,
                        &pTestPluginHdl);
    assert_int_equal(dwError, 0);
    assert_non_null(pTestPluginHdl);
    assert_non_null(pTestPluginVTable);
    assert_non_null(pTestPluginVTable->pfnGetNumber);
    assert_int_equal(pTestPluginVTable->pfnGetNumber(), 0xCA2018);
    assert_null(pTestPluginVTable->pfnGetMessage);

    LwCAPluginDeinitialize(pTestPluginHdl);
    _LwCAPluginFreeTestVTable(pTestPluginVTable);
}


static
DWORD
_LwCAPluginAllocateTestVTable(
    PLWCA_PLUGIN_VTABLE     *ppvTable
    )
{
    DWORD                   dwError = 0;
    PLWCA_PLUGIN_VTABLE     pvTable = NULL;

    if (!ppvTable)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(
                    (DWORD)sizeof(LWCA_PLUGIN_VTABLE),
                    (PVOID *)&pvTable);
    BAIL_ON_LWCA_ERROR(dwError);

    pvTable->pfnGetNumber = NULL;
    pvTable->pfnGetMessage = NULL;

    *ppvTable = pvTable;


cleanup:

    return dwError;

error:

    _LwCAPluginFreeTestVTable(pvTable);
    if (ppvTable)
    {
        *ppvTable = NULL;
    }

    goto cleanup;
}

static
VOID
_LwCAPluginFreeTestVTable(
    PLWCA_PLUGIN_VTABLE     pvTable
    )
{
    if (pvTable)
    {
        pvTable->pfnGetNumber = NULL;
        pvTable->pfnGetMessage = NULL;
        LWCA_SAFE_FREE_MEMORY(pvTable);
    }
}
