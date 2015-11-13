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

import org.junit.Before;
import org.junit.Test;

public class PrincipalAttributeDefinitionTest {

   private final static String NAME = "Name";
   private final static String NAME_FORMAT = "NameFormat";
   private final static String FRIENDLY_NAME = "FriendlyName";

   private PrincipalAttributeDefinition attributeDefinition;

   @Before
   public void init() {
      attributeDefinition = new PrincipalAttributeDefinition(NAME, NAME_FORMAT,
         FRIENDLY_NAME);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreate_NoName() {
      new PrincipalAttributeDefinition(null, "NameFormat", null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreate_NoNameFormat() {
      new PrincipalAttributeDefinition("Name", null, null);
   }

   @Test
   public void testCreateAndGet() {
      assertEquals(NAME, attributeDefinition.getName());
      assertEquals(NAME_FORMAT, attributeDefinition.getNameFormat());
      assertEquals(FRIENDLY_NAME, attributeDefinition.getFriendlyName());
   }

   @Test
   public void testEqualsAndHashCode_WithFriendlyName() {
      PrincipalAttributeDefinition attrDef = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, FRIENDLY_NAME);
      assertEquals(attrDef, attributeDefinition);
      assertEquals(attrDef.hashCode(), attributeDefinition.hashCode());
   }

   @Test
   public void testEqualsAndHashCode_WithoutFriendlyName() {
      PrincipalAttributeDefinition attrDef1 = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, null);
      PrincipalAttributeDefinition attrDef2 = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, null);
      assertEquals(attrDef1, attrDef2);
      assertEquals(attrDef1.hashCode(), attrDef2.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_FriendlyNameNull() {
      PrincipalAttributeDefinition attrDef = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, null);
      assertFalse(attrDef.equals(attributeDefinition));
      assertFalse(attrDef.hashCode() == attributeDefinition.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_FriendlyNameDifferent() {
      PrincipalAttributeDefinition attrDef = new PrincipalAttributeDefinition(
         NAME, NAME_FORMAT, "Different");
      assertFalse(attrDef.equals(attributeDefinition));
      assertFalse(attrDef.hashCode() == attributeDefinition.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_NameFormatDifferent() {
      PrincipalAttributeDefinition attrDef = new PrincipalAttributeDefinition(
         NAME, "different_format", FRIENDLY_NAME);
      assertFalse(attrDef.equals(attributeDefinition));
      assertFalse(attrDef.hashCode() == attributeDefinition.hashCode());
   }

   @Test
   public void testNotEqualsAndHashCode_NameDifferent() {
      PrincipalAttributeDefinition attrDef = new PrincipalAttributeDefinition(
         "different_format", NAME_FORMAT, FRIENDLY_NAME);
      assertFalse(attrDef.equals(attributeDefinition));
      assertFalse(attrDef.hashCode() == attributeDefinition.hashCode());
   }

   @Test
   public void testToString() {
      assertEquals(
         "PrincipalAttributeDefinition [name=Name, format=NameFormat, friendly name=FriendlyName]",
         attributeDefinition.toString());
   }

}
