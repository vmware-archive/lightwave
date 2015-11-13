/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;

public class DomainTrustsInfo implements Serializable
{
    private static final long serialVersionUID = 1L;

    public DomainController dcInfo;

    public boolean IsInforest = false;
    public boolean IsOutBound = false;
    public boolean IsInBound = false;
    public boolean IsRoot = false;
    public boolean IsPrimary = false;
    public boolean IsNativeMode = false;
    public boolean isExternal = false;

    public DomainTrustsInfo(DomainTrustInfoBuilder trustBuilder)
    {
        this.dcInfo = new DomainController();
        this.IsInBound = trustBuilder.IsInBound;
        this.IsOutBound = trustBuilder.IsOutBound;
        this.IsInforest = trustBuilder.IsInforest;
        this.IsRoot = trustBuilder.IsRoot;
        this.IsPrimary = trustBuilder.IsPrimary;
        this.IsNativeMode = trustBuilder.IsNativeMode;
        this.isExternal = trustBuilder.isExternal;
        this.dcInfo = trustBuilder.dcInfo;
    }

    public static class DomainController implements Serializable
    {
        private static final long serialVersionUID = 1L;
        public String domainName;
        public String domainIpAddress;
        public String domainFQDN;
        public String domainDnsForestName;
        public String domainNetBiosName;
    }

    public static class DomainTrustInfoBuilder
    {
        private boolean IsInforest = false;
        private boolean IsOutBound = false;
        private boolean IsInBound = false;
        private boolean IsRoot = false;
        private boolean IsPrimary = false;
        private boolean IsNativeMode = false;
        private boolean isExternal = false;
        public DomainController dcInfo = null;

        public DomainTrustInfoBuilder(boolean IsInforest, boolean IsOutBound, boolean IsInBound)
        {
            this.IsInforest = IsInforest;
            this.IsOutBound = IsOutBound;
            this.IsInBound = IsInBound;
        }

        public DomainTrustInfoBuilder IsRoot(boolean IsRoot)
        {
            this.IsRoot = IsRoot;
            return this;
        }

        public DomainTrustInfoBuilder IsPrimary(boolean IsPrimary)
        {
            this.IsPrimary = IsPrimary;
            return this;
        }

        public DomainTrustInfoBuilder IsNativeMode(boolean IsNativeMode)
        {
            this.IsNativeMode = IsNativeMode;
            return this;
        }

        public DomainTrustInfoBuilder isExternal(boolean isExternal)
        {
            this.isExternal = isExternal;
            return this;
        }

        public DomainTrustInfoBuilder dcInfo(String domainName, String domainNetBiosName, String domainAddress, String domainFQDN, String domainDnsForestName)
        {
            this.dcInfo = new DomainController();
            this.dcInfo.domainDnsForestName = domainDnsForestName;
            this.dcInfo.domainFQDN = domainFQDN;
            this.dcInfo.domainIpAddress = domainAddress;
            this.dcInfo.domainName = domainName;
            this.dcInfo.domainNetBiosName = domainNetBiosName;
            return this;
        }

        public DomainTrustsInfo build() {
            return new DomainTrustsInfo(this);
        }
    }
}
