/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”);; you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* authz.c */

VOID
Test_LwCAAuthZInitialize_NoPlugin_Valid(
    VOID        **state
    );

VOID
Test_LwCAAuthZInitialize_PluginPath_Invalid(
    VOID        **state
    );

/* lightwave-authz.c */

int
Test_LwCAAuthZLW_Setup(
    VOID        **state
    );

int
Test_LwCAAuthZLW_Teardown(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCACreate_Valid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCACreate_InValid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCARevoke_Valid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCARevoke_InValid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCSR_Valid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCSR_InValid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCRL_Valid(
    VOID        **state
    );

VOID
Test_LwCAAuthZLWCheckCRL_InValid(
    VOID        **state
    );
