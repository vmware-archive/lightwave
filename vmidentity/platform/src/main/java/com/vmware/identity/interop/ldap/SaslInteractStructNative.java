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

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * The type is used to pass the prompt and response data between ldap_sasl_interactive_bind_s and
 *  its callback, ldapSaslSrpInteractFunc.
 */
public class SaslInteractStructNative extends Structure{
    public long id;
    public Pointer challenge;
    public Pointer prompt;
    public Pointer defResult;
    public Pointer result;
    public int val;

    public SaslInteractStructNative(Pointer ptr, int offset) {
        useMemory(ptr, offset);
        read();
    }

    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[] { "id", "challenge", "prompt",
                "defResult", "result", "val" });
    }
}