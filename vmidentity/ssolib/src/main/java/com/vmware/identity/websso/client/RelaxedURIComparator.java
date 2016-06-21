/* *************************************************************************
 * Copyright 2011 VMware, Inc. All rights reserved. VMware Confidential
 * ************************************************************************/
package com.vmware.identity.websso.client;

import org.opensaml.common.binding.decoding.BasicURLComparator;
import org.opensaml.common.binding.decoding.URIComparator;

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

    /**
     * Protocal relaxed comparison of URI
     * @return true the two URIs are equal
     * @see org.opensaml.common.binding.decoding.URIComparator#compare(java.lang.String, java.lang.String)
     */
    public boolean compare(String arg0, String arg1) {
        // convert https in the beginning of the url into http and then compare
        // that ensures relaxed protocol check

        arg0 = fixProtocol(arg0);
        arg1 = fixProtocol(arg1);

        return this.comparator.compare(arg0, arg1);
    }

    // normalize protocol string to HTTP
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
