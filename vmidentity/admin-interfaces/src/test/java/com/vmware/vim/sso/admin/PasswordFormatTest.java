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

package com.vmware.vim.sso.admin;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.vim.sso.admin.PasswordFormat.AlphabeticRestriction;
import com.vmware.vim.sso.admin.PasswordFormat.LengthRestriction;

public final class PasswordFormatTest {

   private final static int VALID_MANY = 1000;

   @Test(expected=IllegalArgumentException.class)
   public void testConflictingLength() {
      new LengthRestriction(2,1);
   }
   @Test
   public void testStrictLength() {
      new LengthRestriction(1,1);
   }

   @Test
   public void testConflictingMaxLengthAndMinAlphabetic() {
      AlphabeticRestriction ar = new AlphabeticRestriction(1,0,0);
      LengthRestriction lr = new LengthRestriction(0,2);
      try {
         new PasswordFormat(lr, ar, 1,1,VALID_MANY);
         Assert.fail("Expected exception was not thrown");
      } catch (IllegalArgumentException e) {
      /* expected */}
   }
   @Test
   public void testConformingMaxLengthAndMinAlphabetic() {
      AlphabeticRestriction ar = new AlphabeticRestriction(1,0,0);
      LengthRestriction lr = new LengthRestriction(0,3);
      new PasswordFormat(lr, ar, 1,1,VALID_MANY);
   }

   @Test(expected=IllegalArgumentException.class)
   public void testConflictingMinAlphabeticAndMinLowerCasePlusUpperCase() {
      new AlphabeticRestriction(1,1,1);
   }
   @Test
   public void testConformingMinAlphabeticAndMinLowerCasePlusUpperCase () {
      new AlphabeticRestriction(2,1,1);
   }

   @Test
   public void testZeroMaxIdenticalAdjacentCharacters () {
      AlphabeticRestriction ar = new AlphabeticRestriction(0,0,0);
      LengthRestriction lr = new LengthRestriction(0,VALID_MANY);
      try {
         new PasswordFormat(lr, ar, 0,0,0);
         Assert.fail("Expected exception was not thrown");
      } catch (IllegalArgumentException e) {
      /* expected */}
   }
}
