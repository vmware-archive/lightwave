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

import java.io.Serializable;
import java.net.URI;
import java.net.URISyntaxException;

import org.apache.commons.lang3.Validate;


/**
 * OIDC issuer.
 *
 * @author Jun Sun
 */
public class Issuer implements Serializable {

    private static final long serialVersionUID = 2L;

    private final URI uri;

    Issuer(String value) {
        Validate.notEmpty(value, "value");
        try {
            this.uri = new URI(value);
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException(e);
        }
    }

    Issuer(URI uri) {
        Validate.notNull(uri, "uri");
        this.uri = uri;
    }

    public URI getURI() {
        return this.uri;
    }

    /**
     * Get issuer value
     *
     * @return                          String value of issuer
     */
    public String getValue() {
        return this.uri.toString();
    }
}
