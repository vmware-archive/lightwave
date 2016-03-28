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

import java.security.InvalidAlgorithmParameterException;
import java.security.NoSuchAlgorithmException;
import java.security.Provider;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dom.DOMStructure;
import javax.xml.crypto.dsig.CanonicalizationMethod;
import javax.xml.crypto.dsig.DigestMethod;
import javax.xml.crypto.dsig.Reference;
import javax.xml.crypto.dsig.SignatureMethod;
import javax.xml.crypto.dsig.SignedInfo;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMSignContext;
import javax.xml.crypto.dsig.keyinfo.KeyInfo;
import javax.xml.crypto.dsig.keyinfo.KeyInfoFactory;
import javax.xml.crypto.dsig.spec.C14NMethodParameterSpec;
import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.soap.SOAPException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;

/**
 * Class implementing parts WS Security protocol regarding XML signatures. It
 * signs {@link SoapMessage} messages with the provided certificate/private key.
 */
abstract class WsSecuritySignatureImpl implements WsSecuritySignature {

    protected static final String SIGNATURE_ELEMENT_NAME = "Signature";
    protected static final String WSU_NAMESPACE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd";
    protected static final String WSU_ID_LOCAL_NAME = "Id";

    protected final HolderOfKeyConfig holderOfKeyConfig;

    private static final String DIGITAL_SIGNATURE_NAMESPACE_PREFIX = "ds";
    private static final String WSU_TIMESTAMP_LOCAL_NAME = "Timestamp";
    private static final String WSU_PREFIX = "wsu";
    private static final String WSSE_JAXB_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0";
    private static final String WST_JAXB_PACKAGE = "org.oasis_open.docs.ws_sx.ws_trust._200512";
    private static final String RSA_WITH_SHA512 = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha512";
    private static final String PARSING_XML_ERROR_MSG = "Error while parsing the SOAP request (signature creation)";
    private static final String CREATING_SIGNATURE_ERR_MSG = "Error while creating SOAP request signature";
    private static final String MARSHALL_EXCEPTION_ERR_MSG = "Error marshalling JAXB document";

    private static final Logger log = LoggerFactory.getLogger(WsSecuritySignatureImpl.class);

    private static JAXBContext jaxBContext = createJAXBContext();

    /**
     *
     * @param holderOfKeyConfig
     *            Contains holder-of-key configuration. Cannot be null.
     */
    protected WsSecuritySignatureImpl(HolderOfKeyConfig holderOfKeyConfig) {

        assert holderOfKeyConfig != null;

        this.holderOfKeyConfig = holderOfKeyConfig;
    }

    /**
     * Signs a SoapMessage with the holder-of-key configuration provided on
     * class creation. This method changes the SoapMessage.
     *
     * @param message
     *            cannot be null
     * @return The signed SoapMessage
     * @throws ParserException
     * @throws SignatureException
     */
    @Override
    public final SoapMessage sign(SoapMessage message) throws ParserException, SignatureException {

        assert message != null;

        Provider securityProvider = holderOfKeyConfig.getSecurityProvider();
        XMLSignatureFactory xmlSigFactory = (securityProvider != null) ? XMLSignatureFactory.getInstance("DOM",
                securityProvider) : XMLSignatureFactory.getInstance();

        try {
            String bodyUuid = createSoapBodyUuid(message);
            CanonicalizationMethod canonicalizationMethod = xmlSigFactory.newCanonicalizationMethod(
                    CanonicalizationMethod.EXCLUSIVE, (C14NMethodParameterSpec) null);
            SignatureMethod signatureMethod = xmlSigFactory.newSignatureMethod(RSA_WITH_SHA512, null);
            ArrayList<String> refList = new ArrayList<String>();
            refList.add(bodyUuid);
            refList.add(createTimestampUuid(message));
            List<Reference> references = createSignatureReferences(xmlSigFactory, refList);
            SignedInfo signedInfo = xmlSigFactory.newSignedInfo(canonicalizationMethod, signatureMethod, references);

            KeyInfoFactory kif = KeyInfoFactory.getInstance();
            KeyInfo ki = kif.newKeyInfo(Collections.singletonList(new DOMStructure(createKeyInfoContent(message))));

            XMLSignature signature = xmlSigFactory.newXMLSignature(signedInfo, ki, null, addUseKeySignatureId(message),
                    null);

            DOMSignContext dsc = new DOMSignContext(holderOfKeyConfig.getPrivateKey(), message.getHeader()
                    .getFirstChild());
            dsc.putNamespacePrefix(XMLSignature.XMLNS, DIGITAL_SIGNATURE_NAMESPACE_PREFIX);

            signature.sign(dsc);

            log.debug("Message with SOAPBody id: " + bodyUuid + " is signed.");
        } catch (NoSuchAlgorithmException e) {
            log.debug(CREATING_SIGNATURE_ERR_MSG);
            throw new SignatureException(CREATING_SIGNATURE_ERR_MSG, e);
        } catch (InvalidAlgorithmParameterException e) {
            log.debug(CREATING_SIGNATURE_ERR_MSG);
            throw new SignatureException(CREATING_SIGNATURE_ERR_MSG, e);
        } catch (MarshalException e) {
            log.debug(CREATING_SIGNATURE_ERR_MSG);
            throw new SignatureException(CREATING_SIGNATURE_ERR_MSG, e);
        } catch (XMLSignatureException e) {
            log.debug(CREATING_SIGNATURE_ERR_MSG);
            throw new SignatureException(CREATING_SIGNATURE_ERR_MSG, e);
        }

        return message;
    }

    /**
     * Adds the Signature Id to the UseKey sig attribute. The default
     * implementation should return <code>null</code> i.e. no id will be
     * assigned to the signature element.
     *
     * @return the Id which should be set to the Signature
     */
    protected abstract String addUseKeySignatureId(SoapMessage message) throws ParserException;

    /**
     * Creates KeyInfo content.
     *
     * @param message
     * @throws ParserException
     */
    protected abstract Node createKeyInfoContent(SoapMessage message) throws ParserException;

    /**
     * Marshall a jaxbElement into a Document
     *
     * @param jaxbElement
     * @return Document
     */
    protected final Document marshallJaxbElement(Object jaxbElement) throws ParserException {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(true);
        Document result = null;
        try {
            Marshaller marshaller = jaxBContext.createMarshaller();
            result = dbf.newDocumentBuilder().newDocument();
            marshaller.marshal(jaxbElement, result);
        } catch (JAXBException jaxbException) {
            log.debug(MARSHALL_EXCEPTION_ERR_MSG, jaxbException);
            throw new ParserException(MARSHALL_EXCEPTION_ERR_MSG, jaxbException);
        } catch (ParserConfigurationException pce) {
            log.debug(MARSHALL_EXCEPTION_ERR_MSG, pce);
            throw new ParserException(MARSHALL_EXCEPTION_ERR_MSG, pce);
        }

        return result;
    }

    /**
     * Creates 'wsu:Id' SOAP body attribute needed for signature.
     *
     * @param message
     * @return
     * @throws ParserException
     */
    private String createSoapBodyUuid(SoapMessage message) throws ParserException {

        String bodyId = Util.randomNCNameUUID();
        try {
            message.getMessage().getSOAPBody()
                    .addAttribute(new QName(WSU_NAMESPACE, WSU_ID_LOCAL_NAME, WSU_PREFIX), bodyId);
        } catch (SOAPException e) {
            log.debug(PARSING_XML_ERROR_MSG);
            throw new ParserException(PARSING_XML_ERROR_MSG, e);
        }

        log.debug("Created wsu:Id for SOAPBody: " + bodyId);

        return bodyId;
    }

    /**
     * Creates 'wsu:Id' attribute for wsu:Timestamp needed for signature.
     *
     * @param message
     * @return
     * @throws ParserException
     */
    private String createTimestampUuid(SoapMessage message) throws ParserException {

        NodeList timestampList = message.getHeader().getOwnerDocument()
                .getElementsByTagNameNS(WSU_NAMESPACE, WSU_TIMESTAMP_LOCAL_NAME);

        assert timestampList.getLength() <= 1;

        if (timestampList.getLength() == 1) {
            assert timestampList.item(0).getNodeType() == Node.ELEMENT_NODE;

            Element timestamp = (Element) timestampList.item(0);
            String timestampId = Util.randomNCNameUUID();

            Attr wsuId = timestamp.getOwnerDocument().createAttributeNS(WSU_NAMESPACE, WSU_ID_LOCAL_NAME);
            wsuId.setPrefix(timestamp.getPrefix());

            wsuId.setValue(timestampId);
            timestamp.setAttributeNodeNS(wsuId);
            timestamp.setIdAttributeNode(wsuId, true);

            log.trace("Created wsu:Id for wsu:Timestamp: " + timestampId);

            return timestampId;
        }

        log.trace("Timestamp element not found in the message");

        return null;
    }

    /**
     * Creates all references needed for this signature
     *
     * @param xmlSigFactory
     * @param referenceIdList
     * @return
     * @throws NoSuchAlgorithmException
     * @throws InvalidAlgorithmParameterException
     */
    private List<Reference> createSignatureReferences(XMLSignatureFactory xmlSigFactory, List<String> referenceIdList)
            throws NoSuchAlgorithmException, InvalidAlgorithmParameterException {

        List<Reference> result = new ArrayList<Reference>();

        for (String refId : referenceIdList) {
            if (refId == null) {
                continue;
            }

            Reference ref = xmlSigFactory.newReference("#" + refId, xmlSigFactory.newDigestMethod(DigestMethod.SHA512,
                    null), Collections.singletonList(xmlSigFactory.newCanonicalizationMethod(
                    CanonicalizationMethod.EXCLUSIVE, (C14NMethodParameterSpec) null)), null, null);

            result.add(ref);
        }

        return Collections.unmodifiableList(result);
    }

    private static JAXBContext createJAXBContext() {
        JAXBContext jaxbContext;
        try {
            jaxbContext = JAXBContext.newInstance(WSSE_JAXB_PACKAGE + ":" + WST_JAXB_PACKAGE);
            return jaxbContext;
        } catch (JAXBException jaxbException) {
            log.debug(MARSHALL_EXCEPTION_ERR_MSG, jaxbException);
            throw new IllegalStateException(MARSHALL_EXCEPTION_ERR_MSG, jaxbException);
        }
    }
}
