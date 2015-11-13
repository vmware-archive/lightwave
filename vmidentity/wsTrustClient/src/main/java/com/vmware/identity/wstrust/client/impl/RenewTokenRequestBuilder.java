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

import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.vim.sso.client.SamlToken;

/**
 * RenewTokenRequestBuilder implementation of the {@link RequestBuilder}. It can
 * create requests specific for token renewal.
 */
public final class RenewTokenRequestBuilder implements RequestBuilder {

    private static final String RENEW_REQUEST_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Renew";
    private static final String RENEW_SOAP_ACTION = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Renew";
    private static final String RENEW_TARGET_ELEMENT_NAME = "RenewTarget";

    private final SamlToken token;
    private final long tokenLifetimeSec;
    private final RequestBuilderHelper requestBuilderHelper;

    private static final Logger log = LoggerFactory.getLogger(RenewTokenRequestBuilder.class);

    /**
     * Renew token request builder.
     *
     * @param token
     *            The token to be renewed. Cannot be <code>null</code>.
     * @param tokenLifetime
     *            token lifetime period in seconds from now; positive number is
     *            required
     */
    public RenewTokenRequestBuilder(SamlToken token, long tokenLifetime, JAXBContext jaxbContext,
            int requestValidityInSeconds) {
        assert token != null;
        assert tokenLifetime > 0 : tokenLifetime;

        this.token = token;
        this.tokenLifetimeSec = tokenLifetime;
        this.requestBuilderHelper = new RequestBuilderHelper(jaxbContext, requestValidityInSeconds, RENEW_SOAP_ACTION);
    }

    @Override
    public final SoapMessage createRequest() throws ParserException {
        ObjectFactory wstFactory = new ObjectFactory();

        if (log.isDebugEnabled()) {
            log.debug("Creating WS-Trust request: " + createRedactedDescription() + " with validity "
                    + requestBuilderHelper.getRequestValidityInSeconds() + "sec.");
        }
        SecurityHeaderType secHeader = requestBuilderHelper.createSecurityHeader();
        SoapMessage result = requestBuilderHelper.wrapToSoapMessage(createBody(wstFactory), secHeader);

        // Due to problems with JAXB marshaling/unmarshaling SAML tokens renew
        // target element should be manually added
        requestBuilderHelper.insertSamlToken(result, Constants.WS_TRUST_NAMESPACE, RENEW_TARGET_ELEMENT_NAME, token);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    public int getRequestValidityInSeconds() {
        return requestBuilderHelper.getRequestValidityInSeconds();
    }

    private Object createBody(ObjectFactory wstFactory) throws ParserException {
        RequestSecurityTokenType request = wstFactory.createRequestSecurityTokenType();
        request.setTokenType(RequestBuilderHelper.TOKEN_TYPE_SAML2);
        request.setRequestType(RENEW_REQUEST_TYPE);
        request.setLifetime(requestBuilderHelper.createLifetimeElement(tokenLifetimeSec));
        request.setRenewTarget(wstFactory.createRenewTargetType());

        return request;
    }

    private String createRedactedDescription() {
        return getClass().getSimpleName() + " [token=" + Util.createRedactedDescription(token) + "tokenLifetimeSec="
                + tokenLifetimeSec + "]";
    }
}
