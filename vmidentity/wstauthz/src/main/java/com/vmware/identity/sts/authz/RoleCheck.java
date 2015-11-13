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
package com.vmware.identity.sts.authz;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.WSTrustRole;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * Provide means to check users' roles
 */
public interface RoleCheck {

   /**
    * Check whether presence of user's role.
    *
    * @param principalId
    *           user identifier, required
    * @param role
    *           role to be checked, required
    * @return whether the user has that role
    * @throws InvalidPrincipalException
    *            when there is no such user
    */
   boolean hasRole(PrincipalId principalId, WSTrustRole role)
      throws InvalidPrincipalException;

}
