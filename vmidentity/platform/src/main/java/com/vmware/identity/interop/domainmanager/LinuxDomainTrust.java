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

import com.sun.jna.Platform;
import com.sun.jna.Pointer;

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 04/09/2013
 * Time: 10:59 PM
 */
public class LinuxDomainTrust
{
    class LsaTrustType
    {
        public static final int LSA_TRUST_TYPE_DOWNLEVEL = 0x00000001;
        public static final int LSA_TRUST_TYPE_UPLEVEL = 0x00000002;
        public static final int LSA_TRUST_TYPE_MIT = 0x00000003;
        public static final int LSA_TRUST_TYPE_DCE = 0x00000004;
    }

    class LsaTrustAttribute
    {
        public static final int LSA_TRUST_ATTRIBUTE_NON_TRANSITIVE = 0x00000001;
        public static final int LSA_TRUST_ATTRIBUTE_UPLEVEL_ONLY = 0x00000002;
        public static final int LSA_TRUST_ATTRIBUTE_FILTER_SIDS = 0x00000004;
        public static final int LSA_TRUST_ATTRIBUTE_FOREST_TRANSITIVE = 0x00000008;
        public static final int LSA_TRUST_ATTRIBUTE_CROSS_ORGANIZATION = 0x00000010;
        public static final int LSA_TRUST_ATTRIBUTE_WITHIN_FOREST = 0x00000020;
    }

    class LsaTrustFlag
    {
        public static final int LSA_TRUST_FLAG_IN_FOREST = 0x00000001;
        public static final int LSA_TRUST_FLAG_OUTBOUND = 0x00000002;
        public static final int LSA_TRUST_FLAG_TREEROOT = 0x00000004;
        public static final int LSA_TRUST_FLAG_PRIMARY = 0x00000008;
        public static final int LSA_TRUST_FLAG_NATIVE = 0x00000010;
        public static final int LSA_TRUST_FLAG_INBOUND = 0x00000020;
    }

    public boolean IsInforest;
    public boolean IsOutBound;
    public boolean IsInBound;
    public boolean IsRoot;
    public boolean IsPrimary;
    public boolean IsNativeMode;

    public DomainControllerInfo dcInfo;

    public String domainName;
    public String domainIpAddress;
    public String domainFQDN;
    public String domainDnsForestName;

    public String pszDnsDomain;
    public String pszNetbiosDomain;
    public String pszTrusteeDnsDomain;
    public String pszDomainSID;
    public String pszDomainGUID;
    public String pszForestName;
    public String pszClientSiteName;

    public LinuxDomainTrust(LsaTrustedDomainInfoNative trust)
    {
        assert Platform.isLinux();

        this.IsInforest = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_IN_FOREST) != 0 ;
        this.IsOutBound = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_OUTBOUND) != 0;
        this.IsInBound = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_INBOUND) != 0;
        this.IsRoot = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_TREEROOT) != 0;
        this.IsPrimary = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_PRIMARY) != 0;
        this.IsNativeMode = (trust.dwTrustFlags & LsaTrustFlag.LSA_TRUST_FLAG_NATIVE) != 0;

        if (trust.pDCInfo != Pointer.NULL)
        {
            LsaDcInfoNative lsaDcInfo = new LsaDcInfoNative(trust.pDCInfo);
            this.dcInfo = new DomainControllerInfo(trust.pszDnsDomain,
                                                   trust.pszNetbiosDomain,
                                                   lsaDcInfo.pszAddress,
                                                   lsaDcInfo.pszName,
                                                   trust.pszForestName);
        }
        else
        {
            this.dcInfo = new DomainControllerInfo(trust.pszDnsDomain,
                                                   trust.pszNetbiosDomain,
                                                   "",
                                                   "",
                                                   trust.pszForestName);
        }
    }
}

