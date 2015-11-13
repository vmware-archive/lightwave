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

import javax.xml.namespace.QName;

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.KeyIdentifierType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityTokenReferenceType;
import org.w3c.dom.Node;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;

/**
 * {@link WsSecuritySignature} implementation that works with SAML Assertions.
 */
final class WsSecuritySignatureAssertion extends WsSecuritySignatureImpl {

    private static final String WSSE11_NAMESPACE = "http://docs.oasis-open.org/wss/oasis-wss-wssecurity-secext-1.1.xsd";
    private static final String WSSE11_PREFIX = "wsse11";
    private static final String WSSE11_TOKEN_TYPE_ATTR_NAME = "TokenType";
    private static final String SAML_TOKEN_TYPE = "http://docs.oasis-open.org/wss/oasis-wss-saml-tokenprofile-1.1#SAMLV2.0";
    private static final String SAML_KEY_ID_TYPE = "http://docs.oasis-open.org/wss/oasis-wss-saml-token-profile-1.1#SAMLID";

    private final String _assertionId;

    public WsSecuritySignatureAssertion(HolderOfKeyConfig config, String assertionId) {
        super(config);

        assert assertionId != null;

        _assertionId = assertionId;
    }

    @Override
    protected Node createKeyInfoContent(SoapMessage message) throws ParserException {

        return createSecurityTokenReference();
    }

    @Override
    protected String addUseKeySignatureId(SoapMessage message) throws ParserException {
        return null;
    }

    /**
     * Creates SecurityTokenReference element that points to the refId
     * parameter.
     *
     * @param refId
     *            the reference to which this element points
     *
     * @return Node
     * @throws ParserException
     */
    private Node createSecurityTokenReference() throws ParserException {
        ObjectFactory secExtFactory = new ObjectFactory();
        SecurityTokenReferenceType stRef = secExtFactory.createSecurityTokenReferenceType();
        KeyIdentifierType ki = secExtFactory.createKeyIdentifierType();
        ki.setValue(_assertionId);
        ki.setValueType(SAML_KEY_ID_TYPE);
        stRef.getAny().add(ki);
        stRef.getOtherAttributes().put(new QName(WSSE11_NAMESPACE, WSSE11_TOKEN_TYPE_ATTR_NAME, WSSE11_PREFIX),
                SAML_TOKEN_TYPE);

        return marshallJaxbElement(stRef).getFirstChild();
    }
}
