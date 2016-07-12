/*
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
 */

package com.vmware.identity.openidconnect.client;

/**
 * @author Yehia Zayour
 */
public final class HighAvailabilityConfig {
    private final String domainName;
    private final ClientDCCacheFactory clientDCCacheFactory;

    public HighAvailabilityConfig() {
        this(null, new ClientDCCacheFactory(null));
    }

    /**
     * Constructor
     *
     * @param domainName          HA domain name
     */
    public HighAvailabilityConfig(String domainName) {
        this(domainName, new ClientDCCacheFactory(domainName)); // domainName is nullable
    }

    // for unit tests
    HighAvailabilityConfig(String domainName, ClientDCCacheFactory clientDCCacheFactory) {
        this.domainName = domainName;
        this.clientDCCacheFactory = clientDCCacheFactory;
    }

    /**
     * Get domain name
     *
     * @return                          High availability domain name
     */
    public String getDomainName() {
        return this.domainName;
    }

    ClientDCCacheFactory getClientDCCacheFactory() {
        return this.clientDCCacheFactory;
    }
}
