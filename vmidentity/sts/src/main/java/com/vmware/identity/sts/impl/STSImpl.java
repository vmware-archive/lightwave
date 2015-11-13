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

import java.util.Date;

import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ValidateTargetType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.TimestampType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
import com.vmware.identity.saml.DelegationException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.RenewException;
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.saml.UnsupportedTokenLifetimeException;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.InvalidSecurityException;
import com.vmware.identity.sts.InvalidSecurityHeaderException;
import com.vmware.identity.sts.InvalidSignatureException;
import com.vmware.identity.sts.InvalidTimeRangeException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestExpiredException;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.STS;
import com.vmware.identity.sts.UnableToRenewException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.authz.RoleCheck;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.SsoStatisticsService;
import com.vmware.identity.sts.idm.STSConfiguration;
import com.vmware.identity.sts.util.PrincipalIdConvertor;
import com.vmware.identity.util.TimePeriod;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

public final class STSImpl implements STS {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(STSImpl.class);

   // TODO [848560] make the number of max simultaneous session configurable
   private final LRURequests spnegoSessions = new LRURequests(1024);
   private final TokenAuthority tokenAuthority;
   private final TokenValidator tokenValidator;
   private final Authenticator authenticator;
   private final SamlTokenSpecBuilder specBuilder;
   private final STSConfigExtractor configExtractor;
   private final RoleCheck roleCheck;
   private final SsoStatisticsService ssoStatistics;

   public STSImpl(TokenAuthority tokenAuthority, TokenValidator tokenValidator, TokenValidator authnOnlyValidator,
      Authenticator authenticator, DelegationParser delegationParser,
      STSConfigExtractor configExtractor, RoleCheck roleCheck, SsoStatisticsService ssoStatistics) {
      assert tokenAuthority != null;
      assert tokenValidator != null;
      assert authenticator != null;
      assert delegationParser != null;
      assert configExtractor!= null;
      assert roleCheck != null;
      assert ssoStatistics != null;

      this.tokenAuthority = tokenAuthority;
      this.tokenValidator = tokenValidator;
      this.authenticator = authenticator;
      this.specBuilder = new SamlTokenSpecBuilder(delegationParser,
          authnOnlyValidator);
      this.configExtractor = configExtractor;
      this.roleCheck = roleCheck;
      this.ssoStatistics = ssoStatistics;
   }

   @Override
   public RequestSecurityTokenResponseCollectionType challenge(
      RequestSecurityTokenResponseType rstr, SecurityHeaderType header)
      throws AuthenticationFailedException, InvalidSignatureException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      RequestExpiredException, InvalidTimeRangeException,
      InvalidSecurityException, NoSuchIdPException, RequestFailedException {
      assert rstr != null && header != null;
      log.trace("challenge() token...");

      final Request req = new Request(header, toRst(rstr), null, null, null);
      validateChallengeReq(req);

      assert rstr.getContext() != null && rstr.getContext().length() > 0;
      final Request firstReq = spnegoSessions.retrieve(rstr.getContext());
      if (firstReq == null) {
         throw new InvalidCredentialsException("Unknown SPNEGO session!");
      }

      final Result authResult = authenticator.authenticate(req);
      assert authResult != null : "Program error! No BETAuthenticator or bug in it!";
      if (authResult.getAuthnMethod() == Result.AuthnMethod.EXTERNAL_ASSERTION) {
          throw new UnsupportedSecurityTokenException("External assertion is not supported in challenge operation.");
      }

      return RSTRBuilder.wrapInCollection(processIssueRequest(firstReq, req, authResult));
   }

   @Override
   public RequestSecurityTokenResponseCollectionType issue(Request req)
      throws AuthenticationFailedException, InvalidSignatureException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      UnableToRenewException, RequestExpiredException,
      InvalidTimeRangeException, InvalidSecurityException, NoSuchIdPException,
      RequestFailedException {
      assert req != null;
      log.trace("issue() token...");

      validateIssueRequest(req);

      final Result authResult = authenticator.authenticate(req);
      if (authResult == null) {
         throw new InvalidCredentialsException("Request not authenticated!");
      }

      return RSTRBuilder.wrapInCollection(processIssueRequest(req, req,
         authResult));
   }

   @Override
   public RequestSecurityTokenResponseType renew(Request req)
      throws AuthenticationFailedException, InvalidSignatureException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      UnableToRenewException, RequestExpiredException,
      InvalidTimeRangeException, InvalidSecurityException, NoSuchIdPException,
      RequestFailedException {
      assert req != null;
      log.trace("renew() token...");

      validateRenewRequest(req);

      final Result authResult = authenticator.authenticate(req);
      if (authResult == null) {
         throw new InvalidCredentialsException("Request not authenticated!");
      }
      assert authResult.completed() : "Token is passed in the request but the authN has not finished";

      if (authResult.getAuthnMethod() == Result.AuthnMethod.EXTERNAL_ASSERTION) {
          throw new UnsupportedSecurityTokenException("External assertion is not supported in renew operation.");
      }

      final SamlTokenSpec spec = specBuilder.buildRenewTokenSpec(req,
         authResult);

      RequestSecurityTokenResponseType response = RSTRBuilder.createRenewResponse(req, issueToken(spec));
      ssoStatistics.incrementRenewedTokens();
      return response;
   }

   @Override
   public RequestSecurityTokenResponseType validate(Request req)
      throws InvalidSecurityException, InvalidSignatureException,
      RequestExpiredException, NoSuchIdPException, RequestFailedException {
      assert req != null;
      log.trace("validate() token...");

      validateValidateReq(req);

      boolean valid = true;
      try {
         tokenValidator.validate(req.getSamlToken());
      } catch (com.vmware.identity.saml.InvalidSignatureException e) {
         throw new InvalidSignatureException(e);
      } catch (InvalidTokenException e) {
         valid = false;
      }

      return RSTRBuilder.createValidateResponse(req, valid);
   }

   private void validateChallengeReq(Request req)
      throws RequestFailedException, InvalidSecurityException,
      RequestExpiredException {

      assert req != null && req.getRst() != null;

      final String context = req.getRst().getContext();
      if (context == null || context.length() == 0) {
         throw new RequestFailedException("Bad request! Missing context value!");
      }
      final BinaryExchangeType binaryExchange = req.getRst()
         .getBinaryExchange();
      if (binaryExchange == null || binaryExchange.getValue() == null) {
         throw new InvalidSecurityHeaderException(
            "Bad request! Missing binary exchange!");
      }
      validateRequest(req);
   }

   private void validateIssueRequest(Request req)
      throws InvalidSecurityException, RequestExpiredException {

      assert req != null && req.getRst() != null;

      if (req.getRst().getDelegateTo() != null
         && req.getRst().getActAs() != null) {
         assert req.getActAsToken() != null;
         throw new RequestFailedException(
            "The issue request is ambiguous - both 'RST::DelegateTo' and 'RST::ActAs' are present!");
      }
      validateRequest(req);
   }

   private void validateRenewRequest(Request req)
      throws RequestFailedException, InvalidSecurityException,
      RequestExpiredException {

      assert req != null && req.getRst() != null;

      RenewTargetType renewTarget = req.getRst().getRenewTarget();
      if (renewTarget == null || renewTarget.getAssertion() == null) {
         throw new RequestFailedException(
            "The renew request does not contain token for renewal!");
      }
      assert req.getSamlToken() != null;
      validateRequest(req);
   }

   private void validateValidateReq(Request req) throws RequestFailedException,
      InvalidSecurityException, RequestExpiredException {

      assert req != null && req.getRst() != null;

      ValidateTargetType validateTarget = req.getRst().getValidateTarget();
      if (validateTarget == null || validateTarget.getAny() == null) {
         throw new RequestFailedException(
            "The validate request does not contain token for validation!");
      }
      assert req.getSamlToken() != null;
      validateRequest(req);
   }

   /**
    * Method neutral request validation.
    *
    * @param req
    *           mandatory
    */
   private void validateRequest(Request req) throws RequestExpiredException,
      InvalidSecurityException {

      assert req != null;
      log.debug("Validation of request");
      final Date now = new Date();

      TimestampType reqValidity = req.getHeader().getTimestamp();
      {
         String schemaErr = "Request is not validated against the schema";
         assert reqValidity != null && reqValidity.getCreated() != null
            && reqValidity.getExpires() != null : schemaErr;
      }

      log.debug("The request received is valid from {} till {} and now is: {}",
         new Object[] { reqValidity.getCreated().getValue(),
            reqValidity.getExpires().getValue(), now });

      final STSConfiguration config = configExtractor.getConfig();
      final long clockTolerance = config.getClockTolerance();
      final TimePeriod requestValidityPlusTolerance = TimePeriod.expand(
         LifetimeConvertor.fromRequestLifetime(reqValidity), clockTolerance);

      if (!requestValidityPlusTolerance.contains(now)) {
         log.error(
                    VmEvent.CLOCK_SKEW_ERROR,
                    "The current time {} does not fall in the request" +
                    "lifetime interval extended with clock tolerance of {} ms: [{}; {}). " +
                    "This might be due to a clock skew problem.",
                    new Object[] { now, clockTolerance, requestValidityPlusTolerance.getStartTime(), requestValidityPlusTolerance.getEndTime() }
                    );
         throw new RequestExpiredException("The time now " + now
            + " does not fall in the request lifetime interval"
            + " extended with clock tolerance of " + clockTolerance + " ms: [ "
            + requestValidityPlusTolerance.getStartTime() + "; "
            + requestValidityPlusTolerance.getEndTime() + ")."
            + " This might be due to a clock skew problem.");
      }
   }

   private RequestSecurityTokenType toRst(RequestSecurityTokenResponseType rstr) {
      ObjectFactory wstFactory = new ObjectFactory();
      RequestSecurityTokenType rst = wstFactory
         .createRequestSecurityTokenType();
      rst.setContext(rstr.getContext());
      rst.setBinaryExchange(rstr.getBinaryExchange());
      return rst;
   }

   private RequestSecurityTokenResponseType processIssueRequest(
      Request initialReq, Request currentReq, Result authResult)
      throws RequestFailedException, InvalidRequestException,
      UnableToRenewException, InvalidTimeRangeException, NoSuchIdPException {
      assert initialReq != null && currentReq != null && authResult != null;

      final String context = initialReq.getRst().getContext();
      assert context == null
         || context.equals(currentReq.getRst().getContext());

      final RequestSecurityTokenResponseType response;
      if (authResult.completed()) {
         log.debug("Authenticated principal: {} at time: {}",
            authResult.getPrincipalId(), authResult.getAuthnInstant());
         checkPermissions(initialReq, authResult);

         if (context != null) {
            spnegoSessions.remove(context);
         }
         SamlTokenSpec spec = specBuilder.buildIssueTokenSpec(initialReq,
            authResult);

         response = RSTRBuilder.createIssueResponse(initialReq,
            issueToken(spec), authResult);
         // increment generated tokens.
         ssoStatistics.incrementGeneratedTokens();
      } else {
         if (context == null) {
            throw new RequestFailedException("Missing context!");
         }
         // TODO consider whether this could be done only on first negotiation
         // request
         spnegoSessions.save(currentReq.getRst().getContext(), initialReq);
         response = RSTRBuilder.createSPNEGOResponse(currentReq, authResult);
      }
      return response;
   }

   private SamlToken issueToken(SamlTokenSpec spec)
      throws InvalidRequestException, UnableToRenewException,
      InvalidTimeRangeException {
      try {
         return tokenAuthority.issueToken(spec);
      } catch (UnsupportedTokenLifetimeException e) {
         throw new InvalidTimeRangeException(
            "The token authority rejected an issue request for "
               + spec.getLifespan(), e);
      } catch (DelegationException e) {
         throw new InvalidRequestException(e);
      } catch (RenewException e) {
         throw new UnableToRenewException(e);
      }
   }

   private void checkPermissions(Request req, Result authResult)
      throws NoSuchIdPException {
      assert req != null;
      assert authResult != null && authResult.completed();

      if (req.getActAsToken() != null) {

         final PrincipalId principalId = PrincipalIdConvertor
            .fromIdmPrincipalId(authResult.getPrincipalId());

         try {
            if (!roleCheck.hasRole(principalId,
               RoleManagement.WSTrustRole.ActAsUser)) {

               throw new InvalidRequestException("Access not authorized!");
            }
         } catch (InvalidPrincipalException e) {
            throw new InvalidRequestException(
               "The authenticated principal is no more accessible!", e);
         }
      }

   }
}
