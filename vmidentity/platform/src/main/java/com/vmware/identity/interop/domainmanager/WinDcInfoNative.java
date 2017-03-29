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

public class WinDcInfoNative extends Structure
{
    public String domainControllerName;
    public String domainControllerAddress;
    public int domainControllerAddressType;
    public WinGuidNative domainGuid;
    public String domainName;
    public String dnsForestName;
    public int flags;
    public String dcSiteName;
    public String clientSideName;

    public WinDcInfoNative()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : null);
    }

    public WinDcInfoNative(Pointer p)
    {
        this();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "domainControllerName", "domainControllerAddress", "domainControllerAddressType", "domainGuid", "domainName",
                "dnsForestName", "flags", "dcSiteName", "clientSideName"
        });
    }
}
