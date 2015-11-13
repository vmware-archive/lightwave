/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotNull;

import com.vmware.vim.sso.PrincipalId;

/**
 * Principal represents either person user, solution user or group object
 * residing at SSO repository. Each principal is uniquely identified by his ID
 * and domain name.
 * <p>
 * Principal's alias is similar to the ID except that it is based on the domain
 * alias instead of the domain name. When the corresponding domain has no alias
 * specified then the principal's alias should match to the principal's ID. It
 * means that principal's alias could also be used as an alternative identifier.
 * <p>
 * Two principals are equal when their IDs are equal, nevertheless that their
 * aliases might be different.
 */
public abstract class Principal {

   private final PrincipalId _id;
   private final PrincipalId _alias;

   /**
    * Constructs principal by primary and alternative IDs
    *
    * @param id
    *           principal id; {@code not-null} value is required
    * @param alias
    *           principal alias; {@code null} value when alias is not known or
    *           the corresponding domain has no alias specified
    */
   protected Principal(PrincipalId id, PrincipalId alias) {
      validateNotNull(id, "id");
      _id = id;
      _alias = alias;
   }

   /**
    * @return the id; {@code not-null} value
    */
   public final PrincipalId getId() {
      return _id;
   }

   /**
    * @return the alias; might be {@code null}
    */
   public final PrincipalId getAlias() {
      return _alias;
   }

   /**
    * @return the details; {@code not-null} value
    */
   public abstract PrincipalDetails getDetails();

   /**
    * Get canonical text representation of principal's identity.
    * <p>
    * The returned value is case sensitive. Two SSO principals match if and only
    * when {@link String#equals(Object)} is {@code true} for their canonical names .
    *
    * @return the canonical name; {@code not-null} and {@code not-empty} value
    */
   public abstract String getCanonicalName();

   /**
    * {@inheritDoc}
    */
   @Override
   public final boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      if (obj == null || getClass() != obj.getClass()) {
         return false;
      }

      Principal other = (Principal) obj;
      return _id.equals(other._id);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public final int hashCode() {
      return _id.hashCode();
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public final String toString() {

      return String.format("Principal: %s, %s, details {%s}", getClass()
         .getSimpleName(), _id, getDetails());

   }
}
