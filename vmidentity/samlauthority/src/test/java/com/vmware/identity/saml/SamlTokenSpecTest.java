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
package com.vmware.identity.saml;

import static com.vmware.identity.saml.TestUtil.createDate;
import static org.easymock.EasyMock.createMock;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice.Attribute;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;
import com.vmware.identity.util.TimePeriod;

public class SamlTokenSpecTest {

   private static final PrincipalId DELEGATE1 = new PrincipalId("user1",
      "abc.com");
   private static final PrincipalId DELEGATE2 = new PrincipalId("user2",
      "abc.com");

   private static final List<TokenDelegate> DELEGATES1 = Arrays
      .asList(new TokenDelegate(DELEGATE2, new Date()));
   private static final List<TokenDelegate> DELEGATES2 = Arrays.asList(
      new TokenDelegate(DELEGATE2, new Date()), new TokenDelegate(DELEGATE1,
         new Date()));

   private static final Date DELEGATION_EXPIRES1 = new Date();
   private static final Date DELEGATION_EXPIRES2 = TestUtil.shiftDate(
      new Date(), 1);

   private static final DelegationHistory HISTORY1 = new DelegationHistory(
      DELEGATE1, DELEGATES1, 2, DELEGATION_EXPIRES1);

   private static final DelegationHistory HISTORY2 = new DelegationHistory(
      DELEGATE1, DELEGATES2, 1, DELEGATION_EXPIRES1);

   private static final Confirmation BEARER_CONF = new SamlTokenSpec.Confirmation();

   private static final SamlTokenSpec.AuthenticationData authnData = new SamlTokenSpec.AuthenticationData(
      DELEGATE1, createDate(2011, 5, 22, 10, 1, 5, 12),
      SamlTokenSpec.AuthenticationData.AuthnMethod.KERBEROS, "idAttr");

   private static final Collection<String> attributeNames = Arrays
      .asList(new String[] { "attr1", "attr2" });

   private static final String AUDIENCE1 = "ITP:ddf";
   private static final String AUDIENCE2 = "ITP:dfd";

   @Test
   public void testCreateSamlTokenOK() {
      final TimePeriod lifespan = new TimePeriod(createDate(2011, 5, 22, 10, 0,
         5, 12), createDate(2011, 5, 22, 10, 10, 5, 12));
      final SamlTokenSpec spec = new SamlTokenSpec.Builder(lifespan,
         BEARER_CONF, authnData, attributeNames).setSignatureAlgorithm(
         SignatureAlgorithm.RSA_SHA256).createSpec();

      assertNotNull("Saml spec cannot be null", spec);

      assertNotNull(spec.getConfirmationData());
      assertEquals(BEARER_CONF, spec.getConfirmationData());

      assertNotNull(spec.getLifespan());
      assertEquals(lifespan, spec.getLifespan());

      assertNotNull(spec.getAuthenticationData().getPrincipalId());
      assertEquals(authnData, spec.getAuthenticationData());

      assertEmpty(spec.getAudience());
      assertEmpty(spec.getRequestedAdvice());
      assertEmpty(spec.getPresentAdvice());

      assertEquals(SignatureAlgorithm.RSA_SHA256, spec.getSignatureAlgorithm());
   }

   @Test
   public void testCreateAudienceSpec() {
      final SamlTokenSpec.Builder builder = newBuilder();
      builder.addAudience(AUDIENCE1);
      builder.addAudience(AUDIENCE2);
      final SamlTokenSpec spec = builder.createSpec();

      final Set<String> audiencesExp = new HashSet<String>();
      audiencesExp.add(AUDIENCE1);
      audiencesExp.add(AUDIENCE2);
      assertEquals(audiencesExp, spec.getAudience());

      assertEmpty(spec.getRequestedAdvice());
      assertEmpty(spec.getPresentAdvice());

      assertNull(spec.getSignatureAlgorithm());
   }

   @Test
   public void testCreateAdviceSpec() {
      final Attribute attr = new Attribute("nameURI", null,
         Arrays.asList("attrvalue1"));
      final Advice advice1 = new Advice("sourceURI1", Arrays.asList(attr));
      final Advice advice2 = new Advice("sourceURI2", Arrays.asList(attr));
      final Advice advice3 = new Advice("sourceURI3", Arrays.asList(attr));

      final SamlTokenSpec.Builder builder = newBuilder();
      builder.addRequestedAdvice(advice1);
      builder.addRequestedAdvice(advice2);
      builder.addPresentAdvice(advice3);
      builder.addPresentAdvice(advice1);

      final SamlTokenSpec spec = builder.createSpec();

      assertEquals(Arrays.asList(advice1, advice2), spec.getRequestedAdvice());
      assertEquals(Arrays.asList(advice3, advice1), spec.getPresentAdvice());
   }

   @Test
   public void testSpecEqualsNOK() {
      final SamlTokenSpec spec1 = newBuilder().createSpec();

      final Attribute attr = new Attribute("nameURI", null,
         Arrays.asList("attrvalue1"));
      final Advice advice1 = new Advice("sourceURI1", Arrays.asList(attr));
      final Advice advice2 = new Advice("sourceURI2", Arrays.asList(attr));
      final Advice advice3 = new Advice("sourceURI3", Arrays.asList(attr));
      final SamlTokenSpec spec2 = newBuilder().addRequestedAdvice(advice1)
         .createSpec();

      final SamlTokenSpec spec3 = newBuilder().addRequestedAdvice(advice2)
         .createSpec();

      final SamlTokenSpec spec4 = newBuilder().addPresentAdvice(advice3)
         .createSpec();

      final SamlTokenSpec spec5 = newBuilder().addPresentAdvice(advice1)
         .createSpec();

      final SamlTokenSpec spec6 = newBuilder().addAudience(AUDIENCE1)
         .createSpec();

      final SamlTokenSpec spec7 = newBuilder().addAudience(AUDIENCE2)
         .createSpec();

      final SamlTokenSpec[] specs = new SamlTokenSpec[] { spec1, spec2, spec3,
         spec4, spec5, spec6, spec7 };
      for (SamlTokenSpec specA : specs) {
         for (SamlTokenSpec specB : specs) {
            if (specA == specB) {
               assertEquals(specA, specB);
            } else {
               assertFalse(
                  String.format("Failure: %s is equal to %s", specA, specB),
                  specA.equals(specB));
            }
         }
      }
   }

   private Builder newBuilder() {
      return new SamlTokenSpec.Builder(null, BEARER_CONF, authnData,
         attributeNames);
   }

   @Test
   public void testCreateSamlTokenSpecWithNoLifespan() {
      SamlTokenSpec spec = newBuilder().createSpec();
      assertNotNull(spec.getLifespan());
      assertNull(spec.getLifespan().getStartTime());
      assertNull(spec.getLifespan().getEndTime());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateSamlTokenWithNoConfirmation() {
      TimePeriod lifespan = createMock(TimePeriod.class);

      new SamlTokenSpec.Builder(lifespan, null, authnData, attributeNames)
         .createSpec();
   }

   @Test
   public void testRequesterIsTokenOwner_NoTemplateToken() {
      final SamlTokenSpec spec = newBuilder().createSpec();
      Assert.assertTrue(spec.requesterIsTokenOwner());
   }

   @Test
   public void testRequesterIsTokenOwner_TemplateToken() {
      final SamlTokenSpec spec = newBuilder().setDelegationSpec(
         new DelegationSpec(null, false, new DelegationHistory(authnData
            .getPrincipalId(), DELEGATES1, 2, DELEGATION_EXPIRES1)))
         .createSpec();
      Assert.assertTrue(spec.requesterIsTokenOwner());
   }

   @Test
   public void testRequesterIsNotTokenOwner() {
      final AuthenticationData authn = authnData;
      final PrincipalId tokenOwner = DELEGATE2;
      assertFalse("Token owner should not match the requester!",
         tokenOwner.equals(authn.getPrincipalId()));

      final SamlTokenSpec spec = new SamlTokenSpec.Builder(null, BEARER_CONF,
         authn, attributeNames).setDelegationSpec(
         new DelegationSpec(null, false, new DelegationHistory(tokenOwner,
            DELEGATES1, 2, DELEGATION_EXPIRES1))).createSpec();
      Assert.assertFalse(spec.requesterIsTokenOwner());
   }

   @Test
   public void testCreateAuthDataOK() {
      final SamlTokenSpec.AuthenticationData newAuthnData = new SamlTokenSpec.AuthenticationData(
         authnData.getPrincipalId(), authnData.getAuthnTime(),
         authnData.getAuthnMethod(), authnData.getIdentityAttrName());
      assertEquals(authnData.getPrincipalId(), newAuthnData.getPrincipalId());
      assertEquals(authnData.getAuthnTime(), newAuthnData.getAuthnTime());
      assertEquals(authnData.getAuthnMethod(), newAuthnData.getAuthnMethod());
      assertEquals(authnData.getIdentityAttrName(),
         newAuthnData.getIdentityAttrName());
      assertEquals(newAuthnData, newAuthnData);
      assertEquals(authnData, newAuthnData);
      assertEquals(authnData.hashCode(), newAuthnData.hashCode());

      assertEquals(
         authnData,
         new SamlTokenSpec.AuthenticationData(authnData.getPrincipalId(),
            authnData.getAuthnTime(), authnData.getAuthnMethod(), authnData
               .getIdentityAttrName()));
   }

   @Test
   public void testNotEqualAuthData() {
      final SamlTokenSpec.AuthenticationData newAuthnData = new SamlTokenSpec.AuthenticationData(
         authnData.getPrincipalId(), authnData.getAuthnTime(),
         authnData.getAuthnMethod(), authnData.getIdentityAttrName(),
         "sess1234", authnData.getSessionExpireDate());
      assertFalse(authnData.equals(newAuthnData));
      assertFalse(authnData.hashCode() == newAuthnData.hashCode());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateAuthDataWithNoPrincipalId() {

      new SamlTokenSpec.AuthenticationData(null, authnData.getAuthnTime(),
         authnData.getAuthnMethod(), authnData.getIdentityAttrName());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateAuthDataWithNoAuthenticationTime() {

      new SamlTokenSpec.AuthenticationData(authnData.getPrincipalId(), null,
         authnData.getAuthnMethod(), authnData.getIdentityAttrName());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateAuthDataWithNoAuthMethod() {

      new SamlTokenSpec.AuthenticationData(authnData.getPrincipalId(),
         authnData.getAuthnTime(), null, authnData.getIdentityAttrName());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateAuthDataWithNoIdAttrName() {

      new SamlTokenSpec.AuthenticationData(authnData.getPrincipalId(),
         authnData.getAuthnTime(), authnData.getAuthnMethod(), null);
   }

   @Test()
   public void testCreateSamlBearerConfirmation() {
      checkConfirmation(BEARER_CONF, SamlTokenSpec.ConfirmationType.BEARER,
         null, null, null);
   }

   @Test()
   public void testCreateSamlBearerConfirmationWithAttr() {
      checkConfirmation(new SamlTokenSpec.Confirmation(null, null),
         SamlTokenSpec.ConfirmationType.BEARER, null, null, null);
      final String inResponseTo = "abc:d:rwe";
      final String recipient = "foo eng/";
      checkConfirmation(
         new SamlTokenSpec.Confirmation(inResponseTo, recipient),
         SamlTokenSpec.ConfirmationType.BEARER, inResponseTo, recipient, null);

      assertEquals(BEARER_CONF, new SamlTokenSpec.Confirmation(null, null));
   }

   @Test()
   public void testCreateSamlHolderOfKeyConfirmation() {
      X509Certificate cert = createMock(X509Certificate.class);
      SamlTokenSpec.Confirmation confirmation = new SamlTokenSpec.Confirmation(
         cert);

      checkConfirmation(confirmation,
         SamlTokenSpec.ConfirmationType.HOLDER_OF_KEY, null, null,
         confirmation.getCertificate());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateSamlHolderOfKeyConfirmationWithNoCertificate() {
      new SamlTokenSpec.Confirmation(null);
   }

   @Test()
   public void testCreateSamlLifespan() {
      Date startDate = createDate(2011, 5, 22, 10, 0, 5, 12);
      Date endDate = createDate(2011, 5, 22, 10, 10, 5, 12);
      TimePeriod lifespan = new TimePeriod(startDate, endDate);
      assertNotNull(lifespan);

      assertNotNull(lifespan.getStartTime());
      assertEquals(startDate, lifespan.getStartTime());

      assertNotNull(lifespan.getEndTime());
      assertEquals(endDate, lifespan.getEndTime());
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateSamlLifespanWithEndDateBeforeStartDate() {
      Date startDate = createDate(2011, 5, 22, 10, 0, 5, 12);
      Date endDate = createDate(2001, 5, 22, 10, 10, 5, 12); // Different year
      new TimePeriod(startDate, endDate);
   }

   @Test
   public void testContains() {
      Date startDate = createDate(2012, 1, 23, 10, 0, 5, 0);
      Date endDate = createDate(2012, 1, 23, 10, 10, 5, 12);
      TimePeriod lifespan = new TimePeriod(startDate, endDate);

      Assert.assertFalse(lifespan.contains(TestUtil.shiftDate(startDate, -1)));
      Assert.assertTrue(lifespan.contains(startDate));
      Assert.assertTrue(lifespan.contains(TestUtil.shiftDate(startDate, 1)));
      Assert.assertTrue(lifespan.contains(TestUtil.shiftDate(endDate, -1)));
      Assert.assertFalse(lifespan.contains(endDate));
      Assert.assertFalse(lifespan.contains(TestUtil.shiftDate(endDate, 1)));

   }

   @Test
   public void testCreateDelegationSpecOK() {
      testCreateDelegationSpecOKInt(null, false, null);
      testCreateDelegationSpecOKInt(null, true, null);
      testCreateDelegationSpecOKInt(DELEGATE1, false, null);
      testCreateDelegationSpecOKInt(DELEGATE1, true, null);
      testCreateDelegationSpecOKInt(null, false, HISTORY1);
      testCreateDelegationSpecOKInt(null, true, HISTORY1);
      testCreateDelegationSpecOKInt(DELEGATE1, false, HISTORY2);
      testCreateDelegationSpecOKInt(DELEGATE1, true, HISTORY2);
   }

   @Test
   public void testNotEqualDelegationSpec() {
      testNotEqualDelegationSpecInt(null, false, null, DELEGATE1, false, null);
      testNotEqualDelegationSpecInt(DELEGATE2, false, null, DELEGATE1, false,
         null);
      testNotEqualDelegationSpecInt(DELEGATE2, true, null, DELEGATE1, true,
         null);
      testNotEqualDelegationSpecInt(DELEGATE2, false, HISTORY1, DELEGATE1,
         false, HISTORY1);
      testNotEqualDelegationSpecInt(DELEGATE2, true, HISTORY2, DELEGATE1, true,
         HISTORY2);

      testNotEqualDelegationSpecInt(null, false, null, null, true, null);
      testNotEqualDelegationSpecInt(DELEGATE1, false, null, DELEGATE1, true,
         null);
      testNotEqualDelegationSpecInt(DELEGATE2, false, HISTORY1, DELEGATE2,
         true, HISTORY1);

      testNotEqualDelegationSpecInt(null, false, null, null, false, HISTORY1);
      testNotEqualDelegationSpecInt(null, false, HISTORY2, null, false,
         HISTORY1);
      testNotEqualDelegationSpecInt(DELEGATE1, false, HISTORY2, DELEGATE1,
         false, HISTORY1);
      testNotEqualDelegationSpecInt(DELEGATE2, true, HISTORY2, DELEGATE2, true,
         HISTORY1);
   }

   @Test
   public void testCreateDelegationHistoryOK() {
      final PrincipalId subject = DELEGATE2;
      final Date delegationTokenExpires = DELEGATION_EXPIRES1;
      final int remainingDelegations = 0;
      final DelegationHistory history = new DelegationHistory(subject,
         DELEGATES1, remainingDelegations, delegationTokenExpires);

      assertEquals(subject, history.getTokenSubject());
      assertEquals(DELEGATES1, history.getCurrentDelegateList());
      assertEquals(remainingDelegations, history.getRemainingDelegations());
      assertEquals(delegationTokenExpires, history.getDelegatedTokenExpires());
      assertEquals(history, history);
      assertEquals(history.hashCode(), history.hashCode());

      final DelegationHistory historyOther = new DelegationHistory(subject,
         DELEGATES1, remainingDelegations, delegationTokenExpires);
      assertEquals(historyOther, history);
      assertEquals(historyOther.hashCode(), history.hashCode());
   }

   @Test
   public void testCreateDelegationHistoryFail() {
      testCreateDelegationHistoryFailInt(null, DELEGATES1, 1,
         DELEGATION_EXPIRES1);
      testCreateDelegationHistoryFailInt(DELEGATE1, null, 1,
         DELEGATION_EXPIRES1);
      testCreateDelegationHistoryFailInt(DELEGATE1, DELEGATES1, -1,
         DELEGATION_EXPIRES1);
      testCreateDelegationHistoryFailInt(DELEGATE1, DELEGATES1, 1, null);
   }

   @Test
   public void testNotEqualDelegationHistory() {
      testNotEqualDelegationHistoryInt(DELEGATE1, DELEGATES1, 1,
         DELEGATION_EXPIRES1, DELEGATE2, DELEGATES1, 1, DELEGATION_EXPIRES1);
      testNotEqualDelegationHistoryInt(DELEGATE1, DELEGATES1, 1,
         DELEGATION_EXPIRES1, DELEGATE1, DELEGATES2, 1, DELEGATION_EXPIRES1);
      testNotEqualDelegationHistoryInt(DELEGATE1, DELEGATES1, 1,
         DELEGATION_EXPIRES1, DELEGATE1, DELEGATES1, 2, DELEGATION_EXPIRES1);
      testNotEqualDelegationHistoryInt(DELEGATE1, DELEGATES1, 1,
         DELEGATION_EXPIRES1, DELEGATE1, DELEGATES1, 1, DELEGATION_EXPIRES2);
   }

   @Test
   public void testCreateRenewSpecOK() {
      final boolean renewable = false;
      final SamlTokenSpec.RenewSpec renewSpec = new SamlTokenSpec.RenewSpec(
         renewable);
      assertEquals(renewSpec, renewSpec);
      assertEquals(renewSpec, new SamlTokenSpec.RenewSpec(renewable));
      assertFalse(renewSpec.equals(new SamlTokenSpec.RenewSpec(!renewable)));
   }

   @Test
   public void testCreateRenewSpecFullOK() {
      final boolean renewable = false;
      final boolean renew = true;
      final int count = 12;
      final SamlTokenSpec.RenewSpec renewSpec = new SamlTokenSpec.RenewSpec(
         renewable, renew, count);
      assertEquals(renewSpec, renewSpec);
      assertEquals(renewSpec, new SamlTokenSpec.RenewSpec(renewable, renew,
         count));
      assertFalse(renewSpec.equals(new SamlTokenSpec.RenewSpec(!renewable)));
      assertFalse(renewSpec.equals(new SamlTokenSpec.RenewSpec(!renewable,
         renew, count)));
      assertFalse(renewSpec.equals(new SamlTokenSpec.RenewSpec(renewable,
         !renew, count)));
      assertFalse(renewSpec.equals(new SamlTokenSpec.RenewSpec(renewable,
         renew, count + 1)));

      assertEquals(new SamlTokenSpec.RenewSpec(renewable, false, 0),
         new SamlTokenSpec.RenewSpec(renewable));
   }

   private void testCreateDelegationSpecOKInt(PrincipalId delegate,
      boolean delegable, DelegationHistory history) {

      final DelegationSpec spec = newDelegSpec(delegate, delegable, history);

      assertEquals(delegate, spec.getDelegate());
      assertEquals(delegable, spec.isDelegable());
      assertEquals(history, spec.getDelegationHistory());
      assertEquals(spec, spec);
      assertEquals(spec.hashCode(), spec.hashCode());

      final DelegationSpec specOther = newDelegSpec(delegate, delegable,
         history);
      assertEquals(specOther, spec);
      assertEquals(specOther.hashCode(), spec.hashCode());
   }

   private void testNotEqualDelegationSpecInt(PrincipalId delegate1,
      boolean delegable1, DelegationHistory history1, PrincipalId delegate2,
      boolean delegable2, DelegationHistory history2) {

      assertFalse(newDelegSpec(delegate1, delegable1, history1).equals(
         newDelegSpec(delegate2, delegable2, history2)));

   }

   private void testNotEqualDelegationHistoryInt(PrincipalId subject1,
      List<TokenDelegate> delegates1, int remainingDelegations1,
      Date delegatedTokenExpires1, PrincipalId subject2,
      List<TokenDelegate> delegates2, int remainingDelegations2,
      Date delegatedTokenExpires2) {

      assertFalse(new DelegationHistory(subject1, delegates1,
         remainingDelegations1, delegatedTokenExpires1)
         .equals(new DelegationHistory(subject2, delegates2,
            remainingDelegations2, delegatedTokenExpires2)));

   }

   private void testCreateDelegationHistoryFailInt(PrincipalId subject,
      List<TokenDelegate> delegates, int remainingDelegations,
      Date delegatedTokenExpires) {

      try {
         new DelegationHistory(subject, delegates, remainingDelegations,
            delegatedTokenExpires);
         Assert.fail();
      } catch (IllegalArgumentException e) {
         // expected
      }

   }

   private DelegationSpec newDelegSpec(PrincipalId delegate, boolean delegable,
      DelegationHistory history) {

      return (history == null) ? new DelegationSpec(delegate, delegable)
         : new DelegationSpec(delegate, delegable, history);
   }

   private void checkConfirmation(SamlTokenSpec.Confirmation confirmation,
      ConfirmationType expType, String inResponseTo, String recipient,
      X509Certificate certificate) {

      assertEquals(expType, confirmation.getType());
      assertEquals(inResponseTo, confirmation.getInResponseTo());
      assertEquals(recipient, confirmation.getRecipient());
      assertEquals(certificate, confirmation.getCertificate());
   }

   private <R> void assertEmpty(Collection<R> collection) {
      assertNotNull(collection);
      assertTrue(collection.isEmpty());
   }

}
