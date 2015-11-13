/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents a realm of principals SSO server is working with.
 *
 * @see IdentitySource
 */
public final class Domain {

   private final String _name;
   private final String _alias;

   /**
    * Same as Domain(name, null)
    */
   public Domain(String name) {
      this(name, null);
   }

   /**
    * Populates a new domain
    *
    * @param name, required
    * @param alias, optional
    */
   public Domain(String name, String alias) {
      ValidateUtil.validateNotEmpty(name, "name");
      if (alias != null) {
         ValidateUtil.validateNotEmpty(alias, "alias");
      }
      _name = name;
      _alias = alias;
   }

   /**
    * Returns the name associated with the authentication domain, not empty
    * <p>
    * The name is unique in the sense that no other domain may have the same
    * name <i>or</i> alias and cannot be changed after the Domain is created.
    * The name is also case-insensitive so e.g. "DOMAIN.COM" and "domain.com"
    * refer to the <i>same</i> Domain.
    */
   public String getName() {
      return _name;
   }

   /**
    * Returns an optional alias associated with the authentication domain.
    * If no alias is associated with the domain, {@code null} is returned.
    * If it is present, the alias is not empty.
    * <p>
    * No other domain may have the same name or alias; the alias cannot be
    * changed and is case-insensitive.
    */
   public String getAlias() {
      return _alias;
   }



   /* (non-Javadoc)
    * Domain names and aliases are case-insensitive.
    *
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      final int prime = 37;
      int result = 1;
      result = prime * result +
         ((_alias == null) ? 0 : _alias.toLowerCase().hashCode());

      assert _name != null;
      result = prime * result + _name.toLowerCase().hashCode();
      return result;
   }

   /* (non-Javadoc)
    * Domain names and aliases are case-insensitive.
    *
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(Object obj) {
      if (this == obj)
         return true;
      if (obj == null)
         return false;
      if (getClass() != obj.getClass())
         return false;
      Domain other = (Domain) obj;
      if (_alias == null) {
         if (other._alias != null)
            return false;
      } else if (!_alias.equalsIgnoreCase(other._alias))
         return false;

      assert _name != null;
      return _name.equalsIgnoreCase(other._name);
   }

   @Override
   public String toString() {
      return "Domain [name=" + _name + ", alias=" + _alias + "]";
   }
}
