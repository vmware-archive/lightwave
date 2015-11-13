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

import java.security.cert.CertificateEncodingException;

import javax.xml.soap.SOAPException;

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.ws_sx.ws_trust._200512.UseKeyType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ReferenceType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityTokenReferenceType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.DOMException;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;

/**
 * {@link WsSecuritySignature} implementation that works with
 * BinarySecurityTokens (X509Certificates)
 */
final class WsSecuritySignatureCertificate extends WsSecuritySignatureImpl {

    private static final String X509_CERTIFICATE_TYPE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3";
    private static final String SECURITY_ELEMENT = "Security";
    private static final String ERR_CREATING_USE_KEY_ELEMENT = "Error creating UseKey element";

    private final Logger _log = LoggerFactory.getLogger(WsSecuritySignatureCertificate.class);

    /**
     *
     * @param holderOfKeyConfig
     *            Contains holder-of-key configuration. Cannot be null.
     */
    public WsSecuritySignatureCertificate(HolderOfKeyConfig config) {
        super(config);
    }

    /**
     * Inserts KeyInfo data into the signature.
     *
     * @param message
     * @throws ParserException
     */
    @Override
    protected Node createKeyInfoContent(SoapMessage message) throws ParserException {

        String bstId = Util.randomNCNameUUID();

        // insert BinarySecurityToken in the Security header
        NodeList secNodeList = message.getMessage().getSOAPPart()
                .getElementsByTagNameNS(Constants.WSSE_NAMESPACE, SECURITY_ELEMENT);
        if (secNodeList.getLength() != 1) {
            throw new ParserException("No/too many security elements found");
        }
        secNodeList.item(0).appendChild(
                message.getMessage().getSOAPPart().importNode(createBinarySecurityToken(bstId), true /* deep */));

        return createSecurityTokenReference(bstId);
    }

    @Override
    protected String addUseKeySignatureId(SoapMessage message) throws ParserException {

        String sigId = Util.randomNCNameUUID();
        try {
            message.getMessage()
                    .getSOAPBody()
                    .getFirstChild()
                    .appendChild(
                            message.getMessage().getSOAPPart().importNode(createUseKeyElement(sigId), true /* deep */));
        } catch (DOMException e) {
            _log.debug(ERR_CREATING_USE_KEY_ELEMENT, e);
            throw new ParserException(ERR_CREATING_USE_KEY_ELEMENT, e);
        } catch (SOAPException e) {
            _log.debug(ERR_CREATING_USE_KEY_ELEMENT, e);
            throw new ParserException(ERR_CREATING_USE_KEY_ELEMENT, e);
        }

        return sigId;
    }

    /**
     * Creates UseKey element. It points to confirmation certificate used for
     * signing the signature. This certificate will be embedded in the requested
     * token as confirmation data.
     *
     * @param sigId
     *            the signature element id
     * @return
     * @throws ParserException
     */
    private Node createUseKeyElement(String sigId) throws ParserException {
        org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory wstFactory = new org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory();
        UseKeyType useKey = wstFactory.createUseKeyType();
        useKey.setSig(sigId);
        return marshallJaxbElement(useKey).getFirstChild();
    }

    /**
     * Creates a BinarySecurityToken element and sets its value to the base64
     * encoded version of the holder-of-key certificate
     *
     * @return BST element Id
     * @throws ParserException
     */
    private Node createBinarySecurityToken(String uuid) throws ParserException {
        ObjectFactory secExtFactory = new ObjectFactory();
        BinarySecurityTokenType bst = secExtFactory.createBinarySecurityTokenType();
        try {
            bst.setValue(Base64.encodeBase64String(holderOfKeyConfig.getCertificate().getEncoded()));
        } catch (CertificateEncodingException e) {
            String message = "Error creating BinarySecurityToken";
            _log.debug(message, e);
            throw new ParserException(message, e);
        }

        bst.setValueType(X509_CERTIFICATE_TYPE);
        bst.setEncodingType(Constants.ENCODING_TYPE_BASE64);
        bst.setId(uuid);

        return marshallJaxbElement(bst).getFirstChild();
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
    private Node createSecurityTokenReference(String refId) throws ParserException {
        ObjectFactory secExtFactory = new ObjectFactory();
        SecurityTokenReferenceType stRef = secExtFactory.createSecurityTokenReferenceType();
        ReferenceType ref = secExtFactory.createReferenceType();
        ref.setURI("#" + refId);
        ref.setValueType(X509_CERTIFICATE_TYPE);
        stRef.getAny().add(ref);

        return marshallJaxbElement(stRef).getFirstChild();
    }
}
