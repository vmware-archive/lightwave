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
package com.vmware.vim.sso.client;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;

import org.junit.Test;

import com.vmware.vim.sso.PrincipalId;

public class PrincipalIdTest {

   private static final String NAME = "test";
   private static final String DOMAIN = "eng.vmware.com";

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNullDomain() {
      new PrincipalId(NAME, null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNoDomain() {
      new PrincipalId(NAME, "");
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNullName() {
      new PrincipalId(null, DOMAIN);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNoName() {
      new PrincipalId("", DOMAIN);
   }

   @Test
   public void testEquality() {
      PrincipalId user1 = new PrincipalId("u1", "d1");
      PrincipalId user2 = new PrincipalId("u1", "d1");

      assertSymmetricEquality(true, user1, user2);
      assertEquals(user1.hashCode(), user2.hashCode());

      PrincipalId localUser1 = new PrincipalId("local1", DOMAIN);
      PrincipalId localUser2 = new PrincipalId(localUser1.getName(), DOMAIN);
      assertSymmetricEquality(true, localUser1, localUser2);
      assertEquals(localUser1.hashCode(), localUser2.hashCode());
   }

   @Test
   public void testEqualityNegative() {
      PrincipalId user1 = new PrincipalId("u1", "d1");
      PrincipalId user2 = new PrincipalId("u1", "d2");
      PrincipalId user3 = new PrincipalId("u2", "d1");

      assertSymmetricEquality(false, user1, user2);
      assertSymmetricEquality(false, user1, user3);
      assertSymmetricEquality(false, user2, user3);

      PrincipalId localUser1 = new PrincipalId("local1", DOMAIN);
      PrincipalId localUser2 = new PrincipalId("local2", DOMAIN);
      PrincipalId localUser1a = new PrincipalId(localUser1.getName(), DOMAIN);

      assertSymmetricEquality(false, localUser1, localUser2);
      assertSymmetricEquality(true, localUser1, localUser1a);
   }

   private static void assertSymmetricEquality(boolean expectedEquality,
      Object a, Object b) {

      assert a != null;
      assert b != null;

      assertSame(expectedEquality, a.equals(b));
      assertSame(expectedEquality, b.equals(a));
   }
}
