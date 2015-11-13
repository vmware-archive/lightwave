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
import com.sun.jna.platform.win32.WinNT.HANDLE;

/**
http://msdn.microsoft.com/en-us/library/Windows/desktop/aa377189(v=vs.85).aspx

typedef struct _CERT_CONTEXT {
  DWORD      dwCertEncodingType;
  BYTE       *pbCertEncoded;
  DWORD      cbCertEncoded;
  PCERT_INFO pCertInfo;
  HCERTSTORE hCertStore;
} CERT_CONTEXT, *PCERT_CONTEXT;typedef const CERT_CONTEXT *PCCERT_CONTEXT;
*/
public class CertContextNative extends Structure
{
    public int dwCertEncodingType;
    public Pointer pbCertEncoded;
    public int cbCertEncoded;
    public Pointer pCertInfo;
    public HANDLE hCertStore;

    public CertContextNative(Pointer p)
    {
        super();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList("dwCertEncodingType", "pbCertEncoded",
            "cbCertEncoded", "pCertInfo", "hCertStore");
    }

}
