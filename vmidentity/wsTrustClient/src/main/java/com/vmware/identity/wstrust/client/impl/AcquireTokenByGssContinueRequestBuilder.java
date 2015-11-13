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
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A {@link RequestBuilderHelper} implementation which forms the continued (i.e.
 * after the first) "acquire token by GSS negotiation" requests (and parses
 * their responses).
 */
public final class AcquireTokenByGssContinueRequestBuilder implements RequestBuilder {

    private static final String CHALLENGE_SOAP_ACTION = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Issue";
    private final String contextId;
    private final byte[] leg;
    private final RequestBuilderHelper requestBuildHelper;
    private static final Logger log = LoggerFactory.getLogger(AcquireTokenByGssContinueRequestBuilder.class);

    /**
     * @param contextId
     *            The context id (STS session) needed to continue the Kerberos
     *            negotiation. Cannot be <code>null</code>.
     * @param leg
     *            Next Kerberos leg. Cannot be <code>null</code>.
     */
    public AcquireTokenByGssContinueRequestBuilder(String contextId, byte[] leg, JAXBContext jaxbContext,
            int requestValidityInSeconds) {

        assert leg != null;
        assert contextId != null;

        this.contextId = contextId;
        this.leg = leg;

        this.requestBuildHelper = new RequestBuilderHelper(jaxbContext, requestValidityInSeconds, CHALLENGE_SOAP_ACTION);
    }

    @Override
    public final SoapMessage createRequest() throws ParserException {
        ObjectFactory wstFactory = new ObjectFactory();

        if (log.isDebugEnabled()) {
            log.debug("Creating WS-Trust request: " + createRedactedDescription() + " with validity "
                    + requestBuildHelper.getRequestValidityInSeconds() + "sec.");
        }
        SecurityHeaderType secHeader = requestBuildHelper.createSecurityHeader();
        SoapMessage result = requestBuildHelper.wrapToSoapMessage(createBody(wstFactory), secHeader);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    public int getRequestValidityInSeconds() {
        return requestBuildHelper.getRequestValidityInSeconds();
    }

    private Object createBody(ObjectFactory wstFactory) throws ParserException {

        // Continued GSS negotiation requests must be of type
        // RequestSecurityTokenResponse
        RequestSecurityTokenResponseType request = wstFactory.createRequestSecurityTokenResponseType();

        request.setContext(contextId);

        // Add the next authentication leg to the request
        request.setBinaryExchange(requestBuildHelper.createBinaryExchangeElement(wstFactory, leg));

        return request;
    }

    private String createRedactedDescription() {
        return getClass().getSimpleName() + " [contextId=" + contextId + ", leg=" + Base64.encodeBase64String(leg)
                + "]";
    }
}
