/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Provides operations for query and update SSO user accounts lockout policy
 */
public interface LockoutPolicyConfigurator {

   /**
    * Returns the active lockout policy which is currently applied
    *
    * @return the active lockout policy
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   LockoutPolicy getLockoutPolicy() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Replaces the active lockout policy with the given one
    *
    * @param policy
    *           the new lockout policy
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void updateLockoutPolicy(LockoutPolicy policy)
      throws NotAuthenticatedException, NoPermissionException;
}
