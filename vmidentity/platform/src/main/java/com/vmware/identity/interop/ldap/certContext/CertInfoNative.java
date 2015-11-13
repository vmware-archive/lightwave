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
import com.sun.jna.platform.win32.WinBase.FILETIME;

/**
typedef struct _CERT_INFO {
  DWORD                      dwVersion;
  CRYPT_INTEGER_BLOB         SerialNumber;
  CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm;
  CERT_NAME_BLOB             Issuer;
  FILETIME                   NotBefore;
  FILETIME                   NotAfter;
  CERT_NAME_BLOB             Subject;
  CERT_PUBLIC_KEY_INFO       SubjectPublicKeyInfo;
  CRYPT_BIT_BLOB             IssuerUniqueId;
  CRYPT_BIT_BLOB             SubjectUniqueId;
  DWORD                      cExtension;
  PCERT_EXTENSION            rgExtension;
} CERT_INFO, *PCERT_INFO;
*/
public class CertInfoNative extends Structure
{
    public int dwVersion;
    public CryptBlobNative SerialNumber;
    public CryptAlgorithmIdentifierNative SignatureAlgorithm;

    /**
     * CERT_NAME_BLOB is same typedef as CRYPT_INTEGER_BLOB {@link CryptBlobNative}
     */
    public CryptBlobNative Issuer;

    public FILETIME NotBefore;
    public FILETIME NotAfter;

    /**
     * CERT_NAME_BLOB is same typedef as CRYPT_INTEGER_BLOB {@link CryptBlobNative}
     */
    public CryptBlobNative Subject;

    public CryptPublicKeyInfoNative SubjectPublicKeyInfo;
    public CryptBitBlobNative IssuerUniqueId;
    public CryptBitBlobNative SubjectUniqueId;
    public int cExtension;
    public Pointer rgExtension;

    public CertInfoNative(Pointer p)
    {
        super();
        useMemory(p);
        read();
    }

    public String getSerialNumberStr()
    {
        byte[] serialNumberBuf = new byte[SerialNumber.cbData];
        SerialNumber.pbData.read(0, serialNumberBuf, 0, SerialNumber.cbData);
        StringBuilder sb = new StringBuilder();
        for (int i = serialNumberBuf.length -1; i >=0; i--) {
           sb.append(String.format("%02x",serialNumberBuf[i]));
        }
        return sb.toString();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList("dwVersion", "SerialNumber", "SignatureAlgorithm",
                "Issuer", "NotBefore", "NotAfter", "Subject",
                "SubjectPublicKeyInfo", "IssuerUniqueId", "SubjectUniqueId",
                "cExtension", "rgExtension");
    }

}
