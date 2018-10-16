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

#ifndef _LWCA_SECURITY_AWS_KMS_PROTOTYPES_H_
#define _LWCA_SECURITY_AWS_KMS_PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* aws_kms_api.cpp */
DWORD
LwAwsKmsInitialize(
    PAWS_KMS_API_CONTEXT *ppContext
    );

DWORD
LwAwsKmsEncrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_BINARY_DATA pData,
    PLWCA_BINARY_DATA *ppEncryptedData
    );

DWORD
LwAwsKmsDecrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_BINARY_DATA pEncryptedData,
    PLWCA_BINARY_DATA *ppDecryptedData
    );

VOID
LwAwsKmsShutdown(
    PAWS_KMS_API_CONTEXT pContext
    );

VOID
LwAwsKmsFreeBinaryData(
    PLWCA_BINARY_DATA pBinaryData
    );

/* security.c */
VOID
LwSecurityAwsKmsFreeMemory(
    PVOID pMemory
    );

VOID
LwSecurityAwsKmsSecureFreeString(
    PSTR pszData
    );

/* security_pkcs.c */
DWORD
LwCreateKeyPair(
    PCSTR pszPassPhrase, /* optional */
    size_t nKeyLength,
    PSTR *ppszPrivateKey,
    PSTR *ppszPublicKey
    );

DWORD
LwX509Verify(
    PCSTR pszCertificate,
    PLWCA_BINARY_DATA pKeyData,
    PBOOLEAN pbValid
    );

DWORD
LwX509Sign(
    X509 *pX509,
    PLWCA_BINARY_DATA pKeyData,
    LWCA_SECURITY_MESSAGE_DIGEST md
    );

#ifdef __cplusplus
}
#endif

#endif /* _LWCA_SECURITY_AWS_KMS_PROTOTYPES_H_ */
