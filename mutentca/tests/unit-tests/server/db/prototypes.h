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

int
Test_LwCADbInitCtx(
    VOID **state
    );

VOID
Test_LwCADbInitCtx_Invalid(
    VOID **state
    );

VOID
Test_LwCADbAddCA(
    VOID **state
    );

VOID
Test_LwCADbAddCA_Invalid(
    VOID **state
    );

VOID
Test_LwCADbAddCertData(
    VOID **state
    );

VOID
Test_LwCADbAddCertData_Invalid(
    VOID **state
    );

VOID
Test_LwCADbCheckCA(
    VOID **state
    );

VOID
Test_LwCADbCheckCA_Invalid(
    VOID **state
    );

VOID
Test_LwCADbCheckCertData(
    VOID **state
    );

VOID
Test_LwCADbCheckCertData_Invalid(
    VOID **state
    );

VOID
Test_LwCADbGetCA(
    VOID **state
    );

VOID
Test_LwCADbGetCA_Invalid(
    VOID **state
    );

VOID
Test_LwCADbGetCACertificates(
    VOID **state
    );

VOID
Test_LwCADbGetCACertificates_Invalid(
    VOID **state
    );

VOID
Test_LwCADbGetCertData(
    VOID **state
    );

VOID
Test_LwCADbGetCertData_Invalid(
    VOID **state
    );

VOID
Test_LwCADbGetCACRLNumber(
    VOID **state
    );

VOID
Test_LwCADbGetCACRLNumber_Invalid(
    VOID **state
    );

VOID
Test_LwCADbGetParentCAId(
    VOID **state
    );

VOID
Test_LwCADbGetParentCAId_Invalid(
    VOID **state
    );

VOID
Test_LwCADbUpdateCA(
    VOID **state
    );

VOID
Test_LwCADbUpdateCA_Invalid(
    VOID **state
    );

VOID
Test_LwCADbUpdateCAStatus(
    VOID **state
    );

VOID
Test_LwCADbUpdateCAStatus_Invalid(
    VOID **state
    );

VOID
Test_LwCADbUpdateCertData(
    VOID **state
    );

VOID
Test_LwCADbUpdateCertData_Invalid(
    VOID **state
    );

VOID
Test_LwCADbUpdateCACRLNumber(
    VOID **state
    );

VOID
Test_LwCADbUpdateCACRLNumber_Invalid(
    VOID **state
    );

int
Test_LwCADbFreeCtx(
    VOID **state
    );

VOID
Test_LwCADbCAData(
    VOID **state
    );

VOID
Test_LwCADbCAData_Invalid(
    VOID **state
    );

VOID
Test_LwCADbCertData(
    VOID **state
    );

VOID
Test_LwCADbCertData_Invalid(
    VOID **state
    );

VOID
Test_LwCAPostDbInitCtx(
    VOID    **state
    );

VOID
Test_LwCAPostDbFreeCtx(
    VOID    **state
    );

int
PreTest_LwCAPostPlugin(
    VOID    **state
    );

int
PostTest_LwCAPostPlugin(
    VOID    **state
    );

VOID
Test_LwCAPostDbAddCA(
    VOID    **state
    );

VOID
Test_LwCASerializeRootCAToJson(
    VOID    **state
    );

VOID
Test_LwCASerializeIntermediateCAToJson(
    VOID    **state
    );

VOID
Test_LwCASerializeConfigRootCAToJson(
    VOID    **state
    );

VOID
Test_LwCADeserializeJsonToRootCA(
    VOID    **state
    );

VOID
Test_LwCADeserializeJsonToIntrCA(
    VOID **state
    );

VOID
Test_LwCAPostDbCheckCA(
    VOID **state
    );

VOID
Test_LwCAPostDbGetCA(
    VOID **state
    );
