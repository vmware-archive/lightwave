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

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;

/**
 * Contain tests for DelegationInfo
 */
public final class DelegationInfoTest {

   private static final String USER_DOMAIN = "domainX";

   @Test
   public void testLastDelegateNoDelagation() {
      Assert.assertNull(new DelegationInfo(new ArrayList<TokenDelegate>(), 10)
         .lastDelegate());
   }

   @Test
   public void testLastDelegateOneDelegate() {
      final List<TokenDelegate> delegationChain = new ArrayList<TokenDelegate>();
      final TokenDelegate delegate = newDelegate("user");
      delegationChain.add(delegate);

      Assert.assertEquals(delegate,
         new DelegationInfo(delegationChain, 10).lastDelegate());
   }

   @Test
   public void testLastDelegateManyDelegates() {
      final List<TokenDelegate> delegationChain = new ArrayList<TokenDelegate>();
      delegationChain.add(newDelegate("firstDelegate"));
      final TokenDelegate delegate = newDelegate("user");
      delegationChain.add(delegate);

      Assert.assertEquals(delegate,
         new DelegationInfo(delegationChain, 10).lastDelegate());
   }

   private TokenDelegate newDelegate(String userName) {
      return new TokenDelegate(new PrincipalId(userName, USER_DOMAIN),
         new Date());
   }

}
