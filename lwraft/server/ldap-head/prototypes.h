/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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



/*
 * Module Name: Directory ldap-head
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// opstatistic.c
VOID
VmDirOPStatisticUpdate(
    ber_tag_t opTag,
    uint64_t iThisTimeInMilliSecs
    );

// vecs.c
DWORD
VmDirGetVecsMachineCert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    );

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

