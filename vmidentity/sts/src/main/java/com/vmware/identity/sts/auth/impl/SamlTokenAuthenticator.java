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
package com.vmware.identity.sts.auth.impl;

import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SamlTokenDelegate;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.InvalidSignatureException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;

final class SamlTokenAuthenticator implements Authenticator {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(SamlTokenAuthenticator.class);

   private final TokenValidator validator;

   SamlTokenAuthenticator(TokenValidator validator) {
      assert validator != null;

      this.validator = validator;
   }

   /**
    * @throws InvalidSignatureException
    *            when the token in the request cannot be validated against a set
    *            of trusted certificates.
    */
   @Override
   public Result authenticate(Request req) throws InvalidSignatureException,
      AuthenticationFailedException, UnsupportedSecurityTokenException,
      RequestFailedException {
      assert req != null;
      log.debug("Authenticating by Assertion...");

      if (req.getSamlToken() == null) {
         log.debug("No assertion found in the request.");
         return null;
      }

      final Result authenticationResult = doAuthenticate(req);

      return authenticationResult;
   }

   private Result doAuthenticate(Request req)
      throws InvalidSignatureException, InvalidCredentialsException,
      UnsupportedSecurityTokenException, RequestFailedException {
      assert req != null && req.getSamlToken() != null;

      final ServerValidatableSamlToken token = req.getSamlToken();
      // token validation should happen before any other get methods on it
      final ServerValidatableSamlToken samlToken = validate(token);

      // we will only support authentication by external tokens when
      // subject's existence could be confirmed as a regular user in sso2
      if ( ( samlToken.isExternal() )
           &&
           ( ( samlToken.getSubject().subjectValidation() != SubjectValidation.Regular)
             ||
             ( samlToken.getSubject().subjectUpn() == null )
           )
         ) {
          throw new InvalidCredentialsException( "External SAML token's subject could not be found." );
      }


      final X509Certificate confirmationCertificate = token
         .getConfirmationCertificate();
      if ( (samlToken.isExternal() == false) && (confirmationCertificate == null)) {
         throw new UnsupportedSecurityTokenException(
            "Only HoK tokens are supported for authentication!");
      }

      final Signature signature = req.getSignature();
      if ( (confirmationCertificate != null) && (signature == null) ) {
          throw new InvalidCredentialsException("Signature is required!");
      }
      if ( (confirmationCertificate != null) && (!confirmationCertificate.equals(signature.getCertificate()))) {
         throw new InvalidCredentialsException(
            "The message signature was not computed with private key corresponding to the assertion confirmation certificate.");
      }

      final PrincipalId authenticatedPrincipal = determineAuthPrincipal(samlToken);
      final Date authnInstanceTime = new Date();
      log.debug("Authenticated(by assertion) principal: {} at time: {}",
         authenticatedPrincipal, authnInstanceTime);

      return new Result(
          authenticatedPrincipal, authnInstanceTime,
          ((samlToken.isExternal() == false) ? AuthnMethod.ASSERTION : AuthnMethod.EXTERNAL_ASSERTION)
      );
   }

   private ServerValidatableSamlToken validate(ServerValidatableSamlToken token)
      throws InvalidSignatureException, InvalidCredentialsException,
      RequestFailedException {
      assert token != null;

      final ServerValidatableSamlToken validationResult;
      try {
          validationResult = validator.validate(token);
      } catch (com.vmware.identity.saml.InvalidSignatureException e) {
         throw new InvalidSignatureException(e);
      } catch (InvalidTokenException e) {
         // TODO uncomment this when the ssoClient is submitted
         // throw new InvalidCredentialsException("Invalid assertion", e);
         throw new InvalidSignatureException("Invalid assertion", e);
      } catch (SystemException e) {
         throw new RequestFailedException(
            "Unexpected error during token validation", e);
      }
      return validationResult;
   }

   private PrincipalId determineAuthPrincipal(ServerValidatableSamlToken samlToken) {
      assert samlToken != null;

      final List<SamlTokenDelegate> delegationChain = samlToken.getDelegationChain();
      if ( ( samlToken.isExternal() ) && (delegationChain.isEmpty() == false) )
      {
          throw new UnsupportedSecurityTokenException( "Delegated external tokens are not supported for authentication." );
      }

      final PrincipalId result =
              (delegationChain.isEmpty())
              ?
              samlToken.getSubject().subjectUpn()
              :
              delegationChain.get(delegationChain.size() - 1).subject().subjectUpn();
      return result;
   }
}
