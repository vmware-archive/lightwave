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

public class DomainTest {

   @Test
   public void testEqualsHashcode() {
      verifyEqualsHashcode(new Domain("a"), null, false);
      verifyEqualsHashcode(new Domain("b"), new Domain("a"), false);
      verifyEqualsHashcode(new Domain("aB"), new Domain("Ab"), true);

      verifyEqualsHashcode(new Domain("aB", "b"), new Domain("Ab", null), false);
      verifyEqualsHashcode(new Domain("aB", "bC"), new Domain("Ab", "Bc"), true);
   }

   private void verifyEqualsHashcode(Domain left, Domain right,
      boolean expectedEqual) {
      assert left != null;
      Assert.assertEquals(expectedEqual, left.equals(right));
      if (right != null) {
         Assert.assertEquals(expectedEqual, left.hashCode() == right.hashCode());
      }
   }
}
