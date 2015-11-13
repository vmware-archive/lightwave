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

import java.util.Arrays;
import java.util.List;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.saml.Advice.Attribute;

/**
 * Contain unit tests for Advice
 */
public final class AdviceTest {

   private static final String SOURCE1 = "sourceURI1";
   private static final String SOURCE2 = "sourceURI2";

   private static final Attribute ATTR1 = new Attribute("nameURI", null,
      Arrays.asList("attrvalue1"));
   private static final Attribute ATTR1_OTHER_VALUE = new Attribute(
      ATTR1.nameURI(), ATTR1.friendlyName(), Arrays.asList("attrvalue2",
         "attrvalue3"));
   private static final Attribute ATTR2 = new Attribute("nameURI2",
      ATTR1.friendlyName(), ATTR1.values());

   @Test
   public void testNewInstance() {
      final List<Attribute> attrs = Arrays.asList(ATTR1);
      final Advice advice = new Advice(SOURCE1, attrs);

      Assert.assertEquals(SOURCE1, advice.sourceURI());
      Assert.assertEquals(attrs, advice.attributes());
   }

   @Test
   public void testEqualsAndHashCode() {
      testEqualityTrue(new Advice(SOURCE1, Arrays.asList(ATTR1)), new Advice(
         SOURCE1, Arrays.asList(ATTR1)));

      testEqualityFalse(new Advice(SOURCE1, Arrays.asList(ATTR1)), new Advice(
         SOURCE2, Arrays.asList(ATTR1)));

      testEqualityFalse(new Advice(SOURCE1, Arrays.asList(ATTR1)), new Advice(
         SOURCE1, Arrays.asList(ATTR2)));
   }

   @Test
   public void testIntersect() {
      final Advice advice1 = new Advice(SOURCE1, Arrays.asList(ATTR1));
      Assert.assertTrue(advice1.intersect(advice1));
      Assert.assertTrue(advice1.intersect(new Advice(advice1.sourceURI(),
         Arrays.asList(ATTR1_OTHER_VALUE))));
      Assert.assertTrue(advice1.intersect(new Advice(advice1.sourceURI(),
         Arrays.asList(ATTR1_OTHER_VALUE, ATTR2))));

      Assert.assertFalse(advice1.intersect(new Advice(SOURCE2, advice1
         .attributes())));
      Assert.assertFalse(advice1.intersect(new Advice(advice1.sourceURI(),
         Arrays.asList(ATTR2))));
   }

   private void testEqualityFalse(Advice advice1, Advice advice2) {
      Assert.assertFalse(advice1.equals(advice2));
      Assert.assertFalse(advice1.hashCode() == advice2.hashCode());
   }

   private void testEqualityTrue(Advice advice1, Advice advice2) {
      Assert.assertEquals(advice1, advice2);
      Assert.assertTrue(advice1.hashCode() == advice2.hashCode());
   }
}
