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

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotNull;

import com.vmware.vim.sso.PrincipalId;

/**
 * Person user type representation.
 */
public final class PersonUser extends Principal {

   private final PersonDetails _details;
   private final boolean _disabled;
   private final boolean _locked;

   /**
    * Constructs person user type of principal with no alias if its domain is
    * not configured with such one.
    *
    * @param id
    *           user id; {@code not-null} value is required
    * @param details
    *           user details; {@code not-null} value is required
    * @param disabled
    *           whether the user is disabled
    * @param locked
    *           whether the user is locked
    */
   public PersonUser(PrincipalId id, PersonDetails details, boolean disabled,
      boolean locked) {

      this(id, null /* no alias */, details, disabled, locked);
   }

   /**
    * Constructs person user type of principal.
    *
    * @param id
    *           principal id; {@code not-null} value is required
    * @param alias
    *           principal alias; {@code null} value when alias is not known or
    *           the corresponding domain has no alias specified; note that the
    *           alias should not be equal to the ID
    * @param details
    *           user details; {@code not-null} value is required
    * @param disabled
    *           whether the user is disabled
    * @param locked
    *           whether the user is locked
    */
   public PersonUser(PrincipalId id, PrincipalId alias, PersonDetails details,
      boolean disabled, boolean locked) {

      super(id, alias);

      validateNotNull(details, "Person Details");
      _disabled = disabled;
      _locked = locked;
      _details = details;
   }

   @Override
   public PersonDetails getDetails() {
      return _details;
   }

   /**
    * Specified whether the user is disabled.
    * <p>
    * Disabled users cannot acquire tokens from STS and their existing tokens
    * cannot be validated.
    *
    * @return whether the user is disabled
    */
   public boolean isDisabled() {
      return _disabled;
   }

   /**
    * Specified whether the user is locked.
    * <p>
    * Users become locked on a number of unsuccessful authentication attempts as
    * specified at {@link LockoutPolicy}. Locked users cannot acquire token from
    * STS ( except in some cases by SSPI ) but their existing tokens can still
    * be validated.
    *
    * @return whether the user is locked
    */
   public boolean isLocked() {
      return _locked;
   }

   /**
    * Canonical form for users is UPN format, e.g.<br>
    * <i>&lt;account_name&gt;&#64;&lt;FQDN&gt;</i><br>
    * Domain part will always be upper case. The character case of the account
    * name depends on the particular domain implementation.
    */
   @Override
   public final String getCanonicalName() {
      final PrincipalId id = getId();
      return String.format("%s@%s", id.getName(), id.getDomain().toUpperCase());
   }

}
