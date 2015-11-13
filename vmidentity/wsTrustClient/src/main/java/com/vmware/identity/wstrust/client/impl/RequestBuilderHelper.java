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

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;
import java.util.concurrent.TimeUnit;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.binary.StringUtils;
import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.AttributedDateTime;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.TimestampType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.vim.sso.client.SamlToken;

/**
 * Abstract implementation of {@link RequestBuilder}. Contains various common
 * methods needed by its subclasses.
 */
class RequestBuilderHelper {

    public static final String TOKEN_TYPE_SAML2 = com.vmware.vim.sso.client.Constants.SAML_NAMESPACE;

    private static final String XML_DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'";
    private static final Logger log = LoggerFactory.getLogger(RequestBuilderHelper.class);
    private static final String PROCESS_RST_ERROR = "Error processing request for security token";

    private final int requestValidityInSeconds;
    private final JAXBContext jaxbContext;
    private final String soapAction;

    public RequestBuilderHelper(JAXBContext jaxbContext, int requestValidityInSeconds, String soapAction) {
        assert jaxbContext != null;
        assert soapAction != null;

        this.jaxbContext = jaxbContext;
        this.requestValidityInSeconds = requestValidityInSeconds;
        this.soapAction = soapAction;
    }

    public int getRequestValidityInSeconds() {
        return requestValidityInSeconds;
    }

    /**
     * Create and return the JAXB representation of a <wst:BinaryExchange>
     * element, populated with the base64-encoded form of the given binary data.
     * This is a helper method to be used by this class' concrete descendants.
     *
     * @param wstFactory
     *            The factory to use for JAXB object creation.
     * @param data
     *            The data to populate the created element with.
     * @return The object representation of a <wst:BinaryExchange> element. The
     *         created element will always have encoding type = base64 and value
     *         type = spnego.
     */
    protected final BinaryExchangeType createBinaryExchangeElement(ObjectFactory wstFactory, byte[] data) {

        BinaryExchangeType xchg = wstFactory.createBinaryExchangeType();

        xchg.setEncodingType(Constants.ENCODING_TYPE_BASE64);
        xchg.setValueType(Constants.BINARY_EXCHANGE_TYPE_SPNEGO);
        xchg.setValue(StringUtils.newStringUtf8(Base64.encodeBase64(data, false /*
                                                                                 * non
                                                                                 * -
                                                                                 * chunked
                                                                                 */)));

        return xchg;
    }

    /**
     * Inserts SAML token into the SOAP message. Due to problems with JAXB
     * marshaling/unmarshaling SAML tokens should be added to the message after
     * it is marshalled to DOM. There should be only 1
     * <elementNamespace:elementLocalName /> node in the request.
     *
     * @param message
     *            The request SoapMessage
     * @param elementNamespace
     *            The namespace of the element where the SAML token should be
     *            inserted
     * @param elementLocalName
     *            The local name of the element where the SAML token should be
     *            inserted
     * @param token
     *            The SAML token that should be inserted
     * @throws ParserException
     */
    protected final void insertSamlToken(SoapMessage message, String elementNamespace, String elementLocalName,
            SamlToken token) throws ParserException {

        Document messageDocument = message.getMessage().getSOAPPart();
        NodeList targetElement = messageDocument.getElementsByTagNameNS(elementNamespace, elementLocalName);
        if (targetElement == null || targetElement.getLength() != 1) {
            String errMsg = "Error inserting SAML token into the SOAP message. " + "No/Too many " + elementLocalName
                    + " found.";
            log.debug(errMsg);

            throw new ParserException(errMsg);
        }

        // only 1 target element is expected in the STS request
        Node copiedToken = token.importTo(messageDocument);
        targetElement.item(0).appendChild(copiedToken);
    }

    /**
     * Creates lifetime element. It is used to restrict token lifetime.
     *
     * @param tokenLifetime
     *            token lifetime period in seconds from now; positive number is
     *            required
     * @return Lifetime element
     */
    protected final LifetimeType createLifetimeElement(long tokenLifetime) {
        assert tokenLifetime > 0 : tokenLifetime;

        ObjectFactory wstFactory = new ObjectFactory();
        LifetimeType lifetime = wstFactory.createLifetimeType();

        org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.ObjectFactory wssuObjFactory = new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.ObjectFactory();
        AttributedDateTime created = wssuObjFactory.createAttributedDateTime();

        DateFormat wssDateFormat = createDateFormatter();
        final long now = System.currentTimeMillis();
        String startDate = wssDateFormat.format(new Date(now));
        created.setValue(startDate);

        AttributedDateTime expires = wssuObjFactory.createAttributedDateTime();
        String endDate = wssDateFormat.format(new Date(now + TimeUnit.SECONDS.toMillis(tokenLifetime)));
        expires.setValue(endDate);

        lifetime.setCreated(created);
        lifetime.setExpires(expires);

        if (log.isDebugEnabled()) {
            log.debug("Lifitime created with following values -- Create date: " + startDate + " Expire date: "
                    + endDate);
        }

        return lifetime;
    }

    /**
     * Wrap the request/header into a SOAP envelope
     *
     * @param request
     * @param secHeader
     * @return the result of the SOAP method call
     * @throws ParserException
     * @throws TransportException
     * @throws InvalidCredentialException
     */
    SoapMessage wrapToSoapMessage(Object request, SecurityHeaderType secHeader) throws ParserException {

        Node body = marshallJaxbElement(request).getDocumentElement();
        Node header = marshallJaxbElement(secHeader).getDocumentElement();

        SoapMessage message = new SoapMessage(body, header, soapAction);

        return message;
    }

    /**
     * Marshall a jaxbElement into a Document
     *
     * @param jaxbElement
     * @return Document
     */
    private Document marshallJaxbElement(Object jaxbElement) throws ParserException {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(true);
        Document result = null;
        try {
            result = dbf.newDocumentBuilder().newDocument();
            jaxbContext.createMarshaller().marshal(jaxbElement, result);
        } catch (JAXBException jaxbException) {
            log.debug(PROCESS_RST_ERROR, jaxbException);
            throw new ParserException(PROCESS_RST_ERROR, jaxbException);
        } catch (ParserConfigurationException pce) {
            log.debug(PROCESS_RST_ERROR, pce);
            throw new ParserException(PROCESS_RST_ERROR, pce);
        }

        return result;
    }

    /**
     * Create a WS-Security header. It should be included in all requests.
     *
     * @return created header with timestamp included
     */
    SecurityHeaderType createSecurityHeader() {
        org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory wsseObjFactory = new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory();

        SecurityHeaderType secHeader = wsseObjFactory.createSecurityHeaderType();
        secHeader.getAny().add(createTimestamp());

        log.debug("Security header successfully created");

        return secHeader;
    }

    /**
     * Creates a timestamp WS-Security element. It is needed for the STS to tell
     * if the request is invalid due to slow delivery
     *
     * @return timestamp element issued with start date = NOW and expiration
     *         date = NOW + REQUEST_VALIDITY_IN_MINUTES
     */
    private TimestampType createTimestamp() {
        org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.ObjectFactory wssuObjFactory = new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.ObjectFactory();

        TimestampType timestamp = wssuObjFactory.createTimestampType();

        final long now = System.currentTimeMillis();
        Date createDate = new Date(now);
        Date expirationDate = new Date(now + TimeUnit.SECONDS.toMillis(getRequestValidityInSeconds()));

        DateFormat wssDateFormat = createDateFormatter();
        AttributedDateTime createTime = wssuObjFactory.createAttributedDateTime();
        createTime.setValue(wssDateFormat.format(createDate));

        AttributedDateTime expirationTime = wssuObjFactory.createAttributedDateTime();
        expirationTime.setValue(wssDateFormat.format(expirationDate));

        timestamp.setCreated(createTime);
        timestamp.setExpires(expirationTime);

        if (log.isDebugEnabled()) {
            log.debug("Timestamp created with following values -- Create date: " + createTime.getValue()
                    + " Expire date: " + expirationTime.getValue());
        }

        return timestamp;
    }

    /**
     * Creates a datetime formatter needed for populating objects containing XML
     * requests/responses.
     */
    private static DateFormat createDateFormatter() {
        DateFormat dateFormat = new SimpleDateFormat(XML_DATE_FORMAT);

        // always send UTC/GMT time
        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));

        return dateFormat;
    }

}
