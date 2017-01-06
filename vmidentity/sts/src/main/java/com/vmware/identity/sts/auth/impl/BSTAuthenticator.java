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

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.SystemException;
import com.vmware.identity.sts.util.JAXBExtractor;
import com.vmware.identity.util.TimePeriod;

/**
 * Insert your comment for BSLAuthenticator here
 */
final class BSTAuthenticator implements Authenticator {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(BSTAuthenticator.class);

   private final PrincipalDiscovery principalDiscovery;

   BSTAuthenticator(PrincipalDiscovery principalDiscovery) {
      assert principalDiscovery != null;

      this.principalDiscovery = principalDiscovery;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public Result authenticate(Request req)
      throws AuthenticationFailedException, NoSuchIdPException,
      RequestFailedException {
      assert req != null && req.getHeader() != null;
      log.trace("Authenticating by BST...");

      final BinarySecurityTokenType bsToken = JAXBExtractor.extractFromSecurityHeader(req.getHeader(), BinarySecurityTokenType.class);
      if (bsToken == null || bsToken.getValue() == null) {
         log.debug("No BST found!");
         return null;
      }

      final Signature signature = req.getSignature();
      if (signature == null) {
         throw new InvalidCredentialsException("Signature is required!");
      }

      if (!signature.getLocation().equals(Request.CertificateLocation.BST)) {
         throw new InvalidCredentialsException(
            "The provided BST is not used to sign the request!");
      }

      final Date authnInstant = new Date();
      final SolutionUser solutionUser = doAuthenticate(
         signature.getCertificate(), authnInstant);
      assert solutionUser == null || solutionUser.getId() != null;

      final Result result = (solutionUser == null) ? null : new Result(
         solutionUser.getId(), authnInstant, AuthnMethod.DIG_SIG);

      if (result == null) {
         log.debug("Not authenticated since no user has been found!");
      } else {
         log.debug("Authenticated principal '{}' at '{}'",
            result.getPrincipalId(), result.getAuthnInstant());
      }
      return result;
   }

   private SolutionUser doAuthenticate(X509Certificate signingCert,
      Date authnInstant) throws InvalidCredentialsException,
      NoSuchIdPException, RequestFailedException {
      final String subjectDN = getSubjectDN(signingCert);

      log.trace("Looking up an user by subjectDN {} ...", subjectDN);
      final SolutionUser solutionUser = findSolutionUser(subjectDN);

      if (solutionUser != null) {
         X509Certificate solutionUserCert = solutionUser.getCert();
         if (solutionUser.getId() == null || solutionUserCert == null) {
            throw new InvalidCredentialsException(
               "Solution user found but is invalid!");
         }

         checkActiveSolutionUser(subjectDN, solutionUser);
         checkValidCertificate(solutionUserCert, authnInstant);
         checkMatchingCertificate(signingCert, solutionUserCert);
      }

      return solutionUser;
   }

   private SolutionUser findSolutionUser(String subjectDN)
      throws NoSuchIdPException, RequestFailedException {
      assert subjectDN != null;
      final SolutionUser solutionUser;
      try {
         solutionUser = principalDiscovery.findSolutionUser(subjectDN);
      } catch (SystemException e) {
         throw new RequestFailedException(
            "Error occured looking for solution user", e);
      }
      return solutionUser;
   }

   private void checkActiveSolutionUser(String subjectDN,
      SolutionUser solutionUser) throws InvalidCredentialsException {
      if (solutionUser.isDisabled()) {
         throw new InvalidCredentialsException("Solution user " + subjectDN
            + " is disabled.");
      }
   }

   private void checkValidCertificate(X509Certificate certificate,
      Date authnInstant) throws InvalidCredentialsException {
      assert certificate != null;
      assert authnInstant != null;

      TimePeriod certValidity = new TimePeriod(certificate.getNotBefore(),
         new Date(certificate.getNotAfter().getTime() + 1));
      if (!certValidity.contains(authnInstant)) {
         throw new InvalidCredentialsException(
            "Solution user cert is not valid.");
      }
   }

   private void checkMatchingCertificate(X509Certificate signingCert,
      X509Certificate solutionUserCert) throws InvalidCredentialsException {
      if (!solutionUserCert.equals(signingCert)) {
         throw new InvalidCredentialsException(
            "Solution user's certificate does not match the one in BST!");
      }
   }

   private String getSubjectDN(X509Certificate signingCert)
      throws InvalidCredentialsException {
      // getSubjectX500Principal() always returns != null
      final String subjectDN = signingCert.getSubjectX500Principal().getName();
      assert subjectDN != null;

      if (subjectDN.isEmpty()) {
         throw new InvalidCredentialsException(
            "The certificate subjectDN is empty!");
      }
      return subjectDN;
   }

}
