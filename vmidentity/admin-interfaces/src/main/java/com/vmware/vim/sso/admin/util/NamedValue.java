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

package com.vmware.vim.sso.admin.util;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Wrapper for a String value that exposes it through a name() method
 * which makes it compatible with the API of an enum
 */
public class NamedValue {

   private final String _name;

   public NamedValue(String name) {
      ValidateUtil.validateNotNull(name, "name");
      _name = name;
   }

   public String name() {
      return _name;
   }

   @Override
   public int hashCode() {
      return _name.hashCode();
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj)
         return true;
      if (obj == null)
         return false;
      if (getClass() != obj.getClass())
         return false;
      return _name.equals(((NamedValue)obj)._name);
   }

   @Override
   public String toString() {
      return _name;
   }

   /**
    * Maps a given name either to a known value or a "other" type instance
    *
    * @param name, required
    * @param enumType, required, must implement T
    * @param otherValue, returned if name is not in enumType, required
    * @return a T instance, not null
    */
   protected static <T, E extends Enum<E>>
   T valueOf(String name, Class<E> enumType, T otherValue) {
      ValidateUtil.validateNotEmpty(name, "name");
      try {
         return (T) Enum.valueOf(enumType, name);
      } catch (IllegalArgumentException e) {
         return otherValue;
      }
   }
}
