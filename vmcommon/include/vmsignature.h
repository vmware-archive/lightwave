/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef VMCOMMON_SIGNATURE_H
#define VMCOMMON_SIGNATURE_H

typedef enum
{
    VMSIGN_DIGEST_METHOD_MD5,
    VMSIGN_DIGEST_METHOD_SHA256
} VMSIGN_DIGEST_METHOD;

DWORD
VmSignatureComputeRSASignature(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const unsigned char     *pData,
    size_t                  dataSize,
    PCSTR                   pszRSAPrivateKeyPEM,
    unsigned char**         ppRSASignature,
    size_t*                 pRSASignatureSize
    );

DWORD
VmSignatureVerifyRSASignature(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const unsigned char     *pData,
    size_t                  szDataSize,
    PCSTR                   pcszRSAPublicKeyPEM,
    unsigned char           *pRSASignature,
    size_t                  RSASignatureSize,
    PBOOLEAN                pbVerified
    );

DWORD
VmSignatureEncodeHex(
    const unsigned char     data[],
    const size_t            length,
    PSTR                    *ppHex
    );

DWORD
VmSignatureDecodeHex(
    PCSTR               pcszHexStr,
    unsigned char       **ppData,
    size_t              *pLength
    );

DWORD
VmSignatureComputeMessageDigest(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const unsigned char     *pData,
    size_t                  dataSize,
    unsigned char           **ppMD,
    size_t                  *pMDSize
    );

#endif //VMCOMMON_SIGNATURE_H
