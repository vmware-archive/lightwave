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

#pragma once

class LwKMSEncryptedData
{
private:
    CryptoBuffer m_encryptedData;
    CryptoBuffer m_encryptedEncryptionKey;
    CryptoBuffer m_iv;
public:
    LwKMSEncryptedData()
    {
    }
    LwKMSEncryptedData(
        CryptoBuffer& encryptedData,
        CryptoBuffer& encryptionKey,
        CryptoBuffer& iv)
    :m_encryptedData(encryptedData),
     m_encryptedEncryptionKey(encryptionKey),
     m_iv(iv)
    {
    }
    void SetEncryptedData(const CryptoBuffer& data)
    {
        m_encryptedData = data;
    }
    void SetEncryptedKey(const CryptoBuffer& key)
    {
        m_encryptedEncryptionKey = key;
    }
    void SetIV(const CryptoBuffer& iv)
    {
        m_iv = iv;
    }

    CryptoBuffer& GetEncryptedData()
    {
        return m_encryptedData;
    }
    CryptoBuffer& GetEncryptedKey()
    {
        return m_encryptedEncryptionKey;
    }
    CryptoBuffer& GetIV()
    {
        return m_iv;
    }
    int HasData()
    {
        return m_encryptedData.GetLength() > 0 && m_iv.GetLength() > 0;
    }
};
