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

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public class CryptBitBlobNative extends Structure
{
    /**
    typedef struct _CRYPT_BIT_BLOB {
      DWORD cbData;
      BYTE  *pbData;
      DWORD cUnusedBits;
    } CRYPT_BIT_BLOB, *PCRYPT_BIT_BLOB;
    */

    public int cbData;
    public Pointer pbData;
    public int cUnusedBits;

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList("cbData", "pbData", "cUnusedBits");
    }
}
