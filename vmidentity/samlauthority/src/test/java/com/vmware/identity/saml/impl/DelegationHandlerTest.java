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

import java.util.Arrays;
import java.util.Date;
import java.util.List;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.DelegationException;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;

public class DelegationHandlerTest {

   private static final int INITIAL_TOKEN_DELEGATION_COUNT = 10;
   private static final int REMAINING_DELEGATIONS = 9;
   private static final int NO_DELEGATION_COUNT = 0;
   private static final PrincipalId TOKEN_OWNER = new PrincipalId("tokenOwner",
      "vmware.com");
   private static final PrincipalId DELEGATE = new PrincipalId("delegate",
      "vmware.com");
   private static final PrincipalId NO_DELEGATE = null;
   private static final List<TokenDelegate> CURRENT_DEL_LIST = Arrays
      .asList(new TokenDelegate(DELEGATE, new Date()));
   private static final DelegationHistory DEF_HISTORY = new DelegationHistory(
      TOKEN_OWNER, CURRENT_DEL_LIST, REMAINING_DELEGATIONS, new Date());
   private static final DelegationSpec NO_DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC = new DelegationSpec(
      NO_DELEGATE, false);
   private static final DelegationSpec DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC = new DelegationSpec(
      DELEGATE, false);
   private static final DelegationSpec DELEGATE_NON_DELEGABLE_CHAIN_SPEC = new DelegationSpec(
      DELEGATE, false, DEF_HISTORY);
   private static final DelegationSpec NO_DELEGATE_NON_DELEGABLE_CHAIN_SPEC = new DelegationSpec(
      NO_DELEGATE, false, DEF_HISTORY);
   private static final DelegationSpec NO_DELEGATE_DELEGABLE_NO_CHAIN_SPEC = new DelegationSpec(
      NO_DELEGATE, true);
   private static final DelegationSpec DELEGATE_DELEGABLE_NO_CHAIN_SPEC = new DelegationSpec(
      DELEGATE, true);
   private static final DelegationSpec DELEGATE_DELEGABLE_CHAIN_SPEC = new DelegationSpec(
      DELEGATE, true, DEF_HISTORY);
   private static final DelegationSpec NO_DELEGATE_DELEGABLE_CHAIN_SPEC = new DelegationSpec(
      NO_DELEGATE, true, DEF_HISTORY);

   private final DelegationHandler delegationHandler = new DelegationHandler(
      INITIAL_TOKEN_DELEGATION_COUNT, new PrincipalAttributeExtractorImpl());

   @Test
   public void testDelegationCount_NonDelegableToken_PresenterIsOwner() {
      checkDelegationCount(NO_DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
      checkDelegationCount(DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
      checkDelegationCount(true, DELEGATE_NON_DELEGABLE_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
      checkDelegationCount(true, NO_DELEGATE_NON_DELEGABLE_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
   }

   @Test
   public void testDelegationCount_NonDelegableToken_PresenterIsNotOwner() {
      checkDelegationCount(false, DELEGATE_NON_DELEGABLE_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
      checkDelegationCount(false, NO_DELEGATE_NON_DELEGABLE_CHAIN_SPEC,
         NO_DELEGATION_COUNT);
   }

   @Test
   public void testDelegationCount_DelegableToken_PresenterIsOwner() {
      checkDelegationCount(NO_DELEGATE_DELEGABLE_NO_CHAIN_SPEC,
         INITIAL_TOKEN_DELEGATION_COUNT);
      checkDelegationCount(DELEGATE_DELEGABLE_NO_CHAIN_SPEC,
         INITIAL_TOKEN_DELEGATION_COUNT);
      checkDelegationCount(true, DELEGATE_DELEGABLE_CHAIN_SPEC,
         INITIAL_TOKEN_DELEGATION_COUNT);
      checkDelegationCount(true, NO_DELEGATE_DELEGABLE_CHAIN_SPEC,
         INITIAL_TOKEN_DELEGATION_COUNT);
   }

   @Test
   public void testDelegationCount_DelegableToken_PresenterIsNotOwner() {
      checkDelegationCount(false, DELEGATE_DELEGABLE_CHAIN_SPEC,
         REMAINING_DELEGATIONS - 1);
      checkDelegationCount(false, NO_DELEGATE_DELEGABLE_CHAIN_SPEC,
         REMAINING_DELEGATIONS);
   }

   @Test
   public void testDelegateList() {
      checkDelegateList(NO_DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC, 0);
      checkDelegateList(DELEGATE_NON_DELEGABLE_NO_CHAIN_SPEC, 1);
      checkDelegateList(DELEGATE_NON_DELEGABLE_CHAIN_SPEC, DEF_HISTORY
         .getCurrentDelegateList().size() + 1);
      checkDelegateList(NO_DELEGATE_NON_DELEGABLE_CHAIN_SPEC, DEF_HISTORY
         .getCurrentDelegateList().size());
   }

   @Test
   public void testDelegateNoRemainingDelegationsLeft() {
      DelegationSpec spec = new DelegationSpec(DELEGATE, false,
         new DelegationHistory(TOKEN_OWNER, CURRENT_DEL_LIST, 0, new Date()));

      testDelegationNOK(delegationHandler, spec);
   }

   @Test
   public void testDelegateNotActive() {
      final PrincipalAttributesExtractor principalAttributesExtractor = new PrincipalAttributeExtractorImpl(
         new String[0], false);
      final DelegationHandler notActiveDelegateHandler = new DelegationHandler(
         INITIAL_TOKEN_DELEGATION_COUNT, principalAttributesExtractor);
      final DelegationSpec spec = new DelegationSpec(DELEGATE, true,
         new DelegationHistory(TOKEN_OWNER, CURRENT_DEL_LIST, 10, new Date()));

      testDelegationNOK(notActiveDelegateHandler, spec);
   }

   private void testDelegationNOK(DelegationHandler delegationHandler,
      DelegationSpec spec) {
      try {
         delegationHandler.getDelegationInfo(false, spec);
         Assert.fail();
      } catch (DelegationException e) {
         // expected
      }
   }

   private void checkDelegationCount(DelegationSpec spec, int expectedCount) {
      assert spec != null && spec.getDelegationHistory() == null : "Use this "
         + "overload only when issue a completely new token, no template token!";
      checkDelegationCount(true, spec, expectedCount);
   }

   private void checkDelegationCount(boolean requesterIsTokenOwner,
      DelegationSpec spec, int expectedCount) {

      Assert.assertEquals(expectedCount,
         delegationHandler.getDelegationInfo(requesterIsTokenOwner, spec)
            .getDelegationCount());
   }

   private void checkDelegateList(DelegationSpec spec, int expectedCount) {
      Assert.assertEquals(
         expectedCount,
         delegationHandler
            .getDelegationInfo(spec.getDelegationHistory() == null, spec)
            .getDelegationChain().size());
   }
}
