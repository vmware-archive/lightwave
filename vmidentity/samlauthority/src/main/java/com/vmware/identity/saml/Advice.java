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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.Validate;

/**
 * Represents an advice to be added in assertion
 */
public final class Advice {

   private final String sourceURI;
   private final List<Attribute> attributes;

   public Advice(String sourceURI, List<Attribute> attributes) {
      Validate.notNull(sourceURI);
      Validate.notEmpty(attributes);

      this.sourceURI = sourceURI;
      this.attributes = shallowCopy(attributes);
   }

   /**
    * @return the sourceURI, not null
    */
   public String sourceURI() {
      return sourceURI;
   }

   /**
    * @return the attributes, not null, contains at least one element
    */
   public List<Attribute> attributes() {
      return shallowCopy(attributes);
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      return sourceURI.hashCode() + prime * attributes.hashCode();
   }

   @Override
   public boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      boolean result = false;
      if (obj instanceof Advice) {
         Advice other = (Advice) obj;
         result = sourceURI.equals(other.sourceURI)
            && attributes.equals(other.attributes);
      }
      return result;
   }

   @Override
   public String toString() {
      return String.format("Advice [sourceURI=%s, attributes=%s]", sourceURI,
         attributes);
   }

   public boolean intersect(Advice other) {
      return sourceURI.equals(other.sourceURI)
         && hasAttributeIntersection(other.attributes);
   }

   private boolean hasAttributeIntersection(List<Attribute> attributes) {

      for (Attribute attr1 : attributes) {
         assert attr1 != null;
         for (Attribute attr2 : this.attributes) {
            assert attr2 != null;
            if (attr2.hasSameName(attr1)) {
               return true;
            }
         }
      }
      return false;
   }

   public final static class Attribute {
      private final String nameURI;
      private final String friendlyName;
      private final List<String> values;

      public Attribute(String nameURI, String friendlyName, List<String> values) {
         Validate.notNull(nameURI);
         Validate.notNull(values);

         this.nameURI = nameURI;
         this.friendlyName = friendlyName;
         this.values = shallowCopy(values);

      }

      /**
       * @return the nameURI, not null
       */
      public String nameURI() {
         return nameURI;
      }

      /**
       * @return the friendlyName
       */
      public String friendlyName() {
         return friendlyName;
      }

      /**
       * @return the values, not null but possibly empty
       */
      public List<String> values() {
         return shallowCopy(values);
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = nameURI.hashCode();
         result = prime * result + ObjectUtils.hashCode(friendlyName);
         result = prime * result + values.hashCode();

         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (obj == this) {
            return true;
         }

         boolean result = false;
         if (obj instanceof Attribute) {
            Attribute other = (Attribute) obj;
            result = nameURI.equals(other.nameURI)
               && ObjectUtils.equals(friendlyName, other.friendlyName)
               && values.equals(other.values);
         }
         return result;
      }

      @Override
      public String toString() {
         return String.format("Attribute [nameURI=%s, friendlyName=%s, values=%s]", nameURI,
            friendlyName, values);
      }

      public boolean hasSameName(Attribute other) {
         return nameURI.equals(other.nameURI());
      }
   }

   private static <T> List<T> shallowCopy(List<T> values) {
      return new ArrayList<T>(values);
   }

}
