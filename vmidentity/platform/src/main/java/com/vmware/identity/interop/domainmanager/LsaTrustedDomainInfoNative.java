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

public class LsaTrustedDomainInfoNative extends Structure
{
    public String pszDnsDomain;
    public String pszNetbiosDomain;
    public String pszTrusteeDnsDomain;
    public String pszDomainSID;
    public String pszDomainGUID;
    public String pszForestName;
    public String pszClientSiteName;
    public int dwTrustFlags;
    public int dwTrustType;
    public int dwTrustAttributes;
    public int dwTrustDirection;
    public int dwTrustMode;
    public int dwDomainFlags;
    public Pointer pDCInfo; // PLSA_DC_INFO
    public Pointer pGCInfo; // PLSA_DC_INFO

    public LsaTrustedDomainInfoNative()
    {
        super();
    }
    public LsaTrustedDomainInfoNative(Pointer p)
    {
        this();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "pszDnsDomain", "pszNetbiosDomain", "pszTrusteeDnsDomain", "pszDomainSID", "pszDomainGUID",
                "pszForestName", "pszClientSiteName", "dwTrustFlags",
                "dwTrustType", "dwTrustAttributes", "dwTrustDirection", "dwTrustMode",
                "dwDomainFlags", "pDCInfo", "pGCInfo"
        });
    }
}
