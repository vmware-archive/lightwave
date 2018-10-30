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

#ifndef _LWCA_SRV_COMMON_PROTOTYPES_H_
#define _LWCA_SRV_COMMON_PROTOTYPES_H_

/* MutentCA Config File Parser Unit Tests */

VOID
LwCAJsonLoadObjectFromFile_ValidInput(
    VOID            **state
    );

VOID
LwCAJsonLoadObjectFromFile_InvalidInput(
    VOID            **state
    );

VOID
LwCAJsonGetObjectFromKey_Valid(
    VOID            **state
    );

VOID
LwCAJsonGetObjectFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetStringFromKey_Valid(
    VOID            **state
    );

VOID
LwCAJsonGetStringFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetStringArrayFromKey_Valid(
    VOID            **state
    );

VOID
LwCAJsonGetStringArrayFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetTimeFromKey_Valid(
    VOID            **state
    );

VOID
LwCAJsonGetTimeFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetIntegerFromKey_PositiveValid(
    VOID            **state
    );

VOID
LwCAJsonGetIntegerFromKey_NegativeValid(
    VOID            **state
    );

VOID
LwCAJsonGetIntegerFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetUnsignedIntegerFromKey_PositiveValid(
    VOID            **state
    );

VOID
LwCAJsonGetUnsignedIntegerFromKey_NegativeValid(
    VOID            **state
    );

VOID
LwCAJsonGetUnsignedIntegerFromKey_Invalid(
    VOID            **state
    );

VOID
LwCAJsonGetBooleanFromKey_Valid(
    VOID            **state
    );

VOID
LwCAJsonGetBooleanFromKey_Invalid(
    VOID            **state
    );

// structs.c

VOID
Test_LwCACreateCertificate_Valid(
    VOID **state
    );

VOID
Test_LwCACreateCertificate_Invalid(
    VOID **state
    );

VOID
Test_LwCACreateCertArray_Valid(
    VOID **state
    );

VOID
Test_LwCACreateCertArray_Invalid(
    VOID **state
    );

VOID
Test_LwCACreateKey_Valid(
    VOID **state
    );

VOID
Test_LwCACreateKey_Invalid(
    VOID **state
    );

VOID
Test_LwCACopyCertArray_Valid(
    VOID **state
    );

VOID
Test_LwCACopyCertArray_Invalid(
    VOID **state
    );

VOID
Test_LwCACopyKey_Valid(
    VOID **state
    );

VOID
Test_LwCACopyKey_Invalid(
    VOID **state
    );

VOID
LwCAAuthTokenGetHOTK_Valid(
    VOID **state
    );

VOID
LwCAAuthTokenGetHOTK_InvalidInput(
    VOID **state
    );

#endif /* _LWCA_SRV_COMMON_PROTOTYPES_H_ */
