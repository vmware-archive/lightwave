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

package com.vmware.vim.sso.admin;

import java.util.Collections;
import java.util.Set;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents an identity source which connects at least one domain to SSO.
 */
public abstract class IdentitySource {

   protected final String _name;
   protected final Set<Domain> _domains;

   /**
    * Populate an IdentitySource instance
    *
    * @param name, required, not empty
    * @param domains, required, not empty
    */
   public IdentitySource(String name, Set<Domain> domains) {
      ValidateUtil.validateNotEmpty(name, "name");
      ValidateUtil.validateNotNull(domains, "domains");
      ValidateUtil.validatePositiveNumber(domains.size(), "domains.size");

      _name = name;
      _domains = Collections.unmodifiableSet(domains);
   }

   /**
    * @return the unique name of the IdS, not empty.
    * The name is used as a handle in management operations
    */
   public String getName() {
      return _name;
   }

   /**
    * @return the authentication domains, provided by this IdS, not empty
    */
   public Set<Domain> getDomains() {
      return _domains;
   }

   @Override
   public String toString() {
      return getClass().getSimpleName() + " [name=" + _name
         + ", domains=" + _domains + "]";
   }

   /*
    * (non-Javadoc) Two IdS instances are considered equal if their
    * corresponding names are case-insensitively equal.
    *
    * <p>The identity sources are compared as entity objects
    * i.e. as possibly incomplete views of entities in the database.
    *
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public final boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      if (getClass() != obj.getClass()) {
         return false;
      }

      return _name.equalsIgnoreCase(((IdentitySource) obj)._name);
   }

   /*
    * (non-Javadoc) The IdS instances are hashed by their lower-cased name.
    *
    * @see java.lang.Object#hashCode()
    */
   @Override
   public final int hashCode() {
      return _name.toLowerCase().hashCode();
   }
}