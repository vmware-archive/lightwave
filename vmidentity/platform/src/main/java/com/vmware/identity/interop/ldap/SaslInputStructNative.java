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

    public SaslInputStructNative(String userName, String password) {
        Validate.notEmpty(userName, "userName");
        Validate.notEmpty(password, "password");

        this.nativeMemories = new NativeMemory[2];
        String normalizedUserName = userName.toLowerCase();
        this.authName = this.nativeMemories[0] = allocateMemoryForString(normalizedUserName);
        authNameLength = userName != null? userName.length(): 0;
        this.password = this.nativeMemories[1] = allocateMemoryForString(password);
        passwordLength = password != null? password.length(): 0;
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

    private static NativeMemory allocateMemoryForString(String s){
        NativeMemory m = null;
        if(s!=null && s.length() > 0){
            byte[] bytes = Native.toByteArray(s);
            m = new NativeMemory(bytes.length + 1);
            m.write(0, bytes, 0, bytes.length);
            m.setByte(bytes.length, (byte)0);
        }
        return m;
    }
}
