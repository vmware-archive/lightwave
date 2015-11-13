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

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.saml.RenewException;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;

/**
 * Insert your comment for RenewRestrictorTest here
 */
public final class RenewRestrictorTest {

   private static final int INITIAL_RENEW_COUNT = 8;
   private static final int REMAINING_RENEWS = 5;
   private static final int ONE_RENEW = 1;
   private static final int NO_RENEWS = 0;

   private static final boolean OWNER = true;
   private static final boolean DELEGATE = !OWNER;

   private static final boolean RENEWABLE = true;
   private static final boolean NOTRENEWABLE = !RENEWABLE;

   private static final boolean RENEW = true;
   private static final boolean EXCHANGE = !RENEW;

   @Test
   public void testNewRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(INITIAL_RENEW_COUNT,
         restrictor.processRequest(new RenewSpec(RENEWABLE)));
   }

   @Test
   public void testNewNotRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(NO_RENEWS,
         restrictor.processRequest(new RenewSpec(NOTRENEWABLE)));
   }

   @Test
   public void testExchangeNotDelegatedTokenForRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(INITIAL_RENEW_COUNT, restrictor
         .processRequest(new RenewSpec(RENEWABLE, EXCHANGE, REMAINING_RENEWS)));
      Assert.assertEquals(INITIAL_RENEW_COUNT, restrictor
         .processRequest(new RenewSpec(RENEWABLE, EXCHANGE, NO_RENEWS)));
      Assert.assertEquals(INITIAL_RENEW_COUNT,
         restrictor.processRequest(new RenewSpec(RENEWABLE)));
   }

   @Test
   public void testExchangeNotDelegatedTokenForNotRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, EXCHANGE, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, EXCHANGE, NO_RENEWS)));
      Assert.assertEquals(NO_RENEWS,
         restrictor.processRequest(new RenewSpec(NOTRENEWABLE)));
   }

   @Test
   public void testExchangeDelegatedTokenForRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(DELEGATE,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(REMAINING_RENEWS, restrictor
         .processRequest(new RenewSpec(RENEWABLE, EXCHANGE, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         RENEWABLE, EXCHANGE, NO_RENEWS)));
   }

   @Test
   public void testExchangeDelegatedTokenForNotRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(DELEGATE,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, EXCHANGE, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, EXCHANGE, NO_RENEWS)));
   }

   @Test
   public void testRenewNotDelegatedTokenForRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(INITIAL_RENEW_COUNT, restrictor
         .processRequest(new RenewSpec(RENEWABLE, RENEW, REMAINING_RENEWS)));
      Assert.assertEquals(INITIAL_RENEW_COUNT,
         restrictor.processRequest(new RenewSpec(RENEWABLE, RENEW, NO_RENEWS)));
      Assert.assertEquals(INITIAL_RENEW_COUNT,
         restrictor.processRequest(new RenewSpec(RENEWABLE)));
   }

   @Test
   public void testRenewNotDelegatedTokenForNotRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(OWNER,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, RENEW, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, RENEW, NO_RENEWS)));
      Assert.assertEquals(NO_RENEWS,
         restrictor.processRequest(new RenewSpec(NOTRENEWABLE)));
   }

   @Test
   public void testRenewDelegatedTokenForRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(DELEGATE,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(REMAINING_RENEWS - 1, restrictor
         .processRequest(new RenewSpec(RENEWABLE, RENEW, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS,
         restrictor.processRequest(new RenewSpec(RENEWABLE, RENEW, ONE_RENEW)));
      testNoMoreRenewsInt(restrictor, RENEWABLE);
   }

   @Test
   public void testRenewDelegatedTokenForNotRenewableToken() {
      final RenewRestrictor restrictor = new RenewRestrictor(DELEGATE,
         INITIAL_RENEW_COUNT);

      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, RENEW, REMAINING_RENEWS)));
      Assert.assertEquals(NO_RENEWS, restrictor.processRequest(new RenewSpec(
         NOTRENEWABLE, RENEW, ONE_RENEW)));
      testNoMoreRenewsInt(restrictor, NOTRENEWABLE);
   }

   private void testNoMoreRenewsInt(RenewRestrictor restrictor,
      boolean renewable) {
      try {
         restrictor.processRequest(new RenewSpec(renewable, RENEW, NO_RENEWS));
         Assert.fail();
      } catch (RenewException e) {
         // expected
      }
   }

}
