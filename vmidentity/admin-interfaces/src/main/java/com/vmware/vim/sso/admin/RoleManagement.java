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

import java.lang.annotation.Documented;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Manages the roles of SSO users. Roles apply when managed method is being
 * invoked and method privileges need to be checked.<br>
 * Every users is assigned exactly one single role. The default role for newly
 * created users is {@link Role#GuestUser}.
 */
public interface RoleManagement {

   /**
    * A comparable list of user roles. Roles are incremental which means each
    * higher role is a superset of its weaker ones.
    * <p>
    * It is <b>very important</b> that role constants are defined in ascending
    * order so that the lowest role is at the top and thus has the smallest
    * numeric value.
    * <p>
    * For anonymous user role, use {@code NoPrivilege}.
    */
   enum Role {

      /**
       * Guest access. The most restrictive access which in addition to
       * unauthorized access also allows limited self-management capabilities
       * such as updating own password and details. All users do have at least
       * this role.
       */
      GuestUser,

      /**
       * Read access. In addition to guest role it also allows browsing SSO
       * identity store.
       */
      RegularUser,

      /**
       * Access to certain configuration aspects. Allows querying of management
       * information.
       */
      ConfigurationUser,

      /**
       * Administrative access is the highest possible role. It provides access
       * to all operations which include regular access as well as server
       * configuration, user management etc.
       */
      Administrator;

       /**
       * Check whether this role is higher or equal to another role
       *
       * @param other
       *           role which to compare this one with
       * @return true, when this role is higher or equal to the other one,
       *         otherwise - false
       */
      public boolean isHigherOrEqualTo(Role other) {
         return compareTo(other) >= 0;
      }
   }

   /**
    * This annotation indicates the minimal required role that should be
    * possessed by the caller for a successful method invocation
    */
   @Documented
   public @interface Privilege {
      Role value();
   }

   /**
    * This annotation indicates the method can be invoked by an anonymous,
    * unauthorized user
    */
   @Documented
   public @interface NoPrivilege {
   }


   /**
    * Assigns a role to the given person or solution user. Setting a role lower
    * than the actual one, actually means decreasing the role of the user. Refer
    * to {@link Role} for the order of the roles.
    *
    * @param userId
    *           user who will be assigned a role. cannot be {@code null}
    * @param role
    *           role to be assigned; cannot be {@code null}
    *
    * @return true, when the user role has actually changed, otherwise - false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean setRole(PrincipalId userId, Role role)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Check whether the given user has {@link Role#Administrator} role
    *
    * @param userId
    *           user which role to check. cannot be {@code null}
    * @return true, when the user has the role, otherwise - false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean hasAdministratorRole(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Check whether the given user has {@link Role#RegularUser} or higher role
    *
    * @param userId
    *           user which role to check. cannot be {@code null}
    * @return true, when the user has the role, otherwise - false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   boolean hasRegularUserRole(PrincipalId userId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * A set of STS WSTrust component roles.
    */
   enum WSTrustRole {

      /**
       * Act-as user. This role allows users to act on behalf of other users. In
       * practice, this role allows an eligible user to request token delegated
       * to himself presenting a valid client token. Initially, nobody has this
       * role.
       */
      ActAsUser,
   }

   /**
    * Grants a WSTrust-specific role to the given user.
    *
    * @param userId
    *           user who will be granted a role
    * @param role
    *           role to be granted; refer to {@link WSTrustRole} for valid
    *           options
    *
    * @return true, when the user role has actually been granted, otherwise -
    *         false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean grantWSTrustRole(PrincipalId userId, WSTrustRole role)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Revokes a WSTrust-specific role from the given user.
    *
    * @param userId
    *           user who will be revoked a role
    * @param role
    *           role to be revoked; refer to {@link WSTrustRole} for valid
    *           options
    *
    * @return true, when the user role has actually been revoked, otherwise -
    *         false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean revokeWSTrustRole(PrincipalId userId, WSTrustRole role)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * IDP provisioning API roles.
    */
   enum IDPProvisioningRole {

      /**
       * Administrative access.
       */
      IDPAdministrator;
   }

   /**
    * Grants an IDP provisioning API-specific role to the given user.
    *
    * @param userId
    *           user who will be granted a role; {@code not-null} value is
    *           required
    * @param role
    *           role to be granted; {@code not-null} value is required
    *
    * @return true, when the user role has actually been revoked, otherwise -
    *         false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean grantIDPProvisioningRole(PrincipalId userId, IDPProvisioningRole role)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Revokes an IDP provisioning API-specific role from the given user.
    *
    * @param userId
    *           user who will be revoked a role; {@code not-null} value is
    *           required
    * @param role
    *           role to be revoked; {@code not-null} value is required
    *
    * @return true, when the user role has actually been revoked, otherwise -
    *         false
    *
    * @throws InvalidPrincipalException
    *            when no person or solution user with the specified id exists
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean revokeIDPProvisioningRole(PrincipalId userId,
      IDPProvisioningRole role) throws InvalidPrincipalException,
      NotAuthenticatedException, NoPermissionException;
}
