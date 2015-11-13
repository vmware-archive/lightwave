/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import org.w3c.dom.Element;

/**
 * Represents a result of a GSS round-trip (request/response) to the STS
 */
public class GssResult {

    private final byte[] leg;
    private final Element token;
    private final String contextId;

    /**
     * Create a GSS round-trip result
     *
     * @param leg
     *            The next authentication leg returned from the STS
     * @param token
     *            The root element of the SAML Token returned from the STS, if
     *            any.
     * @param contextId
     *            The unique identifier of the GSS negotiation session.
     */
    public GssResult(byte[] leg, Element token, String contextId) {
        this.leg = leg;
        this.token = token;
        this.contextId = contextId;
    }

    /**
     * Returns the next leg of authentication (extracted from the
     * &lt;wst:BinaryExchange&gt; element)
     *
     * @return
     */
    public byte[] getLeg() {
        return leg;
    }

    /**
     * Returns the SAML Token (extracted &lt;saml:Assertion&gt; element), if any
     *
     * @return
     */
    public Element getToken() {
        return token;
    }

    /**
     * Returns the current contextId (STS session)
     *
     * @return
     */
    public String getContextId() {
        return contextId;
    }
}
