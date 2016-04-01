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

package com.vmware.identity.interop.ldap;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;

/**
 * This class is mapping to the native SecPkgContext_ConnectionInfo structure used to return the security information.
 * The structure is used by the InitializeSecurityContext (Schannel) function.
 * The the native structure:
 * <pre>
 * typedef struct _SecPkgContext_ConnectionInfo {
 *  DWORD  dwProtocol;
 *  ALG_ID aiCipher;
 *  DWORD  dwCipherStrength;
 *  ALG_ID aiHash;
 *  DWORD  dwHashStrength;
 *  ALG_ID aiExch;
 *  DWORD  dwExchStrength;
 *  }
 * </pre>
 *
 */
public class ConnectionInfoNative extends Structure {

    public int dwProtocol;
    public long aiCipher;
    public int dwCipherStrength;
    public long aiHash;
    public int dwHashStrength;
    public long aiExch;
    public int dwExchStrength;

    public ConnectionInfoNative() {
        read();
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(new String[] { "dwProtocol", "aiCipher", "dwCipherStrength", "aiHash", "dwHashStrength",
                "aiExch", "dwExchStrength" });
    }
}
