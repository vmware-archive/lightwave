/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotNull;

import com.vmware.vim.sso.PrincipalId;

/**
 * Solution user type representation.
 */
public final class SolutionUser extends Principal {

   private final SolutionDetails _details;
   private final boolean _disabled;
   private final boolean _external;

   /**
    * Constructs internal solution user type of principal with no alias if its
    * domain is not configured with such one.
    *
    * @param id
    *           user id; {@code not-null} value is required
    * @param details
    *           user details; {@code not-null} value is required
    * @param disabled
    *           whether the user is disabled
    */
   public SolutionUser(PrincipalId id, SolutionDetails details, boolean disabled) {

      this(id, null /* no alias */, details, disabled);
   }

   /**
    * Constructs internal solution user type of principal.
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
    */
   public SolutionUser(PrincipalId id, PrincipalId alias,
      SolutionDetails details, boolean disabled) {
      this(id, alias, details, disabled, false /* internal user */);
   }

   /**
    * Constructs solution user type of principal.
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
    * @param external
    *           whether the user is external or internal (the default)
    */
   public SolutionUser(PrincipalId id, PrincipalId alias,
      SolutionDetails details, boolean disabled, boolean external) {

      super(id, alias);

      validateNotNull(details, "Solution Details");
      _details = details;
      _disabled = disabled;
      _external = external;
   }

   @Override
   public SolutionDetails getDetails() {
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
    * Specify whether the user is external or internal.
    *
    * @return whether the user is external or internal
    */
   public boolean isExternal() {
      return _external;
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
