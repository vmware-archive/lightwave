/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.exception.PasswordPolicyViolationException;

/**
 * Principal self management methods. Self-management methods apply to the
 * person or solution user which has authenticated when instance object was
 * created.
 */
public interface PrincipalSelfManagement {

   /**
    * Updates the details of the currently logged in person user. This method
    * updates all the values specified at details argument.
    *
    * @param details
    *           the new details to update
    *
    * @return The identifier (PricipalId) of the affected local person user.
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user is not a local person user
    *            method call
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   PrincipalId updateLocalPersonUserDetails(PersonDetails details)
      throws InvalidPrincipalException, NotAuthenticatedException;

   /**
    * Updates the details of currently logged-in solution user. This method
    * updates all the values specified at the solution user details.
    *
    * @param details
    *           the new details to update; valid format string for email address
    *           should be specified
    * @return The identifier (PricipalId) of the affected local solution user.
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user is not a solution user
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   PrincipalId updateSolutionUserDetails(SolutionDetails details)
      throws InvalidPrincipalException, NotAuthenticatedException;

   /**
    * Delete the currently logged in solution user.
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user in not a solution user
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   void deleteSolutionUser() throws InvalidPrincipalException,
      NotAuthenticatedException;

   /**
    * Resets the password of the currently logged in person user. The previous
    * password will be overridden with the given one.
    *
    * @param newPassword
    *           the new password to apply
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user is not a local person user
    * @throws PasswordPolicyViolationException
    *            when the provided password doesn't correspond to the default
    *            {@link PasswordPolicy}
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   void resetLocalPersonUserPassword(char[] newPassword)
      throws InvalidPrincipalException, PasswordPolicyViolationException,
      NotAuthenticatedException;

   /**
    * Retrieve the number of days left until password expiration for the
    * currently logged-in person user.
    * <p>
    * The returned value is:
    * <ul>
    * <li>a positive integer value - for number of days (rounded up) after which
    * password will expire</li>
    * <li>zero value - when password has already expired</li>
    * <li>-1 (minus one) value - when password policy is disabled or set to
    * never expire</li>
    * </ul>
    *
    * @return number of days until password expiration; returned value is
    *         greater or equal to -1
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user in not a person user
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   int getDaysRemainingUntilPasswordExpiration()
      throws InvalidPrincipalException, NotAuthenticatedException;

   /**
    * Retrieve currently logged in person user.
    *
    * @return currently logged in person user
    *
    * @throws InvalidPrincipalException
    *            when the currently logged-in user in not a person user
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    */
   @Privilege(Role.GuestUser)
   PersonUser getPersonUser() throws InvalidPrincipalException,
      NotAuthenticatedException;
}
