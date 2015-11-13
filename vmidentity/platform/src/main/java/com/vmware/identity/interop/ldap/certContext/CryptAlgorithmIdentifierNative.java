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

/**
typedef struct _CRYPT_ALGORITHM_IDENTIFIER {
  LPSTR            pszObjId;
  CRYPT_OBJID_BLOB Parameters;
} CRYPT_ALGORITHM_IDENTIFIER, *PCRYPT_ALGORITHM_IDENTIFIER;
*/
public class CryptAlgorithmIdentifierNative extends Structure
{
    public Pointer pszObjId;
    /**
     * CRYPT_OBJID_BLOB is same typedef CRYPT_INTEGER_BLOB, see
     * {@link CryptBlobNative}:
     */
    public CryptBlobNative Parameters;

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList("pszObjId", "Parameters");
    }
}
