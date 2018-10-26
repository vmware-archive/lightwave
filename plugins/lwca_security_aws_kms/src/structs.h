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

#ifndef _LWCA_SECURITY_AWS_KMS_STRUCTS_H_
#define _LWCA_SECURITY_AWS_KMS_STRUCTS_H_

#define MAX_ENCRYPTED_DATA_LENGTH 1024*1024*3 /* 3MB. 1MB per encryption material */

typedef struct _AWS_KMS_API_CONTEXT_ AWS_KMS_API_CONTEXT, *PAWS_KMS_API_CONTEXT;

typedef struct _LWCA_SECURITY_HANDLE_
{
    PAWS_KMS_API_CONTEXT pContext;
    PLWCA_SECURITY_CAP_OVERRIDE pCapOverride;
}LWCA_SECURITY_HANDLE, *PLWCA_SECURITY_HANDLE;


typedef enum
{
    ENCRYPTED_MATERIAL_HEADER = 0,
    ENCRYPTED_MATERIAL_DATA   = 1,
    ENCRYPTED_MATERIAL_KEY    = 2,
    ENCRYPTED_MATERIAL_IV     = 3,
    ENCRYPTED_MATERIAL_COUNT  = 3
}ENCRYPTED_MATERIAL_TYPE;

/*
 * encryption materials concatenated as byte arrays
 * materials are
 * 1. encrypted data
 * 2. encrypted encryption key (kms encrypted encryption key)
 * 3. initialization vector (iv. needed for decryption. safe to store as is)
*/
typedef struct _LWCA_KMS_RAW_ENCRYPTED_MATERIALS_
{
    int arLengths[ENCRYPTED_MATERIAL_COUNT + 1];
    PBYTE pData;
}LWCA_KMS_RAW_ENCRYPTED_MATERIALS, *PLWCA_KMS_RAW_ENCRYPTED_MATERIALS;

#endif /* _LWCA_SECURITY_AWS_KMS_STRUCTS_H_ */
