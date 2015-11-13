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

import static org.easymock.EasyMock.anyBoolean;
import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.fail;

import javax.servlet.ServletRequest;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPHeader;
import javax.xml.soap.SOAPMessage;
import javax.xml.soap.SOAPPart;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPMessageContext;
import javax.xml.ws.soap.SOAPFaultException;

import org.junit.Test;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3._2005._08.addressing.AttributedURIType;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.sts.ws.WsConstants;

/**
 * TODO fix run from ant and run from eclipse to work - location of schema
 */
public class SOAPHeadersExtractorTest {

   private static final String WSA_ACTION_URI_MOCK = "wsa action uri";

   private static final boolean testExpectedToPass = true;
   private static final boolean testExpectedToFail = false;
   private final SOAPHeadersExtractor extractor;
   private final Node securityNode;
   private final Node invalidSecurityNode;
   private final Node wsaActionNode;

   public SOAPHeadersExtractorTest() throws ParserConfigurationException {
      extractor = new SOAPHeadersExtractor(
          "WEB-INF/wsdl/sts_xsd/",
         "profiled-ws-header.xsd",
         "ws-addr.xsd");
      securityNode = createSecurity(true);
      wsaActionNode = createWSAAction();
      invalidSecurityNode = createSecurity(false);
   }

   @Test
   public void testOutboundHandling() throws JAXBException {
      boolean isOutbound = true;
      SOAPMessageContext ctx = createSOAPMessageContext(isOutbound);
      extractor.handleMessage(ctx);
      verify(ctx);
   }

   @Test
   public void testNoSOAPMessageInContext() throws JAXBException {
      SOAPMessage msg = null;
      SOAPMessageContext ctx = createSOAPMessageContext(msg);
      try {
         extractor.handleMessage(ctx);
      } catch (SOAPFaultException e) {
         // expected
      } finally {
         verify(ctx);
      }
   }

   @Test
   public void testValidateFails() throws Exception {
      testSOAPHeaderCreateChildNodes(1, invalidSecurityNode, 0, null, null,
         testExpectedToFail);
   }

   @Test
   public void testSuccessfulExtractionOfSecurityHeaderAndWSAAction()
      throws Exception {
      SOAPPart soapPart = createSOAPPart();
      testSOAPHeaderCreateChildNodes(1, securityNode, 1, wsaActionNode,
         soapPart, testExpectedToPass);
      verify(soapPart);
   }

   @Test
   public void testSuccessfulExtractionOfSecurityHeaderOnly() throws Exception {
      SOAPPart soapPart = createSOAPPart();
      testSOAPHeaderCreateChildNodes(1, securityNode, 0, wsaActionNode,
         soapPart, testExpectedToPass);
      verify(soapPart);
   }

   @Test
   public void testMultipleSecurityHeaders() throws Exception {
      SOAPPart soapPart = createSOAPPart();
      testSOAPHeaderCreateChildNodes(2, securityNode, 0, null, null,
         testExpectedToFail);
      verify(soapPart);
   }

   @Test
   public void testNoSecurityHeader() throws Exception {
      SOAPPart soapPart = createSOAPPart();
      testSOAPHeaderCreateChildNodes(0, securityNode, 0, null, null,
         testExpectedToFail);
      verify(soapPart);
   }

   @Test
   public void testMultipleActionHeaders() throws Exception {
      SOAPPart soapPart = createSOAPPart();
      testSOAPHeaderCreateChildNodes(1, securityNode, 2, wsaActionNode,
         soapPart, testExpectedToFail);
      verify(soapPart);
   }

   private void testSOAPHeaderCreateChildNodes(
      int soapHeaderSecurityNodesLength, Node securityNode,
      int soapHeaderWSAActionNodesLength, Node wsaActionNode,
      SOAPPart soapPart, boolean testExpectedToPass) throws Exception {

      NodeList soapHeaderSecurityChildNodes = createSoapHeaderNodes(
         soapHeaderSecurityNodesLength, securityNode);
      NodeList soapHeaderWSAActionChildNodes = new NodeList() {

         @Override
         public Node item(int index) {
            return null;
         }

         @Override
         public int getLength() {
            return 0;
         }
      };

      if (soapHeaderWSAActionNodesLength > 0) {
         soapHeaderWSAActionChildNodes = createSoapHeaderNodes(
            soapHeaderWSAActionNodesLength, wsaActionNode);
      }

      SOAPHeader soapHeader = createSoapHeader(soapHeaderSecurityChildNodes,
         securityNode, soapHeaderWSAActionChildNodes,
         soapHeaderWSAActionNodesLength, wsaActionNode);

      SOAPMessage msg = createSOAPMessage(soapHeader, soapPart);
      SOAPMessageContext ctx;

      JAXBElement<SecurityHeaderType> jaxbSecurityHeader = null;
      JAXBElement<AttributedURIType> jaxbWsaAction = null;
      ServletRequest request = null;
      // having soapPart means we also want to extract from request entire DOM
      if (soapPart != null) {
         SecurityHeaderType secHeader = new SecurityHeaderType();

         jaxbSecurityHeader = createJaxbSecHeader(secHeader);
         Object[] headers = new Object[] { jaxbSecurityHeader };

         Object[] wsaHeaders = null;
         String wsaAction = null;
         if (wsaActionNode != null && soapHeaderWSAActionNodesLength > 0) {
            wsaAction = WSA_ACTION_URI_MOCK;
            AttributedURIType value = new AttributedURIType();
            value.setValue(wsaAction);

            jaxbWsaAction = createJaxbSecHeader(value);
            wsaHeaders = new Object[] { jaxbWsaAction };
         }
         request = createServletRequest(soapPart, secHeader, wsaAction);

         ctx = createSOAPMessageContext(msg, request, headers, wsaHeaders);
      } else {
         ctx = createSOAPMessageContext(msg);
      }
      try {
         extractor.handleMessage(ctx);
         if (!testExpectedToPass) {
            fail();
         }
      } catch (SOAPFaultException e) {
         // expected
         if (testExpectedToPass) {
            fail();
         }
      }

      if (testExpectedToPass) {
         verify(soapHeaderSecurityChildNodes, soapHeader, msg, ctx);
         if (request != null) {
            verify(request);
         }
         if (jaxbSecurityHeader != null) {
            verify(jaxbSecurityHeader);
         }
         if (jaxbWsaAction != null) {
            verify(jaxbWsaAction);
         }
      }
   }

   private SOAPMessage createSOAPMessage(SOAPHeader soapHeader,
      SOAPPart soapPart) throws SOAPException {
      SOAPMessage msg = createMock(SOAPMessage.class);
      expect(msg.getSOAPHeader()).andReturn(soapHeader).anyTimes();
      if (soapPart != null) {
         expect(msg.getSOAPPart()).andReturn(soapPart);
      }
      replay(msg);

      return msg;
   }

   private SOAPPart createSOAPPart() {
      SOAPPart soapPart = createMock(SOAPPart.class);
      replay(soapPart);

      return soapPart;
   }

   private SOAPHeader createSoapHeader(NodeList securityHeadersInSoapHeader,
      Node secHeaderNode, NodeList actionHeadersInSoapHeader,
      int soapHeaderWSAActionNodesLength, Node actionNode) throws SOAPException {

      SOAPHeader soapHeader = createMock(SOAPHeader.class);
      String secHeaderNS = secHeaderNode.getNamespaceURI();
      String secHeaderName = secHeaderNode.getLocalName();
      expect(
         soapHeader.getElementsByTagNameNS(eq(secHeaderNS), eq(secHeaderName)))
         .andReturn(securityHeadersInSoapHeader);

      if (actionNode != null) {
         String actionHeaderNS = actionNode.getNamespaceURI();
         String actionHeaderName = actionNode.getLocalName();
         expect(
            soapHeader.getElementsByTagNameNS(eq(actionHeaderNS),
               eq(actionHeaderName))).andReturn(actionHeadersInSoapHeader);
      }

      replay(soapHeader);

      return soapHeader;
   }

   private NodeList createSoapHeaderNodes(int soapHeaderChildNodesLength,
      Node elementToReturn) {
      NodeList soapHeaderChildNodes = createMock(NodeList.class);
      expect(soapHeaderChildNodes.getLength()).andReturn(
         soapHeaderChildNodesLength).anyTimes();

      expect(soapHeaderChildNodes.item(eq(0))).andReturn(elementToReturn)
         .anyTimes();

      replay(soapHeaderChildNodes);

      return soapHeaderChildNodes;
   }

   private SOAPMessageContext createSOAPMessageContext(boolean isOutbound)
      throws JAXBException {
      return createSOAPMessageContext(null, isOutbound, null, null, null);
   }

   private SOAPMessageContext createSOAPMessageContext(SOAPMessage msg)
      throws JAXBException {
      return createSOAPMessageContext(msg, false, null, null, null);
   }

   private SOAPMessageContext createSOAPMessageContext(SOAPMessage msg,
      ServletRequest request, Object[] securityheaders, Object[] wsaHeaders)
      throws JAXBException {
      return createSOAPMessageContext(msg, false, request, securityheaders,
         wsaHeaders);
   }

   private SOAPMessageContext createSOAPMessageContext(SOAPMessage msg,
      boolean isOutbound, ServletRequest request, Object[] securityHeaders,
      Object[] wsaHeaders) throws JAXBException {
      SOAPMessageContext ctx = createMock(SOAPMessageContext.class);
      expect(ctx.get(MessageContext.MESSAGE_OUTBOUND_PROPERTY)).andReturn(
         isOutbound);
      if (!isOutbound) {
         expect(ctx.getMessage()).andReturn(msg).anyTimes();

         if (request != null) {
            // here real objects given cannot be matched from easymock
            expect(
               ctx.getHeaders((QName) anyObject(), (JAXBContext) anyObject(),
                  anyBoolean())).andReturn(securityHeaders);
            if (wsaHeaders != null) {
               expect(
                  ctx.getHeaders((QName) anyObject(),
                     (JAXBContext) anyObject(), anyBoolean())).andReturn(
                  wsaHeaders).anyTimes();
            }

            expect(ctx.get(eq(MessageContext.SERVLET_REQUEST))).andReturn(
               request).anyTimes();
         }
      }
      replay(ctx);

      return ctx;
   }

   private <T> JAXBElement<T> createJaxbSecHeader(T secHeader) {
      @SuppressWarnings("unchecked")
      JAXBElement<T> jaxbSecurityHeader = createMock(JAXBElement.class);
      expect(jaxbSecurityHeader.getValue()).andReturn(secHeader).anyTimes();
      replay(jaxbSecurityHeader);

      return jaxbSecurityHeader;
   }

   private ServletRequest createServletRequest(SOAPPart entireDOM,
      SecurityHeaderType secHeader, String wsaAction) {
      ServletRequest request = createMock(ServletRequest.class);
      request.setAttribute(eq(WsConstants.SECURITY_HEADER_KEY), eq(secHeader));
      request.setAttribute(eq(WsConstants.DOM_REQUEST_KEY), eq(entireDOM));
      if (wsaAction != null) {
         request.setAttribute(eq(WsConstants.WSA_ACTION_KEY), eq(wsaAction));
      }
      replay(request);

      return request;
   }

   private Node createSecurity(boolean createId)
      throws ParserConfigurationException {
      Document document = buildDocument();

      Element security = document.createElementNS(WsConstants.WSSE_NS,
         "Security");
      document.appendChild(security);
      security.appendChild(createTimestampNode(document, createId));

      return security;
   }

   private Node createWSAAction() throws ParserConfigurationException {
      Document document = buildDocument();

      Element wsaAction = document
         .createElementNS(WsConstants.WSA_NS, "Action");
      wsaAction.setNodeValue("wsaAction_OperationURL");
      document.appendChild(wsaAction);

      return wsaAction;
   }

   private Document buildDocument() throws ParserConfigurationException {
      Document document;
      DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
      DocumentBuilder builder = factory.newDocumentBuilder();
      document = builder.newDocument();
      return document;
   }

   private Node createTimestampNode(Document document, boolean valid) {
      Element created = createElement(document, "Created", WsConstants.WSU_NS,
         "2012-03-20T13:18:37.219Z");
      Element expires = createElement(document, "Expires", WsConstants.WSU_NS,
         "2012-03-20T13:28:37.219Z");

      final Attr idAttribute = valid ? document.createAttributeNS(
         WsConstants.WSU_NS, "Id") : document.createAttribute("Id");
      idAttribute.setValue("_a6da0aa6-7719-422e-ad7f-1ea8ac97505a");

      return createTimestamp(document, created, expires, idAttribute);
   }

   private Element createTimestamp(Document document, Element created,
      Element expires, Attr idAttribute) {
      Element timestamp = document.createElementNS(WsConstants.WSU_NS,
         "Timestamp");
      timestamp.setAttributeNodeNS(idAttribute);
      timestamp.setIdAttributeNode(idAttribute, true);

      timestamp.appendChild(created);
      timestamp.appendChild(expires);
      return timestamp;
   }

   private Element createElement(Document parentDocument, String qualifiedName,
      String ns, String content) {
      Element created = parentDocument.createElementNS(ns, qualifiedName);
      created.appendChild(parentDocument.createTextNode(content));
      return created;
   }

}
