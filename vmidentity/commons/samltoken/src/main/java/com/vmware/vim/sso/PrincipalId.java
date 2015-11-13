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
package com.vmware.vim.sso;

import java.util.Arrays;

import com.vmware.identity.token.impl.ValidateUtil;

/**
 * Uniquely identifies particular principal
 */
public final class PrincipalId {

   /** Principal name */
   private final String _name;

   /** Principal domain */
   private final String _domain;

   /**
    * Construct a principal identifier by domain name where he/she is located
    * and short name which should be unique in scope of the given domain
    *
    * @param name
    *           principal short name (e.g. jdoe); requires {@code not-null} and
    *           not empty string value
    * @param domain
    *           domain name or alias (e.g. vmware.com); requires {@code
    *           not-null} and not empty string value;
    */
   public PrincipalId(String name, String domain) {

      ValidateUtil.validateNotEmpty(name, "name");
      ValidateUtil.validateNotEmpty(domain, "domain");

      _name = name;
      _domain = domain;
   }

   /**
    * @return the name; {@code not-null} and not empty string value
    */
   public String getName() {
      return _name;
   }

   /**
    * @return the domain; {@code not-null} and not empty string value
    */
   public String getDomain() {
      return _domain;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      if (obj == null) {
         return false;
      }

      if (!obj.getClass().equals(PrincipalId.class)) {
         return false;
      }

      PrincipalId other = (PrincipalId) obj;
      return _name.equals(other._name)
         && _domain.equalsIgnoreCase(other._domain);

   }

   /**
    * {@inheritDoc}
    */
   @Override
   public int hashCode() {
      return Arrays.hashCode(new Object[] { _name, _domain.toLowerCase() });
   }

   @Override
   public String toString() {
      return String.format("{Name: %s, Domain: %s}", getName(), getDomain());
   }
}
