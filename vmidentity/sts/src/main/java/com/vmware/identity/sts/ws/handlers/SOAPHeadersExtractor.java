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
package com.vmware.identity.sts.ws.handlers;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.servlet.ServletRequest;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPHeader;
import javax.xml.soap.SOAPMessage;
import javax.xml.transform.dom.DOMSource;
import javax.xml.validation.Schema;
import javax.xml.validation.Validator;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.soap.SOAPMessageContext;

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3._2005._08.addressing.AttributedURIType;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.ws.SOAPFaultHandler;
import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;
import com.vmware.identity.sts.ws.WSFaultException;
import com.vmware.identity.sts.ws.WsConstants;
import com.vmware.identity.token.impl.Util;

/**
 * This class obtains the meaningful data from the SOAP header, specifically DOM
 * of the entire request and also SecurityHeader and puts them in the request as
 * parameters. Before extracting the Security header it is validated against the
 * profiled schema.
 *
 * The implementation is a Java-WS protocol specific handler for SOAP.
 */
public final class SOAPHeadersExtractor implements
   SOAPHandler<SOAPMessageContext> {

   private static final String METRO_JAXB_CONTEXT_IMPL_NAME = "com.sun.xml.bind.v2.runtime.JAXBContextImpl";
   private static final String WSSE_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0";
   private static final String WSU_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0";
   private static final String WSA_PACKAGE = "org.w3._2005._08.addressing";
   private static final String XMLDSIG_PACKAGE = "org.w3._2000._09.xmldsig_";
   private static final String SAML_PACKAGE = "oasis.names.tc.saml._2_0.assertion";

   private static final String SCHEMAS_LOCATION = "../wsdl/sts_xsd/";
   private static final String PROFILED_WS_SECURITY_HEADER_SCHEMA_NAME = "profiled-ws-header.xsd";
   private static final String PROFILED_WS_ADDRESSING_HEADER_SCHEMA_NAME = "ws-addr.xsd";

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(SOAPHeadersExtractor.class);
   private static final QName WSSE_SECURITY_QNAME = new QName(
      WsConstants.WSSE_NS, WsConstants.WSSE_SECURITY_ELEMENT_NAME);
   private static final QName WSA_ACTION_QNAME = new QName(WsConstants.WSA_NS,
      WsConstants.WSA_ACTION_ELEMENT_NAME);

   private static final QName wsuTimestamp = new QName(WsConstants.WSU_NS,
      WsConstants.WSU_TIMESTAMP_ELEMENT_NAME);
   private static final QName wsseUsernameToken = new QName(
      WsConstants.WSSE_NS, WsConstants.WSSE_USERNAME_TOKEN_ELEMENT_NAME);
   private static final QName wssePassword = new QName(WsConstants.WSSE_NS,
      WsConstants.WSSE_PASSWORD_ELEMENT_NAME);
   private static final QName wssePasscode = new QName(WsConstants.WSSE_NS,
           WsConstants.WSSE_PASSCODE_ELEMENT_NAME);
   private static final QName wsseBinarySecurityToken = new QName(
      WsConstants.WSSE_NS, WsConstants.WSSE_BINARY_SECURITY_TOKEN_ELEMENT_NAME);
   private static final QName saml2Assertion = new QName(
      WsConstants.ASSERTION_NS, WsConstants.ASSERTION_ELEMENT_NAME);
   private static final QName dsSignature = new QName(WsConstants.DS_NS,
      WsConstants.DS_SIGNATURE_ELEMENT_NAME);

   private static final JAXBContext jaxbSecurityHeaderCtx;
   private static final JAXBContext jaxbWSAActionCtx;

   private final Schema securityHeaderSchema;
   private final Schema wsaActionSchema;

   static
   {
       jaxbSecurityHeaderCtx = initJaxbContext(WSSE_PACKAGE + ":"
               + WSU_PACKAGE + ":" + XMLDSIG_PACKAGE + ":" + SAML_PACKAGE);
       checkImplementation(jaxbSecurityHeaderCtx);

       jaxbWSAActionCtx = initJaxbContext(WSA_PACKAGE);
   }

   public SOAPHeadersExtractor() {
      this(SCHEMAS_LOCATION, PROFILED_WS_SECURITY_HEADER_SCHEMA_NAME,
         PROFILED_WS_ADDRESSING_HEADER_SCHEMA_NAME);
   }

   public SOAPHeadersExtractor(
      String schemasLocation,
      String wsHeaderSchemaName,
      String wsAddressingSchemaName) {

      this.securityHeaderSchema = loadSchema(schemasLocation, wsHeaderSchemaName);
      this.wsaActionSchema = loadSchema(schemasLocation, wsAddressingSchemaName);
   }

   @Override
   public boolean handleMessage(SOAPMessageContext ctx) {
      final boolean result = true;
      Boolean outbound = (Boolean) ctx
         .get(MessageContext.MESSAGE_OUTBOUND_PROPERTY);
      if (outbound != null && outbound.equals(Boolean.TRUE)) {
         return result;
      }

      storeObjectInRequest(ctx, WsConstants.SECURITY_HEADER_KEY,
         extractSecurityHeader(ctx));

      storeObjectInRequest(ctx, WsConstants.DOM_REQUEST_KEY,
         extractRequestDOM(ctx));

      String wsaAction = extractWSAction(ctx);
      if (wsaAction != null) {
         storeObjectInRequest(ctx, WsConstants.WSA_ACTION_KEY, wsaAction);
      }

      return result;
   }

   @Override
   public boolean handleFault(SOAPMessageContext arg) {
      return true;
   }

   @Override
   public void close(MessageContext arg) {
      // nothing to dispose
   }

   @Override
   public Set<QName> getHeaders() {
      final Set<QName> headers = new HashSet<QName>();
      headers.add(WSSE_SECURITY_QNAME);
      headers.add(WSA_ACTION_QNAME);
      return headers;
   }

   /**
    * Stores needed object in the request for further processing.
    *
    * @param ctx
    *           cannot be null
    * @param header
    *           can be null
    */
   private void storeObjectInRequest(SOAPMessageContext ctx, String key,
      Object value) {
      assert ctx != null;
      assert key != null;
      assert value != null;

      // TODO investigate if MessageContext can be used
      ServletRequest request = (ServletRequest) ctx
         .get(MessageContext.SERVLET_REQUEST);
      request.setAttribute(key, value);
   }

   /**
    * This method extracts the WS-Security header from the SOAP header
    *
    * @param ctx
    *           cannot be null
    */
   private SecurityHeaderType extractSecurityHeader(SOAPMessageContext ctx) {
      assert ctx != null;
      final Node secHeaderNode = extractSecurityNode(ctx);
      final SecurityHeaderType header = extractFromSOAPHeader(ctx,
         WSSE_SECURITY_QNAME, jaxbSecurityHeaderCtx, secHeaderNode,
         securityHeaderSchema.newValidator());

      validateWsseSecurity(secHeaderNode);
      return header;
   }

   private String extractWSAction(SOAPMessageContext ctx) {
      assert ctx != null;

      Node wsaActionNode = extractWSAActionNode(ctx);

      return wsaActionNode != null ? this
         .<AttributedURIType> extractFromSOAPHeader(ctx, WSA_ACTION_QNAME,
            jaxbWSAActionCtx, wsaActionNode, wsaActionSchema.newValidator())
         .getValue() : null;
   }

   private <T> T extractFromSOAPHeader(SOAPMessageContext ctx, QName toExtract,
      JAXBContext extractFromCtx, Node nodeToValidate, Validator validator) {
      assert ctx != null;
      assert toExtract != null;
      assert extractFromCtx != null;
      assert nodeToValidate != null;
      assert validator != null;

      validateSOAPHeader(nodeToValidate, validator);

      Object[] headers = ctx.getHeaders(toExtract, extractFromCtx, true);
      logger.info("Found {} {} headers", headers.length, toExtract.toString());

      // this is checked during validation of soap header(for wsa:action and
      // wsse:security)
      assert headers.length == 1;

      @SuppressWarnings("unchecked")
      T extractedObject = ((JAXBElement<T>) headers[0]).getValue();

      return extractedObject;
   }

   /**
    * @param ctx
    *           cannot be null.
    * @return DOM representation of the current request.
    */
   private Document extractRequestDOM(SOAPMessageContext ctx) {
      assert ctx != null;

      return getSOAPMessage(ctx).getSOAPPart();
   }

   private SOAPMessage getSOAPMessage(SOAPMessageContext ctx) {
      assert ctx != null;

      SOAPMessage message = ctx.getMessage();
      if (message == null) {
         throwSoapFault(FaultKey.WSSE_INVALID_SECURITY,
            "SOAP message not found in the request");
      }
      return message;
   }

   private SOAPHeader getSOAPHeader(SOAPMessageContext ctx) {
      assert ctx != null;

      SOAPMessage msg = getSOAPMessage(ctx);
      SOAPHeader soapHeader = null;
      try {
         soapHeader = msg.getSOAPHeader();
      } catch (SOAPException e) {
         throwSoapFault(FaultKey.WSSE_INVALID_SECURITY,
            "SOAPHeader not found in the request");
      }
      assert soapHeader != null;
      return soapHeader;
   }

   private static void throwSoapFault(FaultKey key, String msg) {
      SOAPFaultHandler.throwSoapFault(new WSFaultException(key, msg));
   }

   /**
    * Validates SOAP Header against WS-SEC-EXT schema.
    *
    * @param noteToValidate
    *           cannot be null
    * @param validator
    *           cannot be null
    */
   private void validateSOAPHeader(Node noteToValidate, Validator validator) {
      assert validator != null;
      assert noteToValidate != null;

      DOMSource validationSource = new DOMSource(noteToValidate);
      try {
         validator.validate(validationSource);
      } catch (Exception e) {
         logger.error("SOAP header validation failed: {}", e.getMessage());
         SOAPFaultHandler.throwSoapFault(new WSFaultException(
            FaultKey.WSSE_INVALID_SECURITY, e));
      }
   }

   private Node extractSecurityNode(SOAPMessageContext ctx) {
      assert ctx != null;

      Node securityHeader = extractSingleHeader(ctx, WsConstants.WSSE_NS,
         WsConstants.WSSE_SECURITY_ELEMENT_NAME);

      if (securityHeader == null) {
         throwSoapFault(FaultKey.WSSE_INVALID_SECURITY,
            "There is no Security element in the soap header");
      }

      return securityHeader;
   }

   private Node extractWSAActionNode(SOAPMessageContext ctx) {
      assert ctx != null;

      return extractSingleHeader(ctx, WsConstants.WSA_NS,
         WsConstants.WSA_ACTION_ELEMENT_NAME);
   }

   private Node extractSingleHeader(SOAPMessageContext ctx,
      String namespaceURI, String elementName) {
      assert ctx != null;
      assert namespaceURI != null;
      assert elementName != null;

      logger.debug("Searching for {} in {} namespace.", elementName,
         namespaceURI);

      NodeList elements = getSOAPHeader(ctx).getElementsByTagNameNS(
         namespaceURI, elementName);

      Node elementNode = null;
      if (elements.getLength() != 0) {
         if (elements.getLength() != 1) {
            throwSoapFault(FaultKey.WST_INVALID_REQUEST, "There are "
               + elements.getLength() + " elements(" + elementName
               + ") in the soap header. It should be only one");
         }

         elementNode = elements.item(0);
         assert elementNode != null;
      }
      return elementNode;
   }

   private Schema loadSchema(String schemaLocation, String schemaName) {
      assert schemaLocation != null;

      Schema schema = null;
      try
      {
         schema = Util.loadXmlSchemaFromResource(this.getClass().getClassLoader(), schemaLocation, schemaName);
      }
      catch(IllegalArgumentException e)
      {
          String message = String.format("Unable to load schema resource location=[%s], name=[%s]", schemaLocation, schemaName);
          logger.error(message, e);
          throwSoapFault(FaultKey.WST_REQUEST_FAILED, message);
      }
      catch (SAXException e)
      {
         String message = String.format("Unable to instantiate schema location=[%s], name=[%s]", schemaLocation, schemaName);
         logger.error(message, e);
         throwSoapFault(FaultKey.WST_REQUEST_FAILED, message);
      }
      assert schema != null;
      return schema;
   }

   private void validateWsseSecurity(Node secHeaderNode) {
      assert secHeaderNode != null;
      final NodeList childNodes = secHeaderNode.getChildNodes();
      assert childNodes != null;

      checkElementPresent(childNodes, wsuTimestamp, 1, 1,
         FaultKey.WSSE_INVALID_SECURITY);
      final List<Node> unts = checkElementPresent(childNodes,
         wsseUsernameToken, 0, 1, FaultKey.WSSE_INVALID_SECURITY);
      if (!unts.isEmpty()) {
         assert unts.size() == 1;
         checkElementPresent(unts.get(0).getChildNodes(), wssePassword, 0, 1,
            FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN);
         checkElementPresent(unts.get(0).getChildNodes(), wssePasscode, 0, 1,
            FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN);
      }
      checkElementPresent(childNodes, wsseBinarySecurityToken, 0, 1,
         FaultKey.WSSE_INVALID_SECURITY);
      checkElementPresent(childNodes, saml2Assertion, 0, 1,
         FaultKey.WSSE_INVALID_SECURITY);
      checkElementPresent(childNodes, dsSignature, 0, 1,
         FaultKey.WSSE_INVALID_SECURITY);
   }

   private static JAXBContext initJaxbContext(String contextPath) {
      assert contextPath != null;

      JAXBContext jaxbCtx = null;
      try {
         jaxbCtx = JAXBContext.newInstance(contextPath);
      } catch (JAXBException e) {
         String message = "Unable to instantiate Jaxb context for "
            + contextPath + " context path!";
         throwSoapFault(FaultKey.WST_REQUEST_FAILED, message);
      }
      assert jaxbCtx != null;
      return jaxbCtx;
   }

   private List<Node> checkElementPresent(NodeList nodes, QName elementQName,
      int minOccurs, int maxOccurs, FaultKey faultKey) {

      final List<Node> result = new ArrayList<Node>();
      int occurance = 0;
      for (int i = 0; i < nodes.getLength(); i++) {
         final Node node = nodes.item(i);
         assert node != null;
         if (node.getNodeType() == Node.ELEMENT_NODE) {
            final QName nodeQName = new QName(node.getNamespaceURI(),
               node.getLocalName());
            logger.trace("Encountering node {} searching for {}", nodeQName,
               elementQName);
            if (nodeQName.equals(elementQName)) {
               occurance++;
               result.add(node);
            }
         }
      }
      if (occurance < minOccurs || occurance > maxOccurs) {
         throwSoapFault(faultKey, String.format(
            "Element %s occurance %s is not within [%s, %s]", elementQName,
            occurance, minOccurs, maxOccurs));
      }
      return result;
   }

   private static void checkImplementation(JAXBContext jaxbCtx) {
      assert jaxbCtx != null;

      if (!jaxbCtx.getClass().getName().equals(METRO_JAXB_CONTEXT_IMPL_NAME)) {
         String message = "Cannot deploy STS because jaxb implementation is "
            + "different " + jaxbCtx.getClass().getName()
            + " than expected: " + METRO_JAXB_CONTEXT_IMPL_NAME;
         throwSoapFault(FaultKey.WST_REQUEST_FAILED, message);
      }
   }

}
