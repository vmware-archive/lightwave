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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;

import org.junit.Before;
import org.junit.Test;

public class PrincipalAttributeTest {

   private final static String NAME = "Name";
   private final static String NAME_FORMAT = "NameFormat";
   private final static String FRIENDLY_NAME = "FriendlyName";
   private final static String VALUE = "value";

   private PrincipalAttribute attribute;

   @Before
   public void init() {
      attribute = new PrincipalAttribute(NAME, NAME_FORMAT, FRIENDLY_NAME,
         VALUE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreate_NoName() {
      new PrincipalAttribute(null, "NameFormat", null, (String) null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreate_NoNameFormat() {
      new PrincipalAttribute("Name", null, null, (String) null);
   }

   @Test
   public void testCreateAndGet() {
      assertEquals(NAME, attribute.getName());
      assertEquals(NAME_FORMAT, attribute.getNameFormat());
      assertEquals(FRIENDLY_NAME, attribute.getFriendlyName());
      assertTrue(Arrays.equals(new String[] { VALUE }, attribute.getValues()));
   }

   @Test
   public void testCreate2AndGet() {
      PrincipalAttributeDefinition attributeDefinition = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, FRIENDLY_NAME);
      attribute = new PrincipalAttribute(attributeDefinition,
         new String[] { VALUE });
      assertEquals(NAME, attribute.getName());
      assertEquals(NAME_FORMAT, attribute.getNameFormat());
      assertEquals(FRIENDLY_NAME, attribute.getFriendlyName());
      assertTrue(Arrays.equals(new String[] { VALUE }, attribute.getValues()));
   }

   @Test
   public void testEqualsAndHashCode_WithFriendlyName() {
      PrincipalAttribute attr = new PrincipalAttribute(NAME, NAME_FORMAT,
         FRIENDLY_NAME, VALUE);
      assertEquals(attr, attribute);
      assertEquals(attr.hashCode(), attribute.hashCode());
   }

   @Test
   public void testEqualsAndHashCode_WithoutFriendlyName() {
      PrincipalAttribute attr1 = new PrincipalAttribute(NAME, NAME_FORMAT,
         null, VALUE);
      PrincipalAttribute attr2 = new PrincipalAttribute(NAME, NAME_FORMAT,
         null, VALUE);
      assertEquals(attr1, attr2);
      assertEquals(attr1.hashCode(), attr2.hashCode());
   }

   @Test
   public void testEqualsAndHashCode_WithoutFriendlyNameAndValue() {
      PrincipalAttribute attr1 = new PrincipalAttribute(NAME, NAME_FORMAT,
         null, (String[]) null);
      PrincipalAttribute attr2 = new PrincipalAttribute(NAME, NAME_FORMAT,
         null, (String) null);
      assertEquals(attr1, attr2);
      assertEquals(attr1.hashCode(), attr2.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_FriendlyNameNull() {
      PrincipalAttribute attr = new PrincipalAttribute(NAME, NAME_FORMAT, null,
         VALUE);
      assertFalse(attr.equals(attribute));
      assertFalse(attr.hashCode() == attribute.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_FriendlyNameDifferent() {
      PrincipalAttribute attr = new PrincipalAttribute(NAME, NAME_FORMAT,
         "Different", VALUE);
      assertFalse(attr.equals(attribute));
      assertFalse(attr.hashCode() == attribute.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_NameFormatDifferent() {
      PrincipalAttribute attr = new PrincipalAttribute(NAME,
         "different_format", FRIENDLY_NAME, VALUE);
      assertFalse(attr.equals(attribute));
      assertFalse(attr.hashCode() == attribute.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_NameDifferent() {
      PrincipalAttribute attr = new PrincipalAttribute("different_format",
         NAME_FORMAT, FRIENDLY_NAME, VALUE);
      assertFalse(attr.equals(attribute));
      assertFalse(attr.hashCode() == attribute.hashCode());
   }

   @Test
   public void testToString() {
      assertEquals(
         "PrincipalAttribute [name=Name, format=NameFormat, friendly name=FriendlyName, value=[value]]",
         attribute.toString());
   }

}
