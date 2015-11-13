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

import javax.xml.bind.JAXBContext;

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.TokenSpec;

/**
 * A {@link RequestBuilder} which forms the first
 * "acquire token by GSS negotiation" request. For <b>internal</b> use by the
 * SecurityTokenService only, do not use this class directly.
 */
public final class AcquireTokenByGssInitiateRequestBuilder extends AcquireTokenRequestBuilder {

    private final byte[] leg;
    private final String contextId;

    private final Logger log = LoggerFactory.getLogger(AcquireTokenByGssInitiateRequestBuilder.class);

    /**
     * Create the request builder.
     *
     * @param tokenSpec
     *            The desired properties of the token being negotiated. Cannot
     *            be {@code null}.
     * @param initialLeg
     *            The first client authentication leg (as generation by the
     *            client's GSS implementation). Cannot be {@code null}.
     * @param Parameter
     *            indicating if the SSO client configuration contains
     *            holder-of-key information.
     */
    public AcquireTokenByGssInitiateRequestBuilder(TokenSpec tokenSpec, byte[] initialLeg, boolean hokConfirmation,
            JAXBContext jaxbContext, int requestValidityInSeconds) {

        super(tokenSpec, hokConfirmation, jaxbContext, requestValidityInSeconds);

        assert initialLeg != null;

        this.leg = initialLeg;
        this.contextId = Util.randomNCNameUUID();
    }

    @Override
    public final SoapMessage createRequest() throws ParserException {
        ObjectFactory wstFactory = new ObjectFactory();

        if (log.isDebugEnabled()) {
            log.debug("Creating WS-Trust request: " + createRedactedDescription() + " with validity "
                    + getRequestValidityInSeconds() + "sec.");
        }
        SecurityHeaderType secHeader = createSecurityHeader();
        Object request = createBody(wstFactory);
        addRequestData((RequestSecurityTokenType) request, wstFactory);
        SoapMessage result = wrapToSoapMessage(request, secHeader);
        postProcessRequest(result);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    protected String createRedactedDescription() {
        return String.format("%s [leg=%s, contextId=%s]", getClass().getSimpleName(), Base64.encodeBase64String(leg),
                contextId);
    }

    private void addRequestData(RequestSecurityTokenType request, ObjectFactory wstFactory) {
        request.setContext("urn:" + contextId);
        // Add the authentication leg to the request
        request.setBinaryExchange(createBinaryExchangeElement(wstFactory, leg));
    }
}
