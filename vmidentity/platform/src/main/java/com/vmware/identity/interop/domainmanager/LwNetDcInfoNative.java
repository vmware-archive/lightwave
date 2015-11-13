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

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public class LwNetDcInfoNative extends Structure
{
    public int dwPingTime;
    public int dwDomainControllerAddressType;
    public int dwFlags;
    public int dwVersion;
    public int wLMToken;
    public int wNTToken;
    public String pszDomainControllerName;
    public String pszDomainControllerAddress;
    public byte[] pucDomainGUID;
    public String pszNetBIOSDomainName;
    public String pszFullyQualifiedDomainName;
    public String pszDnsForestName;
    public String pszDCSiteName;
    public String pszClientSiteName;
    public String pszNetBIOSHostName;
    public String pszUserName;

    public LwNetDcInfoNative()
    {
        pucDomainGUID = new byte[LinuxDomainAdapter.LWNET_GUID_SIZE];
    }

    public LwNetDcInfoNative(Pointer p)
    {
        this();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "dwPingTime", "dwDomainControllerAddressType", "dwFlags", "dwVersion", "wLMToken",
                "wNTToken", "pszDomainControllerName", "pszDomainControllerAddress",
                "pucDomainGUID", "pszNetBIOSDomainName", "pszFullyQualifiedDomainName", "pszDnsForestName",
                "pszDCSiteName", "pszClientSiteName", "pszNetBIOSHostName", "pszUserName"
        });
    }
}
