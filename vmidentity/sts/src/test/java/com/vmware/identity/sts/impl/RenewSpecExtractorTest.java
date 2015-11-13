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

import java.math.BigInteger;

import junit.framework.Assert;
import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;

import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewingType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;

import com.rsa.names._2009._12.std_ext.saml2.RenewRestrictionType;
import com.vmware.identity.saml.SamlTokenSpec;

/**
 * Insert your comment for RenewSpecExtractorTest here
 */
public final class RenewSpecExtractorTest {

   @Test
   public void testIssueDefaultRenewableDefaultCount() {
      testIssueInt(new SamlTokenSpec.RenewSpec(true), null, null);
      testIssueInt(new SamlTokenSpec.RenewSpec(true), null, new RenewingType());
   }

   @Test
   public void testIssueDefaultRenewableExplicitCount() {
      final int count = 4;
      final RenewRestrictionType renewRestriction = new RenewRestrictionType();
      renewRestriction.setCount(BigInteger.valueOf(count));

      testIssueInt(new SamlTokenSpec.RenewSpec(true, false, count),
         renewRestriction, null);
   }

   @Test
   public void testIssueRenewableDefaultCount() {
      testIssueRenewableDefaultCountInt(true);
      testIssueRenewableDefaultCountInt(false);
   }

   @Test
   public void testIssueRenewableExplicitCount() {
      final int count = 13;
      final RenewRestrictionType renewRestriction = new RenewRestrictionType();
      renewRestriction.setCount(BigInteger.valueOf(count));

      final RenewingType renewing = new RenewingType();
      renewing.setAllow(Boolean.TRUE);

      testIssueInt(new SamlTokenSpec.RenewSpec(true, false, count),
         renewRestriction, renewing);
   }

   @Test
   public void testRenewExplicitCount() {
      final int count = 4;
      final RenewRestrictionType renewRestriction = new RenewRestrictionType();
      renewRestriction.setCount(BigInteger.valueOf(count));

      testRenewInt(new SamlTokenSpec.RenewSpec(true, true, count),
         renewRestriction);
      testRenewInt(new SamlTokenSpec.RenewSpec(true, true, 0), null);
   }

   private void testIssueRenewableDefaultCountInt(boolean renewable) {
      final RenewingType renewing = new RenewingType();
      renewing.setAllow(Boolean.valueOf(renewable));

      testIssueInt(new SamlTokenSpec.RenewSpec(renewable), null, renewing);
      testIssueInt(new SamlTokenSpec.RenewSpec(renewable),
         new RenewRestrictionType(), renewing);
   }

   private void testIssueInt(SamlTokenSpec.RenewSpec exp,
      RenewRestrictionType renewRestriction, RenewingType renewing) {

      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      rst.setRenewing(renewing);

      AssertionType inputToken = newAssertion(renewRestriction);

      Assert
         .assertEquals(exp, RenewSpecExtractor.extractIssue(rst, inputToken));
   }

   private void testRenewInt(SamlTokenSpec.RenewSpec exp,
      RenewRestrictionType renewRestriction) {

      AssertionType inputToken = newAssertion(renewRestriction);

      Assert.assertEquals(exp, RenewSpecExtractor.extractRenew(inputToken));
   }

   private AssertionType newAssertion(RenewRestrictionType renewRestriction) {
      AssertionType inputToken = new AssertionType();
      final ConditionsType condition = new ConditionsType();
      if (renewRestriction != null) {
         condition.getConditionOrAudienceRestrictionOrOneTimeUseOrProxyRestriction().add(
            renewRestriction);
      }
      inputToken.setConditions(condition);
      return inputToken;
   }
}
