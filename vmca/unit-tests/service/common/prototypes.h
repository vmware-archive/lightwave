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

#ifndef _VMCA_SRV_COMMON_PROTOTYPES_H_
#define _VMCA_SRV_COMMON_PROTOTYPES_H_

/* VMCA Policy Validation Unit Tests */

VOID
VMCAPolicyInit_ValidInput(
    VOID            **state
    );

/* VMCA Config File Parser Unit Tests */

VOID
VMCAJsonLoadObjectFromFile_ValidInput(
    VOID            **state
    );

VOID
VMCAJsonLoadObjectFromFile_InvalidInput(
    VOID            **state
    );

VOID
VMCAJsonGetObjectFromKey_Valid(
    VOID            **state
    );

VOID
VMCAJsonGetObjectFromKey_Invalid(
    VOID            **state
    );

VOID
VMCAJsonGetStringFromKey_Valid(
    VOID            **state
    );

VOID
VMCAJsonGetStringFromKey_Invalid(
    VOID            **state
    );

#endif /* _VMCA_SRV_COMMON_PROTOTYPES_H_ */
