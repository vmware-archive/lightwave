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
