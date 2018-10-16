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

#include "includes.h"
#include "aws_kms_includes.h"

LwKMSCryptoHelper::LwKMSCryptoHelper(const LwKMSCryptoConfig& config)
:m_config(config)
{
}

int
LwKMSCryptoHelper::DoEncrypt(const CryptoBuffer& data, const CryptoBuffer& key, LwKMSEncryptedData& result)
{
    auto cipher = CreateAES_CBCImplementation(key);
    auto part1 = cipher->EncryptBuffer(data);
    auto part2 = cipher->FinalizeEncryption();
    result.SetEncryptedData(CryptoBuffer({&part1, &part2}));
    result.SetIV(cipher->GetIV());
    return result.HasData() ? 0 : LWCA_SECURITY_AWS_KMS_ENCRYPT_ERROR;
}

int
LwKMSCryptoHelper::DoDecrypt(LwKMSEncryptedData& encrypted, const CryptoBuffer& key, CryptoBuffer& data)
{
    auto cipher = CreateAES_CBCImplementation(key, encrypted.GetIV());
    auto part1 = cipher->DecryptBuffer(encrypted.GetEncryptedData());
    auto part2 = cipher->FinalizeDecryption();
    data = CryptoBuffer({&part1, &part2});
    return data.GetLength() ? 0 : LWCA_SECURITY_AWS_KMS_DECRYPT_ERROR;
}

int
LwKMSCryptoHelper::Encrypt(CryptoBuffer& data, LwKMSEncryptedData& result)
{
    int nError = 0;
    auto req = Aws::KMS::Model::GenerateDataKeyRequest()
                   .WithKeyId(m_config.GetCMKId())
                   .WithKeySpec(m_config.GetDataKeySpec());
    auto out = Aws::KMS::KMSClient(m_config.GetClientConfig()).GenerateDataKey(req);
    if (out.IsSuccess())
    {
        const CryptoBuffer& key = out.GetResult().GetPlaintext();
        nError = DoEncrypt(data, key, result);
        if (!nError)
        {
            result.SetEncryptedKey(out.GetResult().GetCiphertextBlob());
        }
    }
    else
    {
        nError = LWCA_SECURITY_AWS_KMS_ENCRYPT_ERROR;
    }
    return nError;
}

int
LwKMSCryptoHelper::Decrypt(LwKMSEncryptedData& result, CryptoBuffer& data)
{
    int nError = 0;
    auto req = Aws::KMS::Model::DecryptRequest();
    req.SetCiphertextBlob(result.GetEncryptedKey());
    auto out = Aws::KMS::KMSClient(m_config.GetClientConfig()).Decrypt(req);
    if (out.IsSuccess())
    {
        const CryptoBuffer& key = out.GetResult().GetPlaintext();
        nError = DoDecrypt(result, key, data);
    }
    else
    {
        nError = LWCA_SECURITY_AWS_KMS_DECRYPT_ERROR;
    }
    return nError;
}
