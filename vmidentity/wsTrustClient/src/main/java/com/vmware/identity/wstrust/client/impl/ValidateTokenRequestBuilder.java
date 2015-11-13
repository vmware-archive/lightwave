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
import org.oasis_open.docs.ws_sx.ws_trust._200512.ValidateTargetType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.vim.sso.client.SamlToken;

/**
 * ValidateTokenRequestBuilder implementation of the {@link RequestBuilder}. It
 * can create requests specific for server side token validation.
 */
public final class ValidateTokenRequestBuilder implements RequestBuilder {

    private static final String REQUEST_TYPE_VALIDATE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Validate";
    private static final String SOAP_ACTION_VALIDATE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Validate";
    private static final String TOKEN_TYPE_STATUS = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Status";
    private static final String VALIDATE_TARGET_ELEMENT_NAME = "ValidateTarget";

    private final SamlToken token;
    private final RequestBuilderHelper requestBuilderHelper;
    private static final Logger log = LoggerFactory.getLogger(ValidateTokenRequestBuilder.class);

    /**
     * Validate token request builder
     *
     * @param token
     *            The token to be validated. Cannot be <code>null</code>.
     */
    public ValidateTokenRequestBuilder(SamlToken token, JAXBContext jaxbContext, int requestValidityInSeconds) {

        assert token != null;

        this.token = token;
        this.requestBuilderHelper = new RequestBuilderHelper(jaxbContext, requestValidityInSeconds,
                SOAP_ACTION_VALIDATE);
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

        // Due to problems with JAXB marshaling/unmarshaling SAML tokens
        // validate
        // target element should be manually added to the validate token STS
        // request
        requestBuilderHelper.insertSamlToken(result, Constants.WS_TRUST_NAMESPACE, VALIDATE_TARGET_ELEMENT_NAME, token);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    public int getRequestValidityInSeconds() {
        return requestBuilderHelper.getRequestValidityInSeconds();
    }

    private Object createBody(ObjectFactory wstFactory) throws ParserException {
        RequestSecurityTokenType request = wstFactory.createRequestSecurityTokenType();
        request.setRequestType(REQUEST_TYPE_VALIDATE);
        request.setTokenType(TOKEN_TYPE_STATUS);
        request.setValidateTarget(new ValidateTargetType());

        return request;
    }

    private String createRedactedDescription() {
        return getClass().getSimpleName() + " [token=" + Util.createRedactedDescription(token) + "]";
    }
}
