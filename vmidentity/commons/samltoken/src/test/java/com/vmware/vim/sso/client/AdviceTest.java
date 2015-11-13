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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import com.vmware.vim.sso.client.Advice.AdviceAttribute;

public class AdviceTest {

   private static final String ADVICE_SOURCE = "urn:vc.vmware.com";
   private static final String ATTRIBUTE_NAME1 = "urn:vc:attr:1";
   private static final String ATTRIBUTE_FRIENDLY_NAME1 = "myattribute";
   private static final String ATTRIBUTE_NAME2 = "urn:vc:attr:2";
   private static final String ATTR2_VALUE1 = "advice 1";
   private static final String ATTR2_VALUE2 = "advice 2";

   @Test
   public void createAdviceOneAttributeNoValue() {
      AdviceAttribute attribute = new AdviceAttribute(ATTRIBUTE_NAME1);
      List<AdviceAttribute> attrList = new ArrayList<AdviceAttribute>();
      attrList.add(attribute);

      Advice advice = new Advice(ADVICE_SOURCE, attrList);
      assertEquals(ADVICE_SOURCE, advice.getSource());
      assertEquals(attrList, advice.getAttributes());
   }

   @Test
   public void createAdviceWithAttributes() {
      Advice advice = createAdvice();
      assertEquals(ADVICE_SOURCE, advice.getSource());
      assertEquals(2, advice.getAttributes().size());
   }

   @Test
   public void checkAdviceEquals() {
      Advice advice1 = createAdvice();
      Advice advice2 = createAdvice();
      assertEquals(advice1, advice2);
   }

   @Test
   public void checkAdviceAttributeEquality() {
      AdviceAttribute attr1 = new AdviceAttribute(ATTRIBUTE_NAME1, ATTRIBUTE_FRIENDLY_NAME1, new ArrayList<String>());
      AdviceAttribute attr11 = new AdviceAttribute(ATTRIBUTE_NAME1, ATTRIBUTE_FRIENDLY_NAME1, new ArrayList<String>());
      AdviceAttribute attr2 = new AdviceAttribute(ATTRIBUTE_NAME1, null, new ArrayList<String>());
      AdviceAttribute attr3 = new AdviceAttribute(ATTRIBUTE_NAME2, ATTRIBUTE_FRIENDLY_NAME1, new ArrayList<String>());
      List<String> attributeValues = new ArrayList<String>();
      attributeValues.add(ATTR2_VALUE1);
      List<String> attributeValues41 = new ArrayList<String>();
      attributeValues41.add(ATTR2_VALUE1);
      AdviceAttribute attr4 = new AdviceAttribute(ATTRIBUTE_NAME1, ATTRIBUTE_FRIENDLY_NAME1, attributeValues);
      AdviceAttribute attr41 = new AdviceAttribute(ATTRIBUTE_NAME1, ATTRIBUTE_FRIENDLY_NAME1, attributeValues41);

      assertTrue("attribute should equal self", attr1.equals(attr1));
      assertTrue("attribute should equal another instance with same values", attr1.equals(attr11));
      assertFalse("attribute should Not equal when friendly name differs", attr1.equals(attr2));
      assertFalse("attribute should Not equal another attribute", attr1.equals(attr3));
      assertFalse("attribute should Not equal another attribute", attr1.equals(attr4));
      assertFalse("attribute should Not equal attribute with different values", attr1.equals(attr4));
      assertFalse("attribute should Not equal attribute with different values", attr1.equals(attr41));
      assertTrue("attribute should equal another instance with same values", attr4.equals(attr41));
   }

   @Test
   public void createAdviceAttribute() {
      AdviceAttribute attribute1 = new AdviceAttribute(ATTRIBUTE_NAME1);
      assertEquals(ATTRIBUTE_NAME1, attribute1.getName());
      assertTrue(attribute1.getValue().isEmpty());
      assertNull("Friendly name should not be set.", attribute1.getFriendlyName());

      List<String> attributeValues = new ArrayList<String>();
      attributeValues.add(ATTR2_VALUE1);
      attributeValues.add(ATTR2_VALUE2);
      AdviceAttribute attribute2 = new AdviceAttribute(ATTRIBUTE_NAME2,
         attributeValues);
      assertEquals(2, attribute2.getValue().size());
   }

   @Test
   public void createAdviceAttributeWithFriendlyName() {
      AdviceAttribute attribute1 =
          new AdviceAttribute(ATTRIBUTE_NAME1, ATTRIBUTE_FRIENDLY_NAME1, new ArrayList<String>());
      assertEquals(ATTRIBUTE_NAME1, attribute1.getName());
      assertTrue(attribute1.getValue().isEmpty());
      assertEquals("Friendly name should match.", ATTRIBUTE_FRIENDLY_NAME1, attribute1.getFriendlyName());
   }

   @Test(expected = IllegalArgumentException.class)
   public void createAdviceNullSource() {
      AdviceAttribute attribute = new AdviceAttribute(ATTRIBUTE_NAME1);
      List<AdviceAttribute> attrList = new ArrayList<AdviceAttribute>();
      attrList.add(attribute);
      new Advice(null, attrList);
   }

   @Test(expected = IllegalArgumentException.class)
   public void createAdviceNoAttributes() {
      new Advice(ADVICE_SOURCE, new ArrayList<AdviceAttribute>());
   }

   @Test(expected = IllegalArgumentException.class)
   public void createAdviceNullAttributes() {
      new Advice(ADVICE_SOURCE, null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void createAttrbuteNullName() {
      new AdviceAttribute(null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void createAttributeNullValues() {
      new AdviceAttribute(ATTRIBUTE_NAME1, null);
   }

   private Advice createAdvice() {
      AdviceAttribute attribute1 = new AdviceAttribute(ATTRIBUTE_NAME1);
      List<String> attributeValues = new ArrayList<String>();
      attributeValues.add(ATTR2_VALUE1);
      attributeValues.add(ATTR2_VALUE2);
      AdviceAttribute attribute2 = new AdviceAttribute(ATTRIBUTE_NAME2,
         attributeValues);

      List<AdviceAttribute> attrList = new ArrayList<AdviceAttribute>();
      attrList.add(attribute1);
      attrList.add(attribute2);
      return new Advice(ADVICE_SOURCE, attrList);
   }
}
