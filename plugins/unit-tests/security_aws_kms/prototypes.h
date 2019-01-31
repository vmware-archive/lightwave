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

#ifndef _LWCA_SECURITY_AWS_KMS_UNITTEST_PROTOTYPES_H_
#define _LWCA_SECURITY_AWS_KMS_UNITTEST_PROTOTYPES_H_

int
Security_Aws_Kms_Tests_Create_Key_Pair_Setup(
    void **state
    );

VOID
Security_Aws_Kms_Tests_Create_Key_Pair (
    void **state
    );

VOID
Security_Aws_Kms_Tests_Validate_Interface(
    VOID **state
    );

VOID
Security_Aws_Kms_Tests_Validate_Interface_Cleared(
    VOID **state
    );

/* global */
int
Security_Aws_Kms_Tests_Load (
    VOID **state
    );

int
Security_Aws_Kms_Tests_Unload_Interface(
    VOID **state
    );

int
Security_Aws_Kms_Tests_Unload (
    VOID **state
    );

int
Security_Aws_Kms_Tests_Load_Interface(
    void **state
    );

/* get version */
int
Security_Aws_Kms_Tests_Get_Version (
    void **state
    );

VOID
Security_Aws_Kms_Tests_Check_Version (
    void **state
    );

/* get name */
int
Security_Aws_Kms_Tests_Get_Name (
    void **state
    );

VOID
Security_Aws_Kms_Tests_Check_Name (
    void **state
    );

/* config parse */
VOID
Security_Aws_Kms_Tests_Config_Parse_Good(
    void **state
    );

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_Root(
    void **state
    );

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_CMKId(
    void **state
    );

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_KeySpec(
    void **state
    );

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_Empty_Region(
    void **state
    );

#endif /* _LWCA_SECURITY_AWS_KMS_UNITTEST_PROTOTYPES_H_ */
