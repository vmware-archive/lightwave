package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

public interface SsoHealthManagement {

	@Privilege(Role.Administrator)
	SsoHealthStats getSsoStatistics() throws NotAuthenticatedException,
			NoPermissionException;
}
