/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * **********************************************************************/

package com.vmware.vim.sso.admin;

import java.util.Collection;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

public interface ComputerManagement {

   @Privilege(Role.ConfigurationUser)
   Collection<VmHost> getComputers(boolean getDCOnly)
         throws NotAuthenticatedException, NoPermissionException;

}
