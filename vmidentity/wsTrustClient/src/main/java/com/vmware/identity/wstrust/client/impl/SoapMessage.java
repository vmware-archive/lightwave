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

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.soap.MessageFactory;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPMessage;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

/**
 * This class encapsulates all SOAP message creation/parsing
 */
class SoapMessage {

    private final Node body;
    private final Node header;
    private final SOAPMessage message;
    private final String action;

    private static final String CREATION_ERROR_MSG = "Error creating SOAP message";
    private static final String PARSING_ERROR_MSG = "Error parsing SOAP message";
    private static final String SOAP_FAULT_FOUND = "SOAP fault found in the message";

    private final Logger log = LoggerFactory.getLogger(SoapMessage.class);

    /**
     * Creates new SOAP message including adopted body and header as single SOAP
     * body and header elements
     *
     * @param body
     *            - Cannot be null.
     * @param header
     *            - Cannot be null.
     * @param soapAction
     *            - Cannot be null.
     * @throws ParserException
     */
    public SoapMessage(Node body, Node header, String soapAction) throws ParserException {
        assert body != null;
        assert header != null;
        assert soapAction != null;

        log.debug("Creating SoapMessage from body and header");
        this.message = createMessage(body, header);
        this.action = soapAction;
        try {
            this.body = message.getSOAPBody();
            this.header = message.getSOAPHeader();
        } catch (SOAPException e) {
            throw new ParserException("Cannot init header and body", e);
        }
    }

    /**
     * Parses SOAP message. In the process single body and single header
     * elements are extracted.
     *
     * @param message
     *            - Cannot be null
     * @param soapAction
     *            - Cannot be null
     * @throws ParserException
     */
    public SoapMessage(SOAPMessage message, String soapAction) throws ParserException, SoapFaultException {
        assert message != null;
        assert soapAction != null;

        log.debug("Creating SoapMessage from SOAPMessage");
        parseForSOAPFault(message);
        this.message = message;
        this.body = parseBody(message);
        this.header = null;
        this.action = soapAction;
    }

    /**
     * Get the body of the SoapMessage
     *
     * @return body element
     */
    public Node getBody() {
        return body;
    }

    /**
     * Get the header of the SoapMessage. Might be null.
     *
     * @return header element
     */
    public Node getHeader() {
        return header;
    }

    /**
     * Get the soap message
     *
     * @return soap message
     */
    public SOAPMessage getMessage() {
        return message;
    }

    /**
     * Get the soap action of the SoapMessage
     *
     * @return soap action URI. Cannot be null
     */
    public String getSoapAction() {
        return action;
    }

    /**
     * Creates new SOAP message by adopting the passed body and header elements.
     *
     * @param body
     * @param header
     * @return
     * @throws ParserException
     */
    private SOAPMessage createMessage(Node body, Node header) throws ParserException {

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(true);
        SOAPMessage message = null;
        try {
            message = MessageFactory.newInstance().createMessage();
            Document adoptedBody = dbf.newDocumentBuilder().newDocument();
            adoptedBody.appendChild(adoptedBody.importNode(body, true));
            message.getSOAPBody().addDocument(adoptedBody);

            Node adoptedHeader = message.getSOAPPart().importNode(header, true);
            message.getSOAPHeader().appendChild(adoptedHeader);
        } catch (ParserConfigurationException e) {
            log.error(CREATION_ERROR_MSG, e);
            throw new ParserException(CREATION_ERROR_MSG, e);
        } catch (SOAPException e) {
            log.error(CREATION_ERROR_MSG, e);
            throw new ParserException(CREATION_ERROR_MSG, e);
        }

        return message;
    }

    /**
     * Extracts the single body node of the passed SOAPMessage
     *
     * @param message
     * @return
     * @throws ParserException
     */
    private Node parseBody(SOAPMessage message) throws ParserException {
        try {
            return message.getSOAPBody().getFirstChild();
        } catch (SOAPException e) {
            log.error(PARSING_ERROR_MSG, e);
            throw new ParserException(PARSING_ERROR_MSG, e);
        }
    }

    /**
     * Parses the SOAPMessage for SOAPFault if there is such.
     *
     * @param message
     * @return
     * @throws ParserException
     * @throws SoapFaultException
     */
    private void parseForSOAPFault(SOAPMessage message) throws ParserException, SoapFaultException {
        try {
            if (message.getSOAPBody().hasFault()) {
                log.error(SOAP_FAULT_FOUND);
                throw new SoapFaultException(SOAP_FAULT_FOUND, new SoapFault(message.getSOAPBody().getFault()));
            }
        } catch (SOAPException e) {
            log.error(PARSING_ERROR_MSG, e);
            throw new ParserException(PARSING_ERROR_MSG, e);
        }
    }

}
