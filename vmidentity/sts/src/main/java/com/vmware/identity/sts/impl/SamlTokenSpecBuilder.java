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

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import oasis.names.tc.saml._2_0.assertion.AssertionType;

import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200802.ActAsType;

import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceSetType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AttributeType;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.Advice.Attribute;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.InvalidSecurityHeaderException;
import com.vmware.identity.sts.InvalidTimeRangeException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.util.JAXBExtractor;
import com.vmware.identity.util.TimePeriod;

/**
 * Insert your comment for SamlTokenSpecBuilder here
 */
final class SamlTokenSpecBuilder {
   private final static String UPN = "http://schemas.xmlsoap.org/claims/UPN";
   private final static String FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
   private final static String LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
   private final static String GROUPS = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   private final static String SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";
   private final static IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(SamlTokenSpecBuilder.class);

   private final HoKConditionsAnalyzer hokAnalyzer = new HoKConditionsAnalyzer();
   private final Collection<String> attributeNames = new HashSet<String>();
   private final DelegationParser delegationParser;
   private final TokenValidator tokenValidator;

   SamlTokenSpecBuilder(DelegationParser delegationParser,
      TokenValidator tokenValidator) {
      assert delegationParser != null;
      assert tokenValidator != null;

      this.attributeNames.add(FIRST_NAME);
      this.attributeNames.add(LAST_NAME);
      this.attributeNames.add(GROUPS);
      this.attributeNames.add(SUBJECT_TYPE);
      this.delegationParser = delegationParser;
      this.tokenValidator = tokenValidator;
   }

   /**
    * Builds token specification based on issue (incl. delegateTo and actAs)
    * requests.
    *
    * @param req
    * @param authResult
    * @return
    * @throws InvalidRequestException
    *            if the request is invalid or malformed, incl. if the delegate is invalid
    * @throws InvalidTimeRangeException
    *            if the requested time range is invalid
    * @throws NoSuchIdPException
    *            if the underlying idp cannot be found
    * @throws RequestFailedException
    *            if the delegate cannot be found due to a system error
    */
   SamlTokenSpec buildIssueTokenSpec(Request req, Result authResult)
      throws InvalidRequestException, InvalidTimeRangeException,
      RequestFailedException, NoSuchIdPException {
      assert req != null;
      assert authResult != null && authResult.completed();

      if ( authResult.getAuthnMethod() == Result.AuthnMethod.EXTERNAL_ASSERTION ) {
          validateNoDelegation(req);
          validateRequesterIsNotDelegate(req);
      } else {
          validateSignedActAsRequest(req);
          validateDelegationAmbiguity(req);
          validateActAsRequesterIsNotDelegate(req);
          validateActAsToken(req);
      }
      SolutionUser delegate = delegationParser.extractDelegate(req.getRst());
      final SamlTokenSpec.Confirmation confirmation = figureOutConfirmation(
         req,
         delegate,
         ( (authResult.getAuthnMethod() == Result.AuthnMethod.EXTERNAL_ASSERTION)
           ||
           (authResult.getAuthnMethod() == Result.AuthnMethod.ASSERTION)
         ) ? req.getSamlToken() : null
      );
      assert !actAsReq(req) || delegate == null;

      final ServerValidatableSamlToken templateToken = actAsReq(req) ? req.getActAsToken() : req
         .getSamlToken();
      final AssertionType templateAssertion = actAsReq(req) ? getActAsAssertion(req)
         : getRequestersAssertion(req);

      final PrincipalId delegateId = actAsReq(req) ? authResult
         .getPrincipalId() : ((delegate != null) ? delegate.getId() : null);

      final DelegationSpec delSpec = buildDelegationSpec(req.getRst(),
         templateToken, templateAssertion, delegateId);

      final RenewSpec renewSpec = RenewSpecExtractor.extractIssue(req.getRst(),
         templateAssertion);

      final Iterable<String> audience = new RSTAudience(req.getRst());

      final List<Advice> requestedAdvice = req.getRst().getAdviceSet() == null ? null
         : toSamlAuthorityAdvice(req.getRst().getAdviceSet());

      final List<Advice> presentAdvice = templateToken == null ? null
         : templateToken.getAdvice();

      return buildTokenSpec(req, authResult, confirmation, delSpec, renewSpec,
         audience, requestedAdvice, presentAdvice);
   }

   /**
    * Builds token specification based on renew requests.
    *
    * @param req
    * @param authResult
    * @return
    * @throws InvalidRequestException
    *            if the request is invalid or malformed
    * @throws InvalidTimeRangeException
    *            if the requested time range is invalid
    */
   SamlTokenSpec buildRenewTokenSpec(Request req, Result authResult)
      throws InvalidRequestException, InvalidTimeRangeException {
      assert req != null && req.getSamlToken() != null
         && req.getSignature() != null && req.getRst().getRenewTarget() != null
         && req.getRst().getRenewTarget().getAssertion() != null;
      assert authResult != null && authResult.completed();

      final ServerValidatableSamlToken renewToken = req.getSamlToken();
      if (renewToken.isExternal())
      {
          throw new InvalidRequestException("Cannot renew external assertion. It must be renewed at it's issuer.");
      }
      final AssertionType renewAssertion = getRenewAssertion(req);
      assert renewToken != null && renewAssertion != null;

      assert renewToken.getConfirmationCertificate() != null
         && renewToken.getConfirmationCertificate().equals(
            req.getSignature().getCertificate());
      final Confirmation confirmation = new Confirmation(req.getSignature()
         .getCertificate());

      final DelegationSpec delSpec = buildDelegationSpec(renewToken,
         renewAssertion, null, renewToken.isDelegable());

      final RenewSpec renewSpec = RenewSpecExtractor
         .extractRenew(renewAssertion);

      final Set<String> audience = renewToken.getAudience();

      final List<Advice> presentAdvice = renewToken
         .getAdvice();

      // advice list should be kept intact on renewal, so interpret it as
      // requested advice too. Otherwise, if no requested advice, token owners
      // will get no advice in the renewed token
      final List<Advice> requestedAdvice = presentAdvice;

      return buildTokenSpec(req, authResult, confirmation, delSpec, renewSpec,
         audience, requestedAdvice, presentAdvice);
   }

   private void validateSignedActAsRequest(Request req) {
      if (actAsReq(req) && req.getSignature() == null) {
         throw new InvalidSecurityHeaderException(
            "ActAs requests must be signed!");
      }
   }

   private void validateDelegationAmbiguity(Request req) {
      if (actAsReq(req) && req.getRst().getDelegateTo() != null) {
         throw new InvalidRequestException(
            "Ambiguity found: 'delegateTo' and 'actAs' used in same request!");
      }
   }

   private void validateNoDelegation(Request req) {
       if (actAsReq(req) || req.getRst().getDelegateTo() != null) {
           throw new InvalidRequestException(
              "Delegation is not supported for externally issued tokens.");
        }
   }

   private void validateActAsRequesterIsNotDelegate(Request req) {
      if (actAsReq(req) && req.getSamlToken() != null
         && !req.getSamlToken().getDelegationChain().isEmpty()) {
         throw new InvalidRequestException(
            "'ActAs' requester cannot be a delegate!");
      }
   }

   private void validateRequesterIsNotDelegate(Request req) {
       if ( req.getSamlToken() != null
          && !req.getSamlToken().getDelegationChain().isEmpty()) {
          throw new InvalidRequestException(
             "Requester authenticating by external saml token cannot be a delegate!");
       }
    }

   private void validateActAsToken(Request req) {
      if (actAsReq(req)) {
          ServerValidatableSamlToken actAsToken = req.getActAsToken();
         try {
            tokenValidator.validate(actAsToken);
         } catch (InvalidTokenException e) {
            throw new InvalidRequestException("'ActAs' token is invalid!");
         }
      }
   }

   private SamlTokenSpec buildTokenSpec(Request req, Result authResult,
      Confirmation confirmation, DelegationSpec delSpec, RenewSpec renewSpec,
      Iterable<String> audience, List<Advice> requestedAdvice,
      List<Advice> presentAdvice) throws InvalidRequestException,
      InvalidTimeRangeException {

      assert req != null && authResult != null && confirmation != null
         && delSpec != null && renewSpec != null && audience != null;

      final TimePeriod reqLifetime = extractLifetime(req);

      final AuthenticationData authN = convertAuthnResultToAuthnData(authResult);
      assert authN != null;

      final Builder builder = new SamlTokenSpec.Builder(reqLifetime,
         confirmation, authN, attributeNames).setDelegationSpec(delSpec)
         .setRenewSpec(renewSpec);
      setSignatureAlgorithm(req, builder);
      for (String audienceParty : audience) {
         builder.addAudience(audienceParty);
      }
      if (requestedAdvice != null) {
         for (Advice advice : requestedAdvice) {
            builder.addRequestedAdvice(advice);
         }
      }
      if (presentAdvice != null) {
         for (Advice advice : presentAdvice) {
            builder.addPresentAdvice(advice);
         }
      }
      return builder.createSpec();
   }

   private TimePeriod extractLifetime(Request req) {
      final LifetimeType reqLifetime = req.getRst().getLifetime();
      return (reqLifetime != null) ? LifetimeConvertor
         .fromRequestedTokenLifetime(reqLifetime) : null;
   }

   private void setSignatureAlgorithm(Request req, Builder builder) {
      assert req != null;

      final String requestedAlgoURI = req.getRst().getSignatureAlgorithm();
      if (requestedAlgoURI != null) {
         SignatureAlgorithm requestedAlgo = SignatureAlgorithm
            .getSignatureAlgorithmForURI(requestedAlgoURI);
         if (requestedAlgo != null) {
            builder.setSignatureAlgorithm(requestedAlgo);
         } else {
            logger.warn("Unknown signature algorithm '{}'has been requested!",
               requestedAlgoURI);
         }
      }
   }

   private SamlTokenSpec.Confirmation figureOutConfirmation(Request req,
      SolutionUser delegate, ServerValidatableSamlToken authSamlToken) {
      final X509Certificate delegateCertificate = (delegate == null) ? null
         : delegate.getCert();
      X509Certificate hokCertificate = hokAnalyzer.getSigningCertificate(req,
         delegateCertificate, authSamlToken);

      final Confirmation result = (hokCertificate == null) ? new Confirmation()
         : new Confirmation(hokCertificate);
      logger.debug("Confirmation will be {}", result);
      return result;
   }

   private DelegationSpec buildDelegationSpec(RequestSecurityTokenType rst,
      ServerValidatableSamlToken samlToken, AssertionType assertion, PrincipalId delegateId) {
      assert rst != null;
      assert (samlToken == null) == (assertion == null);
      // TODO [849937] pass in one token representation only

      Boolean reqIsDelegable = rst.isDelegatable();
      final boolean delegable = (reqIsDelegable != null) ? reqIsDelegable
         : false;

      return buildDelegationSpec(samlToken, assertion, delegateId, delegable);
   }

   private DelegationSpec buildDelegationSpec(ServerValidatableSamlToken token,
      AssertionType assertion, PrincipalId delegateId, boolean delegable) {

      return (token == null) ? new DelegationSpec(delegateId, delegable)
         : new DelegationSpec(delegateId, delegable,
            delegationParser.extractDelegationHistory(token, assertion));
   }

   private AuthenticationData convertAuthnResultToAuthnData(Result authnResult) {
      assert authnResult.completed();

      final AuthnMethod method;
      switch (authnResult.getAuthnMethod()) {
      case PASSWORD:
         method = AuthnMethod.PASSWORD;
         break;
      case KERBEROS:
         method = AuthnMethod.KERBEROS;
         break;
      case ASSERTION:
      case EXTERNAL_ASSERTION:
         method = AuthnMethod.ASSERTION;
         break;
      case DIG_SIG:
         method = AuthnMethod.XMLDSIG;
         break;
      case NTLM:
         method = AuthnMethod.NTLM;
         break;
      case SMARTCARD:
          method = AuthnMethod.SMARTCARD;
          break;
      case TIMESYNCTOKEN:
          method = AuthnMethod.TIMESYNCTOKEN;
          break;
      default:
         throw new IllegalStateException("Unknown authentication method");
      }
      return new AuthenticationData(authnResult.getPrincipalId(),
         authnResult.getAuthnInstant(), method, UPN);
   }

   private static AssertionType getActAsAssertion(Request req) {
      assert req != null && actAsReq(req);

      final ActAsType actAsElement = req.getRst().getActAs();
      assert actAsElement != null && actAsElement.getAssertion() != null : "Missing ActAs element in RST although there is a token already found there!";

      return actAsElement.getAssertion();
   }

   private static AssertionType getRequestersAssertion(Request req) {
      assert req != null;

      // TODO [849937] converge to using one internal representation of SAML
      // token, currently there are two: the client one and JAXB

      AssertionType result = null;
      if (req.getSamlToken() != null) {
         result = JAXBExtractor.extractFromSecurityHeader(req.getHeader(), AssertionType.class);
         assert result != null : "Missing Assertion element in request header although there is a token already found there!";
      }
      return result;
   }

   private static AssertionType getRenewAssertion(Request req) {
      assert req != null && req.getSamlToken() != null;

      final RenewTargetType renewElement = req.getRst().getRenewTarget();
      assert renewElement != null && renewElement.getAssertion() != null : "Missing RenewTarget element in RST although there is a token already found there!";

      return renewElement.getAssertion();
   }

   private static boolean actAsReq(Request req) {
      return req.getActAsToken() != null;
   }

   private static List<Advice> toSamlAuthorityAdvice(AdviceSetType adviceSet) {
      assert adviceSet != null;
      final List<AdviceType> advice = adviceSet.getAdvice();
      assert advice != null;

      final List<Advice> result = new ArrayList<Advice>(advice.size());
      for (AdviceType adviceMember : advice) {
         result.add(new Advice(adviceMember.getAdviceSource(),
            wstrustToSamlAuthorityAttributes(adviceMember.getAttribute())));
      }
      return result;
   }

   private static List<Attribute> wstrustToSamlAuthorityAttributes(
      List<AttributeType> attributes) {
      assert attributes != null;

      final List<Attribute> result = new ArrayList<Attribute>(attributes.size());
      for (AttributeType attribute : attributes) {
         result.add(new Attribute(attribute.getName(), attribute
            .getFriendlyName(), attribute.getAttributeValue()));
      }
      return result;
   }

}
