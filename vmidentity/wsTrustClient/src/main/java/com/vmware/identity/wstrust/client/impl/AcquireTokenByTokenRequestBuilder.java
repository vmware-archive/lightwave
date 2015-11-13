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
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.TokenSpec;

/**
 * AcquireTokenByTokenRequestBuilder implementation of the
 * {@link RequestBuilderHelper}. It can create requests for acquiring token by
 * using previously obtained token for authentication.
 */
public final class AcquireTokenByTokenRequestBuilder extends AcquireTokenRequestBuilder {

    private static final String WSSE_SECURITY = "Security";
    private final Logger log = LoggerFactory.getLogger(AcquireTokenByTokenRequestBuilder.class);

    private final SamlToken token;

    /**
     * Creates new instance of the AcquireTokenByTokenRequestBuilder
     *
     * @param token
     *            The token to be used for authentication. Cannot be
     *            <code>null</code>.
     * @param tokenSpec
     *            The token spec stating the requirements for the new token.
     *            Cannot be <code>null</code>.
     * @param hokConfig
     *            Parameter indicating if the SSO client configuration contains
     *            holder-of-key information.
     */
    public AcquireTokenByTokenRequestBuilder(SamlToken token, TokenSpec tokenSpec, boolean hokConfirmation,
            JAXBContext jaxbContext, int requestValidityInSeconds) {

        super(tokenSpec, hokConfirmation, jaxbContext, requestValidityInSeconds);

        assert token != null;

        this.token = token;
    }

    @Override
    public final SoapMessage createRequest() throws ParserException {
        ObjectFactory wstFactory = new ObjectFactory();

        if (log.isDebugEnabled()) {
            log.debug("Creating WS-Trust request: " + createRedactedDescription() + " with validity "
                    + getRequestValidityInSeconds() + "sec.");
        }
        SecurityHeaderType secHeader = createSecurityHeader();
        SoapMessage result = wrapToSoapMessage(createBody(wstFactory), secHeader);
        postProcessRequest(result);
        insertSamlToken(result, Constants.WSSE_NAMESPACE, WSSE_SECURITY, token);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    protected String createRedactedDescription() {
        return String.format("%s [token=%s]", super.createRedactedDescription(), Util.createRedactedDescription(token));
    }
}
