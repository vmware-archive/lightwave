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

import org.apache.commons.lang.Validate;

/**
 * Represents an immutable structure holding an attribute of a principal.
 */
public class PrincipalAttribute {

   private final PrincipalAttributeDefinition attributeDefinition;
   private final String[] values;

   /**
    *
    * @param name
    *           name of the attribute. Cannot be null.
    * @param nameFormat
    *           URI reference representing the classification of the attribute
    *           name for purposes of interpreting the name. Cannot be null.
    * @param friendlyName
    *           human readable version of the name
    * @param value
    *           value associated with this attribute.
    */
   public PrincipalAttribute(String name, String nameFormat,
      String friendlyName, String value) {

      this(name, nameFormat, friendlyName, value == null ? null
         : new String[] { value });
   }

   /**
    *
    * @param name
    *           name of the attribute. Cannot be null.
    * @param nameFormat
    *           URI reference representing the classification of the attribute
    *           name for purposes of interpreting the name. Cannot be null.
    * @param friendlyName
    *           human readable version of the name
    * @param values
    *           values associated with this attribute.
    */
   public PrincipalAttribute(String name, String nameFormat,
      String friendlyName, String[] values) {
      Validate.notNull(name);
      Validate.notNull(nameFormat);

      this.attributeDefinition = new PrincipalAttributeDefinition(name,
         nameFormat, friendlyName);
      this.values = values;
   }

   /**
    *
    * @param attributeDefinition
    *           attribute definition of current attribute. Cannot be null.
    * @param values
    *           values associated with this attribute.
    */
   public PrincipalAttribute(PrincipalAttributeDefinition attributeDefinition,
      String[] values) {
      assert attributeDefinition != null;

      this.attributeDefinition = attributeDefinition;
      this.values = values;
   }

   public String getName() {
      return attributeDefinition.getName();
   }

   public String getNameFormat() {
      return attributeDefinition.getNameFormat();
   }

   public String getFriendlyName() {
      return attributeDefinition.getFriendlyName();
   }

   public PrincipalAttributeDefinition getAttributeDefinition() {
      return attributeDefinition;
   }

   /**
    *
    * @return non-empty array of values or null, if none
    */
   public String[] getValues() {
      return values;
   }

   @Override
   public String toString() {
      final StringBuilder sb = new StringBuilder();
      sb.append("PrincipalAttribute [name=").append(
         attributeDefinition.getName());
      sb.append(", format=").append(attributeDefinition.getNameFormat());
      sb.append(", friendly name=").append(
         attributeDefinition.getFriendlyName());
      sb.append(", value=").append(Arrays.toString(values)).append(']');
      return sb.toString();
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + attributeDefinition.hashCode();
      result = prime * result
         + ((values == null) ? 0 : Arrays.hashCode(values));
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || this.getClass() != obj.getClass()) {
         return false;
      }

      PrincipalAttribute other = (PrincipalAttribute) obj;
      return attributeDefinition.equals(other.getAttributeDefinition())
         && ((values == null && other.values == null) || Arrays.equals(values,
            other.values));
   }
}
