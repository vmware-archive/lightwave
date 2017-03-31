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

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.util.concurrent.TimeUnit;

import javax.annotation.Resource;
import javax.jws.HandlerChain;
import javax.jws.WebService;
import javax.servlet.ServletRequest;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.ws.WebServiceContext;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.soap.SOAPFaultException;

import oasis.names.tc.saml._2_0.assertion.AssertionType;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ValidateTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.wsdl.STSService;
import org.oasis_open.docs.ws_sx.ws_trust._200802.ActAsType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.context.support.ClassPathXmlApplicationContext;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.sun.xml.ws.developer.SchemaValidation;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlTokenFactory;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.InvalidSecurityException;
import com.vmware.identity.sts.InvalidSecurityHeaderException;
import com.vmware.identity.sts.InvalidTimeRangeException;
import com.vmware.identity.sts.MultiTenantSTS;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.RequestExpiredException;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnableToRenewException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.util.JAXBExtractor;
import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;
import com.vmware.identity.sts.ws.handlers.SoapMsgMetricsCollector;
import com.vmware.identity.util.PerfConstants;

/**
 * This class provides an implementation of the STS web service
 */
@SchemaValidation(outbound = true, inbound = true)
@WebService(endpointInterface = "org.oasis_open.docs.ws_sx.ws_trust._200512.wsdl.STSService")
@HandlerChain(file = "handlers.xml")
public class StsServiceImpl implements STSService {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(StsServiceImpl.class);

   private final static IDiagnosticsLogger perfLog = DiagnosticsLoggerFactory
      .getLogger(PerfConstants.PERF_LOGGER.getClass());

   @Resource
   private WebServiceContext wsContext;
   private final MultiTenantSTS sts;

   public StsServiceImpl() {
      log.debug("Creating web service endpoint.");
      final BeanFactory ctx = new ClassPathXmlApplicationContext(
         "classpath:stsApplicationContext.xml");

      this.sts = (MultiTenantSTS) ctx.getBean("multiTenantSts");
      if(this.sts == null)
      {
          log.info(VmEvent.SERVER_FAILED_TOSTART, "STS Web Service endpoint failed to initialize.");
      }
      else
      {
          log.info(VmEvent.SERVER_STARTED, "Created STS Web Service endpoint.");
      }

      log.debug("STS instance in endpoint : {}", this.sts);

      SoapMsgMetricsCollector.setPerfDataSink((IPerfDataSink) ctx
         .getBean("perfDataSink"));
   }

   @Override
   public RequestSecurityTokenResponseCollectionType challenge(
           RequestSecurityTokenResponseType requestSecurityTokenResponse)
       throws SOAPFaultException, IllegalStateException
   {
       RequestSecurityTokenResponseCollectionType response = null;

      try {
         log.debug("Start handling 'challenge' request");
         final long startedAt = now();

         response = sts.challenge(
            extractTenantNameFromRequest(), requestSecurityTokenResponse,
            extractSecurityHeaderFromRequest());

         perfLog.trace("'sts.challenge' took {} ms.", now() - startedAt);
      } catch (InvalidCredentialsException e) {
         throwFault(FaultKey.WST_AUTHENTICATION_FAILED, e.buildPublic());
      } catch (UnsupportedSecurityTokenException e) {
         throwFault(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN, e);
      } catch (RequestExpiredException e) {
         throwFault(FaultKey.WSSE_MESSAGE_EXPIRED, e);
      } catch (InvalidSecurityHeaderException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (InvalidSecurityException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (InvalidTimeRangeException e) {
         throwFault(FaultKey.WST_INVALID_TIME_RANGE, e);
      } catch (InvalidRequestException e) {
         throwFault(FaultKey.WST_INVALID_REQUEST, e);
      } catch (NoSuchIdPException e) {
         putHTTP404StatusCode(e);
      } catch (RequestFailedException e) {
         throwFault(FaultKey.WST_REQUEST_FAILED, e);
      } catch (Exception e) {
         throwGenericFault(e);
      } catch (Error e) {
         log.error("Error occured while processing Challenge request.", e);
         throw e;
      }
      return response;
   }

   @Override
   public RequestSecurityTokenResponseCollectionType issue(
      RequestSecurityTokenType requestSecurityToken) throws SOAPFaultException,
      IllegalStateException {

      RequestSecurityTokenResponseCollectionType response = null;
      try {
         log.debug("Start handling 'issue' request");
         final long startedAt = now();

         checkRequest(WsConstants.REQUEST_TYPE_ISSUE, requestSecurityToken,
            WsConstants.SOAP_ACTION_ISSUE);

         response = sts.issue(extractTenantNameFromRequest(),
            buildRequest(requestSecurityToken));
         perfLog.trace("'sts.issue' took {} ms.", now() - startedAt);
      } catch (InvalidCredentialsException e) {
         throwFault(FaultKey.WST_AUTHENTICATION_FAILED, e.buildPublic());
      } catch (UnsupportedSecurityTokenException e) {
         throwFault(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN, e);
      } catch (RequestExpiredException e) {
         throwFault(FaultKey.WSSE_MESSAGE_EXPIRED, e);
      } catch (InvalidSecurityHeaderException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (InvalidSecurityException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (InvalidTimeRangeException e) {
         throwFault(FaultKey.WST_INVALID_TIME_RANGE, e);
      } catch (InvalidRequestException e) {
         throwFault(FaultKey.WST_INVALID_REQUEST, e);
      } catch (UnableToRenewException e) {
         throwFault(FaultKey.WST_UNABLE_TO_RENEW, e);
      } catch (NoSuchIdPException e) {
         putHTTP404StatusCode(e);
      } catch (RequestFailedException e) {
         throwFault(FaultKey.WST_REQUEST_FAILED, e);
      } catch (Exception e) {
         throwGenericFault(e);
      } catch (Error e) {
         log.error("Error occured while processing Issue request.", e);
         throw e;
      }
      return response;
   }

   @Override
   public RequestSecurityTokenResponseType renew(
      RequestSecurityTokenType requestSecurityToken) throws SOAPFaultException,
      IllegalStateException {

      RequestSecurityTokenResponseType response = null;
      try {
         log.debug("Start handling 'renew' request");
         final long startedAt = now();
         checkRequest(WsConstants.REQUEST_TYPE_RENEW, requestSecurityToken,
            WsConstants.SOAP_ACTION_RENEW);

         response = sts.renew(extractTenantNameFromRequest(),
            buildRequest(requestSecurityToken));
         perfLog.trace("'sts.renew' took {} ms.", now() - startedAt);
      } catch (InvalidCredentialsException e) {
         throwFault(FaultKey.WST_AUTHENTICATION_FAILED, e.buildPublic());
      } catch (UnsupportedSecurityTokenException e) {
         throwFault(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN, e);
      } catch (RequestExpiredException e) {
         throwFault(FaultKey.WSSE_MESSAGE_EXPIRED, e);
      } catch (InvalidSecurityException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (InvalidTimeRangeException e) {
         throwFault(FaultKey.WST_INVALID_TIME_RANGE, e);
      } catch (InvalidRequestException e) {
         throwFault(FaultKey.WST_INVALID_REQUEST, e);
      } catch (UnableToRenewException e) {
         throwFault(FaultKey.WST_UNABLE_TO_RENEW, e);
      } catch (NoSuchIdPException e) {
         putHTTP404StatusCode(e);
      } catch (RequestFailedException e) {
         throwFault(FaultKey.WST_REQUEST_FAILED, e);
      } catch (Exception e) {
         throwGenericFault(e);
      } catch (Error e) {
         log.error("Error occured while processing Renew request.", e);
         throw e;
      }

      return response;
   }

   @Override
   public RequestSecurityTokenResponseType validate(
      RequestSecurityTokenType requestSecurityToken) throws SOAPFaultException,
      IllegalStateException {

      RequestSecurityTokenResponseType response = null;
      try {
         log.debug("Start handling 'validate' request");
         final long startedAt = now();
         checkRequest(WsConstants.REQUEST_TYPE_VALIDATE, requestSecurityToken,
            WsConstants.SOAP_ACTION_VALIDATE);

         response = sts.validate(extractTenantNameFromRequest(),
            buildRequest(requestSecurityToken));
         perfLog.trace("'sts.validate' took {} ms.", now() - startedAt);
      } catch (RequestExpiredException e) {
         throwFault(FaultKey.WSSE_MESSAGE_EXPIRED, e);
      } catch (InvalidSecurityException e) {
         throwFault(FaultKey.WSSE_INVALID_SECURITY, e);
      } catch (NoSuchIdPException e) {
         putHTTP404StatusCode(e);
      } catch (RequestFailedException e) {
         throwFault(FaultKey.WST_REQUEST_FAILED, e);
      } catch (Exception e) {
         throwGenericFault(e);
      } catch (Error e) {
         log.error("Error occured while processing Validate request.", e);
         throw e;
      }
      return response;
   }

   private Request buildRequest(RequestSecurityTokenType requestSecurityToken) {
      final SecurityHeaderType securityHeader = extractSecurityHeaderFromRequest();
      final Document domRequest = extractDOMRequestObject();
      final ServerValidatableSamlTokenFactory tokenFactory = new ServerValidatableSamlTokenFactory();

      return new Request(securityHeader, requestSecurityToken,
         extractSignatureFromRequest(), extractSamlToken(tokenFactory,
            securityHeader, domRequest,
            requestSecurityToken.getValidateTarget(),
            requestSecurityToken.getRenewTarget()), extractActAsToken(
            tokenFactory, domRequest, requestSecurityToken.getActAs()));
   }

   /**
    * @return the extracted WS-Security Security header from the request. If
    *         there is no such header null will be returned.
    */
   private SecurityHeaderType extractSecurityHeaderFromRequest() {
      return (SecurityHeaderType) extractFromRequest(WsConstants.SECURITY_HEADER_KEY);
   }

   /**
    * @return the extracted WS-Security Security header from the request. If
    *         there is no such header null will be returned.
    */
   private Signature extractSignatureFromRequest() {
      return (Signature) extractFromRequest(WsConstants.SIGNATURE_KEY);
   }

   /**
    * @return the extracted wsa:Action tag value from the request. If there is
    *         no such tag null will be returned.
    */
   private String extractWSAActionFromRequest() {
      return (String) extractFromRequest(WsConstants.WSA_ACTION_KEY);
   }

   /**
    * @return the extracted tenant name from the request. If there is no such
    *         exception will be thrown.
    */
   private String extractTenantNameFromRequest() {
      String extractedTenantName = TenantExtractor
         .extractTenantName(((HttpServletRequest) getRequest()).getPathInfo());
      log.debug("Extracted tenantName from request: {}", extractedTenantName);
      return extractedTenantName;
   }

   private Document extractDOMRequestObject() {
      return (Document) extractFromRequest(WsConstants.DOM_REQUEST_KEY);
   }

   private Object extractFromRequest(String key) {
      ServletRequest request = getRequest();
      return request.getAttribute(key);
   }

   private ServletRequest getRequest() {
      final ServletRequest req = (ServletRequest) wsContext.getMessageContext()
         .get(MessageContext.SERVLET_REQUEST);
      assert req != null;
      return req;
   }

   private ServerValidatableSamlToken extractSamlToken(
      ServerValidatableSamlTokenFactory tokenFactory,
      SecurityHeaderType securityHeader, Document domReq,
      ValidateTargetType validateTarget, RenewTargetType renew) {

      final boolean tokenInValidate = validateTarget != null
         && validateTarget.getAny() != null;
      final boolean tokenInRenew = renew != null
         && renew.getAssertion() != null;
      final boolean tokenInSecHeader = JAXBExtractor.extractFromSecurityHeader(securityHeader, AssertionType.class) != null;

      Element token = null;
      if (tokenInValidate) {
         checkTokenNotIn(tokenInRenew, tokenInSecHeader);
         token = validateTarget.getAny();
      } else if (tokenInSecHeader) {
         checkTokenNotIn(tokenInRenew, tokenInValidate);
         token = tokenFromSecHeader(domReq);
      } else if (tokenInRenew) {
         checkTokenNotIn(tokenInSecHeader, tokenInValidate);
         token = tokenFromRenewTarget(domReq);
      }

      if (log.isDebugEnabled()) {
         try {
            log.debug("Extracted saml token:[{}] ", ((token == null) ? "(NULL)"
               : com.vmware.identity.token.impl.Util.serializeToString(token)));
         } catch (Exception e) {
            log.debug("Failed to SerializeTokenToString.", e);
         }
      }

      return parseSAMLToken(tokenFactory, token);
   }

   private ServerValidatableSamlToken extractActAsToken(
      ServerValidatableSamlTokenFactory tokenFactory, Document domReq,
      ActAsType actAs) {
      log.trace("Found JAXB ActAs entity - {}", actAs);
      return (actAs == null || actAs.getAssertion() == null) ? null
         : parseSAMLToken(tokenFactory, tokenFromActAs(domReq));
   }

   private Element tokenFromRenewTarget(Document req) {
      return getSingleElement(req, WsConstants.WST_NS, "RenewTarget");
   }

   private Element tokenFromActAs(Document req) {
      return getSingleElement(req, WsConstants.WST14_NS, "ActAs");
   }

   private Element getSingleElement(Document req, String ns, String elementName) {
      assert req != null;
      assert ns != null;
      assert elementName != null;
      NodeList renewNode = req.getElementsByTagNameNS(ns, elementName);

      // schema guaranteed
      assert renewNode.getLength() == 1;
      Node assertionNode = renewNode.item(0).getFirstChild();
      if (assertionNode.getNodeType() == Node.ELEMENT_NODE) {
         return (Element) assertionNode;
      } else {
         throw new IllegalStateException(elementName
            + " node is not an instance of DOM Element");
      }
   }

   private Element tokenFromSecHeader(Document req) {
      // get Assertion element from SecurityHeader
      NodeList securityList = req.getElementsByTagNameNS(WsConstants.WSSE_NS,
         WsConstants.WSSE_SECURITY_ELEMENT_NAME);

      // TODO is this really an assert or it should be if
      assert securityList.getLength() == 1;
      final Node securityHeaderNode = securityList.item(0);

      // TODO is this really an assert or it should be if
      assert securityHeaderNode.getNodeType() == Node.ELEMENT_NODE;
      final Element securityHeaderElement = (Element) securityHeaderNode;

      final NodeList assertionElements = securityHeaderElement
         .getElementsByTagNameNS(WsConstants.ASSERTION_NS,
            WsConstants.ASSERTION_ELEMENT_NAME);
      assert assertionElements.getLength() == 1;

      Element token = (Element) assertionElements.item(0);
      return token;
   }

   private ServerValidatableSamlToken parseSAMLToken(
      ServerValidatableSamlTokenFactory tokenFactory, Element token) {
      try {
         return (token != null) ? tokenFactory.parseToken(token) : null;
      } catch (InvalidTokenException e) {
         throw new InvalidSecurityHeaderException(
            "The given assertion cannot be parsed.", e);
      }
   }

   private void checkTokenNotIn(boolean place1, boolean place2) {
      if (place1 || place2) {
         throw new InvalidRequestException(
            "Request should contain only one assertion!");
      }
   }

   private void throwFault(FaultKey faultKey, Throwable e)
      throws SOAPFaultException, IllegalStateException {
      log.debug(getStackTraceNoException(e));
      SOAPFaultHandler.throwSoapFault(new WSFaultException(faultKey, e));
   }

   private void throwGenericFault(Throwable e) throws SOAPFaultException,
      IllegalStateException {
      log.error(getStackTraceNoException(e));
      SOAPFaultHandler.throwSoapFault(new WSFaultException(
         FaultKey.WST_REQUEST_FAILED, e));
   }

   private void putHTTP404StatusCode(Exception e) {
      log.info(e.toString());
      wsContext.getMessageContext().put(MessageContext.HTTP_RESPONSE_CODE,
         HttpServletResponse.SC_NOT_FOUND);
   }

   private String getStackTraceNoException(Throwable cause) {
      String result = null;
      Writer out = new StringWriter();
      try {
         dumpStackTrace(cause, out);
         result = out.toString();
      } catch (Exception e) {
         result = e.getMessage();
      } finally {
         try {
            out.close();
         } catch (IOException ex) {
            log.error("Unable to close stack trace writer! Cause: {}", ex);
         }
      }
      return result;
   }

   private void dumpStackTrace(Throwable e, Writer out) throws IOException {
      final PrintWriter printWriter = new PrintWriter(out);
      try {
         e.printStackTrace(printWriter);
         if (printWriter.checkError()) {
            throw new IOException("Error occured preparing the stack trace!");
         }
      } finally {
         printWriter.close();
      }
   }

   private void checkRequest(String expectedRequestType,
      RequestSecurityTokenType requestSecurityToken, String expectedWsaAction) {
      assert expectedRequestType != null;
      assert requestSecurityToken != null;
      assert expectedWsaAction != null;

      final String requestType = requestSecurityToken.getRequestType();
      assert requestType != null; // this is schema validated

      if (!expectedRequestType.equals(requestType)) {
         throw new InvalidRequestException(
            "RequestType is different than operation executed.");
      }

      final String wsaAction = extractWSAActionFromRequest();
      if (wsaAction != null && !expectedWsaAction.equals(wsaAction)) {
         throw new InvalidRequestException(
            String
               .format(
                  "The passed-in wsa:Action '%s' does not match the expected SOAP action '%s' of the current wsdl operation.",
                  wsaAction, expectedWsaAction));
      }
   }

   private long now() {
      return TimeUnit.NANOSECONDS.toMillis(System.nanoTime());
   }
}
