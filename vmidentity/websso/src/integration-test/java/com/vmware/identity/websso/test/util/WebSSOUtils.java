/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util;

import com.vmware.identity.websso.test.util.common.Assert;
import com.vmware.identity.websso.test.util.common.SAMLConstants;
import com.vmware.identity.websso.test.util.common.SSOConstants;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.security.InvalidKeyException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;
import java.util.UUID;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;
import java.util.zip.InflaterInputStream;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.codec.binary.Base64;
import org.apache.http.conn.ssl.AllowAllHostnameVerifier;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.transform.*;
import javax.xml.transform.dom.*;
import javax.xml.transform.stream.*;

public class WebSSOUtils {

    private static final Logger log = LoggerFactory.getLogger(WebSSOUtils.class);

    /*This method is Suitable for HTTP REDIRECT Binding.
     * With HTTP Redirect Binding, SAML request or response,
     * before its sent, the message is deflated (sans header and checksum),
     * base64-encoded, and URL-encoded, in that order. Upon receipt, the process
     * is reversed to recover the original message.
     */
    public static String decodeInflateSaml(String encodedStr, boolean decodeUrl) {
        String decodedString = null; try {
            // 1). URL decode the string
            String urlDecoded = encodedStr; if (decodeUrl) {
                urlDecoded = URLDecoder.decode(encodedStr, SSOConstants.UTF8_CHARSET);
            }
            // 2). Decode base 64
            Base64 base64Decoder = new Base64();
            byte[] samlBytes = urlDecoded.getBytes(SSOConstants.UTF8_CHARSET);
            byte[] decodedByteArray = base64Decoder.decode(samlBytes);
            // 3). Inflate the data
            decodedString = uncompress(decodedByteArray);
        } catch (UnsupportedEncodingException e) {
            log.error("Cannot URL decode the string - " + encodedStr); e.printStackTrace();
        } return decodedString;
    }

    //method to convert Document to String
    public static String convertDOMtoString(Document doc) throws TransformerException {

        DOMSource domSource = new DOMSource(doc); StringWriter writer = new StringWriter();
        StreamResult result = new StreamResult(writer); TransformerFactory tf = TransformerFactory.newInstance();
        Transformer transformer = tf.newTransformer(); transformer.transform(domSource, result);
        return writer.toString();
    }

    public static byte[] compressStringToBytes(String str) throws UnsupportedEncodingException, IOException {
        Deflater deflater = new Deflater(Deflater.DEFLATED, true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        DeflaterOutputStream deflaterOutputStream = new DeflaterOutputStream(byteArrayOutputStream, deflater);
        deflaterOutputStream.write(str.getBytes("UTF-8")); deflaterOutputStream.close();
        return byteArrayOutputStream.toByteArray();
    }

    public static String signMessage(
        String message, String algorithmName, PrivateKey privateKey
    ) throws NoSuchAlgorithmException, InvalidKeyException, UnsupportedEncodingException, SignatureException {
        Signature sig = Signature.getInstance(algorithmName); sig.initSign(privateKey);
        byte[] messageBytes = message.getBytes("UTF-8"); sig.update(messageBytes);
        byte[] sigBytes = sig.sign();
        byte[] sigBase64Bytes = Base64.encodeBase64(sigBytes, false,  //isChunked=false
                                                                                  true
        ); //isUrlSafe = true

        return new String(sigBase64Bytes);
    }

    public static InputStream fileNameToInputStream(String fileName) throws FileNotFoundException {
        File f = new File(fileName);
        InputStream in = new FileInputStream(f);
        return in;
    }

    /**
     * decodeSaml - URL decodes and then Base64 decodes.
     *
     * @param encodedStr - String to be decode
     * @param decodeUrl  - If URL decoding required
     * @return the decoded string
     */
    // Suitable for HTTP POST Binding Saml requests/responses
    public static String decodeSaml(String encodedStr, boolean decodeUrl) {
        String decodedString = null;

        try {
            // 1). URL decode the string
            String urlDecoded = encodedStr; if (decodeUrl) {
                urlDecoded = URLDecoder.decode(encodedStr, SSOConstants.UTF8_CHARSET);
            }
            // 2). Decode base 64
            Base64 base64Decoder = new Base64();
            byte[] samlBytes = urlDecoded.getBytes(SSOConstants.UTF8_CHARSET);
            byte[] decodedByteArray = base64Decoder.decode(samlBytes);

            decodedString = new String(decodedByteArray, SSOConstants.UTF8_CHARSET);
        } catch (UnsupportedEncodingException e) {
            log.error("Cannot URL decode the string - " + encodedStr); e.printStackTrace();
        } return decodedString;
    }

    private static String uncompress(byte[] decodedByteArray) {
        String decodedString = null; try {
            try {
                Inflater inflater = new Inflater(true); inflater.setInput(decodedByteArray);
                byte[] xmlMessageBytes = new byte[SSOConstants.INFLATE_SIZE];
                int resultLength = inflater.inflate(xmlMessageBytes);
                Assert.assertFalse(inflater.getRemaining() > 0,
                                   "didn't allocate enough space to hold the decompressed data"
                );
                inflater.end();
                decodedString = new String(xmlMessageBytes, 0, resultLength, SSOConstants.UTF8_CHARSET);
            } catch (DataFormatException e) {
                ByteArrayInputStream bais = new ByteArrayInputStream(decodedByteArray);
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                InflaterInputStream iis = new InflaterInputStream(bais);
                byte[] buf = new byte[SSOConstants.BUF_SIZE];
                int count = iis.read(buf); while (count != -1) {
                    baos.write(buf, 0, count); count = iis.read(buf);
                } iis.close(); decodedString = new String(baos.toByteArray());
            }
        } catch (IOException e) {
            System.out.println("Exception: " + e.getMessage()); e.printStackTrace(); decodedString = null;
        } return decodedString;
    }

    public static void getTrustAllSSLConection() throws NoSuchAlgorithmException, KeyManagementException {
        SSLContext sc = SSLContext.getInstance("SSL");
        sc.init(null, new TrustManager[]{new TestX509TrustManager()}, new java.security.SecureRandom());
        HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
        HttpsURLConnection.setDefaultHostnameVerifier(new AllowAllHostnameVerifier());
    }

    public static Document getDocumentFromUrl(String url) {
        DocumentBuilderFactory docBldrFactory = DocumentBuilderFactory.newInstance();
        docBldrFactory.setNamespaceAware(true); docBldrFactory.setValidating(false);
        docBldrFactory.setIgnoringComments(false); docBldrFactory.setIgnoringElementContentWhitespace(true);
        docBldrFactory.setNamespaceAware(true); DocumentBuilder docBuilder = null; Document doc = null;

        try {
            docBuilder = docBldrFactory.newDocumentBuilder(); docBuilder.setEntityResolver(new CustomEntityResolver());

            getTrustAllSSLConection();

            //InputStream in = new FileInputStream(url);
            InputStream in = new URL(url).openStream(); doc = docBuilder.parse(in);
        } catch (ParserConfigurationException e) {
            log.error("Cannot create a Document Buidler: " + e.getMessage()); e.printStackTrace();
        } catch (SAXException e) {
            log.error("Failed to parse the XML from - " + url + ": " + e.getMessage()); e.printStackTrace();
        } catch (IOException e) {
            log.error("Exception while parsing the xml from url - " + url + ": " + e.getMessage()); e.printStackTrace();
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } return doc;
    }

    public static String generateGUID() {
        UUID uuid = UUID.randomUUID(); StringBuilder sb = new StringBuilder(); sb.append("_");
        String temp = uuid.toString(); sb.append(temp.replace("-", ""));

        return sb.toString();
    }

    public static String convertDateToString(Date date) {
        if (date == null) {
            date = new Date();
        }

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss.SSS");
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        String utcDate = sdf.format(date);
        utcDate = utcDate.replace(" ", "T");
        StringBuilder sb = new StringBuilder();
        sb.append(utcDate);
        sb.append("Z");
        return sb.toString();
    }

    public static Document getDocumentFromFile(File xmlFile) {
        DocumentBuilderFactory docBldrFactory = DocumentBuilderFactory.newInstance();
        docBldrFactory.setNamespaceAware(true);
        docBldrFactory.setValidating(false);
        docBldrFactory.setIgnoringComments(false);
        docBldrFactory.setIgnoringElementContentWhitespace(true);
        docBldrFactory.setNamespaceAware(true);
        DocumentBuilder docBuilder = null;
        Document doc = null;

        try {
            docBuilder = docBldrFactory.newDocumentBuilder();
            docBuilder.setEntityResolver(new CustomEntityResolver());
            InputStream in = new FileInputStream(xmlFile);
            doc = docBuilder.parse(in);
        } catch (ParserConfigurationException e) {
            log.error("Cannot create a Document Buidler: " + e.getMessage()); e.printStackTrace();
        } catch (SAXException e) {
            log.error("Failed to parse the XML from - " + xmlFile.getName() + ": " + e.getMessage());
            e.printStackTrace();
        } catch (IOException e) {
            log.error("Exception while parsing the xml from url - " + xmlFile.getName() + ": " + e.getMessage());
            e.printStackTrace();
        } return doc;
    }

    public static String getEntityID(Document metadataDoc) {
        NamedNodeMap attributeMap =
            metadataDoc.getElementsByTagName(SAMLConstants.ENTITY_DESCRIPTOR).item(0).getAttributes();
        String entityId = attributeMap.getNamedItem(SAMLConstants.ENTITY_ID).getTextContent();

        return entityId;
    }

    static class CustomEntityResolver implements EntityResolver {
        @Override public InputSource resolveEntity(String publicId, String systemId) throws SAXException, IOException {
            return new InputSource(new StringReader(""));
        }
    }
}
