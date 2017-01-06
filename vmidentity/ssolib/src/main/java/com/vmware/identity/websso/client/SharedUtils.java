/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

import javax.servlet.http.HttpServletResponse;
import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.Response;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.util.Base64;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * @author root
 *
 */
public class SharedUtils {
    private static Logger logger = LoggerFactory.getLogger(SharedUtils.class);

    private static final String DISALLOW_DOCTYPE_DECL = "http://apache.org/xml/features/disallow-doctype-decl";
    private static final String EXTERNAL_GENERAL_ENTITIES = "http://xml.org/sax/features/external-general-entities";
    private static final String EXTERNAL_PARAMETER_ENTITIES = "http://xml.org/sax/features/external-parameter-entities";
    private static final String LOAD_EXTERNAL_DTD = "http://apache.org/xml/features/nonvalidating/load-external-dtd";

    public static final String CACHE_CONTROL_HEADER = "Cache-Control";
    public static final String PRAGMA = "Pragma";
    /**
     * Create Dom from string.
     */
    public static Document createDOM(String strXML) throws ParserConfigurationException, SAXException, IOException {

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setValidating(false);
        dbf.setIgnoringComments(false);
        dbf.setIgnoringElementContentWhitespace(true);
        dbf.setNamespaceAware(true);

        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        dbf.setFeature(DISALLOW_DOCTYPE_DECL, true);
        dbf.setFeature(EXTERNAL_GENERAL_ENTITIES, false);
        dbf.setFeature(EXTERNAL_PARAMETER_ENTITIES, false);
        dbf.setFeature(LOAD_EXTERNAL_DTD, false);

        DocumentBuilder db = null;
        db = dbf.newDocumentBuilder();
        db.setEntityResolver(new NullResolver());
        db.setErrorHandler(new SamlParserErrorHandler());

        InputSource sourceXML = new InputSource(new StringReader(strXML));
        Document xmlDoc = db.parse(sourceXML);
        return xmlDoc;
    }

    /**
     * Create Dom from iostream.
     */
    public static Document createDOM(InputStream is) throws ParserConfigurationException, SAXException, IOException {

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

        dbf.setValidating(false);
        dbf.setIgnoringComments(false);
        dbf.setIgnoringElementContentWhitespace(true);
        dbf.setNamespaceAware(true);

        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        dbf.setFeature(DISALLOW_DOCTYPE_DECL, true);
        dbf.setFeature(EXTERNAL_GENERAL_ENTITIES, false);
        dbf.setFeature(EXTERNAL_PARAMETER_ENTITIES, false);
        dbf.setFeature(LOAD_EXTERNAL_DTD, false);

        DocumentBuilder db = null;
        db = dbf.newDocumentBuilder();
        db.setEntityResolver(new NullResolver());
        db.setErrorHandler(new SamlParserErrorHandler());

        return db.parse(is);
    }

    public static void formattedPrint(Node xml, OutputStream out) throws TransformerConfigurationException,
            TransformerFactoryConfigurationError, TransformerException, UnsupportedEncodingException {
        TransformerFactory tFactory = TransformerFactory.newInstance();
        // tFactory.setAttribute("indent-number", 4);
        Transformer tf = tFactory.newTransformer();
        tf.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
        tf.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
        tf.setOutputProperty(OutputKeys.INDENT, "yes");
        tf.setOutputProperty(OutputKeys.METHOD, "xml");
        tf.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "5");
        StreamResult result = new StreamResult(new OutputStreamWriter(out, "UTF-8"));
        tf.transform(new DOMSource(xml), result);
    }

    /**
     * Base64 encode byte array
     *
     * @param bytesToEncode
     * @return
     */
    public static String encodeBytes(byte[] bytesToEncode) {
        String retval = Base64.encodeBytes(bytesToEncode, Base64.DONT_BREAK_LINES);
        return retval;
    }

    /**
     * Encode a string
     *
     * @param stringToEncode
     * @return
     */
    public static String encodeString(String stringToEncode) throws UnsupportedEncodingException {
        return encodeBytes(stringToEncode.getBytes("UTF-8"));
    }

    public static void logMessage(Exception err) {
        StackTraceElement[] elements = err.getStackTrace();
        String className = elements[2].getClassName();
        String methodName = elements[2].getMethodName();
        int lineNumber = elements[2].getLineNumber();
        logger.error(className + "." + methodName + "() at line: " + lineNumber + ": " + err.getCause() + " "
                + err.getMessage());
    }

    /**
     * Log the url
     *
     * @param sbRequestUrl
     * @param authnRequest
     * @param relayStateParameter
     * @param signatureAlgorithm
     * @param signature
     * @param extra
     * @throws MarshallingException
     * @throws IOException
     */
    @SuppressWarnings("deprecation")
    public static void logUrl(Logger log, StringBuffer sbRequestUrl, SignableSAMLObject samlObject,
            String relayStateParameter, String signatureAlgorithm, String signature, String extra)
            throws MarshallingException, IOException {
        String samlParameterName = SamlUtils.SAML_REQUEST_PARAMETER;
        boolean doCompress = true; // the value of doCompress may need to
                                   // associate with binding type if we support
                                   // soap binding in future.
        if (samlObject instanceof LogoutResponse || samlObject instanceof Response) {
            samlParameterName = SamlUtils.SAML_RESPONSE_PARAMETER;
            if (samlObject instanceof Response) {
                doCompress = false;
            }
        }
        log.info("We are going to GET URL "
                + sbRequestUrl.toString()
                + (samlObject != null ? "?" + samlParameterName + "="
                        + URLEncoder.encode(SamlUtils.encodeSAMLObject(samlObject, doCompress)) : "")
                + (relayStateParameter != null ? ("&RelayState=" + URLEncoder.encode(relayStateParameter)) : "")
                + (signatureAlgorithm != null ? ("&SigAlg=" + URLEncoder.encode(signatureAlgorithm)) : "")
                + (signature != null ? ("&Signature=" + URLEncoder.encode(signature)) : "")
                + (extra != null ? "&" + extra : ""));
    }

    public static void SetNoCacheHeader(HttpServletResponse httpResponse) {
        httpResponse.addHeader(CACHE_CONTROL_HEADER, "no-store");
        httpResponse.addHeader(PRAGMA, "no-cache");
    }

}

class NullResolver implements EntityResolver {

    @Override
    public InputSource resolveEntity(String publicId, String systemId) throws SAXException, IOException {
        return new InputSource(new StringReader(""));
    }
}
