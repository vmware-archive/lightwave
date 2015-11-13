/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.InvalidPasswordPolicyException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Provides means to query and modify the password policy applicable to the
 * STS's local users.
 */
public interface PasswordPolicyConfigurator {

   /**
    * Replace the local password policy with the given one.
    *
    * <p>
    * The password policy will be rejected if any of the following rules is
    * violated.
    * </p>
    * <ul>
    * <li>0 < Minimum password length <= Maximum password length
    * <li>All other password policy parameters must be in an accepted range
    * (e.g. no negative count restrictions)
    * <li>Maximum number of consecutive identical characters requirement must be
    * greater than zero
    * <li>Minimum alphabetic characters must be no less than the combined
    * uppercase and lowercase requirements
    * <li>Minimum password length must be no less than the combined minimum
    * alphabetic, numeric and special character requirements
    * </ul>
    * Note that, as a result, (min length: 1, min alphabetic: 2) is not
    * considered valid policy. (min length: 2, min alphabetic: 2) is valid.
    * InvalidPasswordPolicyException may be thrown in other cases as well,
    * depending on implementation.
    *
    * @param policy
    *           the updated password policy. Cannot be <code>null</code>
    *
    * @throws InvalidPasswordPolicyException
    *            indicates that server side password policy validation has
    *            failed
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void updateLocalPasswordPolicy(PasswordPolicy policy)
      throws InvalidPasswordPolicyException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Returns the policy currently in effect for the local SSO users.
    *
    * @return the policy currently in effect for the local SSO users.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.GuestUser)
   PasswordPolicy getLocalPasswordPolicy() throws NotAuthenticatedException,
      NoPermissionException;
}
