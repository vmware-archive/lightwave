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

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.NoSuchElementException;

import junit.framework.Assert;

import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;

/**
 * Contains unit tests for RSTAudience
 */
public final class RSTAudienceTest {

   @Test
   public void testNoAudience() {
      testInt(Collections.<String> emptySet());
   }

   @Test
   public void testOneAudience() {
      testInt(toMutableCollection(new String[] { "vim:sol1" }));
   }

   @Test
   public void testManyAudiences() {
      testInt(toMutableCollection(new String[] { "vim:sol1", "imv:los2",
         "mvi:osl3" }));
   }

   @Test
   public void testPrimaryAudience() {
      testInt("vim:sol1", toMutableCollection(new String[] {}));
   }

   @Test
   public void testPrimaryAndParticipantsAudience() {
      testInt("vim:sol0", toMutableCollection(new String[] { "vim:sol1", "imv:los2",
         "mvi:osl3" }));
   }

   private void testInt(Collection<String> audience) {
       testInt(null, audience);
   }
   private void testInt(String primary, Collection<String> audience) {
      final RequestSecurityTokenType rst = TestSetup.createRST(null, null,
         null, primary, audience);

      if(primary != null){
          audience.add(primary);
      }
      final Iterator<String> iterator = new RSTAudience(rst).iterator();
      Assert.assertNotNull(iterator);
      while (iterator.hasNext()) {
         final String audienceParty = iterator.next();
         Assert.assertTrue(audience.remove(audienceParty));
      }
      checkNoMoreElements(iterator);
      Assert.assertTrue(audience.isEmpty());
   }

   private void checkNoMoreElements(Iterator<String> iterator) {
      Assert.assertFalse(iterator.hasNext());
      try {
         iterator.next();
         Assert.fail();
      } catch (NoSuchElementException e) {
         // expected
      }
   }

   private Collection<String> toMutableCollection(String[] strings) {
      final Collection<String> result = new HashSet<String>(strings.length);
      for (String string : strings) {
         result.add(string);
      }
      return result;
   }

}
