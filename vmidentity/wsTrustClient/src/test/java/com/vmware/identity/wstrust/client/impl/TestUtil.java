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

import java.io.StringWriter;

import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.soap.MessageFactory;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPMessage;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;
import com.vmware.identity.wstrust.client.impl.ParserException;

/**
 * Utility class for containing common test helper methods
 */
public class TestUtil {

    public static final String TEST_KEYSTORE_FILENAME = "sso_test.jks";
    public static final String TEST_KEYSTORE_KEY = "vmware";
    public static final String TEST_KEYSTORE_CERT_ALIAS = "vmware";
    public static final String TEST_KEYSTORE_PRIV_KEY_PASSWORD = "vmware";

    /**
     * Loads the default keystore for the test cases
     *
     * @return KeyStoreData
     * @throws SsoKeyStoreOperationException
     */
    public static KeyStoreData loadDefaultKeystore() throws SsoKeyStoreOperationException {

        return new KeyStoreData(TestUtil.class.getResource("/" + TestUtil.TEST_KEYSTORE_FILENAME).getFile(),
                TestUtil.TEST_KEYSTORE_KEY.toCharArray(), TestUtil.TEST_KEYSTORE_CERT_ALIAS);
    }

    /**
     * Creates new SOAP message.
     *
     * @return
     * @throws ParserException
     */
    public static SoapMessage createSoapMessage(String soapAction) throws ParserException {
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            Document doc = dbf.newDocumentBuilder().newDocument();
            Node body = doc.createElement("body");
            Node header = doc.createElement("header");
            return new SoapMessage(body, header, soapAction);
        } catch (DOMException e) {
            throw new ParserException("Error creating SOAP message", e);
        } catch (ParserConfigurationException e) {
            throw new ParserException("Error creating SOAP message", e);
        }
    }

    public static SoapMessage createSoapMessage(String contextId, String soapAction) throws ParserException {
        SOAPMessage message;
        try {
            message = MessageFactory.newInstance().createMessage();
            message.getSOAPBody().addChildElement(new QName("Context"));
            message.getSOAPBody().getFirstChild().setTextContent(contextId);
            return new SoapMessage(message, soapAction);
        } catch (SOAPException e) {
            throw new ParserException("Error creating SOAP message", e);
        } catch (SoapFaultException e) {
            throw new ParserException("Error creating SOAP message", e);
        }
    }

    /**
     * Serializes DOM Node to its String representation.
     *
     * <p>
     * The XML declaration <?xml ... > is omitted for easier
     * embedding/serialization.
     *
     * @param content
     *            , required
     * @return XML as string without XML declaration
     * @throws TransformerException
     */
    public static String serializeToString(Node content) throws ParserException {
        TransformerFactory tf = TransformerFactory.newInstance();
        StringWriter writer = new StringWriter();
        try {
            Transformer trans = tf.newTransformer();
            trans.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            // no need to set encoding here
            trans.transform(new DOMSource(content), new StreamResult(writer));
        } catch (TransformerException e) {
            throw new ParserException("Error while serializing Node to String", e);
        }

        return writer.toString();
    }
}
