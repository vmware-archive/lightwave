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
package com.vmware.identity.rest.core.server.authorization.token.saml;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.apache.commons.codec.binary.Base64;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.AccessTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.exception.ServerException;
import com.vmware.identity.rest.core.util.StringManager;
import com.vmware.identity.token.impl.Constants;
import com.vmware.identity.token.impl.SamlTokenImpl;

/**
 * An implementation of a bearer token builder using the SAML format.
 *
 * @see <a href="http://saml.xml.org/">SAML XML.org</a>
 */
public class SAMLTokenBuilder implements AccessTokenBuilder {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(SAMLTokenBuilder.class);

    protected JAXBContext jaxbContext;
    protected StringManager sm;

    public SAMLTokenBuilder() throws ServerException {
        this.jaxbContext = createJAXBContext();
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    @Override
    public AccessToken build(TokenInfo info) throws InvalidTokenException {
        String xml = new String(Base64.decodeBase64(info.getToken()));

        try {
            SamlTokenImpl token = new SamlTokenImpl(xml, jaxbContext);
            return new SAMLToken(token);
        } catch (com.vmware.vim.sso.client.exception.InvalidTokenException e) {
            log.error("Error parsing the SAML Bearer Token", e);
            throw new InvalidTokenException(sm.getString("auth.ite.parse.malformed"));
        }
    }

    /**
     * @return {@link JAXBContext} for {@link Constants#ASSERTION_JAXB_PACKAGE}
     * @throws ServerException
     */
    private static JAXBContext createJAXBContext() throws ServerException {
        try {
            return JAXBContext.newInstance(Constants.ASSERTION_JAXB_PACKAGE);
        } catch (JAXBException e) {
            throw new ServerException("Cannot initialize JAXBContext.", e);
        }
    }

}
