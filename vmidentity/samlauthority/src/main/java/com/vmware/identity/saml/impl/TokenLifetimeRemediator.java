/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.saml.impl;

import java.security.cert.X509Certificate;
import java.util.Date;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.UnsupportedTokenLifetimeException;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.util.TimePeriod;

/**
 * This class is responsible for getting all token restrictions and clock
 * tolerance and calculate token validity on their basis.
 */
final class TokenLifetimeRemediator {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(TokenLifetimeRemediator.class);

   private final SignInfo signInfo;
   private final TokenRestrictions tokenRestrictions;
   private final long clockTolerance;

   public TokenLifetimeRemediator(SignInfo signInfo,
      TokenRestrictions tokenRestrictions, long clockTolerance) {
      assert signInfo != null;
      assert tokenRestrictions != null;
      assert clockTolerance >= 0;

      this.signInfo = signInfo;
      this.tokenRestrictions = tokenRestrictions;
      this.clockTolerance = clockTolerance;
   }

   /**
    * Validates token start time against issue instant time and clock tolerance.
    * If no start time it takes issue instant time. If no end time it takes
    * start time + max token lifetime. In addition, checks whether the signing
    * certificate is valid at the issue instant time.
    *
    * @param spec
    *           Cannot be null.
    * @param issueInstantTime
    *           Cannot be null.
    * @param clockTolerance
    *           Non-negative
    * @throws UnsupportedTokenLifetimeException
    *            when there is a requested token start lifetime which is in the
    *            past or future, or when the signing certificate is not valid at
    *            the user authentication instant
    * @return time period for which a token could be issued
    */
   TimePeriod remediateTokenValidity(SamlTokenSpec spec, Date issueInstantTime) {
      assert spec != null;
      assert issueInstantTime != null;

      validateSigningCert(issueInstantTime);

      Date tokenStartTime = getTokenStartTime(spec, issueInstantTime);
      log.debug("Token start time will be {}", tokenStartTime);

      Date tokenEndTime = getTokenEndTime(tokenStartTime, spec);
      log.debug("Token end time will be {}", tokenEndTime);

      return new TimePeriod(tokenStartTime, tokenEndTime);
   }

   /**
    * Validates if STS certificate can be used for signing.
    *
    * @param issueInstant
    */
   private void validateSigningCert(Date issueInstant) {
      assert issueInstant != null;

      // validate only first certificate in the chain because this means that
      // all certificates in certificate chain are not expired
      final X509Certificate signingCert = signInfo.getSigningCertificate();
      final TimePeriod signingCertValidity = new TimePeriod(
         signingCert.getNotBefore(), getExpiresTime(signingCert.getNotAfter()));

      if (!signingCertValidity.contains(issueInstant)) {
         throw new UnsupportedTokenLifetimeException(String.format(
            "Signing certificate is not valid at %s, cert validity: %s",
            issueInstant, signingCertValidity));
      }
   }

   /**
    * @param tokenStartTime
    *           request start time with already taken into account clock
    *           tolerance
    * @param spec
    * @param tokeRestrictions
    * @return remediated token end time taking into account startTime, signing
    *         certificate end time, request end time and max token lifetime
    */
   private Date getTokenEndTime(Date tokenStartTime, SamlTokenSpec spec) {
      assert tokenStartTime != null;
      assert spec != null;

      Confirmation confirmation = spec.getConfirmationData();
      Date result = getMaxAllowedTokenEndTime(tokenStartTime, confirmation);
      log.trace("maxAllowedToken end time is {}", result);

      final Date desiredEndTime = spec.getLifespan().getEndTime();
      result = (desiredEndTime != null) ? getMoreRecentTime(result,
         desiredEndTime) : result;
      log.trace("End time set to {} after limitimg with desired time {}",
         result, desiredEndTime);

      final Date signingCertNotAfter = signInfo.getSigningCertificate()
         .getNotAfter();
      log.trace("Signing certificate end time: {}", signingCertNotAfter);
      Date signingCertExpires = getExpiresTime(signingCertNotAfter);
      result = getMoreRecentTime(result, signingCertExpires);
      log.trace(
         "End time set to {} after limitimg with signing certificate expiration time {}",
         result, signingCertExpires);

      result = remediateIfHoKToken(confirmation, result);

      if (spec.getRenewSpec().isRenew()) {
         log.trace("Not going to limit with token expiration time since renew is being requested.");
      } else {
         result = remediateWithTemplateToken(spec.getDelegationSpec(),
            spec.requesterIsTokenOwner(), result);
      }

      log.debug("Remediated token end time is {}", result);
      return result;
   }

   private Date getMaxAllowedTokenEndTime(Date startTime,
      Confirmation confirmation) {
      assert startTime != null;
      assert confirmation != null;

      long maximumTokenLifetime = (confirmation.getType() == ConfirmationType.BEARER) ? tokenRestrictions
         .getMaximumBearerTokenLifetime() : tokenRestrictions
         .getMaximumHoKTokenLifetime();
      return new Date(startTime.getTime() + maximumTokenLifetime);
   }

   private Date getMoreRecentTime(Date firstTime, Date secondTime) {
      return (firstTime.before(secondTime)) ? firstTime : secondTime;
   }

   private Date remediateIfHoKToken(Confirmation confirmation, Date result) {
      assert confirmation != null;
      assert result != null;

      if (confirmation.getType() == ConfirmationType.HOLDER_OF_KEY) {
         Date hokCertNotAfter = confirmation.getCertificate().getNotAfter();
         log.info("There is a HoK confirmation certificate with end time: {}",
            hokCertNotAfter);
         Date hokCertExpires = getExpiresTime(hokCertNotAfter);
         result = getMoreRecentTime(hokCertExpires, result);
         log.trace(
            "End time set to {} after limitimg with HoK certificate expiration time {}",
            result, hokCertExpires);

      }
      return result;
   }

   private Date remediateWithTemplateToken(DelegationSpec delegationSpec,
      boolean requesterIsTokenOwner, Date result) {
      assert delegationSpec != null;
      assert result != null;

      if (!requesterIsTokenOwner) {
         assert delegationSpec.getDelegationHistory() != null;
         result = getMoreRecentTime(result, delegationSpec
            .getDelegationHistory().getDelegatedTokenExpires());
         log.trace(
            "End time set to {} after limitimg with template token expiration time {}",
            result, delegationSpec.getDelegationHistory()
               .getDelegatedTokenExpires());
      }
      return result;
   }

   /**
    * @param certNotAfter
    *           end time of certificate validity. The certificate is valid until
    *           this time inclusive.
    * @return time at which the certificate is no longer valid, expired.
    */
   private Date getExpiresTime(Date certNotAfter) {
      assert certNotAfter != null;

      return new Date(certNotAfter.getTime() + 1);
   }

   /**
    * @param spec
    * @param issueInstantTime
    * @return desired token start time if there is such in spec,
    *         issueInstantTime otherwise.
    */
   private Date getTokenStartTime(SamlTokenSpec spec, Date issueInstantTime) {
      assert spec != null;
      assert issueInstantTime != null;

      Date tokenStartTime = issueInstantTime;

      TimePeriod lifespan = spec.getLifespan();
      assert lifespan != null;
      if (lifespan.getStartTime() != null) {
         tokenStartTime = lifespan.getStartTime();
         log.debug("Token start time will get value from rst:Lifetime");
         validateStartTimeWithTolerance(tokenStartTime, issueInstantTime);
      }
      return tokenStartTime;
   }

   /**
    * Validates if tokenStartTime and issueInstantTime are having difference no
    * longer than clockTolerance
    *
    * @param tokenStartTime
    * @param issueInstantTime
    * @param clockTolerance
    */
   private void validateStartTimeWithTolerance(Date tokenStartTime,
      Date issueInstantTime) {
      assert tokenStartTime != null;
      assert issueInstantTime != null;

      TimePeriod desiredStartTimeRange = TimePeriod.expand(new TimePeriod(
         tokenStartTime, 1), clockTolerance);

      if (!desiredStartTimeRange.contains(issueInstantTime)) {
         throw new UnsupportedTokenLifetimeException(
            "The requested token start time differs from the issue instant "
               + "more than the acceptable deviation (clock tolerance) of "
               + clockTolerance + " ms. Requested token start time="
               + tokenStartTime + ", issue instant time=" + issueInstantTime
               + ". This might be due to a clock skew problem.");
      }
   }

}
