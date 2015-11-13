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
