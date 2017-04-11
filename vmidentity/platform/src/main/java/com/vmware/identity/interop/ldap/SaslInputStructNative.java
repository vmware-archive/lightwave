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

import org.apache.commons.lang.Validate;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.vmware.identity.interop.NativeMemory;

/**
 * The type is used to pass the original data from caller to ldap_sasl_interactive_bind_s;
 *  the same data is passed from ldap_sasl_interactive_bind_s to callback func, ldapSaslSrpInteractFunc
 *      , where data is finally consumed.
 */
public class SaslInputStructNative extends Structure implements AutoCloseable{
    public Pointer authName;
    public Pointer password;
    public int authNameLength;
    public int passwordLength;
    private NativeMemory[] nativeMemories;

    public SaslInputStructNative(String userName, String pwd) {
        Validate.notEmpty(userName, "userName");
        Validate.notEmpty(pwd, "pwd");

        this.nativeMemories = new NativeMemory[2];

        // Set the native memory for username.
        String normalizedUserName = normalizeUserName(userName);
        byte[] bytes = Native.toByteArray(normalizedUserName, "UTF-8");
        nativeMemories[0] = new NativeMemory(bytes.length);
        nativeMemories[0].write(0, bytes, 0, bytes.length);
        authName = nativeMemories[0];
        authNameLength = bytes.length - 1; // Not including the ending '\0'

        // Set the native memory for password.
        bytes = Native.toByteArray(pwd, "UTF-8");
        nativeMemories[1] = new NativeMemory(bytes.length);
        nativeMemories[1].write(0, bytes, 0, bytes.length);
        password = nativeMemories[1];
        passwordLength = bytes.length - 1; // Not including the ending '\0'

        write();
    }

    public SaslInputStructNative(Pointer ptr) {
        useMemory(ptr);
        read();
    }

    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[] { "authName", "password", "authNameLength", "passwordLength"});
    }

    @Override
    public void close() {
        if (this.nativeMemories != null){
            for (NativeMemory mem : this.nativeMemories) {
                if (mem != null) {
                    mem.close();
                }
            }
            this.nativeMemories = null;
        }
    }

    // Normalize the username in the same way as vmdird. This is required in the SRP binding
    // so that the secret/hash generated at client and server sides are consistent.
    private static String normalizeUserName(String userName) {
        StringBuffer sb = new StringBuffer();
        for (int i=0; i<userName.length(); i++) {
            char c = userName.charAt(i);
            if (c >= 'A' && c <= 'Z') {
                c += 32; // to lower case
            }
            sb.append(c);
        }
        return sb.toString();
    }
}