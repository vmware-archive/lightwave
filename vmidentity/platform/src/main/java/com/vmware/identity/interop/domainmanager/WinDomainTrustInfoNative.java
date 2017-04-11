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

package com.vmware.identity.interop.domainmanager;

import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;

public class WinDomainTrustInfoNative extends Structure
{
    private static final int TRUST_FLAG_IN_FOREST       = 0x00000001;
    private static final int TRUST_FLAG_DIRECT_OUTBOUND = 0x00000002;
    private static final int TRUST_FLAG_TREE_ROOT       = 0x00000004;
    private static final int TRUST_FLAG_PRIMARY         = 0x00000008;
    private static final int TRUST_FLAG_NATIVE_MODE     = 0x00000010;
    private static final int TRUST_FLAG_DIRECT_INBOUND  = 0x00000020;

    private static final int TRUST_ATTR_FOREST_TRANSITIVE = 0x00000008;

    public String netBiosDomainName;
    public String dnsDomainName;
    public int Flags;
    public int parentIndex;
    public int trustType;
    public int trustAttributes;
    public Pointer domainSid;
    public WinGuidNative domainGuid;


    public WinDomainTrustInfoNative()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : null);
    }

    public WinDomainTrustInfoNative(Pointer p)
    {
        this();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "netBiosDomainName", "dnsDomainName", "Flags", "parentIndex", "trustType",
                "trustAttributes", "domainSid", "domainGuid"
        });
    }

    public boolean isInBound()
    {
        return (this.Flags & TRUST_FLAG_DIRECT_INBOUND) != 0;
    }

    public boolean isOutBound()
    {
        return (this.Flags & TRUST_FLAG_DIRECT_OUTBOUND) != 0;
    }

    public boolean isInForest()
    {
        return (this.Flags & TRUST_FLAG_IN_FOREST) != 0;
    }

    public boolean isRoot()
    {
        return (this.Flags & TRUST_FLAG_TREE_ROOT) != 0;
    }

    public boolean isPrimary()
    {
        return (this.Flags & TRUST_FLAG_PRIMARY) != 0;
    }

    public boolean isNativeMode()
    {
        return (this.Flags & TRUST_FLAG_NATIVE_MODE) != 0;
    }

    public boolean isExternal()
    {
        return !isInForest() && (this.trustAttributes & TRUST_ATTR_FOREST_TRANSITIVE) == 0;
    }

    public boolean isInOtherForest()
    {
        return !isInForest() && (this.trustAttributes & TRUST_ATTR_FOREST_TRANSITIVE) != 0;
    }

    public static int AllFlags()
    {
        return (TRUST_FLAG_DIRECT_INBOUND | TRUST_FLAG_DIRECT_OUTBOUND | TRUST_FLAG_IN_FOREST
                | TRUST_FLAG_TREE_ROOT | TRUST_FLAG_PRIMARY | TRUST_FLAG_NATIVE_MODE);
    }

    public static int TreeRootFlags()
    {
        return TRUST_FLAG_TREE_ROOT;
    }
}
