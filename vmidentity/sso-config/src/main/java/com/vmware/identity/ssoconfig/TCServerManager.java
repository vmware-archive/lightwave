/**
 *
 * Copyright 2014 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.ssoconfig;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * Simple wrapper to change the client certificate authentication related attributes in TC server.xml
 * @author qiangw
 *
 */
public final class TCServerManager {
    public static String connectorNodeName = "Connector";
    // used to find the right connector node in server.xml to configure
    public static String connectorPortAttrName = "port";
    public static String connectorPortAttrValue = "${bio-ssl-localhost.https.port}";

    // the client authentication attributes to change to enable client
    // certificate authentication
    public static String clientAuthAttrName = "clientAuth";
    public static String clientAuthAttributeCertFalse = "false";

    // the trustStore attributes to change
    public static String trustStoreFileAttrName = "truststoreFile";
    public static String trustStorePasswordAttrName = "truststorePass";
    public static String trustStoreTypeAttrName = "truststoreType";

    private String _serverConfigFile;
    private Document _serverConfigDoc;
    private Node _connector;

    /**
     * ctor using the TC server.xml path.
     * @param path
     * @throws ParserConfigurationException
     * @throws SAXException
     * @throws IOException
     */
    public TCServerManager(String path) throws ParserConfigurationException, SAXException, IOException {
        this._serverConfigFile = path;
        readFromXmlFile();
    }

    /**
     * Get the HTTPS connector attribute value.
     * @param attrName
     * @return
     */
    public String getAttrValue(String attrName) {
        Node n = this._connector.getAttributes().getNamedItem(attrName);
        if (n != null)
            return n.getNodeValue();

        return null;
    }

    /**
     * Set the HTTPS connector attribute value.
     * @param name
     * @param value
     */
    public void setAttrValue(String name, String value) {
        if (name != null) {
            Node foundAttr = this._connector.getAttributes().getNamedItem(name);
            if (foundAttr != null) {
                if (value != null) {
                    foundAttr.setNodeValue(value);
                } else {
                    this._connector.getAttributes().removeNamedItem(name);
                }
            } else if (value != null) {
                Attr attr = this._serverConfigDoc.createAttribute(name);
                attr.setNodeValue(value);
                this._connector.getAttributes().setNamedItem(attr);
            }
        }
    }

    /**
     * Save the changed settings to server.xml file.
     * @throws TransformerFactoryConfigurationError
     * @throws TransformerException
     */
    public void saveToXmlFile() throws TransformerFactoryConfigurationError,
            TransformerException {
        Transformer transformer = TransformerFactory.newInstance()
                .newTransformer();
        DOMSource source = new DOMSource(this._serverConfigDoc);
        StreamResult targetStreamResult = new StreamResult(
                this._serverConfigFile);
        transformer.setOutputProperty(OutputKeys.INDENT, "yes");
        transformer.transform(source, targetStreamResult);
    }

    private void readFromXmlFile() throws ParserConfigurationException,
            SAXException, IOException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder docBuilder;
        docBuilder = factory.newDocumentBuilder();
        this._serverConfigDoc = docBuilder.parse(new File(
                this._serverConfigFile));
        this._connector = findConnectorNode();
    }

    private Node findConnectorNode() {
        NodeList list = this._serverConfigDoc
                .getElementsByTagName(connectorNodeName);
        for (int i = 0; i < list.getLength(); i++) {
            Node subnode = list.item(i);
            NamedNodeMap attrs = subnode.getAttributes();
            Node node = attrs.getNamedItem(connectorPortAttrName);
            if (node.getNodeValue().equals(connectorPortAttrValue)) {
                return subnode;
            }
        }
        return null;
    }
}