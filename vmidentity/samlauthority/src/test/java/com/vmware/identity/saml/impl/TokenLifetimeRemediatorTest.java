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
package com.vmware.identity.saml.impl;

import static com.vmware.identity.saml.TestUtil.DEFAULT_CLOCK_TOLERANCE_2_MINUTES;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.security.PrivateKey;
import java.security.Provider;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;
import com.vmware.identity.saml.UnsupportedTokenLifetimeException;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.util.TimePeriod;

public class TokenLifetimeRemediatorTest {

   private static final PrincipalId TOKEN_SUBJECT = new PrincipalId(
      "tokenSubject", "abv.com");
   private static final PrincipalId TOKEN_PRESENTER = new PrincipalId(
      "tokenPresenter", "abv.com");
   private TokenLifetimeRemediator remediator;
   private X509Certificate cert;
   private PrivateKey privateKey;
   private SignInfo signInfo;
   private X509Certificate hokCertificate;

   private TokenRestrictions tokenRestrictions;
   private final long sixMinutes = 6 * 60 * 1000;
   private final long oneMillisecond = 1;

   private Date startTime;
   private Date issueInstantTime;
   private Date authnTime;
   private Date endTime;
   private Date sixthMinuteAfterStartNotAfter;
   private Date sixthMinuteAfterStartNotOnOrAfter;
   private Date sixthMinuteAfterIssueInstantNotAfter;
   private Date sixthMinuteAfterIssueInstantNotOnOrAfter;
   private Date twentiethMinuteAfterStart;

   @Before
   public void init() throws Exception {

      Provider securityProvider = null;
      privateKey = createMock(PrivateKey.class);
      cert = createMock(X509Certificate.class);

      List<Certificate> certificatesList = new ArrayList<Certificate>();
      certificatesList.add(cert);

      CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
      CertPath certPath = certFactory.generateCertPath(certificatesList);

      signInfo = new SignInfo(privateKey, certPath, securityProvider);

      Calendar calendar = new GregorianCalendar();
      startTime = calendar.getTime();
      calendar.add(Calendar.MINUTE, 1);
      authnTime = calendar.getTime();
      issueInstantTime = calendar.getTime();
      calendar.add(Calendar.MINUTE, 5);
      sixthMinuteAfterStartNotAfter = calendar.getTime();
      sixthMinuteAfterStartNotOnOrAfter = new Date(
         sixthMinuteAfterStartNotAfter.getTime() + 1);
      calendar.add(Calendar.MINUTE, 1);
      sixthMinuteAfterIssueInstantNotAfter = calendar.getTime();
      sixthMinuteAfterIssueInstantNotOnOrAfter = new Date(
         sixthMinuteAfterIssueInstantNotAfter.getTime() + 1);
      calendar.add(Calendar.MINUTE, 3);
      endTime = calendar.getTime();
      calendar.add(Calendar.MINUTE, 10);
      twentiethMinuteAfterStart = calendar.getTime();
      // total lifespan is 10 mins
      hokCertificate = null;

      initializeRemediator();
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeSpecEndTime() {
      testRemediate_SpecEndTime(new TimePeriod(null, endTime),
         issueInstantTime, endTime);
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeMaxEndTime() {
      testRemediate_MaxEndTime(new TimePeriod(null, endTime), issueInstantTime,
         sixthMinuteAfterIssueInstantNotAfter);
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeSigningCertEndTime() {
      testRemediate_SigningCertEndTime(new TimePeriod(null, endTime),
         issueInstantTime, sixthMinuteAfterIssueInstantNotOnOrAfter);
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeHokCertEndTime() {
      testRemediate_HoKCertEndTime(new TimePeriod(null, endTime),
         issueInstantTime, sixthMinuteAfterIssueInstantNotOnOrAfter);
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeOwnerAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(true,
         new TimePeriod(null, endTime), issueInstantTime, endTime);
   }

   @Test
   public void testRemediateLeftOpenedDesiredTimeValidity_IssueInstantStartTimeDelegateAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(false, new TimePeriod(null,
         endTime), issueInstantTime, sixthMinuteAfterIssueInstantNotAfter);
   }

   @Test
   public void testRemediateRightOpenedDesiredTimeValidity_SpecStartTimeMaxEndTime() {
      testRemediate_MaxEndTime(new TimePeriod(startTime, null), startTime,
         sixthMinuteAfterStartNotAfter);
   }

   @Test
   public void testRemediateRightOpenedDesiredTimeValidity_SpecStartTimeSigningCertEndTime() {
      testRemediate_SigningCertEndTime(new TimePeriod(startTime, null),
         startTime, sixthMinuteAfterStartNotOnOrAfter);
   }

   @Test
   public void testRemediateRightOpenedDesiredTimeValidity_SpecStartTimeHokCertEndTime() {
      testRemediate_HoKCertEndTime(new TimePeriod(startTime, null), startTime,
         sixthMinuteAfterStartNotOnOrAfter);
   }

   @Test
   public void testRemediateRightOpenedDesiredTimeValidity_SpecStartTimeOwnerAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(
         true,
         new TimePeriod(startTime, null),
         startTime,
         new Date(startTime.getTime()
            + tokenRestrictions.getMaximumHoKTokenLifetime()));
   }

   @Test
   public void testRemediateRightOpenedDesiredTimeValidity_SpecStartTimeDelegateAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(false, new TimePeriod(startTime,
         null), startTime, sixthMinuteAfterStartNotAfter);
   }

   @Test
   public void testRemediateOpenedDesiredTimeValidity_IssueInstantStartTimeMaxEndTime() {
      testRemediate_MaxEndTime(new TimePeriod(null, null), issueInstantTime,
         sixthMinuteAfterIssueInstantNotAfter);
   }

   @Test
   public void testRemediateOpenedDesiredTimeValidity_IssueInstantStartTimeSigningCertEndTime() {
      testRemediate_SigningCertEndTime(new TimePeriod(null, null),
         issueInstantTime, sixthMinuteAfterIssueInstantNotOnOrAfter);
   }

   @Test
   public void testRemediateOpenedDesiredTimeValidity_IssueInstantStartTimeHokCertEndTime() {
      testRemediate_HoKCertEndTime(new TimePeriod(null, null),
         issueInstantTime, sixthMinuteAfterIssueInstantNotOnOrAfter);
   }

   @Test
   public void testRemediateOpenedDesiredTimeValidity_IssueInstantStartTimeOwnerAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(true, new TimePeriod(null, null),
         issueInstantTime, new Date(issueInstantTime.getTime()
            + tokenRestrictions.getMaximumHoKTokenLifetime()));
   }

   @Test
   public void testRemediateOpenedDesiredTimeValidity_IssueInstantStartTimeDelegateAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(false, new TimePeriod(null, null),
         issueInstantTime, sixthMinuteAfterIssueInstantNotAfter);
   }

   @Test
   public void testRemediateClosedDesiredTimeValidity_SpecStartTimeMaxEndTime() {
      testRemediate_MaxEndTime(null, startTime, sixthMinuteAfterStartNotAfter);
   }

   @Test
   public void testRemediateValidity_SigningCertEndTime() {
      testRemediate_SigningCertEndTime(null, startTime,
         sixthMinuteAfterStartNotOnOrAfter);
   }

   @Test
   public void testRemediateValidity_SpecEndTime() {
      testRemediate_SpecEndTime(null, startTime, endTime);
   }

   @Test
   public void testRemediateValidity_HoKCertEndTime() {
      testRemediate_HoKCertEndTime(null, startTime,
         sixthMinuteAfterStartNotOnOrAfter);
   }

   @Test
   public void testRemediateValidity_OwnerAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(true, null, startTime,
         sixthMinuteAfterStartNotAfter);
   }

   @Test
   public void testRemediateValidity_DelegateAssertionEndTime() {
      testRemediate_DelegateAssertionEndTime(false, null, startTime,
         sixthMinuteAfterStartNotAfter);
   }

   @Test
   public void testNotRemediateWithDelegateAssertionEndTime_RenewRequested() {
      testNotRemediateWithDelegateAssertionEndTime_RenewRequestedInt(false);
   }

   @Test
   public void testNotRemediateWithDelegateAssertionEndTime_RenewAndDelegateRequested() {
      testNotRemediateWithDelegateAssertionEndTime_RenewRequestedInt(true);
   }

   @Test
   public void testRemediateClosedDesiredTimeValidity_StartTimeDiffersFromIssueInstant() {
      // this is smaller than time betw. specStart and issueInstant
      int clockTolerance_20_seconds = 20 * 1000;
      tokenRestrictions = TokenCreationUtil.createTokenRestrictions(sixMinutes,
         oneMillisecond);
      remediator = new TokenLifetimeRemediator(signInfo, tokenRestrictions,
         clockTolerance_20_seconds);
      testUnsupportedTokenLifetimeInt(startTime, endTime, issueInstantTime);
   }

   @Test
   public void testSigningCertNotYetValid() {
      Assert.assertTrue(issueInstantTime
         .before(sixthMinuteAfterIssueInstantNotAfter));
      Assert.assertTrue(sixthMinuteAfterIssueInstantNotAfter
         .before(twentiethMinuteAfterStart));

      testUnsupportedTokenLifetimeInt(sixthMinuteAfterIssueInstantNotAfter,
         twentiethMinuteAfterStart, issueInstantTime);
   }

   @Test
   public void testSigningCertExpired() {
      Assert.assertTrue(startTime.before(endTime));
      Assert.assertTrue(endTime.before(twentiethMinuteAfterStart));

      testUnsupportedTokenLifetimeInt(startTime, endTime,
         twentiethMinuteAfterStart);
   }

   private void testNotRemediateWithDelegateAssertionEndTime_RenewRequestedInt(
      boolean delegateReq) {
      X509Certificate hokCertificate = createMock(X509Certificate.class);
      expect(hokCertificate.getNotAfter()).andReturn(endTime).anyTimes();
      replay(hokCertificate);

      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      if (delegateReq) {
         TokenCreationUtil.createDelegationChain(builder, TOKEN_SUBJECT,
            sixthMinuteAfterStartNotAfter);
      } else {
         TokenCreationUtil.setNoDelegateSpec(builder, TOKEN_SUBJECT,
            sixthMinuteAfterStartNotAfter);
      }
      builder.setRenewSpec(new RenewSpec(false, true, 1));
      remediateAndCheckReturnedValidity(builder.createSpec(), startTime,
         endTime, endTime);
      verify(hokCertificate);
   }

   private void testRemediate_MaxEndTime(TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime) {
      tokenRestrictions = TokenCreationUtil.createTokenRestrictions(sixMinutes,
         oneMillisecond);
      remediator = new TokenLifetimeRemediator(signInfo, tokenRestrictions,
         DEFAULT_CLOCK_TOLERANCE_2_MINUTES);

      Date signingCertEndTime = twentiethMinuteAfterStart;
      remediateAndCheck(timeValidity, expectedStartTime, expectedEndTime,
         signingCertEndTime);
   }

   private void testRemediate_SigningCertEndTime(TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime) {

      Date signingCertEndTime = (timeValidity != null && timeValidity
         .getStartTime() == null) ? sixthMinuteAfterIssueInstantNotAfter
         : sixthMinuteAfterStartNotAfter;

      remediateAndCheck(timeValidity, expectedStartTime, expectedEndTime,
         signingCertEndTime);
   }

   private void testRemediate_SpecEndTime(TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime) {

      Date signingCertEndTime = twentiethMinuteAfterStart;
      remediateAndCheck(timeValidity, expectedStartTime, expectedEndTime,
         signingCertEndTime);
   }

   private void testRemediate_HoKCertEndTime(TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime) {
      Date hokCertEndTime = (timeValidity != null && timeValidity
         .getStartTime() == null) ? sixthMinuteAfterIssueInstantNotAfter
         : sixthMinuteAfterStartNotAfter;

      testRemediate_HoKCertOrDelegateAssertionEndTime(false, timeValidity,
         expectedStartTime, expectedEndTime, hokCertEndTime, null);
   }

   private void testRemediate_DelegateAssertionEndTime(
      boolean tokenPresenterIsOwner, TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime) {
      Date delegateAssertionEndTime = (timeValidity != null && timeValidity
         .getStartTime() == null) ? sixthMinuteAfterIssueInstantNotAfter
         : sixthMinuteAfterStartNotAfter;

      testRemediate_HoKCertOrDelegateAssertionEndTime(tokenPresenterIsOwner,
         timeValidity, expectedStartTime, expectedEndTime,
         twentiethMinuteAfterStart, delegateAssertionEndTime);
   }

   private void remediateAndCheck(TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime, Date signingCertEndTime) {
      SamlTokenSpec spec = timeValidity != null ? TokenCreationUtil.createSpec(
         null, authnTime, timeValidity) : TokenCreationUtil.createSpec(null,
         authnTime, new TimePeriod(startTime, endTime));
      remediateAndCheckReturnedValidity(spec, expectedStartTime,
         expectedEndTime, signingCertEndTime);
   }

   private void testRemediate_HoKCertOrDelegateAssertionEndTime(
      boolean tokenPresenterIsOwner, TimePeriod timeValidity,
      Date expectedStartTime, Date expectedEndTime, Date hokCertEndTime,
      Date delegateAssertionEndTime) {
      hokCertificate = createMock(X509Certificate.class);
      expect(hokCertificate.getNotAfter()).andReturn(hokCertEndTime).anyTimes();
      replay(hokCertificate);

      SamlTokenSpec spec = createSpec(tokenPresenterIsOwner, timeValidity,
         delegateAssertionEndTime);

      Date signingCertEndTime = twentiethMinuteAfterStart;
      remediateAndCheckReturnedValidity(spec, expectedStartTime,
         expectedEndTime, signingCertEndTime);
      verify(hokCertificate);
   }

   private SamlTokenSpec createSpec(boolean tokenPresenterIsOwner,
      TimePeriod timeValidity, Date delegateAssertionEndTime) {

      final PrincipalId tokenOwner = TOKEN_SUBJECT;
      final PrincipalId tokenPresenter = tokenPresenterIsOwner ? tokenOwner
         : TOKEN_PRESENTER;
      Builder builder = timeValidity != null ? TokenCreationUtil
         .createSpecBuilder(tokenPresenter, hokCertificate, authnTime,
            timeValidity) : TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      if (delegateAssertionEndTime != null) {
         TokenCreationUtil.createDelegationChain(builder, tokenOwner,
            delegateAssertionEndTime);
      }
      return builder.createSpec();
   }

   private void testUnsupportedTokenLifetimeInt(Date signingCertStart,
      Date signingCertEnd, Date issueInstant) {
      expect(cert.getNotBefore()).andReturn(signingCertStart).anyTimes();
      expect(cert.getNotAfter()).andReturn(signingCertEnd).anyTimes();
      replay(cert, privateKey);

      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));
      try {
         remediator.remediateTokenValidity(spec, issueInstant);
         fail();
      } catch (UnsupportedTokenLifetimeException e) {
         // expected
      }

      verify(cert, privateKey);
   }

   private TimePeriod remediateAndCheckReturnedValidity(SamlTokenSpec spec,
      Date expectedStartTime, Date expectedEndTime, Date signingCertEndTime) {
      expect(cert.getNotBefore()).andReturn(expectedStartTime).anyTimes();
      expect(cert.getNotAfter()).andReturn(signingCertEndTime).anyTimes();
      replay(cert, privateKey);

      try {
         return checkRemediated(spec, expectedStartTime, expectedEndTime);
      } finally {
         verify(cert, privateKey);
      }
   }

   private TimePeriod checkRemediated(SamlTokenSpec spec,
      Date expectedStartTime, Date expectedEndTime) {

      TimePeriod tokenValidity = remediator.remediateTokenValidity(spec,
         issueInstantTime);

      assertEquals(expectedStartTime, tokenValidity.getStartTime());
      assertEquals(expectedEndTime, tokenValidity.getEndTime());
      return tokenValidity;
   }

   private void initializeRemediator() {
      tokenRestrictions = TokenCreationUtil.createDefaultTokenRestrictions();
      remediator = new TokenLifetimeRemediator(signInfo, tokenRestrictions,
         DEFAULT_CLOCK_TOLERANCE_2_MINUTES);
   }

}
