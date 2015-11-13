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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.vmware.identity.token.impl.ValidateUtil;

/**
 * Immutable class representing an advice used to add arbitrary custom
 * information to a SAML token.
 */
public final class Advice {

   private final String _source;
   private final List<AdviceAttribute> _attributes;

   /**
    * Creates an advice
    *
    * @param source
    *           URI representing the source of the advice. Cannot be
    *           <code>null</code>
    * @param attributes
    *           Attributes containing the advice. Should contain at least one
    *           entry.
    */
   public Advice(String source, List<AdviceAttribute> attributes) {
      ValidateUtil.validateNotNull(source, "Advice source");
      ValidateUtil.validateNotEmpty(attributes, "Advice attributes");

      _source = source;
      _attributes = Collections.unmodifiableList(attributes);
   }

   /**
    * @return URI representing the source of the advice. Cannot be
    *         <code>null</code>
    */
   public String getSource() {
      return _source;
   }

   /**
    * @return Attributes containing the advice. Cannot be <code>null</code>
    */
   public List<AdviceAttribute> getAttributes() {
      return _attributes;
   }

   @Override
   public int hashCode() {
      return _source.hashCode() + _attributes.hashCode();
   }

   @Override
   public boolean equals(Object o) {
      if (o == this) {
         return true;
      }
      if (o instanceof Advice) {
         Advice advice = (Advice) o;
         return _source.equals(advice._source)
            && _attributes.equals(advice._attributes);
      }
      return false;
   }

   /**
    * Immutable class representing the content of an advice.
    */
   public static final class AdviceAttribute {

      private final String _name;
      private final String _friendlyName;
      private final List<String> _value;

      /**
       * Creates an advice attribute
       *
       * @param name
       *           The name of the attribute. Must be URI. Cannot be
       *           <code>null</code>
       */
      public AdviceAttribute(String name) {
         this(name, new ArrayList<String>());
      }

      /**
       * Creates an advice attribute
       *
       * @param name
       *           The name of the attribute. Must be URI. Cannot be
       *           <code>null</code>
       * @param value
       *           List of values representing the content of the advice
       *           attribute. Cannot be <code>null</code>
       */
      public AdviceAttribute(String name, List<String> value) {
         this(name, null, value);
      }

      /**
       * Creates an advice attribute
       *
       * @param name
       *           The name of the attribute. Must be URI. Cannot be
       *           <code>null</code>
       * @param friendlyName
       *           a friendly name of the attribute.
       * @param value
       *           List of values representing the content of the advice
       *           attribute. Cannot be <code>null</code>
       */
      public AdviceAttribute(String name, String friendlyName, List<String> value) {
         ValidateUtil.validateNotNull(name, "Advice attribute name");
         ValidateUtil.validateNotNull(value, "Attribute values");

         _name = name;
         _friendlyName=  friendlyName;
         _value = Collections.unmodifiableList(value);
      }

      /**
       * @return The name of the attribute. Must be URI. Cannot be
       *         <code>null</code>
       */
      public String getName() {
         return _name;
      }

      /**
       * @return the friendlyName
       */
      public String getFriendlyName() {
         return _friendlyName;
      }

      /**
       * @return List of values representing the content of the advice
       *         attribute. Cannot be <code>null</code>
       */
      public List<String> getValue() {
         return _value;
      }

      @Override
      public int hashCode() {
         return _name.hashCode() + _value.hashCode()
                + ((_friendlyName != null) ? _friendlyName.hashCode() : 0);
      }

      @Override
      public boolean equals(Object o) {
         if (o == this) {
            return true;
         }
         if (o instanceof AdviceAttribute) {
            AdviceAttribute adviceAttribute = (AdviceAttribute) o;
            return _name.equals(adviceAttribute._name)
               && _value.equals(adviceAttribute._value)
               &&
               (   // either both null or equal
                   ( ( _friendlyName == null ) && (adviceAttribute._friendlyName == null) )
                   ||
                   ( ( _friendlyName != null ) && (_friendlyName.equals(adviceAttribute._friendlyName) ) )
               );
         }
         return false;
      }
   }
}
