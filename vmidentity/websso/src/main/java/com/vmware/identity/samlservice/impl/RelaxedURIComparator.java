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
package com.vmware.identity.samlservice.impl;

import org.opensaml.common.binding.decoding.BasicURLComparator;
import org.opensaml.common.binding.decoding.URIComparator;

import com.vmware.identity.samlservice.Shared;

/**
 * This replaces default URIComparator from OpenSAML and allows us to accept
 *  requests sent through HTTP protocol to HTTPS endpoint and vice versa.
 *
 * We have noticed that protocol is sometimes changed by reverse proxy.
 *
 */
public final class RelaxedURIComparator implements URIComparator {

    private BasicURLComparator comparator;

    private final static String HTTPS = "https";
    private final static String HTTP = "http";

    /**
     * Create instance
     */
    public RelaxedURIComparator() {
        this.comparator = new BasicURLComparator();
    }

    /* (non-Javadoc)
     * @see org.opensaml.common.binding.decoding.URIComparator#compare(java.lang.String, java.lang.String)
     */
    public boolean compare(String arg0, String arg1) {
        // convert https in the beginning of the url into http and then compare
        // that ensures relaxed protocol check

        arg0 = fixProtocol(arg0);
        arg1 = fixProtocol(arg1);

        arg0 = arg0.replace(Shared.ssoCACEndpoint, Shared.ssoEndpoint);
        arg1 = arg1.replace(Shared.ssoCACEndpoint, Shared.ssoEndpoint);
        arg0 = arg0.replace(Shared.ssoSmartcardRealmEndpoint, Shared.ssoEndpoint);
        arg1 = arg1.replace(Shared.ssoSmartcardRealmEndpoint, Shared.ssoEndpoint);
        return this.comparator.compare(arg0, arg1);
    }

    // fix protocol string
    private String fixProtocol(String arg0) {
        if (arg0 == null) {
            return null;
        } else if (arg0.startsWith(HTTPS)) {
            return arg0.replaceFirst(HTTPS, HTTP);
        } else {
            return arg0;
        }
    }

}
