/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap.certContext;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;

/**
typedef struct _CERT_PUBLIC_KEY_INFO {
  CRYPT_ALGORITHM_IDENTIFIER Algorithm;
  CRYPT_BIT_BLOB             PublicKey;
} CERT_PUBLIC_KEY_INFO, *PCERT_PUBLIC_KEY_INFO;
*/
public class CryptPublicKeyInfoNative extends Structure
{
    public CryptAlgorithmIdentifierNative Algorithm;
    public CryptBitBlobNative PublicKey;

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList("Algorithm", "PublicKey");
    }
}
