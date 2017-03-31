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
package com.vmware.identity.sts.impl;

import java.nio.charset.Charset;

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewingType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestedSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.StatusType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.sts.ws.WsConstants;

/**
 * Insert your comment for RSTSBuilder here
 */
final class RSTRBuilder {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(RSTRBuilder.class);

   private static final String US_ASCII = "US-ASCII";
   // TODO [848547] fix generated JAXB model to include these enumerations
   // emitted
   private static final String WST_VALUE_TYPE = "http://schemas.xmlsoap.org/ws/2005/02/trust/spnego";
   private static final String WST_ENCODING_TYPE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary";

   /**
    * @param req
    *           current request. Cannot be null.
    * @param spec
    *           contains all the collected info from request needed from saml
    *           authority. Cannot be null.
    * @return generated response from given saml spec for issue request.
    */
   static RequestSecurityTokenResponseType createIssueResponse(Request req,
      SamlToken token, Result authResult) {
      assert req != null;
      assert token != null;
      assert authResult != null;

      RequestSecurityTokenResponseType response = createSAMLTokenResponse(req, token);

      response.setKeyType(token.getConfirmationType().toString());
      response.setSignatureAlgorithm(token.getSignatureAlgorithm().toString());
      response.setDelegatable(token.isDelegable());

      if ( authResult.getServerLeg() != null && authResult.getServerLeg().length > 0 )
      {
          BinaryExchangeType betResponse = createBinaryExchangeResponse(authResult);
          response.setBinaryExchange(betResponse);
          response.setContext(req.getRst().getContext());
      }

      return response;
   }

   /**
    * @param req
    *           current request. Cannot be null.
    * @param spec
    *           contains all the collected info from request needed from saml
    *           authority. Cannot be null.
    * @return generated response from given saml spec for renew request.
    */
   static RequestSecurityTokenResponseType createRenewResponse(Request req,
      SamlToken token) {
      assert req != null;
      assert token != null;

      RequestSecurityTokenResponseType response = createSAMLTokenResponse(req, token);

      RenewingType renewing = new RenewingType();
      renewing.setOK(false);
      renewing.setAllow(token.isRenewable());

      response.setRenewing(renewing);

      return response;
   }

   static RequestSecurityTokenResponseType createValidateResponse(Request req,
      boolean valid) {

      assert req != null;
      log.debug("Started creating response for validate method.");

      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenResponseType response = wstFactory
         .createRequestSecurityTokenResponseType();

      StatusType status = new StatusType();
      String code = (valid) ? WsConstants.VALIDATE_STATUS_CODE_VALID
         : WsConstants.VALIDATE_STATUS_CODE_INVALID;
      status.setCode(code);

      response.setStatus(status);
      response.setTokenType(WsConstants.VALIDATE_TOKEN_TYPE);
      response.setContext(req.getRst().getContext());

      log.debug("Created response for validate method, code: {}", code);
      return response;
   }

   static RequestSecurityTokenResponseType createSPNEGOResponse(Request req,
      Result authResult) {
      assert req != null;
      assert authResult != null && !authResult.completed();

      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenResponseType response = wstFactory
         .createRequestSecurityTokenResponseType();

      BinaryExchangeType betResponse = createBinaryExchangeResponse(authResult);
      response.setBinaryExchange(betResponse);
      response.setContext(req.getRst().getContext());

      return response;
   }

   private static BinaryExchangeType createBinaryExchangeResponse(Result authResult)
   {
      assert authResult != null && authResult.getServerLeg() != null && authResult.getServerLeg().length > 0;

      ObjectFactory wstFactory = new ObjectFactory();

      BinaryExchangeType betResponse = wstFactory.createBinaryExchangeType();
      betResponse.setValue(new String(Base64.encodeBase64(authResult
         .getServerLeg()), Charset.forName(US_ASCII)));
      betResponse.setValueType(WST_VALUE_TYPE);
      betResponse.setEncodingType(WST_ENCODING_TYPE);

      return betResponse;
   }

   static RequestSecurityTokenResponseType createSecurIDNegotiationResponse(Result authResult) {
      assert authResult != null && !authResult.completed();

      log.debug("Started creating SecurID negotiation response.");

      if (authResult == null || authResult.getAuthnMethod() != AuthnMethod.TIMESYNCTOKEN || authResult.getSessionID() == null) {
         throw new IllegalArgumentException("SecurID negotiation response can not be created due to invalid authentication result.");
      }

      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenResponseType response = wstFactory
         .createRequestSecurityTokenResponseType();

      response.setContext(authResult.getSessionID());

      log.debug("SecurID negotiation response created.");

      return response;
   }

   static RequestSecurityTokenResponseCollectionType wrapInCollection(
      RequestSecurityTokenResponseType response) {
      assert response != null;

      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenResponseCollectionType resColl = wstFactory
         .createRequestSecurityTokenResponseCollectionType();
      resColl.setRequestSecurityTokenResponse(response);

      return resColl;
   }

   /**
    * @param req
    *           current request. Cannot be null.
    * @param spec
    *           contains all the collected info from request needed from saml
    *           authority. Cannot be null.
    * @return generated response from given saml spec.
    */
   private static RequestSecurityTokenResponseType createSAMLTokenResponse(Request req,
      SamlToken token) {
      assert req != null;
      assert token != null;

      log.debug("Started creating saml token for response.");

      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenResponseType response = wstFactory
         .createRequestSecurityTokenResponseType();
      RequestedSecurityTokenType assertion = wstFactory
         .createRequestedSecurityTokenType();
      assertion.setAny(token.getDocument().getDocumentElement());

      response.setContext(req.getRst().getContext());
      response.setTokenType("urn:oasis:names:tc:SAML:2.0:assertion");
      response.setLifetime(LifetimeConvertor.toResponseLifetime(token
         .getValidity()));
      response.setRequestedSecurityToken(assertion);

      log.debug("Response created.");

      return response;
   }

}
