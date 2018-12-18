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

#ifndef _VMCA_SECURITY_PROTOTYPES_H_
#define _VMCA_SECURITY_PROTOTYPES_H_

DWORD
VMCASecurityStoragePut(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA pEncryptedData
    );

DWORD
VMCASecurityStorageGet(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA *ppEncryptedData
    );

VOID
VMCASecurityFreeBinaryData(
    PLWCA_BINARY_DATA pBinaryData
    );

#endif /* _VMCA_SECURITY_PROTOTYPES_H_ */
