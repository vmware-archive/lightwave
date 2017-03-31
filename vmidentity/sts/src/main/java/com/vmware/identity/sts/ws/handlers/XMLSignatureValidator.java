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

import java.util.Collections;
import java.util.Set;

import javax.servlet.ServletRequest;
import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPHeader;
import javax.xml.soap.SOAPMessage;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.soap.SOAPMessageContext;

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3._2000._09.xmldsig_.SignatureType;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.util.JAXBExtractor;
import com.vmware.identity.sts.ws.SOAPFaultHandler;
import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;
import com.vmware.identity.sts.ws.SignatureValidator;
import com.vmware.identity.sts.ws.WSFaultException;
import com.vmware.identity.sts.ws.WsConstants;

/**
 * This class is responsible for validating the signature (if present) in the
 * WS-Trust request
 */
public class XMLSignatureValidator implements SOAPHandler<SOAPMessageContext> {

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(XMLSignatureValidator.class);
   private final SignatureValidator signatureValidator = new SignatureValidator();

   public XMLSignatureValidator() {
   }

   @Override
   public boolean handleMessage(SOAPMessageContext ctx) {
      logger.debug("Inside XMLSignatureValidator");

      boolean result = true;
      Boolean outbound = (Boolean) ctx
         .get(MessageContext.MESSAGE_OUTBOUND_PROPERTY);
      if (outbound != null && outbound.equals(Boolean.TRUE)) {
         return result;
      }

      SecurityHeaderType header = extractSecurityHeaderFromRequest(ctx);
      SignatureType sig = JAXBExtractor.extractFromSecurityHeader(header, SignatureType.class);
      if (sig != null) {
         String signatureId = sig.getId();
         logger.info("Found signature {}", signatureId);
         Signature signature = null;
         try {
            signature = signatureValidator.validate(getSignatureHeader(ctx),
               header);
         } catch (WSFaultException e) {
            SOAPFaultHandler.throwSoapFault(e);
         }
         logger.info("Signature {} is valid", signatureId);
         storeSignatureInRequest(ctx, signature);
      }

      return result;
   }

   @Override
   public boolean handleFault(SOAPMessageContext ctx) {
      return true;
   }

   @Override
   public Set<QName> getHeaders() {
      return Collections.emptySet();
   }

   @Override
   public void close(MessageContext ctx) {
      // nothing to dispose
   }

   /**
    * @return the extracted WS-Security Security header from the request.
    */
   private SecurityHeaderType extractSecurityHeaderFromRequest(
      SOAPMessageContext ctx) {
      ServletRequest request = (ServletRequest) (ctx
         .get(MessageContext.SERVLET_REQUEST));
      Object securityHeader = request
         .getAttribute(WsConstants.SECURITY_HEADER_KEY);
      assert securityHeader != null;
      return (SecurityHeaderType) securityHeader;
   }

   /**
    * @param ctx
    *           not null
    * @return the SOAP header or throws a SOAP fault if no header is found
    */
   private Node getSignatureHeader(SOAPMessageContext ctx) {
      assert ctx != null;
      try {
         SOAPMessage message = ctx.getMessage();
         if (message == null) {
            SOAPFaultHandler.throwSoapFault(new WSFaultException(
               FaultKey.WSSE_INVALID_SECURITY,
               "SOAP message not found in the request"));
         }
         SOAPHeader header = message.getSOAPHeader();
         NodeList securityList = header.getElementsByTagNameNS(
            WsConstants.WSSE_NS, WsConstants.WSSE_SECURITY_ELEMENT_NAME);
         return securityList.item(0);
      } catch (SOAPException e) {
         SOAPFaultHandler.throwSoapFault(new WSFaultException(
            FaultKey.WSSE_INVALID_SECURITY, e));
         return null;
      }
   }

   /**
    * Stores the signing certificate and its location in the request for further
    * processing
    *
    * @param ctx
    *           cannot be null
    * @param signature
    *           can be null
    */
   private void storeSignatureInRequest(SOAPMessageContext ctx,
      Signature signature) {
      assert ctx != null;
      assert signature != null;
      ServletRequest request = (ServletRequest) ctx
         .get(MessageContext.SERVLET_REQUEST);
      request.setAttribute(WsConstants.SIGNATURE_KEY, signature);
   }
}
