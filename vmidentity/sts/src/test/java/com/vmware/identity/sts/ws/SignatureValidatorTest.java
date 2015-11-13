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

package com.vmware.identity.sts.ws;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.HashSet;
import java.util.Set;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import junit.framework.Assert;

import org.junit.Test;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

public class SignatureValidatorTest {

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(SignatureValidatorTest.class);
   private static final String REQUEST_DIR = "/request/";
   private static final String WSSE_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0";
   private static final String WSU_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0";
   private static final String XMLDSIG_PACKAGE = "org.w3._2000._09.xmldsig_";
   private static final String SAML_PACKAGE = "oasis.names.tc.saml._2_0.assertion";
   private static final String VALID_REQUEST_BST = "valid_bst.xml";
   private static final String VALID_REQUEST2_BST = "valid_bst2.xml";
   private static final String VALID_REQUEST_ASSERTION = "valid_assertion.xml";

   private final SignatureValidator validator = new SignatureValidator();
   private final JAXBContext jaxbCtx;

   public SignatureValidatorTest() throws JAXBException {
      jaxbCtx = JAXBContext.newInstance(WSSE_PACKAGE + ":" + WSU_PACKAGE + ":"
         + XMLDSIG_PACKAGE + ":" + SAML_PACKAGE);
   }

   @Test
   public void validSignatureBST() throws Exception {
      validSignatureBSTInt(VALID_REQUEST_BST);
      validSignatureBSTInt(VALID_REQUEST2_BST);
   }

   private void validSignatureBSTInt(String requestFileName) throws Exception {
      Node securityHeader = getSecurityHeader(requestFileName);
      validator.validate(securityHeader, parseSecurityHeader(securityHeader));
   }

   @Test
   public void validSignatureAssertion() throws Exception {
      Node securityHeader = getSecurityHeader(VALID_REQUEST_ASSERTION);
      validator.validate(securityHeader, parseSecurityHeader(securityHeader));
   }

   @Test
   public void testInvalid() throws Exception {
      Set<String> invalidFileNames = getInvalidFileNames();
      logger.info("Got {} invalid tests.", invalidFileNames.size());
      for (String filename : invalidFileNames) {
         logger.info("Loading test input {}", filename);
         Node securityHeader = getSecurityHeader(filename);
         boolean gotException = false;
         try {
            validator.validate(securityHeader,
               parseSecurityHeader(securityHeader));
         } catch (WSFaultException e) {
            logger.info("Got fault key {}", e.getFaultKey().toString());
            gotException = true;
         }
         if (!gotException) {
            Assert.fail("Expected exception not thrown");
         }
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNullSoapHeader() throws Exception {
      Node securityHeader = getSecurityHeader(VALID_REQUEST_BST);
      validator.validate(null, parseSecurityHeader(securityHeader));
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNullSignatureElement() throws Exception {
      Node securityHeader = getSecurityHeader(VALID_REQUEST_BST);
      validator.validate(securityHeader, null);
   }

   private Set<String> getInvalidFileNames() {
      Set<String> result = new HashSet<String>();

      File requestDirectory = new File(SignatureValidatorTest.class
         .getResource(REQUEST_DIR).getFile());
      for (String fileName : requestDirectory.list()) {
         if (fileName.contains("invalid")) {
            result.add(fileName);
         }
      }

      return result;
   }

   private SecurityHeaderType parseSecurityHeader(Node element)
      throws Exception {
      @SuppressWarnings("unchecked")
      JAXBElement<SecurityHeaderType> parsedHeader = (JAXBElement<SecurityHeaderType>) jaxbCtx
         .createUnmarshaller().unmarshal(element);
      return parsedHeader.getValue();
   }

   private Node getSecurityHeader(String filename) throws Exception {
      return getRequestElement(filename).getElementsByTagNameNS(
         WsConstants.WSSE_NS, WsConstants.WSSE_SECURITY_ELEMENT_NAME).item(0);
   }

   /**
    * Loads a file into string
    *
    * @param fileName
    * @return String content of the file
    * @throws IOException
    */
   private static String loadStreamContent(InputStream stream)
      throws IOException {
      StringBuilder content = new StringBuilder();

      BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
      try {
         char[] buff = new char[1024];
         int i = 0;
         while ((i = reader.read(buff)) != -1) {
            content.append(buff, 0, i);
         }
      } finally {
         reader.close();
      }

      return content.toString();
   }

   /**
    * Load a valid request DOM element.
    *
    * @return
    */
   private Element getRequestElement(String filename) throws Exception {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      dbf.setNamespaceAware(true);
      // TODO define a profiled version of SOAP-ENV schema and specify it here
      // to fix reported errors from XML schema validation, which by default
      // result in error logs
      SchemaFactory sf = SchemaFactory
         .newInstance(javax.xml.XMLConstants.W3C_XML_SCHEMA_NS_URI);
      InputStream resourceAsStream = SignatureValidatorTest.class
         .getClassLoader().getResourceAsStream(
            "WEB-INF/wsdl/sts_xsd/oasis-200401-wss-wssecurity-utility-1.0.xsd");
      assert resourceAsStream != null;

      try {
         Schema schemaStream = sf.newSchema(new StreamSource(resourceAsStream));
         dbf.setSchema(schemaStream);
      } finally {
         resourceAsStream.close();
      }
      DocumentBuilder docBuilder = dbf.newDocumentBuilder();

      String requestAsString = loadStreamContent(SignatureValidatorTest.class
         .getResourceAsStream(REQUEST_DIR + filename));
      InputSource src = new InputSource(new StringReader(requestAsString));
      return docBuilder.parse(src).getDocumentElement();
   }
}
