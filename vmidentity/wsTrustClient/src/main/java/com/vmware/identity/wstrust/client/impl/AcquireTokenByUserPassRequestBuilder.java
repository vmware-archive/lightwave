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

import com.vmware.identity.wstrust.client.TokenSpec;

/**
 * AcquireTokenByUserPassRequestBuilder implementation of the
 * {@link RequestBuilderHelper}. It can create requests for acquiring token by
 * using user/password.
 */
final class AcquireTokenByUserPassRequestBuilder extends AcquireTokenRequestBuilder {

    private final String subject;
    private final String password;
    private final Logger log = LoggerFactory.getLogger(AcquireTokenByUserPassRequestBuilder.class);

    /**
     * Creates AcquireTokenByUserPassRequestBuilder. If the password is not
     * specified (i.e. creating request for a solution user) WS-Security
     * UsernameToken element won't be included in the request.
     *
     * @param subject
     *            The subject which requests the token. Cannot be
     *            <code>null</code>.
     * @param password
     *            The password of the subject. Cannot be <code>null</code>
     * @param spec
     *            The token spec stating the requirements for the new token.
     *            Cannot be <code>null</code>.
     * @param hokConfig
     *            Parameter indicating if the SSO client configuration contains
     *            holder-of-key information.
     */
    public AcquireTokenByUserPassRequestBuilder(String subject, String password, TokenSpec spec,
            boolean hokConfirmation, JAXBContext jaxbContext, int requestValidityInSeconds) {

        super(spec, hokConfirmation, jaxbContext, requestValidityInSeconds);

        assert subject != null;
        assert password != null;

        this.subject = subject;
        this.password = password;
    }

    @Override
    public final SoapMessage createRequest() throws ParserException {
        ObjectFactory wstFactory = new ObjectFactory();

        if (log.isDebugEnabled()) {
            log.debug("Creating WS-Trust request: " + createRedactedDescription() + " with validity "
                    + getRequestValidityInSeconds() + "sec.");
        }
        SecurityHeaderType secHeader = createSecurityHeader();
        addSecurityHeaderInfo(secHeader);
        SoapMessage result = wrapToSoapMessage(createBody(wstFactory), secHeader);
        postProcessRequest(result);

        log.debug("Finished creating WS-Trust request");

        return result;
    }

    @Override
    protected String createRedactedDescription() {
        return super.createRedactedDescription() + " [subject=" + subject + "]";
    }

    private void addSecurityHeaderInfo(SecurityHeaderType header) {
        header.getAny().add(createUsernameToken(subject, password));
    }
}
